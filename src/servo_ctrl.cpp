#include "servo_ctrl.hpp"
#include "config.h"
#include <Arduino.h>

// Define the global exactly once (NOT in main.cpp)
Servo doorServo;

void setupServo() {
  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 1000, 2000);
  doorServo.write(SERVO_MIN_ANGLE);
}

void onV1Write(const BlynkParam& param) {
  int state = param.asInt();
  int targetAngle = (state == 1) ? SERVO_MAX_ANGLE : SERVO_MIN_ANGLE;
  doorServo.write(targetAngle);
  if (state == 1) Serial.println("🔓 Unlock door!");
  else           Serial.println("🔒 Lock door automatically!");
}