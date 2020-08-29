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


#define OUTPUT_PIN (10)
#define MAX_CHANNELS (16)
#define NB_CHANNELS (7)


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

#define CONFIG_CHANNEL_INVERTED (1<<0)
#define CONFIG_CHANNEL_COUPLING (1<<1)
#define CONFIG_CHANNEL_DUALRATE (1<<2)

#define CONFIG_SCREEN_PRESENT (1<<0)


// Non-volatile (eeprom) config, bytes field only
typedef struct {
  // Channel pulse index (1 based)
  byte channel;
  // Channel options: inverted, coupling, dual rate
  byte options;
  // Channel name (pleas restrict to 8 characters only)
  char name[10];
  // Minimum voltage
  int min_mV;
  // Trim center
  int med_mV;
  // Maximum voltage
  int max_mV;
  // Minimum ms output
  int min_ms;
  // Maximum ms output
  int max_ms;
} CHANNEL_CONFIG;

// Non-volatile (eeprom) config, bytes field only
typedef struct {
  // CRC8, computed on all remaining fields below
  byte CRC8;

  // GENERAL CONFIG
  // enum baudrate
  byte serialSpeed;
  // Controller options1 (screen)
  byte options1;
  // Controlle options2 (display)
  byte options2;

  // PPM PULSE MODULE
  // Frame duration (20ms, max 32)
  int frame_length_us;
  // Interval duration (0.3ms)
  int interval_us;
  // Minimum pulse duration
  int min_pulse_us;
  // Maximum pulse duration
  int max_pulse_us;
  // Number of channels (max is 16 for now)
  byte NBchannels;
  
  // CHANNELS
  // Each individual channel configuration
  CHANNEL_CONFIG channels[MAX_CHANNELS];
  
} EEPROM_CONFIG;


extern EEPROM_CONFIG ConfigFile;

int SaveConfigToEEPROM();
int LoadConfigFromEEPROM();
void ResetConfig();

}
