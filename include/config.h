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

#define RFID_SS_PIN 5
#define RFID_RST_PIN 22

// === Timings ===
#define RFID_POLL_MS      100

// === Servo ===
#define SERVO_MIN_ANGLE     0
#define SERVO_MAX_ANGLE   180


// Sensor Hall Analógico
// Limiar (0–4095 no ADC de 12 bits do ESP32, por padrão)
#define HALL_THRESHOLD 2500
// Tempo de confirmação em segundos (após borda de subida)
#define HALL_CONFIRM_SECONDS 3
#define UNLOCK_FORGOT_MS 10000  // 10 segundos
#define OPEN_DEBOUNCE_LOCK_MS 200

#define V_UNLOCKED 1
#define V_LOCKED 0