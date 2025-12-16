#pragma once

#define V_UNLOCKED 1
#define V_LOCKED 0

// === Pins ===
#define RFID_SDA_PIN   5
#define RFID_SCK_PIN  18
#define RFID_MOSI_PIN 23
#define RFID_MISO_PIN 19
#define RFID_RST_PIN  22
#define SERVO_PIN     17
#define PIR_PIN       4
#define HALL_PIN      35

#define RFID_SS_PIN 5
#define RFID_RST_PIN 22

// === Timings ===
// Confirmation time in seconds (after rising edge)
#define HALL_CONFIRM_SECONDS  3
#define UNLOCK_FORGOT_MS      10000  // 10 seconds
#define OPEN_DEBOUNCE_LOCK_MS 200
#define RFID_POLL_MS          100
#define OPEN_TOO_LONG_MS      30000   // 30 seconds
#define ENROLL_WINDOW_MS     15000   // 15 seconds

// === Servo ===
#define SERVO_MIN_ANGLE     0
#define SERVO_MAX_ANGLE    180

// === Hall Sensor ===
// Threshold (0–4095 in the 12-bit ADC of the ESP32, by default)
#define HALL_THRESHOLD 2100