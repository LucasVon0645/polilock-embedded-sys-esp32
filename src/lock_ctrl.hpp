// lock_ctrl.h
#pragma once
#include <Arduino.h>

// Dependências externas (já existem no seu projeto)
#include "sensors.hpp"     // HALL_isAboveThreshold(), HALL_takeConfirmedEvent()
#include "servo_ctrl.hpp"  // SERVO_lock(), SERVO_unlock()  (ajuste os nomes se forem diferentes)

#ifndef OPEN_DEBOUNCE_LOCK_MS        // evita falsos "abriu" por oscilação do hall
#define OPEN_DEBOUNCE_LOCK_MS 200
#endif

#ifndef UNLOCK_FORGOT_SECONDS  // tempo padrão para esquecer destrancamento automático
#define UNLOCK_FORGOT_SECONDS 10
#endif

namespace LockCtrl {

enum class StateLock {
  LOCKED,             // trancado
  UNLOCKED_WAIT_OPEN, // destrancado, aguardando a porta começar a abrir
  UNLOCKED            // destrancado e já houve gesto de abertura
};

void begin(uint32_t unlockForgotMs = UNLOCK_FORGOT_SECONDS * 1000UL,
           uint32_t openDebounceMs = OPEN_DEBOUNCE_LOCK_MS);

// chama no loop (não bloqueante)
void poll(uint32_t now_ms);

// comandos externos (Blynk/botão)
void cmdUnlock(uint32_t now_ms); // destrancar e iniciar janela Y
void cmdLock();                  // trancar imediato (manual/forçado)

// leitura do estado atual
StateLock state();

// opcional: helpers de string p/ logs/telemetria
const __FlashStringHelper* stateName(StateLock s);

} // namespace LockCtrl

bool LOCK_takeFailedLockEvent();

bool LOCK_takeLockEvent();

bool LOCK_takeUnlockEvent();