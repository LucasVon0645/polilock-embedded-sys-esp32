#include "blynk.hpp"

void onV1Write(const BlynkParam& param) {
  int v = param.asInt();
  if (v == 1) {
    LockCtrl::cmdUnlock(millis());
    Serial.println(F("[CMD] unlock 🔓"));
  } else {
    LockCtrl::cmdLock();
    Serial.println(F("[CMD] lock 🔒"));
  }
}