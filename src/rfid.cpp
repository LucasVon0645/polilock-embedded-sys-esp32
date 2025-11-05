#include "rfid.hpp"
#define AUTHORIZED_RFID_UID "72EE856C"

RFIDReader::RFIDReader(uint8_t ssPin, uint8_t rstPin)
  : mfrc522(ssPin, rstPin) {}

void RFIDReader::begin() {
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RC522 initialized. Waiting for card...");
}

void RFIDReader::pool() {
  if (isCardPresent()) {
    String uid = readCardUID();
    Serial.print("Card UID: ");
    Serial.println(uid);

    if(uid == AUTHORIZED_RFID_UID) {
      Serial.println("Authorized card detected. Unlocking door...");
      LockCtrl::cmdUnlock(millis());
    } else {
      Serial.println("Unauthorized card.");
    }
  }
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
