
// Encoder for ESP-IDF NVS v2 partition images. Fields that are not written stay 0xFF,
// which is what an erased flash sector looks like and what the official generator emits.

export const NVS_SIZE = 0x5000;
export const NVS_OFFSET = 0x9000;
export const NVS_NAMESPACE = "divoom";
export const NVS_KEY_MAX = 15;

const NVS_PAGE_SIZE = 4096;
const NVS_PAGE_HEADER_SIZE = 32;
const NVS_BITMAP_SIZE = 32;
const NVS_ENTRY_SIZE = 32;
const NVS_ENTRY_OFFSET = NVS_PAGE_HEADER_SIZE + NVS_BITMAP_SIZE;
const NVS_ENTRIES_PER_PAGE = 126;

const NVS_STATE_ACTIVE = 0xFFFFFFFE;
const NVS_VERSION2 = 0xFE;

const NVS_TYPE_U8 = 0x01;
const NVS_TYPE_U16 = 0x02;
const NVS_TYPE_STR = 0x21;

const CRC_TABLE = (() => {
    const table = new Uint32Array(256);
    for (let i = 0; i < 256; i++) {
        let value = i;
        for (let bit = 0; bit < 8; bit++) value = value & 1 ? (value >>> 1) ^ 0xEDB88320 : value >>> 1;
        table[i] = value >>> 0;
    }
    return table;
})();

/**
 * the crc32 the generator uses is zlib.crc32(data, 0xFFFFFFFF), which starts with a
 * cleared register instead of the all ones a plain crc32 starts with
*/
function crc32(data) {
    let crc = 0;
    for (const byte of data) crc = (crc >>> 8) ^ CRC_TABLE[(crc ^ byte) & 0xFF];
    return (crc ^ 0xFFFFFFFF) >>> 0;
}

/**
 * builds an nvs image for the given entries, each of them { key, type: "str" | "u16", value }.
 * empty values are not filtered here, the caller decides which keys to write at all.
*/
export function buildNvsImage(entries, size = NVS_SIZE, namespace = NVS_NAMESPACE) {
    if (size <= 0 || size % NVS_PAGE_SIZE != 0) throw new Error("nvs size must be a multiple of the 4096 byte page size");

    const encoder = new TextEncoder();
    const image = new Uint8Array(size).fill(0xFF);
    const view = new DataView(image.buffer);
    let used = 0;

    /**
     * claims span entries, writes the header of the first one and returns its offset
    */
    function claim(index, type, key, span) {
        if (used + span > NVS_ENTRIES_PER_PAGE) throw new Error("too many values for a single nvs page");

        const keyBytes = encoder.encode(key);
        if (keyBytes.length < 1 || keyBytes.length > NVS_KEY_MAX) throw new Error(`key "${key}" has to be 1 to ${NVS_KEY_MAX} bytes long`);

        const offset = NVS_ENTRY_OFFSET + used * NVS_ENTRY_SIZE;
        image[offset] = index;
        image[offset + 1] = type;
        image[offset + 2] = span;

        // the key is the only field padded with zeros instead of 0xFF
        image.fill(0x00, offset + 8, offset + 24);
        image.set(keyBytes, offset + 8);

        // two bits per entry, clearing the lower one turns empty into written
        for (let i = 0; i < span; i++) {
            const bit = (used + i) * 2;
            image[NVS_PAGE_HEADER_SIZE + (bit >> 3)] &= ~(1 << (bit & 7));
        }

        used += span;
        return offset;
    }

    /**
     * seals an entry with its crc32, which covers all of it but the crc field itself
    */
    function seal(offset) {
        const covered = new Uint8Array(NVS_ENTRY_SIZE - 4);
        covered.set(image.subarray(offset, offset + 4), 0);
        covered.set(image.subarray(offset + 8, offset + NVS_ENTRY_SIZE), 4);
        view.setUint32(offset + 4, crc32(covered), true);
    }

    // only the first page is written, the remaining ones stay erased
    view.setUint32(0, NVS_STATE_ACTIVE, true);
    view.setUint32(4, 0, true);
    image[8] = NVS_VERSION2;
    view.setUint32(28, crc32(image.subarray(4, 28)), true);

    // the namespace comes first and gets the index every following entry refers to
    const namespaceOffset = claim(0, NVS_TYPE_U8, namespace, 1);
    image[namespaceOffset + 24] = 1;
    seal(namespaceOffset);

    for (const entry of entries) {
        if (entry.type == "u16") {
            const value = Number(entry.value);
            if (!Number.isInteger(value) || value < 0 || value > 0xFFFF) throw new Error(`value of "${entry.key}" is not a 16 bit unsigned integer`);

            const offset = claim(1, NVS_TYPE_U16, entry.key, 1);
            view.setUint16(offset + 24, value, true);
            seal(offset);
            continue;
        }

        if (entry.type != "str") throw new Error(`unsupported type "${entry.type}" for "${entry.key}"`);

        // a string keeps length and checksum in its entry and the payload in the following ones
        const payload = encoder.encode(String(entry.value));
        const data = new Uint8Array(payload.length + 1);
        data.set(payload);

        const offset = claim(1, NVS_TYPE_STR, entry.key, 1 + Math.ceil(data.length / NVS_ENTRY_SIZE));
        view.setUint16(offset + 24, data.length, true);
        view.setUint16(offset + 26, 0xFFFF, true);
        view.setUint32(offset + 28, crc32(data), true);
        image.set(data, offset + NVS_ENTRY_SIZE);
        seal(offset);
    }

    return image;
}
