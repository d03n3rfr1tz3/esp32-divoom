
#ifndef _PLATFORM_H
    #define _PLATFORM_H

    #include "Arduino.h"

    // every difference between the PlatformIO firmware and the ESPHome component
    // is concentrated here

    #ifdef DIVOOM_PLATFORM_ESPHOME
        #include "mdns.h"

        // restarts deferred into the main task, so ESPHome can tear itself down first
        void divoomFail(const char *reason);
        void divoomLog(const char *message);

        #define DIVOOM_FAIL(reason) divoomFail(reason)
        #define DIVOOM_LOG(message) divoomLog(message)

        // ESPHome owns the task watchdog
        #define DIVOOM_WDT_ADD()    ((void)0)
        #define DIVOOM_WDT_RESET()  ((void)0)

        // ESPHome uses the IDF responder instead of ESPmDNS
        #define DIVOOM_MDNS_TXT(service, proto, key, value) mdns_service_txt_item_set(service, proto, key, value)

        // ESPHome sends mqtt and log lines on the stack of the calling task
        #define DIVOOM_TASK_STACK_BLUETOOTH 6144
        #define DIVOOM_TASK_STACK_PARSE     8192
    #else
        #include "ESPmDNS.h"
        #include "esp_task_wdt.h"

        #define DIVOOM_FAIL(reason) ESP.restart()
        #define DIVOOM_LOG(message) ((void)0)
        #define DIVOOM_WDT_ADD()    esp_task_wdt_add(NULL)
        #define DIVOOM_WDT_RESET()  esp_task_wdt_reset()

        #define DIVOOM_MDNS_TXT(service, proto, key, value) MDNS.addServiceTxt(service, proto, key, value)

        #define DIVOOM_TASK_STACK_BLUETOOTH 2048
        #define DIVOOM_TASK_STACK_PARSE     3072
    #endif

    // BluetoothSerial::setPin takes the pin length since arduino-esp32 3.x
    #if ESP_ARDUINO_VERSION_MAJOR >= 3
        #define DIVOOM_BT_SETPIN(bt, pin) (bt).setPin(pin, strlen(pin))
    #else
        #define DIVOOM_BT_SETPIN(bt, pin) (bt).setPin(pin)
    #endif

    // BluetoothSerial::begin defaults to BTDM, which a br/edr only controller refuses;
    // its third parameter picks classic and exists since arduino-esp32 3.x
    #ifdef DIVOOM_PLATFORM_ESPHOME
        #define DIVOOM_BT_BEGIN(bt, name) (bt).begin(name, true, true)
    #else
        #define DIVOOM_BT_BEGIN(bt, name) (bt).begin(name, true)
    #endif
#endif
