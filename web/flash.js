
// Wires the configuration form to esp-web-tools. The manifest and the nvs image are built here
// and handed over as blob urls, so nothing ever leaves the browser.

import { buildNvsImage, NVS_OFFSET } from "./nvs.js";

const FIRMWARE_DIR = "firmware/";

// the identity values the device name fills in, the topic pattern stays on its own
const DERIVED = {
    wifi_name: "Hostname",
    bt_name: "Bluetooth name",
    mqtt_client: "MQTT client ID"
};

// the form field names are the nvs keys, this only adds the type
const FIELDS = [
    { key: "wifi_ssid1", type: "str" },
    { key: "wifi_pass1", type: "str" },
    { key: "wifi_ssid2", type: "str" },
    { key: "wifi_pass2", type: "str" },
    { key: "wifi_name", type: "str" },
    { key: "bt_name", type: "str" },
    { key: "mqtt_host", type: "str" },
    { key: "mqtt_port", type: "u16" },
    { key: "mqtt_user", type: "str" },
    { key: "mqtt_pass", type: "str" },
    { key: "mqtt_client", type: "str" },
    { key: "mqtt_topic", type: "str" }
];

const form = document.getElementById("config");
const unavailable = document.getElementById("unavailable");
const deviceName = document.getElementById("device_name");
const derived = document.getElementById("derived");
const reveal = document.getElementById("reveal");

let parts = null;
let urls = [];

/**
 * collects the values to write, dropping the empty ones so the firmware keeps its own defaults
*/
function entriesFromForm() {
    const data = new FormData(form);
    const value = key => (data.get(key) ?? "").trim();
    const mqtt = value("mqtt_host").length > 0;

    return FIELDS
        .filter(field => mqtt || !field.key.startsWith("mqtt_"))
        .map(field => ({ ...field, value: value(field.key) }))
        .filter(entry => entry.value.length > 0);
}

/**
 * releases the blob urls of the previous run, the dialog only reads them once
*/
function blobUrl(data, type) {
    const url = URL.createObjectURL(new Blob([data], { type }));
    urls.push(url);
    return url;
}

function buildManifest(mode) {
    for (const url of urls) URL.revokeObjectURL(url);
    urls = [];

    const image = buildNvsImage(entriesFromForm());
    const flash = [{ offset: NVS_OFFSET, path: blobUrl(image, "application/octet-stream") }];
    if (mode == "install") flash.push(...parts.map(part => ({ offset: part.offset, path: FIRMWARE_DIR + part.path })));
    flash.sort((left, right) => left.offset - right.offset);

    const manifest = {
        name: mode == "install" ? "ESP32-Divoom" : "ESP32-Divoom configuration",
        new_install_improv_wait_time: 0,
        builds: [{ chipFamily: "ESP32", parts: flash }]
    };

    // without the prompt esp-web-tools erases the whole chip, which would leave the config mode with no firmware
    if (mode != "install") manifest.new_install_prompt_erase = true;

    return blobUrl(JSON.stringify(manifest), "application/json");
}

/**
 * derives the identity values from the device name, as long as they were not typed in by hand
*/
function applyDeviceName() {
    const name = deviceName.value.trim();

    for (const key of Object.keys(DERIVED)) {
        const field = form.elements[key];
        if (!field.dataset.touched) field.value = name;
    }

    showDerived();
}

/**
 * spells out what the device name just filled in, the client id sits in another card and would
 * otherwise change out of sight
*/
function showDerived() {
    const name = deviceName.value.trim();
    const keys = Object.keys(DERIVED).filter(key => !form.elements[key].dataset.touched);

    derived.querySelector("ul").replaceChildren(...keys.map(key => {
        const item = document.createElement("li");
        const value = document.createElement("code");
        value.textContent = name;
        item.append(`${DERIVED[key]}: `, value);
        return item;
    }));

    derived.hidden = name.length == 0 || keys.length == 0;
}

/**
 * loads what the pages workflow put next to this page, the download stays usable without it.
 * the install button stays hidden until the images are known, so it can never run without them
*/
async function loadFirmware() {
    try {
        const response = await fetch(FIRMWARE_DIR + "parts.json");
        if (!response.ok) throw new Error(response.statusText);
        parts = await response.json();
        document.querySelector("[data-mode=install]").hidden = false;
    } catch {
        unavailable.textContent = "No firmware images found next to this page, so a full install is not available right now.";
        unavailable.hidden = false;
    }

    try {
        const response = await fetch(FIRMWARE_DIR + "version.json");
        if (!response.ok) throw new Error(response.statusText);
        document.getElementById("version").textContent = (await response.json()).version;
    } catch {
        document.getElementById("version").hidden = true;
    }
}

for (const key of Object.keys(DERIVED)) {
    form.elements[key].addEventListener("input", event => {
        event.target.dataset.touched = "true";
        showDerived();
    });
}

deviceName.addEventListener("input", applyDeviceName);

const passwords = [...form.querySelectorAll("input[type=password]")];
reveal.addEventListener("change", () => {
    for (const field of passwords) field.type = reveal.checked ? "text" : "password";
});

document.getElementById("download").addEventListener("click", event => {
    event.preventDefault();
    if (!form.reportValidity()) return;

    const url = URL.createObjectURL(new Blob([buildNvsImage(entriesFromForm())], { type: "application/octet-stream" }));
    const link = Object.assign(document.createElement("a"), { href: url, download: "nvs.bin" });
    link.click();
    URL.revokeObjectURL(url);
});

// capture phase, so the manifest is in place before the component opens its dialog
for (const wrapper of document.querySelectorAll(".action")) {
    wrapper.addEventListener("click", event => {
        if (!form.reportValidity()) return event.stopPropagation();
        wrapper.querySelector("esp-web-install-button").manifest = buildManifest(wrapper.dataset.mode);
    }, true);
}

loadFirmware();
