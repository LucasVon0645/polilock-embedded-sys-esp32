#include "servo_ctrl.hpp"
#include <Arduino.h>

namespace {
  Servo doorServo; // instância privada
}

namespace ServoCtrl {

void begin() {
  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 900, 2100);
  doorServo.write(SERVO_MIN_ANGLE);
  Serial.println(F("[SERVO] inicializado"));
}

void lock() {
  doorServo.write(SERVO_MIN_ANGLE);
  Serial.println(F("[SERVO] lock() -> posição fechada"));
}

void unlock() {
  doorServo.write(SERVO_MAX_ANGLE);
  Serial.println(F("[SERVO] unlock() -> posição aberta"));
}

Servo& instance() {
  return doorServo;
}

} // namespace ServoCtrl
