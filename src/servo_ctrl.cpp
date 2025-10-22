#include "servo_ctrl.hpp"
#include "config.h"
#include <Arduino.h>

// Define the global exactly once (NOT in main.cpp)
Servo doorServo;

void setupServo() {
  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 900, 2100);
  doorServo.write(SERVO_MIN_ANGLE);
}

void SERVO_lock() {
  doorServo.write(SERVO_MIN_ANGLE);
}

void SERVO_unlock() {
  doorServo.write(SERVO_MAX_ANGLE);
}