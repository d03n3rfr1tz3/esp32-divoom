#include "Arduino.h"
#include "esp_task_wdt.h"

#include "config.h"
#include "settings.h"

#include "hardware_bluetoothctl.h"
#include "hardware_wifictl.h"

#include "input_base.h"
#include "output_base.h"

/**
 * setup functionality
*/
void setup() {
  Serial.begin(115200);
  esp_task_wdt_init(30, true);
  esp_task_wdt_add(NULL);

  SettingsHandler::setup();
  BluetoothHandler::setup();
  WifiHandler::setup();
  BaseInput::setup();
  BaseOutput::setup();

  esp_task_wdt_reset();
  delay(10);
}

/**
 * loop functionality
*/
void loop() {
  BluetoothHandler::loop();
  WifiHandler::loop();
  BaseInput::loop();

  esp_task_wdt_reset();
  delay(10);
}
