
#include "hardware_bluetoothctl.h"

#include "platform.h"
#include "util.h"
#include "settings.h"

#include "hardware_wifictl.h"
#include "input_base.h"
#include "output_base.h"

BluetoothHandler::BluetoothHandler() {
    timer = millis();
}

/**
 * setup functionality
*/
void BluetoothHandler::setup(void) {
    if (!BluetoothSpp::begin(SettingsHandler::bluetoothName.c_str())) DIVOOM_LOG("bluetooth could not be started, no device will connect");
    BluetoothSpp::onEvent(event);
}

/**
 * loop functionality
*/
void BluetoothHandler::loop(void) {
    if (getElapsed(timer) > 15000) {
        timer = millis();

        if (!isConnecting && !isDiscovering) {
            BaseType_t taskResult = xTaskCreatePinnedToCore(task, "BluetoothTask", DIVOOM_TASK_STACK_BLUETOOTH, NULL, 1, &discoverHandle, 1);
            if (taskResult != pdPASS) DIVOOM_FAIL("could not create the bluetooth task");
        }
    }
}

/**
 * background task for handling connection state and discovery
*/
void BluetoothHandler::task(void *parameter) {
    if (BluetoothSpp::connected(5000)) {
        isConnected = true;
        isConnecting = false;
    } else {
        isConnected = false;
        isConnecting = false;

        // a dropped connection is no discovery case, the peer is still known
        if (remoteAddress && reconnectCount < BLUETOOTH_RETRY) {
            reconnectCount++;
            BluetoothHandler::connect(remoteAddress, remoteChannel);
        } else {
            remoteAddress = BTAddress();

            isDiscovering = true;
            BluetoothHandler::discover(7500);
            isDiscovering = false;
        }
    }

    vTaskDelete(NULL);
}

/**
 * checks connection state
*/
bool BluetoothHandler::check(void) {
    return isConnected;
}

/**
 * connects to the given bluetooth device
*/
bool BluetoothHandler::connect(BTAddress address, uint16_t channel) { return connect(address, channel, nullptr); };
bool BluetoothHandler::connect(BTAddress address, uint16_t channel, const char *pin) {
    if (isConnected) BluetoothHandler::disconnect();
    if (pin != nullptr) BluetoothSpp::setPin(pin);

    // a running inquiry blocks the spp connect, so end it first
    if (isDiscovering) BluetoothSpp::discoverStop();
    delay(10);

    isConnecting = true;
    isConnected = BluetoothSpp::connect(address, channel);
    isConnecting = false;

    if (!isConnected) DIVOOM_LOG("could not connect to the bluetooth device");
    if (isConnected) {
        remoteAddress = address;
        remoteChannel = channel;
        reconnectCount = 0;
    }

    return isConnected;
}

/**
 * disconnects from the current bluetooth device
*/
bool BluetoothHandler::disconnect(void) {
    isConnected = false;
    isConnecting = false;

    // an explicit disconnect is final, so drop the peer to prevent a reconnect
    remoteAddress = BTAddress();

    return BluetoothSpp::disconnect();
}

/**
 * discover bluetooth devices
*/
void BluetoothHandler::discover(int timeout) {
    for (const BluetoothDevice &device : BluetoothSpp::discover(timeout)) {
        // check for supported names
        bool supported = !device.name.empty();
        std::string name = supported ? device.name : "Unknown";
        if (name.find("Aurabox") == std::string::npos && name.find("AuraBox") == std::string::npos &&
            name.find("Timebox") == std::string::npos && name.find("TimeBox") == std::string::npos &&
            name.find("Ditoo") == std::string::npos &&
            name.find("Pixoo") == std::string::npos &&
            name.find("Timoo") == std::string::npos &&
            name.find("Tivoo") == std::string::npos &&
            name.find("Divoom") == std::string::npos) supported = false;
        if (BLUETOOTH_FILTER && !supported) continue;

        // pass it into zeroconf
        if (supported && WifiHandler::mdns()) {
            DIVOOM_MDNS_TXT("_divoom_esp32", "_tcp", "device_mac", device.address.toString().c_str());
            DIVOOM_MDNS_TXT("_divoom_esp32", "_tcp", "device_name", name.c_str());
        }

        // pass it into the input handlers for an advertise announcement
        BaseInput::advertise(device.address.getNative(), name.c_str(), name.size(), supported);
        vTaskDelay(25 / portTICK_PERIOD_MS);
    }
}

/**
 * callback for when some data from a bluetooth device was received
*/
void BluetoothHandler::event(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_OPEN_EVT:
            isConnected = true;
            isConnecting = false;
            MqttInput::update();
            break;
        case ESP_SPP_CLOSE_EVT:
            isConnected = false;
            isConnecting = false;
            MqttInput::update();
            break;
        case ESP_SPP_DATA_IND_EVT:
            // pass it into the output handlers backward channel
            BluetoothOutput::backward(param->data_ind.data, param->data_ind.len);
            break;
    }
}

/**
 * sends data to the bluetooth device
*/
size_t BluetoothHandler::send(const uint8_t *buffer, size_t size) {
    if (!isConnected && !isConnecting) return -1;
    if (!isConnected && isConnecting) return 0;
    return BluetoothSpp::write(buffer, size);
}