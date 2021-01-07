/*
  Management of configuration for IO board
*/
#include "Config.h"
#include <EEPROM.h>
#include "CRC.h"
#include "IO.h"

#define SLOT0_START_ADDR   (0)
#define SLOT1_START_ADDR   (300)
#define SLOT2_START_ADDR   (600)

namespace Config {

byte const g_VersionTag = 0x04;


int SaveConfigToEEPROM( int Slot )
{
   byte* pBlock;       // Pointer to data to be written
   int EE_Addr=0;      // EEPROM Address to be written
   byte crc8 = 0xFF;

   switch(Slot)
   {
      case 0:  EE_Addr=SLOT0_START_ADDR; break;
      case 1:  EE_Addr=SLOT1_START_ADDR; break;
      case 2:  EE_Addr=SLOT2_START_ADDR; break;
      default: EE_Addr=SLOT0_START_ADDR; break;
   }

   /* TODO Check size*/
  //if (EEPROM.length()<sizeof(Inputs_cfg_pst)) {
  //  return -1;
  //}

   // Write VersionTag and update crc
   EEPROM.update(EE_Addr++,g_VersionTag); // Version tag
   crc8 = CRC::crc8x(crc8, &g_VersionTag, 1);

   // Write Inputs_cfg and update crc
   pBlock = (byte*)Inputs_cfg_pst;
   for( int i = 0; i < (int) sizeof(Inputs_cfg_pst); i++ )
   {
      EEPROM.update(EE_Addr++, pBlock[i]);
   }
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Inputs_cfg_pst));

   // Write Mixers_pst and update crc
   pBlock = (byte*)Mixers_pst;
   for( int i = 0; i < (int) sizeof(Mixers_pst); i++ )
   {
      EEPROM.update(EE_Addr++, pBlock[i]);
   }
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Mixers_pst));

   // Write Servos_pst and update crc
   pBlock = (byte*)Servos_pst;
   for( int i = 0; i < (int) sizeof(Servos_pst); i++ )
   {
      EEPROM.update(EE_Addr++, pBlock[i]);
   }
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Servos_pst));

   // Write crc
   EEPROM.write(EE_Addr++, crc8);

   return 0;
}

int LoadConfigFromEEPROM( int Slot )
{
   byte *pBlock;       // Pointer
   int EE_Addr = 0;    // EEPROM Address to be read
   byte crc8 = 0xFF;

   // temporary variables to check crc (could be avoided if we read twice the EEPROM)
   byte l_VersionTag;
   input_cfg_tst Inputs_cfg_temp_pst[NB_INPUTS];
   mixers_tst Mixers_temp_pst[NB_MIXERS];
   servos_tst Servos_temp_pst[NB_SERVOS];

   switch(Slot)
   {
      case 0:  EE_Addr=SLOT0_START_ADDR; break;
      case 1:  EE_Addr=SLOT1_START_ADDR; break;
      case 2:  EE_Addr=SLOT2_START_ADDR; break;
      default: EE_Addr=SLOT0_START_ADDR; break;
   }


   /* TODO Check size*/
   //if (EEPROM.length()<sizeof(EEPROM_CONFIG)) {
   //  return -1;
   //}

   // Read version and update crc
   l_VersionTag = EEPROM.read(EE_Addr++);
   crc8 = CRC::crc8x(crc8, &l_VersionTag, 1);

   // Read Inputs_cfg and update crc
   pBlock = (byte*)Inputs_cfg_temp_pst;
   for( int i = 0; i < (int) sizeof(Inputs_cfg_temp_pst); i++ )
   {
      pBlock[i] = EEPROM.read(EE_Addr++);
   }
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Inputs_cfg_temp_pst) );

   // Read Mixers_temp_pst and update crc
   pBlock = (byte*)Mixers_temp_pst;
   for( int i = 0; i < (int) sizeof(Mixers_temp_pst); i++ )
   {
      pBlock[i] = EEPROM.read(EE_Addr++);
   }
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Mixers_temp_pst) );


   // Read Servos_temp_pst and update crc
   pBlock = (byte*)Servos_temp_pst;
   for( int i = 0; i < (int) sizeof(Servos_temp_pst); i++ )
   {
      pBlock[i] = EEPROM.read(EE_Addr++);
   }
   crc8 = CRC::crc8x(crc8, pBlock, sizeof(Servos_temp_pst) );


   if( l_VersionTag != g_VersionTag )  // If Wrong Version
   {
      return -3;
   }

   if( crc8 != EEPROM.read(EE_Addr++) )  // If Wrong CRC
   {
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
