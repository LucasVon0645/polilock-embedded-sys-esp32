// lock_ctrl.cpp
#include "lock_ctrl.hpp"

namespace LockCtrl {

namespace {
  StateLock     g_state = StateLock::LOCKED;
  uint32_t  g_unlockForgotMs = UNLOCK_FORGOT_SECONDS * 1000UL;
  uint32_t  g_openDebounceMs = OPEN_DEBOUNCE_LOCK_MS;
  bool h_latchedFailedLock = false;

  // timers
  uint32_t  t_deadlineUnlockForgot = 0; // quando expira a janela Y
  uint32_t  t_openDebounceStart    = 0; // início do debounce após queda do Hall
  bool      inOpenDebounce         = false;

  inline bool elapsedSince(uint32_t start, uint32_t dt) {
    return (uint32_t)(millis() - start) >= dt; // safe overflow
  }

  void goLocked() {
    SERVO_lock();
    g_state = StateLock::LOCKED;
    inOpenDebounce = false;
    Serial.println(F("[LOCK] -> LOCKED"));
  }

  void goUnlockedWaitOpen(uint32_t now_ms) {
    SERVO_unlock();
    g_state = StateLock::UNLOCKED_WAIT_OPEN;
    t_deadlineUnlockForgot = now_ms + g_unlockForgotMs;
    inOpenDebounce = false;
    Serial.println(F("[LOCK] -> UNLOCKED_WAIT_OPEN (aguardando abertura)"));
  }

  void goUnlocked() {
    // já está destrancado; apenas troca o estado lógico
    g_state = StateLock::UNLOCKED;
    inOpenDebounce = false;
    Serial.println(F("[LOCK] -> UNLOCKED"));
  }
}

void begin(uint32_t unlockForgotMs, uint32_t openDebounceMs) {
  g_unlockForgotMs = unlockForgotMs;
  g_openDebounceMs = openDebounceMs;

  // Estado inicial a partir do Hall:
  // - se o ímã está presente (acima do threshold), assumimos porta fechada -> trancado
  // - se não, porta aberta -> destrancado
  if (HALL_isAboveThreshold()) {
    goLocked();
  } else {
    SERVO_unlock();
    g_state = StateLock::UNLOCKED;
  }
}

void poll(uint32_t now_ms) {
  // 1) evento de "porta voltou a fechar e ficou estável" (seu HALL_confirmed)
  if (HALL_takeConfirmedEvent()) {
    // Isso só interessa quando estamos destrancados:
    if (g_state == StateLock::UNLOCKED || g_state == StateLock::UNLOCKED_WAIT_OPEN) {
      // Trancamento automático quando a porta fechou de novo
      goLocked();
      // Serial.println(F("[LOCK] Trancamento automático após fechamento confirmado"));
      return;
    }
  }

  // 2) detectar início de abertura (queda do Hall) com debounce
  //    (somente enquanto aguardamos que a pessoa abra)
  if (g_state == StateLock::UNLOCKED_WAIT_OPEN) {
    bool magnet_close = HALL_isAboveThreshold();

    if (!inOpenDebounce) {
      // começamos a detectar abertura quando o sinal cai abaixo
      if (!magnet_close) {
        inOpenDebounce = true;
        t_openDebounceStart = now_ms;
        // Serial.println(F("[LOCK] queda do Hall: iniciou debounce de abertura"));
      }
    } else {
      // estamos no debounce; confirma "abriu" se continua abaixo por OPEN_DEBOUNCE_LOCK_MS
      if (!magnet_close && elapsedSince(t_openDebounceStart, g_openDebounceMs)) {
        goUnlocked(); // cancelamos a janela Y
        Serial.println(F("[LOCK] abertura confirmada: cancelado auto-relock Y"));
      }
      // se o sinal voltou a acima antes de vencer o debounce, zera o processo
      if (magnet_close) {
        inOpenDebounce = false;
      }
    }

    if ( (uint32_t)(now_ms - t_deadlineUnlockForgot) < (uint32_t)0x80000000UL ) {
      // not yet expired (comparação padrão), então só segue
    } else {
      // Expirou a janela
      if (HALL_isAboveThreshold()) {
        // Porta continuou fechada -> re-tranca
        goLocked();
        Serial.println(F("[LOCK] Auto-relock: destrancou e não abriu em Y segundos"));
      } else {
        // Porta já não está fechada; deixa destrancado
        goUnlocked();
      }
    }
  }
}

void cmdUnlock(uint32_t now_ms) {
  // Se já está destrancado, apenas rearmar janela Y quando estiver aguardando abertura
  if (g_state == StateLock::UNLOCKED) {
    // nada a fazer (mantém destrancado)
    return;
  }
  // Se já estava aguardando, apenas reinicia a janela Y
  if (g_state == StateLock::UNLOCKED_WAIT_OPEN) {
    SERVO_unlock(); // idempotente
    t_deadlineUnlockForgot = now_ms + g_unlockForgotMs;
    inOpenDebounce = false;
    // Serial.println(F("[LOCK] Re-armed Y (novo comando de destrancar)"));
    return;
  }
  // Se estava LOCKED
  goUnlockedWaitOpen(now_ms);
}

void cmdLock() {
  if (g_state == StateLock::LOCKED) return;

  bool magnet_close = HALL_isAboveThreshold();

  if (magnet_close) {
    goLocked();
  } else {
    h_latchedFailedLock = true;
  }
}

StateLock state() { return g_state; }

const __FlashStringHelper* stateName(StateLock s) {
  switch (s) {
    case StateLock::LOCKED:             return F("LOCKED");
    case StateLock::UNLOCKED_WAIT_OPEN: return F("UNLOCKED_WAIT_OPEN");
    case StateLock::UNLOCKED:           return F("UNLOCKED");
    default:                        return F("?");
  }
}

} // namespace LockCtrl

bool LOCK_takeFailedLockEvent() {
  bool latched = LockCtrl::h_latchedFailedLock;
  LockCtrl::h_latchedFailedLock = false;
  return latched;
}