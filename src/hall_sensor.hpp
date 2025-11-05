#pragma once
#include <Arduino.h>
#include "config.h"

namespace HallSensor {

enum class Event {
  None,
  Rising,
  Canceled,
  Confirmed
};

void begin(uint8_t pin = HALL_PIN,
           uint16_t threshold = HALL_THRESHOLD,
           uint32_t confirm_ms = HALL_CONFIRM_SECONDS * 1000UL);

void setThreshold(uint16_t threshold);
void setConfirmMs(uint32_t confirm_ms);
Event poll(uint32_t now_ms);
bool takeConfirmedEvent();
int  lastRaw();
bool isAboveThreshold();

} // namespace HallSensor
