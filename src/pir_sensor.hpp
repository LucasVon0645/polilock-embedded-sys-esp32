#pragma once
#include <Arduino.h>
#include "config.h"

namespace PIRSensor {

enum class Event {
  None,
  Rising,
  Canceled,
  TimedOut
};

void begin(uint8_t pin = PIR_PIN,
           uint32_t timeout_ms = 5000,
           uint32_t stabilize_ms = 15000);

void setTimeout(uint32_t timeout_ms);
void setNotifyCooldown(uint32_t cooldown_ms);
Event poll(uint32_t now_ms);
bool takeTimedOutEvent();
bool isHigh();

} // namespace PIRSensor
