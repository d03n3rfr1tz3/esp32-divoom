import test from "node:test";
import assert from "node:assert/strict";

import { readFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { buildNvsImage, NVS_SIZE } from "../../web/nvs.js";

const root = dirname(fileURLToPath(import.meta.url));
const fixtures = resolve(process.env.NVS_FIXTURES || join(root, "fixtures"));
const cases = JSON.parse(readFileSync(join(root, "cases.json"), "utf-8"));

const PAGE_SIZE = 4096;
const HEADER_SIZE = 32;
const BITMAP_SIZE = 32;
const ENTRY_SIZE = 32;

/**
 * names the field an offset falls into, because a raw offset into 20 kilobytes says nothing
*/
function locate(offset) {
    const page = Math.floor(offset / PAGE_SIZE);
    const inPage = offset % PAGE_SIZE;

    if (inPage < HEADER_SIZE) return `page ${page}, header byte ${inPage}`;
    if (inPage < HEADER_SIZE + BITMAP_SIZE) return `page ${page}, bitmap byte ${inPage - HEADER_SIZE}`;

    const entry = Math.floor((inPage - HEADER_SIZE - BITMAP_SIZE) / ENTRY_SIZE);
    return `page ${page}, entry ${entry}, byte ${(inPage - HEADER_SIZE - BITMAP_SIZE) % ENTRY_SIZE}`;
}

/**
 * dumps the entry an offset belongs to as hex, so a diff can actually be read
*/
function dump(label, bytes, offset) {
    const start = Math.max(0, offset - (offset % ENTRY_SIZE));
    const slice = Array.from(bytes.subarray(start, start + ENTRY_SIZE));
    const hex = slice.map((byte, index) => (start + index == offset ? `[${byte.toString(16).padStart(2, "0")}]` : byte.toString(16).padStart(2, "0")));
    return `${label} @${start.toString(16).padStart(4, "0")}: ${hex.join(" ")}`;
}

function compare(expected, actual) {
    assert.equal(actual.length, expected.length, `image size differs: expected ${expected.length}, got ${actual.length}`);

    for (let offset = 0; offset < expected.length; offset++) {
        if (expected[offset] == actual[offset]) continue;

        assert.fail([
            `first difference at offset 0x${offset.toString(16).padStart(4, "0")} (${locate(offset)}):`,
            `expected 0x${expected[offset].toString(16).padStart(2, "0")}, got 0x${actual[offset].toString(16).padStart(2, "0")}`,
            dump("reference", expected, offset),
            dump("encoder  ", actual, offset),
        ].join("\n"));
    }
}

for (const testCase of cases) {
    test(testCase.name, () => {
        const expected = readFileSync(join(fixtures, `${testCase.name}.bin`));
        const actual = buildNvsImage(testCase.entries, testCase.size || NVS_SIZE);

        compare(expected, actual);
    });
}
