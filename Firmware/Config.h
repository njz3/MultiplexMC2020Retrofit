#pragma once
// Arduino Framework
#include <Arduino.h>

#define VERSION "1.1"

#if defined(ARDUINO_AVR_MEGA2560)
#define MC2020_MEGA
//#elif defined(ARDUINO_AVR_NANO)
//#define MC2020_NANO
#else
#error "Unsupported board. Only Mega2560"
#endif

//#define TEST_MEM
//#define HEADER_FOOTER

// Use D10 for all
#define OUTPUT_PIN (10)

const float SCALE_mV_per_ADC = (5000.0f/1023.0f);
const float SCALE_ADC_per_mV = (1023.0f/5000.0f);

#define gd_MIN_mV                (0)
#define gd_MAX_mV                (5000)

#define gd_STEP_TUNING_Up_mV     (50)
#define gd_STEP_TUNING_Down_mV   (5)

#define gd_MIN_us                (800)
#define gd_MAX_us                (2200)

#define gd_STEP_TUNING_Up_us     (100)
#define gd_STEP_TUNING_Down_us   (10)

namespace Config {

enum COMSPEED {
  COM57600 = 0,
  COM115200 = 1,
  COM250000 = 2,
  COM500000 = 3,
  COM1000000 = 4,
};

// Fastest RS232 com (Leonard, Mega2560, Due)
// - 115200 is the standard hihg speed baudrate, but the
//   Mega2560@16Mhz has some timing issues (2-3% frames errors)
//   see here: http://ruemohr.org/~ircjunk/avr/baudcalc/avrbaudcalc-1.0.8.php?postbitrate=&postclock=16
// - 250000, 5000000 or 1000000 is more stable on the Mega2560
//   and other native USB like Leonardo or Due have no issues
//   whatever speed is choosen.
// => Take maximum speed 1000000 to reduce transmission delays
// Note: USB based com (Leonardo, Due) can go up to 2000000 (2Mbps)
#define PCSERIAL_BAUDRATE (COM1000000)

int SaveConfigToEEPROM( int Slot );
int LoadConfigFromEEPROM( int Slot );
void ResetConfig();

}

int freeMemory();
