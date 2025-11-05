#pragma once
#include <Arduino.h>
#include "hall_sensor.hpp"
#include "servo_ctrl.hpp"

#ifndef OPEN_DEBOUNCE_LOCK_MS
#define OPEN_DEBOUNCE_LOCK_MS 200
#endif

#ifndef UNLOCK_FORGOT_MS
#define UNLOCK_FORGOT_MS 10000
#endif

namespace LockCtrl {

enum class StateLock {
  LOCKED,
  UNLOCKED_WAIT_OPEN,
  UNLOCKED
};

void begin(uint32_t unlockForgotMs = UNLOCK_FORGOT_MS,
           uint32_t openDebounceMs = OPEN_DEBOUNCE_LOCK_MS);

void poll(uint32_t now_ms);

void cmdUnlock(uint32_t now_ms);
void cmdLock();

StateLock state();
const __FlashStringHelper* stateName(StateLock s);

// === events ===
bool takeFailedLockEvent();
bool takeLockEvent();
bool takeUnlockEvent();
bool takeOpenTooLongEvent();

} // namespace LockCtrl
