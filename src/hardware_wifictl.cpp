
#include "hardware_wifictl.h"

#include "util.h"
#include "settings.h"

#include "input_base.h"
#include "output_base.h"

#include "WiFi.h"

static bool isConnected;
static bool isMdnsReady;
static uint8_t retryCount;
static unsigned long timer;

static void scanned(WiFiEvent_t event, WiFiEventInfo_t info);
static void connected(WiFiEvent_t event, WiFiEventInfo_t info);
static void disconnected(WiFiEvent_t event, WiFiEventInfo_t info);

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
    WiFi.setHostname(SettingsHandler::wifiName.c_str());
    WiFi.onEvent(scanned, WiFiEvent_t::ARDUINO_EVENT_WIFI_SCAN_DONE);
    WiFi.onEvent(connected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(disconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.scanNetworks(true, false, false, 500);
}

/**
 * loop functionality
*/
void WifiHandler::loop(void) {
    if (getElapsed(timer) > 10000) {
        timer = millis();

        check(false);
    }
}

/**
 * checks connection and scanning state and keeps background tasks up
*/
bool WifiHandler::check(bool fast) {
    if (fast) return isConnected;

    if (WiFi.status() == WL_CONNECTED)
    {
        isConnected = true;
        retryCount = 0;
        return true;
    }
    else
    {
        isConnected = false;
        if (retryCount >= WIFI_RETRY) ESP.restart();

        int8_t wifiStatus = WiFi.status();
        int8_t scanStatus = WiFi.scanComplete();
        if ((wifiStatus == WL_IDLE_STATUS ||
             wifiStatus == WL_NO_SSID_AVAIL ||
             wifiStatus == WL_CONNECT_FAILED ||
             wifiStatus == WL_DISCONNECTED) && scanStatus != -1) {
            WiFi.scanNetworks(true, false, false, 2500);
            retryCount++;
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

        if (!SettingsHandler::wifiSsid1.isEmpty() && ssid == SettingsHandler::wifiSsid1) { WiFi.begin(SettingsHandler::wifiSsid1.c_str(), SettingsHandler::wifiPass1.c_str()); break; }
        if (!SettingsHandler::wifiSsid2.isEmpty() && ssid == SettingsHandler::wifiSsid2) { WiFi.begin(SettingsHandler::wifiSsid2.c_str(), SettingsHandler::wifiPass2.c_str()); break; }
    }

    WiFi.scanDelete();
}

/**
 * checks whether the mdns responder was started
*/
bool WifiHandler::mdns(void) {
    return isMdnsReady;
}

/**
 * callback for when the scan finished
*/
static void scanned(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (info.wifi_scan_done.number > 0) WifiHandler::connect();
}

/**
 * callback for when we connected to a network
*/
static void connected(WiFiEvent_t event, WiFiEventInfo_t info) {
    isConnected = true;
    retryCount = 0;

    WiFi.setTxPower(WIFI_POWER_15dBm);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(SettingsHandler::wifiName.c_str());
    WiFi.persistent(true);

    MDNS.begin(SettingsHandler::wifiName.c_str());
    MDNS.addService("_divoom_esp32", "_tcp", TCP_PORT);
    isMdnsReady = true;

    Serial.print("IP: ");
    WiFi.localIP().printTo(Serial);
    Serial.println("");
}

/**
 * callback for when we disconnected from a network
*/
static void disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    isConnected = false;
}
