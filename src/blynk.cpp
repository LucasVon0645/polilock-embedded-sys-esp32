#include "blynk.hpp"
#include "config.h"

void onV1Write(const BlynkParam& param) {
  int v = param.asInt();
  if (v == V_UNLOCKED) {
    LockCtrl::cmdUnlock(millis());
    Serial.println(F("[CMD] unlock 🔓"));
  } else {
    LockCtrl::cmdLock();
    Serial.println(F("[CMD] lock 🔒"));
  }
}