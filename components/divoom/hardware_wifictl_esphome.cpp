
#ifdef DIVOOM_PLATFORM_ESPHOME

#include "hardware_wifictl.h"
#include "settings.h"

#include "esphome/components/network/util.h"

static bool isConnected;
static bool isMdnsReady;
static unsigned long timer;

WifiHandler::WifiHandler() {
    timer = millis();
}

/**
 * setup functionality, where ESPHome connects on its own
*/
void WifiHandler::setup(void) {}

/**
 * loop functionality
*/
void WifiHandler::loop(void) {
    check(false);
}

/**
 * checks connection and registers the zeroconf service once it is up
*/
bool WifiHandler::check(bool fast) {
    isConnected = esphome::network::is_connected();
    if (!isConnected) return false;

    // the mdns responder is up by now, so the service can simply be added to it
    if (!isMdnsReady) {
        isMdnsReady = mdns_service_add(NULL, "_divoom_esp32", "_tcp", SettingsHandler::tcpPort, NULL, 0) == ESP_OK;
    }

    return isConnected;
}

/**
 * reconnects, which ESPHome already does on its own
*/
void WifiHandler::connect(void) {}

/**
 * checks whether the zeroconf service is registered
*/
bool WifiHandler::mdns(void) {
    return isMdnsReady;
}

#endif
