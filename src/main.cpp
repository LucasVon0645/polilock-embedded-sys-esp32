#include "secrets.h" // It must be first import, it defines BLYNK_AUTH_TOKEN, WIFI_SSID and WIFI_PASS
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "servo_ctrl.hpp"
#include "sensors.hpp"
#include "lock_ctrl.hpp"
#include "blynk.hpp"
#include "rfid.hpp"

RFIDReader rfid(RFID_SS_PIN, RFID_RST_PIN);

void setup() {
  Serial.begin(115200);
  delay(100);

  setupServo();

  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

  // PIR: pino do config.h, timeout = 5000ms, estabilização = 15000ms
  PIR_begin(PIR_PIN, 5000, 15000);

  // Hall: usa defaults de config.h, mas pode ajustar depois com setters
  HALL_begin(HALL_PIN, HALL_THRESHOLD, HALL_CONFIRM_SECONDS * 1000UL);

  // Lock controller
  LockCtrl::begin(
    UNLOCK_FORGOT_MS, // Y
    OPEN_DEBOUNCE_LOCK_MS                // debounce de abertura
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
  PirEvent pir_event = PIR_poll(millis());

  // Log PIR events (debug)
  // if (pir_event == PirEvent::Rising)   Serial.println("[PIR] rising: start time window");
  // if (pir_event == PirEvent::Canceled) Serial.println("[PIR] falling before timeout: canceled");
  // if (pir_event == PirEvent::TimedOut) Serial.println("[PIR] timeout with HIGH: valid event");

  // Send notification to Blynk for PIR TimedOut event
  if (PIR_takeTimedOutEvent()) {
    Serial.println("[PIR] TimedOut event taken: presence confirmed.");
    Blynk.logEvent("presenca_prolongada", "Presença manteve-se por mais que o limite.");
  }

  if (LOCK_takeLockEvent()) {
    Blynk.virtualWrite(V1, V_LOCKED); // Update V1 to locked
  }
  if (LOCK_takeUnlockEvent()) {
    Blynk.virtualWrite(V1, V_UNLOCKED); // Update V1 to unlocked
  }

  if (LOCK_takeFailedLockEvent()) {
    Blynk.logEvent("trancamento_falho", "Porta está aberta. Trancamento não foi possível.");
    Blynk.virtualWrite(V1, V_UNLOCKED); // Update V1 back to unlocked
  }

  if (LOCK_takeOpenTooLongEvent()) {
    // Create an Event in Blynk (e.g., "porta_aberta_longo_tempo") in your template
    String msg = String("A porta ficou aberta por mais de ") + (OPEN_TOO_LONG_MS / 1000) + " segundos.";
    Blynk.logEvent("porta_aberta_longo_tempo", msg.c_str());
  }

  // --- Hall ---
  // Non-blocking polling of Hall sensor
  HallEvent hall_event = HALL_poll(millis());

  // Log Hall events (debug)
  if (hall_event == HallEvent::Rising) Serial.println("[Hall] rising: start time window.");
  if (hall_event == HallEvent::Canceled) Serial.println("[HALL] falling before timeout: canceled.");
  if (hall_event == HallEvent::Confirmed) {
    Serial.print("[HALL] Confirmed: after ");
    Serial.print(HALL_CONFIRM_SECONDS);
    Serial.print("s. raw=");
    Serial.print(HALL_lastRaw());
    Serial.print(" (threshold=");
    Serial.print(HALL_THRESHOLD);
    Serial.println(")");
  }

  LockCtrl::poll(millis());

  // (opcional) telemetria leve
  // static uint32_t lastPrint = 0;
  // if (millis() - lastPrint >= 500) {
  //   lastPrint = millis();
  //   Serial.print("HALL raw=");
  //   Serial.print(HALL_lastRaw());
  //   Serial.print(" above=");
  //   Serial.print(HALL_isAboveThreshold() ? "YES" : "no");
  //   Serial.print(" | PIR=");
  //   Serial.println(PIR_isHigh() ? "HIGH" : "low");
  // }

  rfid.pool();

}

// Keep the BLYNK_WRITE macros in the same TU that includes BlynkSimpleEsp32.h
BLYNK_WRITE(V1) { onV1Write(param); }

// ---- NOVO: botão para cadastro em V2 ----
BLYNK_WRITE(V2) {
  int state = param.asInt(); // 1=ligado, 0=desligado
  if (state == 1) {
    rfid.startEnroll(15000); // janela de 15s (ajuste à vontade)
  } else {
    rfid.cancelEnroll();
  }
}

BLYNK_CONNECTED() {
  // Pull the last V1 value from the server; this will call BLYNK_WRITE(V1)
  Blynk.syncVirtual(V1);
}