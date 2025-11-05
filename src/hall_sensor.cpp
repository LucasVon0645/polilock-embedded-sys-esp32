#include "hall_sensor.hpp"

namespace {
  uint8_t  h_pin = HALL_PIN;
  uint16_t h_threshold = HALL_THRESHOLD;
  uint32_t h_confirmMs = HALL_CONFIRM_SECONDS * 1000UL;

  bool     h_prevAbove = false;
  bool     h_waiting = false;
  uint32_t h_tStart = 0;
  int      h_lastRaw = 0;
  bool     h_latchedConfirmed = false;

  inline bool elapsed(uint32_t start, uint32_t dt) {
    return (uint32_t)(millis() - start) >= dt;
  }
}

namespace HallSensor {

void begin(uint8_t pin, uint16_t threshold, uint32_t confirm_ms) {
  h_pin = pin;
  h_threshold = threshold;
  h_confirmMs = confirm_ms;
  pinMode(h_pin, INPUT);
  analogReadResolution(12);
  h_lastRaw = analogRead(h_pin);
  h_prevAbove = (h_lastRaw >= h_threshold);
  h_waiting = false;
  h_tStart = 0;
  h_latchedConfirmed = false;
}

void setThreshold(uint16_t threshold) { h_threshold = threshold; }
void setConfirmMs(uint32_t confirm_ms) { h_confirmMs = confirm_ms; }

Event poll(uint32_t now_ms) {
  (void)now_ms;
  h_lastRaw = analogRead(h_pin);
  bool above = (h_lastRaw >= h_threshold);
  if (!h_prevAbove && above && !h_waiting) {
    h_waiting = true;
    h_tStart = millis();
    h_prevAbove = above;
    return Event::Rising;
  }
  if (h_waiting) {
    if (!above) {
      h_waiting = false;
      h_prevAbove = above;
      return Event::Canceled;
    }
    if (elapsed(h_tStart, h_confirmMs)) {
      h_waiting = false;
      h_latchedConfirmed = true;
      h_prevAbove = above;
      return Event::Confirmed;
    }
  }
  h_prevAbove = above;
  return Event::None;
}

bool takeConfirmedEvent() {
  bool latched = h_latchedConfirmed;
  h_latchedConfirmed = false;
  return latched;
}

int  lastRaw() { return h_lastRaw; }
bool isAboveThreshold() { return h_lastRaw >= h_threshold; }

} // namespace HallSensor
