#include <Blynk/BlynkParam.h>  // for BlynkParam
#pragma once
#include "lock_ctrl.hpp"

// Plain functions that handle writes (called from BLYNK_WRITE in main.cpp)
void onV1Write(const BlynkParam& param);
