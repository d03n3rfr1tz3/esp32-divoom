
#include "settings.h"

#include "Preferences.h"

/**
 * loads the runtime configuration from the nvs partition.
*/
void SettingsHandler::setup(void) {
    Preferences preferences;
    preferences.begin("divoom", true);

    wifiSsid1     = preferences.getString("wifi_ssid1", WIFISSID1);
    wifiPass1     = preferences.getString("wifi_pass1", WIFIPASS1);
    wifiSsid2     = preferences.getString("wifi_ssid2", WIFISSID2);
    wifiPass2     = preferences.getString("wifi_pass2", WIFIPASS2);
    wifiName      = preferences.getString("wifi_name", WIFI_NAME);
    bluetoothName = preferences.getString("bt_name", BLUETOOTH_NAME);

    mqttHost      = preferences.getString("mqtt_host", MQTT_HOST);
    mqttUser      = preferences.getString("mqtt_user", MQTT_USER);
    mqttPass      = preferences.getString("mqtt_pass", MQTT_PASS);
    mqttClient    = preferences.getString("mqtt_client", MQTT_CLIENT);
    mqttTopic     = preferences.getString("mqtt_topic", MQTT_TOPIC);
    mqttPort      = preferences.getUShort("mqtt_port", MQTT_PORT);

    preferences.end();

    if (!isValidHostname(wifiName.c_str())) wifiName = WIFI_NAME;
    if (bluetoothName.isEmpty()) bluetoothName = BLUETOOTH_NAME;
    if (mqttClient.isEmpty()) mqttClient = MQTT_CLIENT;
    if (!isValidTopicFormat(mqttTopic.c_str())) mqttTopic = MQTT_TOPIC;
    if (mqttPort == 0) mqttPort = MQTT_PORT;

    Serial.print("Name: ");
    Serial.println(wifiName);
    Serial.print("SSID: ");
    Serial.print(wifiSsid1);
    Serial.print(" / ");
    Serial.println(wifiSsid2);
    Serial.print("MQTT: ");
    Serial.print(mqttHost);
    Serial.print(":");
    Serial.println(mqttPort);
}
