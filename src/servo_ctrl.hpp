#pragma once
#include <ESP32Servo.h>
#include "config.h"

namespace ServoCtrl {

// Inicializa o servo motor
void begin();

// Movimenta o servo para posição de travado
void lock();

// Movimenta o servo para posição de destravado
void unlock();

// (Opcional) fornece referência para o objeto Servo, se for útil
Servo& instance();

} // namespace ServoCtrl
