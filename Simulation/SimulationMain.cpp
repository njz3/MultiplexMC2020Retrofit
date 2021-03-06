#include <stdio.h>
#include <math.h>

#include "Config.h"
#include "RC_PPMEncoder.h"
#include "Display.h"
#include "Resources.h"
#include "IO.h"
#include "GUI.h"

extern int Simulation_HookStart(void);
extern void Simulation_HookEnd(void);

void Task_Adc2Ppm(void)
{
   IO_InputsProcess();
   IO_MixersProcess();
   IO_ServosProcess();
}

#if 0 //Only for reference
void math_test(void)
{
   int16_t a = 10;
   int b = 100;
   float normalized = (float)a / (float)b;
   float corrected = pow(normalized, 1.5f);
   printf("%f",corrected);
}
#endif

int main()
{
   printf("Hello world!\n");

   Config::ResetConfig();
   Config::SaveConfigToEEPROM(0);

   for( int i=0 ; i<300 ; i++)
   {
      printf("loop %d\n",i);

      if( Simulation_HookStart() )
         break;

      Task_Adc2Ppm();

      IO_ReadButtons();
      ProcessGUI();

      Simulation_HookEnd();
   }

   printf("Fin!\n");

   return 0;
}
