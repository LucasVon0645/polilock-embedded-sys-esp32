#include "rfid.hpp"

RFIDReader::RFIDReader(uint8_t ssPin, uint8_t rstPin)
  : mfrc522(ssPin, rstPin) {}

void RFIDReader::begin() {
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RC522 initialized. Waiting for card...");
}

bool RFIDReader::isCardPresent() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;
  return true;
}

String RFIDReader::readCardUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  return uid;
}
