
#ifdef DIVOOM_PLATFORM_ESPHOME

#include "hardware_wifictl.h"
#include "settings.h"

#include "esphome/components/network/util.h"

static bool isConnected;
static bool isMdnsReady;
static unsigned long timer;

/**
 * ESPHome owns the WiFi connection, so the handler degenerates into a facade
 * over its network state. Only the zeroconf service stays ours, because
 * hass-divoom discovers the device by it.
*/
WifiHandler::WifiHandler() {
    timer = millis();
}

/**
 * setup functionality
*/
void WifiHandler::setup(void) {
    // ESPHome connects on its own
}

/**
 * loop functionality
*/
void WifiHandler::loop(void) {
    check(false);
}

/**
 * checks connection and registers our service once it is up
*/
bool WifiHandler::check(bool fast) {
    isConnected = esphome::network::is_connected();
    if (!isConnected) return false;

    // the mdns component initializes the responder at AFTER_CONNECTION, which is
    // before us, so from here on the service can simply be added to it
    if (!isMdnsReady) {
        isMdnsReady = mdns_service_add(NULL, "_divoom_esp32", "_tcp", SettingsHandler::tcpPort, NULL, 0) == ESP_OK;
    }

    return isConnected;
}

/**
 * reconnects, which ESPHome already does on its own
*/
void WifiHandler::connect(void) {
    // no additional handling needed
}

/**
 * tells whether our service is registered
*/
bool WifiHandler::mdns(void) {
    return isMdnsReady;
}

#endif
