
#ifndef _PLATFORM_H
    #define _PLATFORM_H

    #include "Arduino.h"

    // The shared core builds both as the PlatformIO firmware and as an ESPHome
    // component. Every difference between the two is concentrated here, so the
    // PlatformIO branch expands to exactly the code that was inline before.

    #ifdef DIVOOM_PLATFORM_ESPHOME
        #include "mdns.h"

        // ESPHome owns the task watchdog and reports failures through the
        // component instead of rebooting. Defined in divoom_component.cpp.
        void divoomFail(const char *reason);

        #define DIVOOM_FAIL(reason) divoomFail(reason)
        #define DIVOOM_WDT_ADD()    ((void)0)
        #define DIVOOM_WDT_RESET()  ((void)0)

        // ESPHome talks to the IDF responder directly instead of going through
        // ESPmDNS, so the txt records are set through its own api.
        #define DIVOOM_MDNS_TXT(service, proto, key, value) mdns_service_txt_item_set(service, proto, key, value)
    #else
        #include "ESPmDNS.h"
        #include "esp_task_wdt.h"

        #define DIVOOM_FAIL(reason) ESP.restart()
        #define DIVOOM_WDT_ADD()    esp_task_wdt_add(NULL)
        #define DIVOOM_WDT_RESET()  esp_task_wdt_reset()

        #define DIVOOM_MDNS_TXT(service, proto, key, value) MDNS.addServiceTxt(service, proto, key, value)
    #endif

    // BluetoothSerial::setPin gained a length parameter in arduino-esp32 3.x.
    // PlatformIO pins 2.0.17, ESPHome ships 3.3.10.
    #if ESP_ARDUINO_VERSION_MAJOR >= 3
        #define DIVOOM_BT_SETPIN(bt, pin) (bt).setPin(pin, strlen(pin))
    #else
        #define DIVOOM_BT_SETPIN(bt, pin) (bt).setPin(pin)
    #endif
#endif
