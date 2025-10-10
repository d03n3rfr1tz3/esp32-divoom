
#include "bluetoothctl.h"

#include "util.h"
#include "input/base.h"
#include "output/base.h"

BluetoothHandler::BluetoothHandler() {
    timer = millis();
}

/**
 * setup functionality
*/
void BluetoothHandler::setup(void) {
    serialBT.begin(BLUETOOTH_NAME, true);
    serialBT.setTimeout(5000);
    serialBT.register_callback(event);
}

/**
 * loop functionality
*/
void BluetoothHandler::loop(void) {
    if (getElapsed(timer) > 15000) {
        timer = millis();

        BaseType_t taskResult = xTaskCreatePinnedToCore(task, "BluetoothTask", 2048, NULL, 1, &discoverHandle, 1);
        if (taskResult != pdPASS) ESP.restart();
    }
}

/**
 * background task for handling connection state and discovery
*/
void BluetoothHandler::task(void *parameter) {
    if (serialBT.connected(5000)) {
        isConnected = true;
        isConnecting = false;
    } else {
        isConnected = false;
        isConnecting = false;

        BluetoothHandler::discover(7500);
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
    if (pin != nullptr) serialBT.setPin(pin);
    delay(10);
    
    isConnecting = true;
    isConnected = serialBT.connect(address, channel);
    return isConnected;
}

/**
 * disconnects from the current bluetooth device
*/
bool BluetoothHandler::disconnect(void) {
    isConnected = false;
    isConnecting = false;
    return serialBT.disconnect();
}

/**
 * discover bluetooth devices
*/
void BluetoothHandler::discover(int timeout) {
    BTScanResults* devices = serialBT.discover(timeout);
    if (devices == nullptr) {
        // We have to restart the ESP, because it seems there is no way to get advertising going again after connecting once.
        // This is quite an old bug, that was never fixed and IMO is a major oversight.
        ESP.restart();
    }

    for (int i = 0; i < devices->getCount(); i++) {
        BTAdvertisedDevice* device = devices->getDevice(i);

        // check for supported names
        bool supported = device->haveName();
        std::string name = device->haveName() ? device->getName() : "Unknown";
        if (name.find("Aurabox") == std::string::npos && name.find("AuraBox") == std::string::npos &&
            name.find("Timebox") == std::string::npos && name.find("TimeBox") == std::string::npos &&
            name.find("Ditoo") == std::string::npos &&
            name.find("Pixoo") == std::string::npos &&
            name.find("Timoo") == std::string::npos &&
            name.find("Tivoo") == std::string::npos &&
            name.find("Divoom") == std::string::npos) supported = false;
        if (BLUETOOTH_FILTER && !supported) continue;

        // pass it into zeroconf
        if (supported) {
            MDNS.addServiceTxt("_divoom_esp32", "_tcp", "device_mac", device->getAddress().toString().c_str());
            MDNS.addServiceTxt("_divoom_esp32", "_tcp", "device_name", name.c_str());
        }

        // pass it into the input handlers for an advertise announcement
        BaseInput::advertise((const uint8_t*)device->getAddress().getNative(), name.c_str(), name.size(), supported);
        vTaskDelay(25 / portTICK_PERIOD_MS);
    }

    serialBT.discoverClear();
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
            size_t available;
            uint8_t buffer[64];
            while (available = serialBT.available()) {
                size_t size = serialBT.readBytes(buffer, available);

                // pass it into the output handlers backward channel
                BluetoothOutput::backward(buffer, size);
            }
            break;
    }
}

/**
 * sends data to the bluetooth device
*/
size_t BluetoothHandler::send(const uint8_t *buffer, size_t size) {
    if (!isConnected && !isConnecting) return -1;
    if (!isConnected && isConnecting) return 0;
    return serialBT.write(buffer, size);
}