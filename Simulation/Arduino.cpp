#include <conio.h>


#include "Arduino.h"
#include "U8x8lib.h"
SerialClass Serial;


uint16_t Simu_AdcRaw_Stick[4] = { 600, 512, 512, 512 };
uint16_t Simu_AdcRaw_Trim[4] =  { 512, 512, 512, 512 };


extern int16_t adc_mv[];
extern int16_t chan_mv[];
extern int16_t chan_ms[];

extern U8X8_SSD1327_MIDAS_128X128_HW_I2C u8x8;

uint16_t analogRead(uint8_t ch)
{
   switch( ch )
   {
      case 0:  return Simu_AdcRaw_Stick[0];
      case 1:  return Simu_AdcRaw_Stick[1];
      case 2:  return Simu_AdcRaw_Stick[2];
      case 3:  return Simu_AdcRaw_Stick[3];

      case 6:  return Simu_AdcRaw_Trim[0];
      case 7:  return Simu_AdcRaw_Trim[1];
      case 8:  return Simu_AdcRaw_Trim[2];
      case 9:  return Simu_AdcRaw_Trim[3];

      default: return 512;
   }
}



uint8_t Simu_Btn_Plus  = 0;
uint8_t Simu_Btn_Minus = 0;
uint8_t Simu_Btn_Next  = 0;
uint8_t Simu_Btn_Page  = 0;

uint8_t digitalRead(uint8_t ch)
{
   switch( ch )
   {
      case 2:  return Simu_Btn_Plus;
      case 3:  return Simu_Btn_Minus;
      case 4:  return Simu_Btn_Next;
      case 5:  return Simu_Btn_Page;
      default: return 1; // btn not pressed
   }
}

int Simulation_HookStart( void )
{
   char c = getch();
   printf("input command: '%c' (0x%02x)\n",c,c);
   switch( c )
   {
      case '+' : Simu_Btn_Plus=1;   break;
      case '-' : Simu_Btn_Minus=1;  break;
      case 'p' : Simu_Btn_Next=1;   break;
      case 'n' : Simu_Btn_Page=1;   break;
      case 0x1b: return 1;
      default:
      // release all btn
          Simu_Btn_Plus=0;
          Simu_Btn_Minus=0;
          Simu_Btn_Next=0;
          Simu_Btn_Page=0;
          break;
   }
   return 0;
}

void Simulation_HookEnd( void )
{
   printf(" adc: %04d %04d | chmv: %04d %04d | chus: %04d %04d \n",
          adc_mv[0],  adc_mv[6],
          chan_mv[0], chan_mv[6],
          chan_ms[0], chan_ms[6]);

   printf("%s\n",u8x8.lines[0]);
   printf("%s\n",u8x8.lines[1]);
   printf("%s\n",u8x8.lines[3]);
   printf("%s\n",u8x8.lines[5]);
   printf("%s\n",u8x8.lines[7]);
   printf("%s\n",u8x8.lines[9]);
   printf("%s\n",u8x8.lines[11]);
   printf("%s\n",u8x8.lines[13]);
   printf("%s\n",u8x8.lines[14]);
}
