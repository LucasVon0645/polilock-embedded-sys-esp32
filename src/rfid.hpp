#ifndef RFID_HPP
#define RFID_HPP

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "lock_ctrl.hpp"

class RFIDReader {
public:
  RFIDReader(uint8_t ssPin, uint8_t rstPin);
  void begin();
  void pool();
  bool isCardPresent();
  String readCardUID();

private:
  MFRC522 mfrc522;
};

#endif