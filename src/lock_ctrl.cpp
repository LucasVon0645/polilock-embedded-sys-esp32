#include "lock_ctrl.hpp"

extern String pendingMsgV3;

namespace {
  // intern state
  LockCtrl::StateLock g_state = LockCtrl::StateLock::LOCKED;

  uint32_t g_unlockForgotMs = UNLOCK_FORGOT_MS;
  uint32_t g_openDebounceMs = OPEN_DEBOUNCE_LOCK_MS;
  uint32_t t_openStart = 0;

  bool h_latchedLockEvent = false;
  bool h_latchedUnlockEvent = false;
  bool h_latchedFailedLock = false;
  bool h_latchedOpenTooLong = false;

  bool trackingOpen = false;
  bool openMsgSent = false;

  uint32_t t_deadlineUnlockForgot = 0;
  uint32_t t_openDebounceStart = 0;
  bool inOpenDebounce = false;

  inline bool elapsedSince(uint32_t start, uint32_t dt) {
    return (uint32_t)(millis() - start) >= dt;
  }

  void goLocked() {
    ServoCtrl::lock();
    g_state = LockCtrl::StateLock::LOCKED;
    inOpenDebounce = false;
    trackingOpen = false;
    h_latchedLockEvent = true;
    Serial.println(F("[LOCK] -> LOCKED"));
  }

  void goUnlockedWaitOpen(uint32_t now_ms) {
    ServoCtrl::unlock();
    g_state = LockCtrl::StateLock::UNLOCKED_WAIT_OPEN;
    t_deadlineUnlockForgot = now_ms + g_unlockForgotMs;
    inOpenDebounce = false;
    trackingOpen = false;
    h_latchedUnlockEvent = true;
    Serial.println(F("[LOCK] -> UNLOCKED_WAIT_OPEN"));
  }

  void goUnlocked() {
    g_state = LockCtrl::StateLock::UNLOCKED;
    inOpenDebounce = false;
    Serial.println(F("[LOCK] -> UNLOCKED"));
  }
}

namespace LockCtrl {

void begin(uint32_t unlockForgotMs, uint32_t openDebounceMs) {
  g_unlockForgotMs = unlockForgotMs;
  g_openDebounceMs = openDebounceMs;

  if (HallSensor::isAboveThreshold()) {
    goLocked();
  } else {
    ServoCtrl::unlock();
    g_state = StateLock::UNLOCKED;
  }
}

void poll(uint32_t now_ms) {
  if (HallSensor::takeConfirmedEvent()) {
    if (g_state == StateLock::UNLOCKED || g_state == StateLock::UNLOCKED_WAIT_OPEN) {
      goLocked();
      return;
    }
  }

  if (g_state == StateLock::UNLOCKED_WAIT_OPEN) {
    bool magnet_close = HallSensor::isAboveThreshold();

    if (!inOpenDebounce) {
      if (!magnet_close) {
        inOpenDebounce = true;
        t_openDebounceStart = now_ms;
      }
    } else {
      if (!magnet_close && elapsedSince(t_openDebounceStart, g_openDebounceMs)) {
        goUnlocked();
        Serial.println(F("[LOCK] abertura confirmada"));
      }
      if (magnet_close) inOpenDebounce = false;
    }

    if ((int32_t)(t_deadlineUnlockForgot - now_ms) <= 0) {
      if (HallSensor::isAboveThreshold()) {
        goLocked();
        pendingMsgV3 = "Auto-trancamento: a porta não foi aberta após destrancar.";
      } else {
        goUnlocked();
      }
    }
  }

  if (g_state == StateLock::UNLOCKED) {
    bool magnet_close = HallSensor::isAboveThreshold();
    if (!magnet_close) {
      if (!trackingOpen) {
        trackingOpen = true;
        t_openStart = now_ms;
      } else if (elapsedSince(t_openStart, OPEN_TOO_LONG_MS) && !openMsgSent) {
        openMsgSent = true;
        if (!h_latchedOpenTooLong) {
          h_latchedOpenTooLong = true;
          pendingMsgV3 = "Alerta: a porta ficou aberta por mais de " + String(OPEN_TOO_LONG_MS / 1000) + " segundos.";
        }
      }
    } else {
      trackingOpen = false;
      openMsgSent = false;
    }
  }
}

void cmdUnlock(uint32_t now_ms) {
  if (g_state == StateLock::UNLOCKED) return;
  if (g_state == StateLock::UNLOCKED_WAIT_OPEN) {
    ServoCtrl::unlock();
    t_deadlineUnlockForgot = now_ms + g_unlockForgotMs;
    inOpenDebounce = false;
    return;
  }
  goUnlockedWaitOpen(now_ms);
}

void cmdLock() {
  if (g_state == StateLock::LOCKED) return;
  bool magnet_close = HallSensor::isAboveThreshold();

  if (magnet_close) {
    goLocked();
  } else {
    h_latchedFailedLock = true;
    pendingMsgV3 = "Falha ao trancar: a porta está aberta.";
  }
}

StateLock state() { return g_state; }

const __FlashStringHelper* stateName(StateLock s) {
  switch (s) {
    case StateLock::LOCKED:             return F("LOCKED");
    case StateLock::UNLOCKED_WAIT_OPEN: return F("UNLOCKED_WAIT_OPEN");
    case StateLock::UNLOCKED:           return F("UNLOCKED");
    default:                            return F("?");
  }
}

// === events ===
bool takeFailedLockEvent()  { bool v = h_latchedFailedLock;  h_latchedFailedLock = false;  return v; }
bool takeLockEvent()        { bool v = h_latchedLockEvent;   h_latchedLockEvent = false;   return v; }
bool takeUnlockEvent()      { bool v = h_latchedUnlockEvent; h_latchedUnlockEvent = false; return v; }
bool takeOpenTooLongEvent() { bool v = h_latchedOpenTooLong; h_latchedOpenTooLong = false; return v; }

} // namespace LockCtrl
