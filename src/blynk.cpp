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

void onV2Write(const BlynkParam& param, RFIDReader& rfid) {
  int state = param.asInt(); // 1=ligado, 0=desligado
  if (state == 1) {
    rfid.startEnroll(15000); // janela de 15s (ajuste à vontade)
  } else {
    rfid.cancelEnroll();
  }
}