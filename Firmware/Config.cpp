/*
  Management of configuration for IO board
*/
#include "Config.h"
#include <EEPROM.h>
#include "CRC.h"
#include "IO.h"

namespace Config {

EEPROM_CONFIG ConfigFile;


int SaveConfigToEEPROM()
{
   byte* pBlock;       // Pointer to data to be written
   int EE_Addr = 0;    // EEPROM Address to be written
   byte crc8 = 0xFF;

   /* TODO Check size*/
  //if (EEPROM.length()<sizeof(Inputs_cfg_pst)) {
  //  return -1;
  //}

   pBlock = (byte*)Inputs_cfg_pst;
   for( int i = 0; i < (int) sizeof(Inputs_cfg_pst); i++ )
   {
      EEPROM.write(EE_Addr++, pBlock[i]);
   }

   pBlock = (byte*)Mixers_pst;
   for( int i = 0; i < (int) sizeof(Mixers_pst); i++ )
   {
      EEPROM.write(EE_Addr++, pBlock[i]);
   }

   pBlock = (byte*)Servos_pst;
   for( int i = 0; i < (int) sizeof(Servos_pst); i++ )
   {
      EEPROM.write(EE_Addr++, pBlock[i]);
   }

   // compute and write Crc
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Inputs_cfg_pst));
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Mixers_pst));
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Servos_pst));
   EEPROM.write(EE_Addr++, crc8);

   return 0;
}

int LoadConfigFromEEPROM()
{
   byte *pBlock;       // Pointer
   int EE_Addr = 0;    // EEPROM Address to be read
   byte crc8 = 0xFF;

   // temporary variables to check crc (could be avoided if we read twice the EEPROM)
   input_cfg_tst Inputs_cfg_temp_pst[NB_INPUTS];
   mixers_tst Mixers_temp_pst[NB_MIXERS];
   servos_tst Servos_temp_pst[NB_SERVOS];


   /* TODO Check size*/
   //if (EEPROM.length()<sizeof(EEPROM_CONFIG)) {
   //  return -1;
   //}

   // Read new record from EEPROM
   pBlock = (byte*)Inputs_cfg_temp_pst;
   for( int i = 0; i < (int) sizeof(Inputs_cfg_temp_pst); i++ )
   {
      pBlock[i] = EEPROM.read(EE_Addr++);
   }

   pBlock = (byte*)Mixers_temp_pst;
   for( int i = 0; i < (int) sizeof(Mixers_temp_pst); i++ )
   {
      pBlock[i] = EEPROM.read(EE_Addr++);
   }

   pBlock = (byte*)Servos_temp_pst;
   for( int i = 0; i < (int) sizeof(Servos_temp_pst); i++ )
   {
      pBlock[i] = EEPROM.read(EE_Addr++);
   }


   // Compute CRC8 to detect wrong eeprom data
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Inputs_cfg_temp_pst) );
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Mixers_temp_pst) );
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Servos_temp_pst) );

   // Check CRC match?
   if( crc8 != EEPROM.read(EE_Addr++) )
   {
      // Wrong CRC
      return -2;
   }

   // Ok, store new config
   for( int i = 0; i < NB_INPUTS; i++ )
      Inputs_cfg_pst[i] = Inputs_cfg_temp_pst[i];

   for( int i = 0; i < NB_MIXERS; i++ )
      Mixers_pst[i] = Mixers_temp_pst[i];

   for( int i = 0; i < NB_SERVOS; i++ )
      Servos_pst[i] = Servos_temp_pst[i];

   return 0;
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
