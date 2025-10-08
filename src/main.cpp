#include "secrets.h" // It must be first import, it defines BLYNK_AUTH_TOKEN, WIFI_SSID and WIFI_PASS
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "servo_ctrl.hpp"
#include "sensors.hpp"

// ---------------------------------------------------------------------------
// BLYNK_WRITE(V0)
// This special function is a *callback* that runs automatically
// whenever the dashboard widget bound to V0 changes (e.g. user toggles a switch).
//
// - "param" holds the value sent by the Blynk Cloud
// - You can read it as int, float, string: param.asInt(), param.asFloat(), param.asStr()
// - Example here: a switch widget sends 0 (OFF) or 1 (ON)
//
// NOTE: You never call BLYNK_WRITE yourself — Blynk library does it for you.
// ---------------------------------------------------------------------------
BLYNK_WRITE(V0) {
  int state = param.asInt(); // 0 = OFF, 1 = ON
  if (state == 1) {
    Serial.println("🟢 System on");
  } else {
    Serial.println("🔴 System off");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  setupServo();
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  // PIR: pino do config.h, timeout = 5000ms, estabilização = 15000ms
  PIR_begin(PIR_PIN, 5000, 15000);
  // Hall: usa defaults de config.h, mas pode ajustar depois com setters
  HALL_begin(HALL_PIN, HALL_THRESHOLD, HALL_CONFIRM_SECONDS * 1000UL);
  Serial.println("Connecting to Blynk...");
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
  if (pir_event == PirEvent::Rising)   Serial.println("[PIR] rising: start time window");
  if (pir_event == PirEvent::Canceled) Serial.println("[PIR] falling before timeout: canceled");
  if (pir_event == PirEvent::TimedOut) Serial.println("[PIR] timeout with HIGH: valid event");

  // Send notification to Blynk for PIR TimedOut event
  if (PIR_takeTimedOutEvent()) {
    // Blynk IoT (novo): crie no dashboard o Event "presenca_prolongada"
    Blynk.logEvent("presenca_prolongada", "Presença manteve-se por mais que o limite.");
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

  if (HALL_takeConfirmedEvent()) {
    // Tranca a fechadura
    Serial.println("Trancamento Automático! 🔒");
  }

}

// Keep the BLYNK_WRITE macros in the same TU that includes BlynkSimpleEsp32.h
BLYNK_WRITE(V1) { onV1Write(param); }