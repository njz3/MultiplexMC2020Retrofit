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
  if (EEPROM.length()<sizeof(EEPROM_CONFIG)) {
    return -1;
  }
  // Pointer to record
  byte* pBlock = (byte*)&ConfigFile;  
  // Compute CRC8 to detect wrong eeprom data
  byte crc8 = CRC::crc8x_fast(0, pBlock+1, sizeof(EEPROM_CONFIG)-1);
  // Update CRC in record
  ConfigFile.CRC8 = crc8;
  // Write record to EEPROM
  for (int i = 0 ; i < (int)sizeof(EEPROM_CONFIG); i++) {
    EEPROM.write(i, pBlock[i]);
  }
  return 1;
}

int LoadConfigFromEEPROM()
{
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
  byte crc8 = CRC::crc8x_fast(0, pBlock+1, sizeof(EEPROM_CONFIG)-1);
  // Check CRC match?
  if (crc8 != newCfg.CRC8) {
    // Wrong CRC
    return -2;
  }
  // Ok, store new config
  ConfigFile = newCfg;
  return 1;
}

void ResetConfig()
{
  // CONTROLLER
  ConfigFile.serialSpeed = PCSERIAL_BAUDRATE;
  ConfigFile.options1 = 0;
  ConfigFile.options2 = 0;

  // PPM PULSE MODULE
  ConfigFile.frame_length_us = 20000;
  ConfigFile.interval_us = 300;
  ConfigFile.min_pulse_us = 1000;
  ConfigFile.max_pulse_us = 2200;
  ConfigFile.NBchannels = 7;

  for(int i=0; i<MAX_CHANNELS; i++) {
    ConfigFile.channels[i].channel = i+1;
    ConfigFile.channels[i].options = 0;
    strncpy(ConfigFile.channels[i].name, "", 9);
    ConfigFile.channels[i].min_mV = 0;
    ConfigFile.channels[i].med_mV = 2500;
    ConfigFile.channels[i].max_mV = 5000;
    ConfigFile.channels[i].min_ms = 1000;
    ConfigFile.channels[i].max_ms = 2200;
  }
}

}
