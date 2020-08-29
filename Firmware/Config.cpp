/*
  Management of configuration for IO board
*/
#include "Config.h"
#include <EEPROM.h>
#include "CRC.h"

namespace Config {

EEPROM_CONFIG ConfigFile;

char* ChannelNames[] = {
  "Prof. ", 
  "Direc.",
  "Gaz   ",
  "Ailern",
  "Trim1 ",
  "Trim2 ",
  "Flap  ",
  "Train "
};

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
  ConfigFile.min_pulse_us = 800;
  ConfigFile.max_pulse_us = 2400;
  ConfigFile.NBchannels = 7;

  for(int i=0; i<MAX_CHANNELS; i++) {
    ConfigFile.channels[i].channel = i+1;
    ConfigFile.channels[i].options = 0;

    if (i<8) {
      strncpy(ConfigFile.channels[i].name, ChannelNames[i], 9);
    } else {
      char buf[20];
      sprintf(buf, "C%d", i+1);
      strncpy(ConfigFile.channels[i].name, buf, 9);
    }    
    
    ConfigFile.channels[i].rate = 2.0f;
    ConfigFile.channels[i].master_channel = 0;
    
    if (i<4) {
      ConfigFile.channels[i].min_mV = MIN_MANCHES_mV;
      ConfigFile.channels[i].max_mV = MAX_MANCHES_mV;
      ConfigFile.channels[i].min_us = 1000;
      ConfigFile.channels[i].max_us = 2200;
      ConfigFile.channels[i].trim_us = 0;
    } else {
      ConfigFile.channels[i].min_mV = MIN_AUX_mV;
      ConfigFile.channels[i].max_mV = MAX_AUX_mV;
      ConfigFile.channels[i].min_us = 1000;
      ConfigFile.channels[i].max_us = 2200;
      ConfigFile.channels[i].trim_us = 0;
    }
  }
}

}
