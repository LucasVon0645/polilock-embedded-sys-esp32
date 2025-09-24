#include "secrets.h" // It must be first import, it defines BLYNK_AUTH_TOKEN, WIFI_SSID and WIFI_PASS
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

Servo doorServo;

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

  int targetAngle = (state == 1) ? SERVO_MAX_ANGLE : SERVO_MIN_ANGLE;

  doorServo.write(targetAngle);

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
  Serial.begin(115200);
  delay(100);

  // Configura servo: 50 Hz e faixa de pulso típica
  doorServo.setPeriodHertz(50);       // servos padrão usam ~50 Hz
  doorServo.attach(SERVO_PIN, 1000, 2000); // 1000–2000 µs cobre 0–180° no S3003


  // Posição inicial
  doorServo.write(SERVO_MIN_ANGLE);

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting to Blynk...");
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void loop() {
  Blynk.run();
}
