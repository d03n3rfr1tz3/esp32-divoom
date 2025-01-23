#include "Arduino.h"
#include "esp_task_wdt.h"

#include "config.h"

#include "hardware/bluetoothctl.h"
#include "hardware/wifictl.h"

#include "input/base.h"
#include "output/base.h"

/**
 * setup functionality
*/
void setup() {
  Serial.begin(115200);
  esp_task_wdt_init(30, true);
  esp_task_wdt_add(NULL);
  
  BluetoothHandler::setup();
  WifiHandler::setup();
  BaseInput::setup();

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
