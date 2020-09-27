#ifndef EEPROM_H
#define EEPROM_H

#include "Arduino.h" // for type

#define E2END 1023

struct EEPROMClass{

   uint8_t read( int idx )              { return 0; }
   void write( int idx, uint8_t val )   {  }
   void update( int idx, uint8_t val )  {  }
   uint16_t length()                    { return E2END + 1; }
};

extern EEPROMClass EEPROM;

#endif // EEPROM_H
