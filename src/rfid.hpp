// rfid.hpp
#ifndef RFID_HPP
#define RFID_HPP

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <vector>
#include "lock_ctrl.hpp"

class RFIDReader {
public:
  RFIDReader(uint8_t ssPin, uint8_t rstPin);
  void begin();
  void pool();

  // ---- novas APIs ----
  void startEnroll(uint32_t windowMs = 15000); // inicia modo cadastro por X ms
  void cancelEnroll();                          // cancela/fecha janela
  size_t authorizedCount() const { return authorizedUIDs.size(); }

private:
  MFRC522 mfrc522;

  // ---- storage/lista ----
  Preferences prefs;
  std::vector<String> authorizedUIDs;
  void loadUIDs();
  void saveUIDs();
  bool uidExists(const String& uid) const;

  // ---- leitura RFID ----
  bool isCardPresent();
  String readCardUID();

  // ---- modo cadastro ----
  bool enrollMode = false;
  uint32_t enrollUntilMs = 0;
};

#endif
