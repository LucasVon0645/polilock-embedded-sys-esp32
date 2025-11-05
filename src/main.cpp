#include "secrets.h" // It must be first import, it defines BLYNK_AUTH_TOKEN, WIFI_SSID and WIFI_PASS
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "servo_ctrl.hpp"
#include "pir_sensor.hpp"
#include "hall_sensor.hpp"
#include "lock_ctrl.hpp"
#include "blynk.hpp"
#include "rfid.hpp"

RFIDReader rfid(RFID_SS_PIN, RFID_RST_PIN);

String pendingMsgV3 = "";

void setup() {
  Serial.begin(115200);
  delay(100);

  ServoCtrl::begin();

  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

  // Correct initial states on startup
  Blynk.virtualWrite(V2, 0); // Update RFID to read mode

  PIRSensor::begin(PIR_PIN, 5000, 15000);

  HallSensor::begin(HALL_PIN, HALL_THRESHOLD, HALL_CONFIRM_SECONDS * 1000UL);

  // Lock controller
  LockCtrl::begin(
    UNLOCK_FORGOT_MS, // time to auto-lock if door not opened
    OPEN_DEBOUNCE_LOCK_MS // debounce time for door open detection
  );

  Serial.print(F("[BOOT] Estado inicial: "));
  Serial.println(LockCtrl::stateName(LockCtrl::state()));

  rfid.begin();
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void loop() {
  Blynk.run();

  // --- PIR ---
  // Non-blocking polling of PIR
  PIRSensor::poll(millis());

  // Send notification to Blynk for PIR TimedOut event
  if (PIRSensor::takeTimedOutEvent()) {
    Serial.println("[PIR] TimedOut event taken: presence confirmed.");
    Blynk.logEvent("presenca_prolongada", "Presença manteve-se por mais que o limite.");
  }

  if (LockCtrl::takeLockEvent()) {
    Blynk.virtualWrite(V1, V_LOCKED); // Update V1 to locked
  }
  if (LockCtrl::takeUnlockEvent()) {
    Blynk.virtualWrite(V1, V_UNLOCKED); // Update V1 to unlocked
  }

  if (LockCtrl::takeFailedLockEvent()) {
    Blynk.logEvent("trancamento_falho", "Porta está aberta. Trancamento não foi possível.");
    Blynk.virtualWrite(V1, V_UNLOCKED); // Update V1 back to unlocked
  }

  if (LockCtrl::takeOpenTooLongEvent()) {
    // Create an Event in Blynk (e.g., "porta_aberta_longo_tempo")
    String msg = String("A porta ficou aberta por mais de ") + (OPEN_TOO_LONG_MS / 1000) + " segundos.";
    Blynk.logEvent("porta_aberta_longo_tempo", msg.c_str());
  }

  if (rfid.takeCancelEnrollEvent()) {
    Blynk.virtualWrite(V2, 0); // Update V2 button to off
  }

  if (pendingMsgV3.length() > 0) {
    Blynk.virtualWrite(V3, pendingMsgV3);
    pendingMsgV3 = ""; // clear after sending
  }

  // --- Hall ---
  // Non-blocking polling of Hall sensor
  HallSensor::Event hall_event = HallSensor::poll(millis());

  // Log Hall events (debug)
  if (hall_event == HallSensor::Event::Rising) Serial.println("[Hall] rising: start time window.");
  if (hall_event == HallSensor::Event::Canceled) Serial.println("[HALL] falling before timeout: canceled.");
  if (hall_event == HallSensor::Event::Confirmed) {
    Serial.print("[HALL] Confirmed: after ");
    Serial.print(HALL_CONFIRM_SECONDS);
    Serial.print("s. raw=");
    Serial.print(HallSensor::lastRaw());
    Serial.print(" (threshold=");
    Serial.print(HALL_THRESHOLD);
    Serial.println(")");
  }

  LockCtrl::poll(millis());

  rfid.pool();

}

// Keep the BLYNK_WRITE macros in the same TU that includes BlynkSimpleEsp32.h

// button for lock/unlock in V1
BLYNK_WRITE(V1) { onV1Write(param); }

// button for enrollment in V2
BLYNK_WRITE(V2) { onV2Write(param, rfid); }

// V3 is just for status messages from RFID logic; no BLYNK_WRITE needed

// button to clear all UID
BLYNK_WRITE(V4) { onV4Write(param, rfid); }

BLYNK_CONNECTED() {
  // Pull the last V1 value from the server; this will call BLYNK_WRITE(V1)
  Blynk.syncVirtual(V1);
}