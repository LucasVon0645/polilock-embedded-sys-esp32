#include "sensors.hpp"

HallSensor::HallSensor(int adcPin, float vcc, float sens_mV_per_G)
: _pin(adcPin),
  _vcc(vcc),
  _sens_mV_per_G(sens_mV_per_G),
  _zero_mV((vcc * 1000.0f) * 0.5f),
  _resBits(12),
  _samples(64),
  _atten(ADC_11db) {}

bool HallSensor::begin(uint8_t resolutionBits, adc_attenuation_t atten, uint16_t samples) {
  _resBits = resolutionBits;
  _atten   = atten;
  _samples = samples;

  // Configuração do ADC do Arduino-ESP32
  analogReadResolution(_resBits);
  analogSetPinAttenuation(_pin, _atten);

  // Primeira calibração rápida usando leitura atual como zero aproximado
  calibrate(256);
  return true;
}

void HallSensor::setVcc(float v) {
  _vcc = v;
  // Atualiza zero default para Vcc/2 caso o usuário esteja sem calibração
  if (_zero_mV == 0) _zero_mV = (v * 1000.0f) * 0.5f;
}
float HallSensor::getVcc() const { return _vcc; }

void HallSensor::setSensitivity(float mV_per_G) { _sens_mV_per_G = mV_per_G; }
float HallSensor::getSensitivity() const { return _sens_mV_per_G; }

void HallSensor::setSamples(uint16_t n) { _samples = n == 0 ? 1 : n; }
uint16_t HallSensor::getSamples() const { return _samples; }

uint32_t HallSensor::readRaw() const {
  uint32_t acc = 0;
  for (uint16_t i = 0; i < _samples; ++i) {
    acc += analogRead(_pin);
    delayMicroseconds(150); // um pequeno intervalo ajuda o ADC do ESP32
  }
  return acc / _samples;
}

float HallSensor::rawToMilliVolts(uint32_t raw) const {
  // Para ADC_11db no Arduino-ESP32, 3.3 V ≈ full-scale
  const float fullScale_mV = _vcc * 1000.0f;
  const float maxCount = (1UL << _resBits) - 1;
  return (raw * fullScale_mV) / maxCount;
}

float HallSensor::readMilliVolts() const {
  return rawToMilliVolts(readRaw());
}

float HallSensor::calibrate(uint16_t samples) {
  uint16_t old = _samples;
  _samples = samples;
  float mv = readMilliVolts();
  _zero_mV = mv; // assume zero campo no momento
  _samples = old;
  return _zero_mV;
}

void HallSensor::setZeroOffset_mV(float mv) { _zero_mV = mv; }
float HallSensor::getZeroOffset_mV() const { return _zero_mV; }

float HallSensor::readGauss() const {
  const float mv = readMilliVolts();
  const float delta_mV = mv - _zero_mV;           // mV acima/abaixo do zero
  return delta_mV / _sens_mV_per_G;               // G
}

float HallSensor::readMilliTesla() const {
  return readGauss() * 0.1f; // 1 mT = 10 G
}

// Estado interno (arquivo-local)
namespace {
  uint8_t  g_pirPin = PIR_PIN;
  uint32_t g_timeoutMs = 30000;  // timeout padrão 30s
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