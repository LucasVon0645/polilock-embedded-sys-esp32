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
  Serial.println("Connecting to Blynk...");
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void loop() {
  Blynk.run();

  // Poll não-bloqueante (pode chamar em todo loop)
  PirEvent ev = PIR_poll(millis());

  // Opcional: logar eventos de diagnóstico
  if (ev == PirEvent::Rising)   Serial.println("[PIR] subida -> janela aberta");
  if (ev == PirEvent::Canceled) Serial.println("[PIR] descida antes do timeout -> cancelou");
  if (ev == PirEvent::TimedOut) Serial.println("[PIR] timeout com HIGH -> evento válido");

  // Disparo da notificação (uma vez por ocorrência)
  if (PIR_takeTimedOutEvent()) {
    // Blynk IoT (novo): crie no dashboard o Event "presenca_prolongada"
    Blynk.logEvent("presenca_prolongada", "Presença manteve-se por mais que o limite.");
    // Se estiver no Blynk Legacy: use Blynk.notify("...") no lugar do logEvent.
  }


}

// Keep the BLYNK_WRITE macros in the same TU that includes BlynkSimpleEsp32.h
BLYNK_WRITE(V1) { onV1Write(param); }