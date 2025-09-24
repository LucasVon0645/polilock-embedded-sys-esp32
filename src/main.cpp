#include "secrets.h" // It must be first import, it defines BLYNK_AUTH_TOKEN, WIFI_SSID and WIFI_PASS
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ---------------------------------------------------------------------------
// BLYNK_WRITE(V1)
// This special function is a *callback* that runs automatically
// whenever the dashboard widget bound to V1 changes (e.g. user toggles a switch).
//
// - "param" holds the value sent by the Blynk Cloud
// - You can read it as int, float, string: param.asInt(), param.asFloat(), param.asStr()
// - Example here: a switch widget sends 0 (OFF) or 1 (ON)
//
// NOTE: You never call BLYNK_WRITE yourself — Blynk library does it for you.
// ---------------------------------------------------------------------------
BLYNK_WRITE(V1) {
  int state = param.asInt(); // 0 = OFF, 1 = ON
  if (state == 1) {
    Serial.println("🔓 Unlock door!");
  } else {
    Serial.println("🔒 Lock door automatically!");
  }
}

BLYNK_WRITE(V0) {
  int state = param.asInt(); // 0 = OFF, 1 = ON
  if (state == 1) {
    Serial.println("🟢 System on");
  } else {
    Serial.println("🔴 System off");
  }
}

void setup() {
  Serial.begin(115200); // Start serial monitor for debugging
  delay(100);

  // Connect to Blynk Cloud using credentials from secrets.h
  // - BLYNK_AUTH_TOKEN identifies this *device* in your Blynk Console
  // - WIFI_SSID and WIFI_PASS connect to your Wi-Fi router
  //
  // While connecting, you’ll see debug logs in the Serial Monitor
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

  Serial.println("Connecting to Blynk...");
}

// ---------------------------------------------------------------------------
// loop()
// Runs continuously after setup()
// 
// IMPORTANT: You *must* call Blynk.run() here to:
//   - Keep Wi-Fi + Cloud connection alive
//   - Process incoming messages (like V1 updates)
//   - Trigger callbacks (like BLYNK_WRITE)
// ---------------------------------------------------------------------------
void loop() {
  Blynk.run(); // keeps the connection alive and processes commands
}
