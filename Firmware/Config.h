#pragma once
// Arduino Framework
#include <Arduino.h>

#define VERSION "0.1"

#ifdef ARDUINO_AVR_MEGA2560
#define MC2020_MEGA
#elif ARDUINO_AVR_NANO
#define MC2020_NANO
#else
#error "Unsupported board. Only Mega2560 or Nano 328P"
#endif
