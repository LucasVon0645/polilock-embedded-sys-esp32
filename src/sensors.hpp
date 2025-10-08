#pragma once
#include "config.h"
#include <Arduino.h>

// === PIR presence detector (janela por tempo) ===
enum class PirEvent {
  None,          // nada aconteceu
  Rising,        // borda de subida detectada (janela aberta)
  Canceled,      // borda de descida antes do timeout
  TimedOut       // timeout atingido enquanto PIR continuou HIGH (evento válido)
};

// Inicializa pino, timeout e período de estabilização após boot
void PIR_begin(uint8_t pin = PIR_PIN, uint32_t timeout_ms = 5000, uint32_t stabilize_ms = 15000);

// Opcional: alterar timeout depois
void PIR_setTimeout(uint32_t timeout_ms);

// Deve ser chamado periodicamente (não bloqueante)
PirEvent PIR_poll(uint32_t now_ms);

// Pega e limpa o “latch” de evento válido (TimedOut). Retorna true uma vez por ocorrência.
bool PIR_takeTimedOutEvent();

// Estado atual do pino (true = HIGH)
bool PIR_isHigh();

// === Hall sensor (janela de confirmação após borda de subida) ===
enum class HallEvent {
  None,          // nada aconteceu
  Rising,        // cruzou o limiar (borda de subida)
  Canceled,      // caiu abaixo do limiar antes do fim da confirmação
  Confirmed      // após X segundos ainda acima do limiar (evento válido)
};

// Inicializa: pino, limiar e tempo de confirmação (ms)
void HALL_begin(uint8_t pin = HALL_PIN,
                uint16_t threshold = HALL_THRESHOLD,
                uint32_t confirm_ms = HALL_CONFIRM_SECONDS * 1000UL);

// Ajustes opcionais
void HALL_setThreshold(uint16_t threshold);
void HALL_setConfirmMs(uint32_t confirm_ms);

// Deve ser chamado periodicamente (não bloqueante)
HallEvent HALL_poll(uint32_t now_ms);

// Lê e limpa o “latch” do evento Confirmed
bool HALL_takeConfirmedEvent();

// Utilitários
int  HALL_lastRaw();           // última leitura ADC
bool HALL_isAboveThreshold();  // acima do limiar?
