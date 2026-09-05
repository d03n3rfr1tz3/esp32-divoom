
#ifdef DIVOOM_PLATFORM_ESPHOME

#include "settings.h"

#include "esphome/core/application.h"

/**
 * setup functionality, that fills the names left empty in the YAML
*/
void SettingsHandler::setup(void) {
    if (wifiName.isEmpty()) wifiName = esphome::App.get_name().c_str();
    if (bluetoothName.isEmpty()) bluetoothName = wifiName;
    if (mqttClient.isEmpty()) mqttClient = wifiName;
}

#endif
