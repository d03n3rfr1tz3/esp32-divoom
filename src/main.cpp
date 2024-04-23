#include "Arduino.h"

#include "config.h"

#include "hardware/bluetoothctl.h"
#include "hardware/wifictl.h"

#include "input/base.h"
#include "output/base.h"

/**
 * setup functionality
*/
void setup() {
  Serial.begin(9600);
  
  BluetoothHandler::setup();
  WifiHandler::setup();

  BaseInput::setup();
  delay(10);
}

/**
 * loop functionality
*/
void loop() {
  BluetoothHandler::loop();
  WifiHandler::loop();

  BaseInput::loop();
  delay(10);
}
