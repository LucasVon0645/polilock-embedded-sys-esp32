#pragma once

// === Pins ===
#define RFID_SDA_PIN   5
#define RFID_SCK_PIN  18
#define RFID_MOSI_PIN 23
#define RFID_MISO_PIN 19
#define RFID_RST_PIN  22
#define SERVO_PIN     17
#define PIR_PIN       4
#define HALL_PIN      35

// === Timings ===
#define RFID_POLL_MS      100
#define SENSORS_POLL_MS    50
#define TELEMETRY_MS     1000
#define SERVO_UPDATE_MS    50
#define DEBOUNCE_MS       150
#define UNLOCK_SECONDS      5

// === Servo ===
#define SERVO_MIN_ANGLE     0
#define SERVO_MAX_ANGLE   180