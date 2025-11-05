// rfid.hpp
#ifndef RFID_HPP
#define RFID_HPP

#include "config.h"
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <vector>
#include "lock_ctrl.hpp"

#include <BlynkSimpleEsp32.h>
extern BlynkWifi Blynk;


class RFIDReader {
public:
  RFIDReader(uint8_t ssPin, uint8_t rstPin);
  void begin();
  void pool();

  void startEnroll(uint32_t windowMs = ENROLL_WINDOW_MS); // start enrollment mode for windowMs milliseconds
  void cancelEnroll();                          // cancel enrollment mode
  size_t authorizedCount() const { return authorizedUIDs.size(); }

  bool RFID_takeCancelEnrollEvent(); // returns true if in enroll mode, and cancels it

private:
  MFRC522 mfrc522;

  // ---- storage ----
  Preferences prefs;
  std::vector<String> authorizedUIDs;
  void loadUIDs();
  void saveUIDs();
  bool uidExists(const String& uid) const;

  // ---- RFID reading ----
  bool isCardPresent();
  String readCardUID();

  // ---- enrollment mode ----
  bool enrollMode = false;
  uint32_t enrollUntilMs = 0;

  // ---- event latches ----
  bool h_latchedCancelEnrollEvent = false;
};

#endif
