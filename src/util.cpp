
#include "util.h"

/**
 * helper for calculating the elapsed time in milliseconds
*/
unsigned long getElapsed(unsigned long compareMillis) {
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - compareMillis;
  return elapsedMillis;
}
