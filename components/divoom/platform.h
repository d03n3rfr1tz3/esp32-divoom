
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

    #include "esp_idf_version.h"
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        #define DIVOOM_BT_SPP_INIT()      ({ esp_spp_cfg_t cfg = BT_SPP_DEFAULT_CONFIG(); cfg.mode = ESP_SPP_MODE_CB; esp_spp_enhanced_init(&cfg); })
        #define DIVOOM_BT_SET_NAME(name)  esp_bt_gap_set_device_name(name)
    #else
        #define DIVOOM_BT_SPP_INIT()      esp_spp_init(ESP_SPP_MODE_CB)
        #define DIVOOM_BT_SET_NAME(name)  esp_bt_dev_set_device_name(name)
    #endif
#endif
