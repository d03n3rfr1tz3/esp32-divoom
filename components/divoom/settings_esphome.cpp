
#ifdef DIVOOM_PLATFORM_ESPHOME

#include "settings.h"

#include "esphome/core/application.h"

/**
 * every value arrives from the YAML through the components setters, so nothing
 * is loaded here; only what ESPHome already knows fills the remaining gaps
*/
void SettingsHandler::setup(void) {
    if (wifiName.isEmpty()) wifiName = esphome::App.get_name().c_str();
    if (bluetoothName.isEmpty()) bluetoothName = wifiName;
    if (mqttClient.isEmpty()) mqttClient = wifiName;
}

#endif
