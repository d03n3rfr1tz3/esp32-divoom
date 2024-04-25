
#include "bluetoothctl.h"

#include "util.h"
#include "input/base.h"
#include "output/base.h"

/**
 * setup functionality
*/
void BluetoothHandler::setup(void) {
    serialBT.begin(BLUETOOTH_NAME, true);
    serialBT.setTimeout(5000);

    serialBT.register_callback(event);
    isScanning = serialBT.discoverAsync(discovered, 500);
}

/**
 * loop functionality
*/
void BluetoothHandler::loop(void) {
    if (getElapsed(timer) > 10000) {
        timer = millis();

        check(false);
    }
}

/**
 * checks connection and scanning state and keeps background tasks up
*/
bool BluetoothHandler::check(bool fast) {
    if (fast) return isConnected;
    
    if (!isScanning && serialBT.connected(2500)) {
        isConnected = true;
        isConnecting = false;
        return true;
    } else {
        isConnected = false;
        isConnecting = false;

        if (isScanning) {
            serialBT.discoverAsyncStop();
            serialBT.discoverClear();
            isScanning = false;
        } else {
            serialBT.register_callback(event);
            isScanning = serialBT.discoverAsync(discovered, 2500);
        }

        return false;
    }
}

/**
 * connects to the given bluetooth device
*/
bool BluetoothHandler::connect(BTAddress address, uint16_t channel) { return connect(address, channel, nullptr); };
bool BluetoothHandler::connect(BTAddress address, uint16_t channel, const char *pin) {
    serialBT.discoverAsyncStop();
    serialBT.discoverClear();
    isScanning = false;
    
    if (isConnected) BluetoothHandler::disconnect();
    if (pin != nullptr) serialBT.setPin(pin);
    delay(10);
    
    isConnecting = true;
    return serialBT.connect(address, channel);
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
 * callback for when a bluetooth device was discovered
*/
void BluetoothHandler::discovered(BTAdvertisedDevice* device) {
    if (!device->haveName()) return;

    // check for supported names
    std::string name = device->getName();
    if (name.find("Ditoo") == std::string::npos &&
        name.find("Pixoo") == std::string::npos &&
        name.find("Timebox") == std::string::npos &&
        name.find("Tivoo") == std::string::npos) return;

    // pass it into zeroconf
    MDNS.addServiceTxt("_divoom_esp32", "_tcp", "device_mac", device->getAddress().toString().c_str());
    MDNS.addServiceTxt("_divoom_esp32", "_tcp", "device_name", name.c_str());

    // pass it into the input handlers for an advertise announcement
    BaseInput::advertise((const uint8_t*)device->getAddress().getNative(), name.c_str(), name.size());
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