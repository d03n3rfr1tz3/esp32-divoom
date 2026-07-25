#include "Arduino.h"

unsigned long fake_millis = 0;

unsigned long millis() {
    return fake_millis;
}
