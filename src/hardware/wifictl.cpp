
#include "util.h"
#include "wifictl.h"

WifiHandler::WifiHandler() {
    timer = millis();
}

/**
 * setup functionality
*/
void WifiHandler::setup(void) {
    WiFi.disconnect(true);
    WiFi.persistent(true);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(WIFI_NAME);
    WiFi.onEvent(scanned, WiFiEvent_t::ARDUINO_EVENT_WIFI_SCAN_DONE);
    WiFi.onEvent(connected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(disconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.scanNetworks(true, false, false, 250);
}

/**
 * loop functionality
*/
void WifiHandler::loop(void) {
    if (getElapsed(timer) > 15000) {
        timer = millis();

        check();
    }
}

/**
 * checks connection and scanning state and keeps background tasks up
*/
bool WifiHandler::check(void) {
    if (WiFi.status() == WL_CONNECTED)
    {
        isConnected = true;
        return true;
    }
    else
    {
        isConnected = false;

        int8_t wifiStatus = WiFi.status();
        int8_t scanStatus = WiFi.scanComplete();
        if ((wifiStatus == WL_IDLE_STATUS ||
             wifiStatus == WL_NO_SSID_AVAIL ||
             wifiStatus == WL_CONNECT_FAILED ||
             wifiStatus == WL_DISCONNECTED) && scanStatus != -1) {
            WiFi.scanNetworks(true, false, false, 500);
        }

        return WiFi.status() == WL_CONNECTED;
    }
}

/**
 * connects to one of the scanned wifi networks
*/
void WifiHandler::connect(void) {
    if (WiFi.status() == WL_CONNECTED) return;

    int16_t count = WiFi.scanComplete();
    for (uint8_t i = 0; i < count && i < 99; i++) {
        auto ssid = WiFi.SSID(i);
        if (ssid == NULL || ssid.length() == 0) continue;

        if (ssid == WIFISSID1) { WiFi.begin(WIFISSID1, WIFIPASS1); break; }
        if (ssid == WIFISSID2) { WiFi.begin(WIFISSID2, WIFIPASS2); break; }
    }

    WiFi.scanDelete();
}

/**
 * callback for when the scan finished
*/
void WifiHandler::scanned(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (info.wifi_scan_done.number > 0) connect();
}

/**
 * callback for when we connected to a network
*/
void WifiHandler::connected(WiFiEvent_t event, WiFiEventInfo_t info) {
    isConnected = true;

    WiFi.setTxPower(WIFI_POWER_15dBm);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(WIFI_NAME);
    WiFi.persistent(true);
}

/**
 * callback for when we disconnected from a network
*/
void WifiHandler::disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    isConnected = false;
}
