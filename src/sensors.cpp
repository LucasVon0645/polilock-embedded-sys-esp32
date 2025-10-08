#include "sensors.hpp"

// Estado interno (arquivo-local)
namespace {
  uint8_t  g_pirPin = PIR_PIN;
  uint32_t g_timeoutMs = 5000;
  uint32_t g_stabilizeMs = 5000;

  bool     g_prev = false;           // estado anterior (LOW/HIGH)
  bool     g_windowActive = false;   // janela aberta após subida?
  bool     g_notified = false;       // já sinalizou TimedOut nessa janela?
  uint32_t g_windowStart = 0;        // início da janela (millis)
  uint32_t g_bootMs = 0;             // marca do boot para fase de estabilização

  bool     g_latchedTimedOut = false; // “latch” para o main consumir (uma vez)
}

// API
void PIR_begin(uint8_t pin, uint32_t timeout_ms, uint32_t stabilize_ms) {
  g_pirPin = pin;
  g_timeoutMs = timeout_ms;
  g_stabilizeMs = stabilize_ms;

  pinMode(g_pirPin, INPUT_PULLDOWN);   // a maioria dos pinos do ESP32 tem pulldown
  g_prev = digitalRead(g_pirPin);
  g_windowActive = false;
  g_notified = false;
  g_windowStart = 0;
  g_latchedTimedOut = false;
  g_bootMs = millis();                 // início do período de estabilização
}

void PIR_setTimeout(uint32_t timeout_ms) { g_timeoutMs = timeout_ms; }

bool PIR_isHigh() { return digitalRead(g_pirPin); }

PirEvent PIR_poll(uint32_t now_ms) {
  // Ignora leituras durante a estabilização do PIR após energizar
  if (now_ms - g_bootMs < g_stabilizeMs) {
    g_prev = digitalRead(g_pirPin);
    return PirEvent::None;
  }

  bool cur = digitalRead(g_pirPin);

  // Detecta borda de subida (LOW -> HIGH)
  if (!g_prev && cur) {
    g_windowActive = true;
    g_notified = false;
    g_windowStart = now_ms;
    g_prev = cur;
    return PirEvent::Rising;
  }

  // Janela ativa? checa timeout e/ou descida
  if (g_windowActive) {
    uint32_t elapsed = now_ms - g_windowStart;

    // Timeout atingido e sinal segue HIGH -> evento válido
    if (!g_notified && elapsed >= g_timeoutMs && cur) {
      g_notified = true;
      g_latchedTimedOut = true;   // o main consome via PIR_takeTimedOutEvent()
      // Mantém janela aberta até cair para LOW para só rearmar no próximo ciclo
      g_prev = cur;
      return PirEvent::TimedOut;
    }

    // Descida antes do timeout -> cancela janela sem evento
    if (!cur) {
      g_windowActive = false;
      g_notified = false;
      g_prev = cur;
      return PirEvent::Canceled;
    }
  }

  g_prev = cur;
  return PirEvent::None;
}

bool PIR_takeTimedOutEvent() {
  if (g_latchedTimedOut) {
    g_latchedTimedOut = false;
    return true;
  }
  return false;
}

// ===================== Hall sensor =====================

namespace {
  uint8_t  h_pin = HALL_PIN;
  uint16_t h_threshold = HALL_THRESHOLD;
  uint32_t h_confirmMs = HALL_CONFIRM_SECONDS * 1000UL;

  bool     h_prevAbove = false;     // estado anterior relativo ao limiar
  bool     h_waiting = false;       // aguardando confirmação?
  uint32_t h_tStart = 0;            // início da janela de confirmação
  int      h_lastRaw = 0;           // última leitura ADC

  bool     h_latchedConfirmed = false; // “latch” p/ main consumir

  inline bool elapsed(uint32_t start, uint32_t dt) {
    return (uint32_t)(millis() - start) >= dt; // seguro a overflow
  }
}

void HALL_begin(uint8_t pin, uint16_t threshold, uint32_t confirm_ms) {
  h_pin = pin;
  h_threshold = threshold;
  h_confirmMs = confirm_ms;

  pinMode(h_pin, INPUT);        // leitura analógica
  analogReadResolution(12);     // 0..4095 no ESP32

  h_lastRaw = analogRead(h_pin);
  h_prevAbove = (h_lastRaw >= h_threshold);
  h_waiting = false;
  h_tStart = 0;
  h_latchedConfirmed = false;
}

void HALL_setThreshold(uint16_t threshold) { h_threshold = threshold; }
void HALL_setConfirmMs(uint32_t confirm_ms) { h_confirmMs = confirm_ms; }

HallEvent HALL_poll(uint32_t now_ms) {
  (void)now_ms; // usamos millis() diretamente, mas mantemos assinatura simétrica ao PIR

  h_lastRaw = analogRead(h_pin);
  bool above = (h_lastRaw >= h_threshold);

  // borda de subida (<= limiar -> > limiar)
  if (!h_prevAbove && above && !h_waiting) {
    h_waiting = true;
    h_tStart = millis();
    h_prevAbove = above;
    return HallEvent::Rising;
  }

  if (h_waiting) {
    // se caiu antes do fim da confirmação, cancela
    if (!above) {
      h_waiting = false;
      h_prevAbove = above;
      return HallEvent::Canceled;
    }
    // se cumpriu janela e ainda está acima, confirma
    if (elapsed(h_tStart, h_confirmMs)) {
      h_waiting = false;
      h_latchedConfirmed = true; // main consome via HALL_takeConfirmedEvent()
      h_prevAbove = above;
      return HallEvent::Confirmed;
    }
  }

  h_prevAbove = above;
  return HallEvent::None;
}

bool HALL_takeConfirmedEvent() {
  bool latched = h_latchedConfirmed;
  h_latchedConfirmed = false;
  return latched;
}

int  HALL_lastRaw() { return h_lastRaw; }
bool HALL_isAboveThreshold() { return h_lastRaw >= h_threshold; }