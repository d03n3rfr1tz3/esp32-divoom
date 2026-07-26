#include <unity.h>

#include <ctype.h>
#include <string>
#include <vector>

#include "support.h"
#include "util.h"

void setUp(void) {}
void tearDown(void) {}

typedef std::vector<uint8_t> bytes_t;

static frame_stream_t stream;
static std::vector<bytes_t> handled;

static void collect(const uint8_t *buffer, size_t size) {
    handled.push_back(bytes_t(buffer, buffer + size));
}

static void reset(void) {
    handled.clear();
    stream.size = 0;
}

/** builds a wire frame the way divoom.py::make_message does, checksum included */
static bytes_t make_frame(const bytes_t& payload, size_t checksumSize) {
    size_t length = payload.size() + 2;
    bytes_t result;

    result.push_back(0x01);
    result.push_back(length & 0xff);
    result.push_back((length >> 8) & 0xff);
    result.insert(result.end(), payload.begin(), payload.end());

    uint32_t sum = (length & 0xff) + ((length >> 8) & 0xff);
    for (size_t i = 0; i < payload.size(); i++) sum += payload[i];
    for (size_t i = 0; i < checksumSize; i++) result.push_back((sum >> (i * 8)) & 0xff);

    result.push_back(0x02);
    return result;
}

/** masks 0x01 to 0x03 between the markers, as the Aurabox and TimeboxMini devices expect */
static bytes_t escape_frame(const bytes_t& frame) {
    bytes_t result;

    result.push_back(0x01);
    for (size_t i = 1; i + 1 < frame.size(); i++) {
        uint8_t value = frame[i];
        if (value < 0x01 || value > 0x03) { result.push_back(value); continue; }

        result.push_back(0x03);
        result.push_back(value + 0x03);
    }
    result.push_back(0x02);

    return result;
}

static bytes_t parse_hex(const std::string& line) {
    bytes_t result;
    size_t index = 0;

    while (index < line.size()) {
        if (isspace((unsigned char)line[index])) { index++; continue; }
        result.push_back((uint8_t)strtoul(line.substr(index, 2).c_str(), NULL, 16));
        while (index < line.size() && !isspace((unsigned char)line[index])) index++;
    }

    return result;
}

/** one recorded hass-divoom frame per line, so the file is the expectation itself */
static std::vector<bytes_t> read_frames(const char* name) {
    std::string content;
    TEST_ASSERT_TRUE_MESSAGE(read_stream(name, content), name);

    std::vector<bytes_t> result;
    for (size_t start = 0; start < content.size();) {
        size_t end = content.find('\n', start);
        if (end == std::string::npos) end = content.size();

        bytes_t frame = parse_hex(content.substr(start, end - start));
        if (!frame.empty()) result.push_back(frame);
        start = end + 1;
    }

    TEST_ASSERT_TRUE_MESSAGE(!result.empty(), name);
    return result;
}

static void assert_frames(const std::vector<bytes_t>& expected, const std::string& context) {
    TEST_ASSERT_EQUAL_size_t_MESSAGE(expected.size(), handled.size(), context.c_str());

    for (size_t i = 0; i < expected.size(); i++) {
        std::string message = context + " frame " + std::to_string(i);
        TEST_ASSERT_EQUAL_size_t_MESSAGE(expected[i].size(), handled[i].size(), message.c_str());
        TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected[i].data(), handled[i].data(), expected[i].size(), message.c_str());
    }
}

/**
 * feeds a recorded stream through the reassembler in fixed sized chunks and expects
 * exactly the recorded frames back, in order and byte for byte. chunk 0 is one call
*/
static void assert_stream(const char* name, size_t chunk) {
    std::vector<bytes_t> expected = read_frames(name);

    bytes_t input;
    for (size_t i = 0; i < expected.size(); i++) input.insert(input.end(), expected[i].begin(), expected[i].end());
    if (chunk == 0 || chunk > input.size()) chunk = input.size();

    reset();
    for (size_t offset = 0; offset < input.size(); offset += chunk) {
        size_t size = input.size() - offset;
        appendFrames(&stream, input.data() + offset, size < chunk ? size : chunk, collect);
    }

    assert_frames(expected, std::string(name) + " at chunk " + std::to_string(chunk));
    TEST_ASSERT_EQUAL_size_t_MESSAGE(0, stream.size, name);
}

/** whole · one TCP segment · deliberately offset against the 210 byte frames · prime · single bytes */
static void assert_chunkings(const char* name) {
    static const size_t chunkings[] = { 0, CONFIG_LWIP_TCP_MSS, 213, 97, 1 };
    for (size_t i = 0; i < sizeof(chunkings) / sizeof(chunkings[0]); i++) assert_stream(name, chunkings[i]);
}

static void test_frame_size_of_an_exact_frame(void) {
    bytes_t frame = make_frame({ 0x45, 0x01, 0xff, 0x00, 0x80 }, 2);
    TEST_ASSERT_EQUAL_size_t(11, frame.size());
    TEST_ASSERT_EQUAL_size_t(frame.size(), getFrameSize(frame.data(), frame.size()));
}

static void test_frame_size_ignores_trailing_data(void) {
    bytes_t frame = make_frame({ 0x45, 0x01 }, 2);
    bytes_t input = frame;
    input.insert(input.end(), { 0x69, 0xaa, 0xbb });

    TEST_ASSERT_EQUAL_size_t(frame.size(), getFrameSize(input.data(), input.size()));
}

static void test_frame_size_is_zero_while_incomplete(void) {
    bytes_t frame = make_frame({ 0x45, 0x01, 0xff }, 2);
    for (size_t size = 0; size < frame.size(); size++) {
        TEST_ASSERT_EQUAL_size_t(0, getFrameSize(frame.data(), size));
        TEST_ASSERT_TRUE(isFramePending(frame.data(), size) || size == 0);
    }
}

/** 0x02 is a perfectly normal payload byte as long as the payload is not masked */
static void test_frame_size_allows_end_marker_in_payload(void) {
    bytes_t frame = make_frame({ 0x45, 0x02, 0x02, 0x02, 0x77 }, 2);
    TEST_ASSERT_EQUAL_size_t(frame.size(), getFrameSize(frame.data(), frame.size()));
}

/** divoom.py switches to four checksum bytes once the sum exceeds 65535 */
static void test_frame_size_accepts_four_checksum_bytes(void) {
    bytes_t payload(300, 0xff);
    bytes_t frame = make_frame(payload, 4);

    TEST_ASSERT_EQUAL_size_t(308, frame.size());
    TEST_ASSERT_EQUAL_size_t(frame.size(), getFrameSize(frame.data(), frame.size()));
}

static void test_frame_size_accepts_an_escaped_frame(void) {
    bytes_t frame = escape_frame(make_frame({ 0x44, 0x01, 0x02, 0x03, 0x0a }, 2));

    TEST_ASSERT_EQUAL_size_t(14, frame.size());
    TEST_ASSERT_EQUAL_size_t(frame.size(), getFrameSize(frame.data(), frame.size()));
}

static void test_frame_size_rejects_a_wrong_checksum(void) {
    bytes_t frame = make_frame({ 0x45, 0x01, 0xff, 0x00, 0x80 }, 2);
    frame[4] ^= 0x10;

    TEST_ASSERT_EQUAL_size_t(0, getFrameSize(frame.data(), frame.size()));
}

static void test_frame_size_rejects_foreign_data(void) {
    bytes_t input = { 0x69, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x01 };

    TEST_ASSERT_EQUAL_size_t(0, getFrameSize(input.data(), input.size()));
    TEST_ASSERT_FALSE(isFramePending(input.data(), input.size()));
    TEST_ASSERT_EQUAL_size_t(0, getFrameSize(NULL, 0));
    TEST_ASSERT_FALSE(isFramePending(NULL, 0));
}

static void test_append_splits_two_frames_in_one_packet(void) {
    std::vector<bytes_t> expected = { make_frame({ 0x45, 0x01 }, 2), make_frame({ 0x74, 0x02, 0x03 }, 2) };
    bytes_t input = expected[0];
    input.insert(input.end(), expected[1].begin(), expected[1].end());

    reset();
    appendFrames(&stream, input.data(), input.size(), collect);

    assert_frames(expected, "two frames");
    TEST_ASSERT_EQUAL_size_t(0, stream.size);
}

static void test_append_joins_a_frame_split_over_two_packets(void) {
    bytes_t frame = make_frame({ 0x45, 0x01, 0xff, 0x00, 0x80, 0x4d }, 2);

    reset();
    appendFrames(&stream, frame.data(), 5, collect);
    TEST_ASSERT_EQUAL_size_t(0, handled.size());

    appendFrames(&stream, frame.data() + 5, frame.size() - 5, collect);
    assert_frames({ frame }, "split frame");
}

static void test_append_passes_foreign_data_through_as_one_unit(void) {
    std::vector<bytes_t> expected = { { 0x69, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x01 } };

    reset();
    appendFrames(&stream, expected[0].data(), expected[0].size(), collect);

    assert_frames(expected, "connect statement");
    TEST_ASSERT_EQUAL_size_t(0, stream.size);
}

static void test_append_ignores_an_empty_packet(void) {
    reset();
    appendFrames(&stream, NULL, 0, collect);

    TEST_ASSERT_EQUAL_size_t(0, handled.size());
    TEST_ASSERT_EQUAL_size_t(0, stream.size);
}

/** oversized input cannot be framed, but it must not lose a single byte either */
static void test_append_loses_nothing_beyond_the_buffer(void) {
    bytes_t input(FRAME_BUFFER_SIZE + 392, 0x33);
    size_t length = input.size() - 4;

    input[0] = 0x01;
    input[1] = length & 0xff;
    input[2] = (length >> 8) & 0xff;
    input[input.size() - 1] = 0x02;

    reset();
    appendFrames(&stream, input.data(), input.size(), collect);

    bytes_t forwarded;
    for (size_t i = 0; i < handled.size(); i++) forwarded.insert(forwarded.end(), handled[i].begin(), handled[i].end());

    TEST_ASSERT_EQUAL_size_t(input.size(), forwarded.size());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(input.data(), forwarded.data(), input.size());
    TEST_ASSERT_EQUAL_size_t(0, stream.size);
}

/** the 32x32 device, 29 animation frames of 213 bytes plus a short one */
static void test_stream_pixoomax_ha32(void) { assert_chunkings("pixoomax_ha32"); }
static void test_stream_pixoomax_ha16(void) { assert_chunkings("pixoomax_ha16"); }

/** a single 32x32 image, 757 bytes with a four byte checksum, lost entirely before the refactoring */
static void test_stream_pixoomax_smiley32(void) { assert_chunkings("pixoomax_smiley32"); }

static void test_stream_pixoo_ha32(void) { assert_chunkings("pixoo_ha32"); }
static void test_stream_pixoo_ha16(void) { assert_chunkings("pixoo_ha16"); }
static void test_stream_pixoo_smiley16(void) { assert_chunkings("pixoo_smiley16"); }

/** the two masked device families, where the length field cannot be trusted */
static void test_stream_aurabox_ha16(void) { assert_chunkings("aurabox_ha16"); }
static void test_stream_timeboxmini_ha16(void) { assert_chunkings("timeboxmini_ha16"); }

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_frame_size_of_an_exact_frame);
    RUN_TEST(test_frame_size_ignores_trailing_data);
    RUN_TEST(test_frame_size_is_zero_while_incomplete);
    RUN_TEST(test_frame_size_allows_end_marker_in_payload);
    RUN_TEST(test_frame_size_accepts_four_checksum_bytes);
    RUN_TEST(test_frame_size_accepts_an_escaped_frame);
    RUN_TEST(test_frame_size_rejects_a_wrong_checksum);
    RUN_TEST(test_frame_size_rejects_foreign_data);
    RUN_TEST(test_append_splits_two_frames_in_one_packet);
    RUN_TEST(test_append_joins_a_frame_split_over_two_packets);
    RUN_TEST(test_append_passes_foreign_data_through_as_one_unit);
    RUN_TEST(test_append_ignores_an_empty_packet);
    RUN_TEST(test_append_loses_nothing_beyond_the_buffer);
    RUN_TEST(test_stream_pixoomax_ha32);
    RUN_TEST(test_stream_pixoomax_ha16);
    RUN_TEST(test_stream_pixoomax_smiley32);
    RUN_TEST(test_stream_pixoo_ha32);
    RUN_TEST(test_stream_pixoo_ha16);
    RUN_TEST(test_stream_pixoo_smiley16);
    RUN_TEST(test_stream_aurabox_ha16);
    RUN_TEST(test_stream_timeboxmini_ha16);
    return UNITY_END();
}
