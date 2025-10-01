#pragma once
#include "config.h"
#include <Arduino.h>


/**
 * HallSensor (linear, analógico)
 * - Lê tensão no pino ADC e converte para campo magnético.
 * - Usa média de N amostras para reduzir ruído.
 * - Calibração: mede offset (Vcc/2 ~= sem campo) e armazena.
 */
class HallSensor {
public:
  // sens_mV_per_G: sensibilidade do sensor em mV/G a 3.3 V (SS49E ≈ 0.93 mV/G)
  explicit HallSensor(int adcPin = HALL_PIN, float vcc = 3.3f, float sens_mV_per_G = 0.93f);

  // Inicializa ADC (resolução e atenuação). N médias por leitura.
  bool begin(uint8_t resolutionBits = 12, adc_attenuation_t atten = ADC_11db, uint16_t samples = 64);

  // Define/obtém Vcc e sensibilidade
  void   setVcc(float v);                 // volts
  float  getVcc() const;
  void   setSensitivity(float mV_per_G);  // mV/G
  float  getSensitivity() const;

  // Leitura bruta (0..(2^res-1)) com média
  uint32_t readRaw() const;

  // Converte leitura média para tensão em mV
  float readMilliVolts() const;

  // Calibra offset (mV) assumindo ~zero campo (afaste ímãs)
  float calibrate(uint16_t samples = 256);

  // Define/obtém offset manualmente (mV)
  void  setZeroOffset_mV(float mv);
  float getZeroOffset_mV() const;

  // Campo magnético
  float readGauss() const;       // G
  float readMilliTesla() const;  // mT

  // Configura nº de amostras por leitura (média)
  void setSamples(uint16_t n);
  uint16_t getSamples() const;

private:
  int   _pin;
  float _vcc;               // volts
  float _sens_mV_per_G;     // mV/G
  float _zero_mV;           // offset (mV) ~ Vcc/2 ou calibrado
  uint8_t  _resBits;
  uint16_t _samples;
  adc_attenuation_t _atten;
  float rawToMilliVolts(uint32_t raw) const;
};
