/*
  Management of configuration for IO board
*/
#include "Config.h"
#include <EEPROM.h>
#include "CRC.h"

namespace Config {

EEPROM_CONFIG ConfigFile;


int SaveConfigToEEPROM()
{
#if 0
  if (EEPROM.length()<sizeof(EEPROM_CONFIG)) {
    return -1;
  }
  // Pointer to record
  byte* pBlock = (byte*)&ConfigFile;
  // Compute CRC8 to detect wrong eeprom data
  byte crc8 = CRC::crc8x(0, pBlock+1, sizeof(EEPROM_CONFIG)-1);
  // Update CRC in record
  ConfigFile.CRC8 = crc8;
  // Write record to EEPROM
  for (int i = 0 ; i < (int)sizeof(EEPROM_CONFIG); i++) {
    EEPROM.write(i, pBlock[i]);
  }
#endif
  return 1;
}

int LoadConfigFromEEPROM()
{
#if 0
  if (EEPROM.length()<sizeof(EEPROM_CONFIG)) {
    return -1;
  }
  // Pointer to a new record on MCU stack
  EEPROM_CONFIG newCfg;
  byte* pBlock = (byte*)&newCfg;
  // Read new record from EEPROM
  for (int i = 0 ; i < (int)sizeof(EEPROM_CONFIG); i++) {
    pBlock[i] = EEPROM.read(i);
  }
  // Compute CRC8 to detect wrong eeprom data
  byte crc8 = CRC::crc8x(0, pBlock+1, sizeof(EEPROM_CONFIG)-1);
  // Check CRC match?
  if (crc8 != newCfg.CRC8) {
    // Wrong CRC
    return -2;
  }
  // Ok, store new config
  ConfigFile = newCfg;
#endif
  return 1;
}

void ResetConfig()
{
#if 0
  // CONTROLLER
  ConfigFile.options = 0;

  // PPM PULSE MODULE
  ConfigFile.frame_length_us = 20000;
  ConfigFile.interval_us = 300;
  ConfigFile.min_pulse_us = 800;
  ConfigFile.max_pulse_us = 2400;
  ConfigFile.NBchannels = NB_CHANNELS;

  for(int i=0; i<MAX_CHANNELS; i++) {
    ConfigFile.channels[i].channel = i+1;
    ConfigFile.channels[i].options = 0;

    if (i<9) {
      strncpy(ConfigFile.channels[i].name, ChannelNames[i], 9);
    } else {
      char buf[20];
      sprintf(buf, "C%2d   ", i+1);
      strncpy(ConfigFile.channels[i].name, buf, 9);
    }

    ConfigFile.channels[i].rate = 2.0f;
    ConfigFile.channels[i].master_channel = i;
    ConfigFile.channels[i].min_us = 1000;
    ConfigFile.channels[i].max_us = 2200;
    ConfigFile.channels[i].trim_us = 0;

    if (i<4) {
      ConfigFile.channels[i].min_mV = MIN_MANCHES_mV;
      ConfigFile.channels[i].max_mV = MAX_MANCHES_mV;
      ConfigFile.channels[i].trim_mV = ((MIN_MANCHES_mV + MAX_MANCHES_mV)>>1);
    } else if (i>=6 && i<=8) {
      ConfigFile.channels[i].min_mV = MIN_AUX_mV;
      ConfigFile.channels[i].max_mV = MAX_AUX_mV;
      ConfigFile.channels[i].trim_mV = ((MIN_AUX_mV + MAX_AUX_mV)>>1);
    } else {
      ConfigFile.channels[i].min_mV = MIN_AUX_mV;
      ConfigFile.channels[i].max_mV = MAX_AUX_mV;
      ConfigFile.channels[i].trim_mV = ((MIN_AUX_mV + MAX_AUX_mV)>>1);
    }
  }
#endif
}


}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
#ifdef SIMULATION_PC
  return 0;
#else
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
#endif // SIMULATION_PC
}
