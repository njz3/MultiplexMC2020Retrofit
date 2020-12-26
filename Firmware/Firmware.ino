// Arduino Mega
#include "Config.h"
#include "RC_PPMEncoder.h"
#include "Display.h"
#include "Resources.h"
#include "IO.h"
#include "GUI.h"





void setup() {
  
  Serial.begin(115200);
  while(!Serial) {  ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println(mBoot);
  
  Display.begin();
  Display.cleanup();
  
  Serial.println(mConfigureIOs);
  Display.println(mConfigureIOs);
  
  IO_InitPins();

  int resetPin = digitalRead(12);
  if (resetPin==0) {
    Serial.println(mResetPinOn);
    Display.println(mResetPinOn);
    Config::ResetConfig();
    Config::SaveConfigToEEPROM();
    delay(200);
  } else {
    Serial.println(mLoadConfig);
    Display.println(mLoadConfig);
    if (Config::LoadConfigFromEEPROM()<0) {
      Display.println(mWrongConfig);
      Config::ResetConfig();
      Config::SaveConfigToEEPROM();
    }
    delay(200);
  }
  
  Serial.println(mPPMStarted);
  Display.println(mPPMStarted);
  
  ppmEncoder.PPM_INTERVAL_LENGTH_us = Config::ConfigFile.interval_us;
  ppmEncoder.PPM_FRAME_LENGTH_us = Config::ConfigFile.frame_length_us;
  ppmEncoder.MIN_us = Config::ConfigFile.min_pulse_us;
  ppmEncoder.MAX_us = Config::ConfigFile.max_pulse_us;
  // Nombre de voies
  ppmEncoder.begin(OUTPUT_PIN, Config::ConfigFile.NBchannels);
  
  delay(200);
  Display.print(mNChanEq);
  Display.println(Config::ConfigFile.NBchannels);
  
  Display.print(mOutputPinEq);
  Display.println(OUTPUT_PIN);
  
  Display.println(mVersionEq);
  
    
  delay(1000);
  Display.cleanup();
  Display.setNormalFont();
}


void loop() {
   //Task_Adc2Ppm(); //re;oved from here: done in interuption context
   IO_ReadButtons();
   ProcessGUI();
}


/**
 * Read Adc, Process Mixage and calculate the servomechanism values (usec).
 * Normally executed every 20 ms.
 * Done once before to send the ppm sequence7
 */
void Task_Adc2Ppm(void)
{
   IO_InputsProcess();
   IO_MixersProcess();
   IO_ServosProcess();
}

