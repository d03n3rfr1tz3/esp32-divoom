
#ifndef _WIFI_H
    #define _WIFI_H

    #include "config.h"

    #include "platform.h"

    class WifiHandler {
        public:
            WifiHandler();
            static void setup(void);
            static void loop(void);
            static bool check(bool fast);
            static void connect(void);
            static bool mdns(void);
    };
#endif
