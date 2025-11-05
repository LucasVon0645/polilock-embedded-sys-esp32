#include "pir_sensor.hpp"

extern String pendingMsgV3;

namespace {
  uint8_t  g_pirPin = PIR_PIN;
  uint32_t g_timeoutMs = 30000;
  uint32_t g_stabilizeMs = 5000;

  bool     g_prev = false;
  bool     g_windowActive = false;
  uint32_t g_windowStart = 0;
  uint32_t g_bootMs = 0;

  uint32_t g_notifyCooldownMs = 5UL * 60UL * 1000UL;
  uint32_t g_lastNotifyMs     = 0;
  bool     g_latchedTimedOut  = false;
}

namespace PIRSensor {

void begin(uint8_t pin, uint32_t timeout_ms, uint32_t stabilize_ms) {
  g_pirPin = pin;
  g_timeoutMs = timeout_ms;
  g_stabilizeMs = stabilize_ms;
  pinMode(g_pirPin, INPUT_PULLDOWN);
  g_prev = digitalRead(g_pirPin);
  g_windowActive = false;
  g_windowStart = 0;
  g_latchedTimedOut = false; // latch for long presence event
  g_bootMs = millis();
  g_lastNotifyMs = 0;
}

void setTimeout(uint32_t timeout_ms) { g_timeoutMs = timeout_ms; }
void setNotifyCooldown(uint32_t cooldown_ms) { g_notifyCooldownMs = cooldown_ms; }
bool isHigh() { return digitalRead(g_pirPin); }

Event poll(uint32_t now_ms) {
  if (now_ms - g_bootMs < g_stabilizeMs) {
    g_prev = digitalRead(g_pirPin);
    return Event::None;
  }
  bool cur = digitalRead(g_pirPin);
  if (!g_prev && cur) {
    g_windowActive = true;
    g_windowStart = now_ms;
    g_prev = cur;
    return Event::Rising;
  }
  if (g_windowActive) {
    uint32_t elapsed = now_ms - g_windowStart;
    if (cur && elapsed >= g_timeoutMs) {
      if ((g_lastNotifyMs == 0) || (uint32_t)(now_ms - g_lastNotifyMs) >= g_notifyCooldownMs) {
        g_latchedTimedOut = true;
        g_lastNotifyMs = now_ms;
        g_prev = cur;
        pendingMsgV3 = "Alerta: presença detectada por mais de " + String(g_timeoutMs / 1000) + " segundos.";
        return Event::TimedOut;
      }
    }
    if (!cur) {
      g_windowActive = false;
      g_prev = cur;
      return Event::Canceled;
    }
  }
  g_prev = cur;
  return Event::None;
}

bool takeTimedOutEvent() {
  if (g_latchedTimedOut) {
    g_latchedTimedOut = false;
    return true;
  }
  return false;
}

} // namespace PIRSensor
