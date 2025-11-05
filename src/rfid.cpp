// rfid.cpp
#include "secrets.h"
#include "rfid.hpp"

#define PREFS_NS   "rfid"
#define PREFS_KEY  "uids"   // format: "UID1;UID2;UID3"

extern String pendingMsgV3;

RFIDReader::RFIDReader(uint8_t ssPin, uint8_t rstPin)
  : mfrc522(ssPin, rstPin) {}

void RFIDReader::begin() {
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RC522 initialized. Waiting for card...");
  loadUIDs();
  Serial.print("Authorized tags: ");
  Serial.println(authorizedUIDs.size());
  pendingMsgV3 = String("Tags salvas: ") + authorizedUIDs.size();
}

void RFIDReader::loadUIDs() {
  authorizedUIDs.clear();
  if (!prefs.begin(PREFS_NS, true)) return;
  String all = prefs.getString(PREFS_KEY, "");
  prefs.end();

  int start = 0;
  while (start >= 0) {
    int sep = all.indexOf(';', start);
    String token = (sep >= 0) ? all.substring(start, sep) : all.substring(start);
    token.trim();
    if (token.length() > 0) authorizedUIDs.push_back(token);
    if (sep < 0) break;
    start = sep + 1;
  }
}

void RFIDReader::saveUIDs() {
  String all = "";
  for (size_t i = 0; i < authorizedUIDs.size(); i++) {
    all += authorizedUIDs[i];
    if (i + 1 < authorizedUIDs.size()) all += ';';
  }
  if (!prefs.begin(PREFS_NS, false)) return;
  prefs.putString(PREFS_KEY, all);
  prefs.end();
  pendingMsgV3 = String("Tags salvas: ") + authorizedUIDs.size();
}

bool RFIDReader::uidExists(const String& uid) const {
  for (auto& u : authorizedUIDs) {
    if (u.equalsIgnoreCase(uid)) return true;
  }
  return false;
}

void RFIDReader::startEnroll(uint32_t windowMs) {
  enrollMode = true;
  enrollUntilMs = millis() + windowMs;
  Serial.println("[RFID] Modo cadastro ATIVO.");
  pendingMsgV3 = "Modo cadastro ATIVO. Aproxime a nova tag.";
}

void RFIDReader::cancelEnroll() {
  enrollMode = false;
  Serial.println("[RFID] Modo cadastro DESATIVADO.");
  h_latchedCancelEnrollEvent = true;
}

void RFIDReader::clearAllUIDs() {
  // cleans memory RAM
  authorizedUIDs.clear();

  // cleans persistence
  if (prefs.begin(PREFS_NS, false)) {
    prefs.putString(PREFS_KEY, ""); // nada salvo
    prefs.end();
  }

  // exit enroll mode if active
  enrollMode = false;
  h_latchedCancelEnrollEvent = true; // main turns off enroll mode

  // message for the app consumed main loop
  pendingMsgV3 = "Todas as tags foram APAGADAS.";
}

void RFIDReader::pool() {
  // expire enroll mode if time elapsed
  if (enrollMode && millis() > enrollUntilMs) {
    cancelEnroll();
  }

  if (!isCardPresent()) return;

  String uid = readCardUID();
  if (uid.isEmpty()) return;

  Serial.print("Card UID: ");
  Serial.println(uid);

  if (enrollMode) {
    if (uidExists(uid)) {
      Serial.println("Tag already registered.");
      pendingMsgV3 = "Tag já cadastrada: " + uid;
    } else {
      authorizedUIDs.push_back(uid);
      saveUIDs();
      Serial.println("New tag registered!");
      pendingMsgV3 = "Nova tag CADASTRADA: " + uid;
    }
    // ends time window after successful enrollment
    delay(1000); // wait a bit before canceling enrollment
    cancelEnroll();
    return;
  }

  // normal usage
  if (uidExists(uid)) {
    Serial.println("Authorized card detected. Unlocking door...");
    LockCtrl::cmdUnlock(millis());
    pendingMsgV3 = "Acesso liberado: " + uid;
  } else {
    Serial.println("Unauthorized card.");
    pendingMsgV3 = "Acesso NEGADO: " + uid;
  }
}

bool RFIDReader::isCardPresent() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial())   return false;
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

bool RFIDReader::RFID_takeCancelEnrollEvent() {
  bool latched = h_latchedCancelEnrollEvent;
  h_latchedCancelEnrollEvent = false;
  return latched;
}