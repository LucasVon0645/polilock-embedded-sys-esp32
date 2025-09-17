#pragma once

// === Pins ===
#define RFID_SS_PIN   5
#define RFID_RST_PIN  22
#define SERVO_PIN     17
#define PIR_PIN       27
#define HALL_PIN_D    26     // digital Hall
#define HALL_PIN_ADC  34     // analog Hall (ADC1)

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