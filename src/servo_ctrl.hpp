#pragma once
#include <ESP32Servo.h>
#include "config.h"

namespace ServoCtrl {

// Initializes the servo controller
void begin();

// Moves the servo to the locked position
void lock();

// Moves the servo to the unlocked position
void unlock();

// (Optional) provides a reference to the Servo object, if useful
Servo& instance();

} // namespace ServoCtrl
