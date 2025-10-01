#include "secrets.h" // It must be first import, it defines BLYNK_AUTH_TOKEN, WIFI_SSID and WIFI_PASS
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "servo_ctrl.hpp"

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
  Serial.println("Connecting to Blynk...");
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void loop() {
  Blynk.run();
}

// Keep the BLYNK_WRITE macros in the same TU that includes BlynkSimpleEsp32.h
BLYNK_WRITE(V1) { onV1Write(param); }