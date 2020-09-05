#pragma once
// Arduino Framework
#include <Arduino.h>

#define VERSION "0.1"

#if defined(ARDUINO_AVR_MEGA2560)
#define MC2020_MEGA
#elif defined(ARDUINO_AVR_NANO)
#define MC2020_NANO
#else
#error "Unsupported board. Only Mega2560 or Nano 328P"
#endif

//#define TEST_MEM

// Use D10 for all
#define OUTPUT_PIN (10)

#if defined(MC2020_MEGA)

#define MAX_ADC (16)
#define MAX_CHANNELS (12)
#define NB_CHANNELS (6)
#define ADC_VBAT (15)

#elif defined(MC2020_NANO)

#define MAX_ADC (6)
#define MAX_CHANNELS (6)
#define NB_CHANNELS (4)
#define ADC_VBAT (5)

#else

#define MAX_CHANNELS (8)
#define NB_CHANNELS (8)

#endif

const float SCALE_mV_per_ADC = (5000.0f/1023.0f);
const float SCALE_ADC_per_mV = (1023.0f/5000.0f);

const int MIN_MANCHES_mV = 1800;
const int MAX_MANCHES_mV = 3200;

const int MIN_AUX_mV = 0000;
const int MAX_AUX_mV = 5000;

const int STEP_TUNING_mV = 50;
const int STEP_TUNING_us = 20;

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

#define CONFIG_CHANNEL_OPT_INVERTED (1<<0)
#define CONFIG_CHANNEL_OPT_COUPLING (1<<1)
#define CONFIG_CHANNEL_OPT_DUALRATE (1<<2)
#define CONFIG_CHANNEL_OPT_POWERLAW (1<<3)

#define CONFIG_CTL_SCREEN_PRESENT (1<<0)


// Non-volatile (eeprom) config, bytes field only
typedef struct {
  // Channel pulse index (1 based)
  byte channel;
  // Channel options: inverted, coupling, dual rate
  byte options;
  // For coupling
  byte master_channel;
  // Channel name (pleas restrict to 8 characters only)
  char name[8];
  // Minimum analog voltage
  int16_t min_mV;
  // Maximum analog voltage
  int16_t max_mV;
  // Trim offset for analog voltage
  int16_t trim_mV;
  // Minimum us pulse output
  int16_t min_us;
  // Maximum us pulse output
  int16_t max_us;
  // Trim offset for pulse timing
  int16_t trim_us;
  // For dual rate or coupling: scale factor
  float rate;
} CHANNEL_CONFIG;

// Non-volatile (eeprom) config, bytes field only
typedef struct {
  // CRC8, computed on all remaining fields below
  byte CRC8;

  // GENERAL CONFIG
  // Controller options (screen)
  byte options;

  // PPM PULSE MODULE
  // Frame duration (20ms, max 32)
  int16_t frame_length_us;
  // Interval duration (0.3ms)
  int16_t interval_us;
  // Minimum pulse duration
  int16_t min_pulse_us;
  // Maximum pulse duration
  int16_t max_pulse_us;
  // Number of channels (max is 16 for now)
  int16_t NBchannels;
  
  // CHANNELS
  // Each individual channel configuration
  CHANNEL_CONFIG channels[MAX_CHANNELS];
  
} EEPROM_CONFIG;

extern const char* ChannelNames[];
extern EEPROM_CONFIG ConfigFile;

int SaveConfigToEEPROM();
int LoadConfigFromEEPROM();
void ResetConfig();

}

int freeMemory();
