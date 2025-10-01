#pragma once
#include <ESP32Servo.h>
#include <Blynk/BlynkParam.h>  // for BlynkParam

// Declare the servo for other files
extern Servo doorServo;

// Init/utility
void setupServo();

// Plain functions that handle writes (called from BLYNK_WRITE in main.cpp)
void onV1Write(const BlynkParam& param);
