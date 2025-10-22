#pragma once
#include <ESP32Servo.h>

// Declare the servo for other files
extern Servo doorServo;

// Init/utility
void setupServo();

void SERVO_lock();
void SERVO_unlock();