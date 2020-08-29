// Arduino Mega
#include "Config.h"
#include "RC_PPMEncoder.h"
#include "Display.h"

// Use D10 on mega 2560
#define OUTPUT_PIN (10)
#define NB_CHANNELS (7)

void setup() {
  
  Serial.begin(1000000);
  while(!Serial) {  ; // wait for serial port to connect. Needed for native USB
  }
  
  Display.begin();
  Display.cleanup();
  delay(200);
  
  Serial.println("Configuring IOs...");
  Display.println("Configuring IOs");
  
  // Set all IOs in INPUT mode to avoid burning a pin with a wrong wiring
#ifdef MC2020_MEGA
  for(int i=0; i<53; i++) {
#else
  for(int i=0; i<12; i++) {
#endif
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
#ifdef MC2020_MEGA
  pinMode(A8, INPUT);
  pinMode(A9, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);
  pinMode(A12, INPUT);
  pinMode(A13, INPUT);
  pinMode(A14, INPUT);
  pinMode(A15, INPUT);
  pinMode(12, INPUT_PULLUP);
#endif

  delay(200);
  int resetPin = digitalRead(12);
  if (resetPin==0) {
    Serial.println("D12 on! Reset");
    Display.println("D12 on! Reset");
    Config::ResetConfig();
    Config::SaveConfigToEEPROM();
    delay(2000);
  } else {
    Serial.println("Loading config");
    Display.println("Loading config");
    Config::LoadConfigFromEEPROM();
    delay(200);
  }
  
  Serial.println("Starting PPM...");
  Display.println("Starting PPM");
  
  ppmEncoder.PPM_INTERVAL_LENGTH_us = Config::ConfigFile.frame_length_us;
  ppmEncoder.PPM_FRAME_LENGTH_us = Config::ConfigFile.interval_us;
  ppmEncoder.MIN_us = Config::ConfigFile.min_pulse_us;
  ppmEncoder.MAX_us = Config::ConfigFile.max_pulse_us;
  // Nombre de voies
  ppmEncoder.begin(OUTPUT_PIN, Config::ConfigFile.NBchannels);
  
  delay(200);
  Display.println("#chan=" + String(Config::ConfigFile.NBchannels));
  Display.println("Output=D" + String(OUTPUT_PIN));
  Display.println("Version=" VERSION);
  
    
  delay(2000);
  Display.cleanup();
  Display.setNormalFont();
}



enum BUTTONS_PRESSED : uint16_t
{
  BTN_PLUS  = (1<<0),
  BTN_MINUS = (1<<1),
  BTN_NEXT  = (1<<2),
  BTN_OK    = (1<<3),
  BTN_MODE  = (1<<4),
  BTN_OPT1  = (1<<5),
  BTN_OPT2  = (1<<6),
};

uint16_t currbuttons = 0;
uint16_t lastbuttons = 0;
  
uint16_t ReadButtons()
{
  uint16_t buttons = 0;
  
  if (digitalRead(2)==0) buttons |= BUTTONS_PRESSED::BTN_PLUS;
  if (digitalRead(3)==0) buttons |= BUTTONS_PRESSED::BTN_MINUS;
  if (digitalRead(4)==0) buttons |= BUTTONS_PRESSED::BTN_NEXT;
  if (digitalRead(5)==0) buttons |= BUTTONS_PRESSED::BTN_OK;
  if (digitalRead(6)==0) buttons |= BUTTONS_PRESSED::BTN_MODE;
  if (digitalRead(7)==0) buttons |= BUTTONS_PRESSED::BTN_OPT1;
  if (digitalRead(8)==0) buttons |= BUTTONS_PRESSED::BTN_OPT2;
  
  Serial.println("Buttons=" + String(buttons));
  
  return buttons;
}

int adc_vals[16];
int adc_mv[16];
int chan_mv[16];
int chanctr_mv[16];
int chan_ms[16];
int nb_adc = 0;

void ReadValues() {
  Serial.println("Reading ");
  nb_adc = 0;
  adc_vals[nb_adc++] = analogRead(A0);
  adc_vals[nb_adc++] = analogRead(A1);
  adc_vals[nb_adc++] = analogRead(A2);
  adc_vals[nb_adc++] = analogRead(A3);
#ifndef MC2020_NANO
  adc_vals[nb_adc++] = analogRead(A4);
  adc_vals[nb_adc++] = analogRead(A5);
#endif
  adc_vals[nb_adc++] = analogRead(A6);
  adc_vals[nb_adc++] = analogRead(A7);
#ifdef MC2020_MEGA
  adc_vals[nb_adc++] = analogRead(A8);
  adc_vals[nb_adc++] = analogRead(A9);
  adc_vals[nb_adc++] = analogRead(A10);
  adc_vals[nb_adc++] = analogRead(A11);
  adc_vals[nb_adc++] = analogRead(A12);
  adc_vals[nb_adc++] = analogRead(A13);
  adc_vals[nb_adc++] = analogRead(A14);
  adc_vals[nb_adc++] = analogRead(A15);
#endif
  Serial.println("Read " + String(nb_adc));
  
  for(int i=0; i<nb_adc; i++) {
    adc_mv[i] = (int)((float)adc_vals[i]*SCALE_mV_per_ADC);
    chan_mv[i] = constrain(adc_mv[i], Config::ConfigFile.channels[i].min_mV, Config::ConfigFile.channels[i].max_mV);

    /*
    // Power law ? Amplify small difference to center
    if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_POWERLAW)!=0) {
      float range = (float)(Config::ConfigFile.channels[i].max_mV - Config::ConfigFile.channels[i].min_mV);
      float normalized = (float)(chanctr_mv[i] / range);
      float scaled = pow(normalized, Config::ConfigFile.channels[idx].rate)* 2500.0f;
    }
    */
    int min_us = Config::ConfigFile.channels[i].min_us;
    int max_us = Config::ConfigFile.channels[i].max_us;
    
    // Dual rate ? Restrict to a smaller range
    if ((Config::ConfigFile.channels[i].options & CONFIG_CHANNEL_OPT_DUALRATE)!=0) {
      int med = (min_us + max_us)>>1;
      int half_range = (max_us - min_us)>>2;
      min_us = med - half_range;
      max_us = med + half_range;
    }

    // Power law ? Amplify small difference to center
    if ((Config::ConfigFile.channels[i].options & CONFIG_CHANNEL_OPT_DUALRATE)!=0) {
      int med = (min_us + max_us)>>1;
      int half_range = (max_us - min_us)>>2;
      min_us = med - half_range;
      max_us = med + half_range;
    }

    // Inverted ? Invert min and max
    if ((Config::ConfigFile.channels[i].options & CONFIG_CHANNEL_OPT_INVERTED)!=0) {
      min_us = Config::ConfigFile.channels[i].max_us;
      max_us = Config::ConfigFile.channels[i].min_us;
    }
    chan_ms[i] = map(chan_mv[i], Config::ConfigFile.channels[i].min_mV, Config::ConfigFile.channels[i].max_mV, Config::ConfigFile.channels[i].min_us, Config::ConfigFile.channels[i].max_us);
    // Add trim offset
    chan_ms[i] += Config::ConfigFile.channels[i].trim_us;
    if (chan_ms[i]<Config::ConfigFile.channels[i].min_us)
      chan_ms[i] = Config::ConfigFile.channels[i].min_us;
    if (chan_ms[i]>Config::ConfigFile.channels[i].max_us)
      chan_ms[i] = Config::ConfigFile.channels[i].max_us;
    
    // Set PPM value
    ppmEncoder.setChannel(i, chan_ms[i]);
  }
}

int currentEditLine = 0;

int currentDisplayValuesPage = 0;
int currentDisplayValuesMode = 0;
void DisplayValues() {
  
  Serial.print("Values ");

  if ((currbuttons & BUTTONS_PRESSED::BTN_NEXT)!=0) {
    currentDisplayValuesPage += 1;
    int nb_pages = (Config::ConfigFile.NBchannels-1)/7;
    if (currentDisplayValuesPage>nb_pages)
      currentDisplayValuesPage = 0;
  }
  if ((currbuttons & BUTTONS_PRESSED::BTN_OK)!=0) {
    currentDisplayValuesMode += 1;
  }

  if (currentDisplayValuesMode>=3)
    currentDisplayValuesMode = 0;
  
  Display.setFooter("Values p." + String(currentDisplayValuesPage) + "     ");
  Display.refreshFooter();
  Display.setCursor(0,1);
  char buff[20];
  
  float us_to_pct = 100.0f/((float)(PPMEncoder::MAX_us - PPMEncoder::MIN_us));
  for(int i=0; i<7; i++) {
    int idx = (currentDisplayValuesPage*7) + i;
    if (idx>=Config::ConfigFile.NBchannels) {
      Serial.println("clear line " + String((i<<1)+1));
      Display.clearLine((i<<1)+1);
    } else {
      //dtostrf(vals[0], 2, 2, buff);
      switch(currentDisplayValuesMode)
      {
        case 0:
        default:
        sprintf(buff, "%s %3d%%  ", Config::ConfigFile.channels[idx].name, (int)((chan_ms[idx]-PPMEncoder::MIN_us)*us_to_pct));
        break;
        case 1:
        sprintf(buff, "%s %4dmV  ", Config::ConfigFile.channels[idx].name, (int)(adc_vals[idx]*SCALE_mV_per_ADC));
        break;
        case 2:
        sprintf(buff, "%s %4dus  ", Config::ConfigFile.channels[idx].name, (int)(chan_ms[idx]));
        break;
      }
      Serial.println(buff);
      Display.println(buff);
    }
  }
}

int currentDisplayChannelPage = 0;

void DisplayChannels() {
  
  if ((currbuttons & BUTTONS_PRESSED::BTN_NEXT)!=0) {
    currentDisplayChannelPage += 1;
    if (currentDisplayChannelPage>=Config::ConfigFile.NBchannels)
      currentDisplayChannelPage = 0;
  }
  if ((currbuttons & BUTTONS_PRESSED::BTN_OK)!=0) {
    currentEditLine += 1;
 }

  int idx = currentDisplayChannelPage;
  Display.setFooter("Chan: " + String(Config::ConfigFile.channels[idx].name) + "       ");
  Display.refreshFooter();
  Display.setCursor(0,1);
  
  char buff[20];
  sprintf(buff, "A=%4dmV", (int)(adc_mv[idx]));
  Display.println(buff);
  //sprintf(buff, "C=%4dmV", (int)(chan_mv[idx]));
  //Display.println(buff);
  sprintf(buff, "T=%4dus", (int)(chan_ms[idx]));
  Display.println(buff);
  
  sprintf(buff, " Min.C=%4dmV  ", Config::ConfigFile.channels[idx].min_mV);
  Display.println(buff);
  sprintf(buff, " Max.C=%4dmV  ", Config::ConfigFile.channels[idx].max_mV);
  Display.println(buff);
  
  sprintf(buff, " Min.T=%4dus  ", Config::ConfigFile.channels[idx].min_us);
  Display.println(buff);
  sprintf(buff, " Max.T=%4dus  ", Config::ConfigFile.channels[idx].max_us);
  Display.println(buff);
  sprintf(buff, " Trim =%4dus  ", Config::ConfigFile.channels[idx].trim_us);
  Display.println(buff);
  
  if (currentEditLine>=5)
    currentEditLine = 0;
  Display.setCursor(0,(currentEditLine<<1)+5);
  Display.print(">");

  switch(currentEditLine) {
    case 0: {
      if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
        Config::ConfigFile.channels[idx].min_mV += STEP_TUNING_mV;
      }
      if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
        Config::ConfigFile.channels[idx].min_mV -= STEP_TUNING_mV;
      }
      if (Config::ConfigFile.channels[idx].min_mV<0)
        Config::ConfigFile.channels[idx].min_mV = 0;
      if (Config::ConfigFile.channels[idx].min_mV>Config::ConfigFile.channels[idx].max_mV)
        Config::ConfigFile.channels[idx].min_mV = Config::ConfigFile.channels[idx].max_mV;
    }
    break;
    case 1: {
      if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
        Config::ConfigFile.channels[idx].max_mV += STEP_TUNING_mV;
      }
      if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
        Config::ConfigFile.channels[idx].max_mV -= STEP_TUNING_mV;
      }
      if (Config::ConfigFile.channels[idx].max_mV<Config::ConfigFile.channels[idx].min_mV)
        Config::ConfigFile.channels[idx].max_mV = Config::ConfigFile.channels[idx].min_mV;
      if (Config::ConfigFile.channels[idx].max_mV>5000)
        Config::ConfigFile.channels[idx].max_mV = 5000;
    }
    break;
    
    case 2: {
      if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
        Config::ConfigFile.channels[idx].min_us += STEP_TUNING_us;
      }
      if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
        Config::ConfigFile.channels[idx].min_us -= STEP_TUNING_us;
      }
      if (Config::ConfigFile.channels[idx].min_us<0)
        Config::ConfigFile.channels[idx].min_us = 0;
      if (Config::ConfigFile.channels[idx].min_us>Config::ConfigFile.channels[idx].max_us)
        Config::ConfigFile.channels[idx].min_us = Config::ConfigFile.channels[idx].max_us;
    }
    break;
    case 3: {
      if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
        Config::ConfigFile.channels[idx].max_us += STEP_TUNING_us;
      }
      if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
        Config::ConfigFile.channels[idx].max_us -= STEP_TUNING_us;
      }
      if (Config::ConfigFile.channels[idx].max_us<Config::ConfigFile.channels[idx].min_us)
        Config::ConfigFile.channels[idx].max_us = Config::ConfigFile.channels[idx].min_us;
      if (Config::ConfigFile.channels[idx].max_us>5000)
        Config::ConfigFile.channels[idx].max_us = 5000;
    }
    break;
    case 4: {
      if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
        Config::ConfigFile.channels[idx].trim_us += STEP_TUNING_us;
      }
      if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
        Config::ConfigFile.channels[idx].trim_us -= STEP_TUNING_us;
      }
      if (Config::ConfigFile.channels[idx].trim_us<-Config::ConfigFile.channels[idx].min_mV)
        Config::ConfigFile.channels[idx].trim_us = -Config::ConfigFile.channels[idx].min_mV;
      if (Config::ConfigFile.channels[idx].trim_us>Config::ConfigFile.channels[idx].max_mV)
        Config::ConfigFile.channels[idx].trim_us = Config::ConfigFile.channels[idx].max_mV;
    }
    break;
    
    case 5: {
      if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
        Config::ConfigFile.channels[idx].min_mV += STEP_TUNING_mV;
        Config::ConfigFile.channels[idx].max_mV += STEP_TUNING_mV;
      }
      if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
        Config::ConfigFile.channels[idx].min_mV -= STEP_TUNING_mV;
        Config::ConfigFile.channels[idx].max_mV -= STEP_TUNING_mV;
      }
      if (Config::ConfigFile.channels[idx].min_mV<0)
        Config::ConfigFile.channels[idx].min_mV = 0;
      if (Config::ConfigFile.channels[idx].max_mV>5000)
        Config::ConfigFile.channels[idx].max_mV = 5000;
      if (Config::ConfigFile.channels[idx].min_mV>Config::ConfigFile.channels[idx].max_mV)
        Config::ConfigFile.channels[idx].min_mV = Config::ConfigFile.channels[idx].max_mV;
      if (Config::ConfigFile.channels[idx].max_mV<Config::ConfigFile.channels[idx].min_mV)
        Config::ConfigFile.channels[idx].max_mV = Config::ConfigFile.channels[idx].min_mV;
    }
    break;

  }

}

int currentDisplayModePage = 0;

void DisplayModes() {
  
  if ((currbuttons & BUTTONS_PRESSED::BTN_NEXT)!=0) {
    currentDisplayModePage += 1;
    if (currentDisplayModePage>Config::ConfigFile.NBchannels)
      currentDisplayModePage = 0;
    Display.clearBody();
  }
  if ((currbuttons & BUTTONS_PRESSED::BTN_OK)!=0) {
    currentEditLine += 1;
  }
  

  char buff[20];
  switch(currentDisplayModePage) {
    case 0: {
      Display.setFooter("Configure PPM   ");
      Display.refreshFooter();
      Display.setCursor(0,1);

      sprintf(buff, " Frame:%5dus", Config::ConfigFile.frame_length_us);
      Display.println(buff);
      sprintf(buff, " Inter:%5dus", Config::ConfigFile.interval_us);
      Display.println(buff);
      sprintf(buff, " Min:  %5dus", Config::ConfigFile.min_pulse_us);
      Display.println(buff);
      sprintf(buff, " Max:  %5dus", Config::ConfigFile.max_pulse_us);
      Display.println(buff);
      sprintf(buff, " #Chan:%3d", Config::ConfigFile.NBchannels);
      Display.println(buff);
      sprintf(buff, " SAVE ALL?");
      Display.println(buff);
      
      if (currentEditLine>=6)
        currentEditLine = 0;
      Display.setCursor(0,(currentEditLine<<1)+1);
      Display.print(">");

      bool edited = false;
      switch(currentEditLine) {
        case 0: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            Config::ConfigFile.frame_length_us += STEP_TUNING_us;
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            Config::ConfigFile.frame_length_us -= STEP_TUNING_us;
            edited = true;
          }
          if (Config::ConfigFile.frame_length_us<0)
            Config::ConfigFile.frame_length_us = 0;
          if (Config::ConfigFile.frame_length_us>32000)
            Config::ConfigFile.frame_length_us = 32000;
        }
        break;
        case 1: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            Config::ConfigFile.interval_us += STEP_TUNING_us;
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            Config::ConfigFile.interval_us -= STEP_TUNING_us;
            edited = true;
          }
          if (Config::ConfigFile.interval_us<0)
            Config::ConfigFile.interval_us = 0;
          if (Config::ConfigFile.interval_us>5000)
            Config::ConfigFile.interval_us = 5000;
        }
        break;
       case 2: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            Config::ConfigFile.min_pulse_us += STEP_TUNING_us;
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            Config::ConfigFile.min_pulse_us -= STEP_TUNING_us;
            edited = true;
          }
          if (Config::ConfigFile.min_pulse_us<Config::ConfigFile.interval_us)
            Config::ConfigFile.min_pulse_us = Config::ConfigFile.interval_us;
          if (Config::ConfigFile.min_pulse_us>Config::ConfigFile.max_pulse_us)
            Config::ConfigFile.min_pulse_us = Config::ConfigFile.max_pulse_us;
        }
        break;
        case 3: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            Config::ConfigFile.max_pulse_us += 10;
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            Config::ConfigFile.max_pulse_us -= 10;
            edited = true;
          }
          if (Config::ConfigFile.max_pulse_us<Config::ConfigFile.min_pulse_us)
            Config::ConfigFile.max_pulse_us = Config::ConfigFile.min_pulse_us;
          if (Config::ConfigFile.max_pulse_us>Config::ConfigFile.frame_length_us)
            Config::ConfigFile.max_pulse_us = Config::ConfigFile.frame_length_us;
        }
        break;
        case 4: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            Config::ConfigFile.NBchannels += 1;
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            Config::ConfigFile.NBchannels -= 1;
            edited = true;
          }
          if (Config::ConfigFile.NBchannels<1)
            Config::ConfigFile.NBchannels = 1;
          if (Config::ConfigFile.NBchannels>MAX_CHANNELS)
            Config::ConfigFile.NBchannels = MAX_CHANNELS;
        }
        break;
        case 5: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            Display.setCursor(0,(currentEditLine<<1)+1);
            Display.print(">SAVING...");
            Config::SaveConfigToEEPROM();
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            Display.setCursor(0,(currentEditLine<<1)+1);
            Display.print(">SAVING...");
            Config::SaveConfigToEEPROM();            
          }
        }
        break;
      }
      if (edited) {
        ppmEncoder.PPM_INTERVAL_LENGTH_us = Config::ConfigFile.frame_length_us;
        ppmEncoder.PPM_FRAME_LENGTH_us = Config::ConfigFile.interval_us;
        ppmEncoder.MIN_us = Config::ConfigFile.min_pulse_us;
        ppmEncoder.MAX_us = Config::ConfigFile.max_pulse_us;
        // Nombre de voies
        ppmEncoder.setNbChannel(Config::ConfigFile.NBchannels);
      }
      
    } break;
    default: {
        
      int idx = currentDisplayModePage-1;
      Display.setFooter("Config. #" + String(idx+1) + "    ");
      Display.refreshFooter();
      Display.setCursor(0,1);
      
      sprintf(buff, "Name=%s", Config::ConfigFile.channels[idx].name);
      Display.println(buff);
      if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_INVERTED)!=0)
        Display.println(" Inverted");
      else
        Display.println(" Normal  ");
        
      if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_COUPLING)!=0)
        Display.println(" Coupled    ");
      else
        Display.println(" Not coupled");
            
      if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_DUALRATE)!=0)
        Display.println(" Dual rate  ");
      else
        Display.println(" Normal rate");       
      if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_POWERLAW)!=0)
        Display.println(" Power law ");
      else
        Display.println(" Linear law");

      int master = Config::ConfigFile.channels[idx].master_channel;
      Display.println(" Master:" + String(Config::ConfigFile.channels[master].name) + "  " );
      Display.println(" Rate:" + String(Config::ConfigFile.channels[master].rate) + "  ");

      
      if (currentEditLine>=6)
        currentEditLine = 0;
      Display.setCursor(0,(currentEditLine<<1)+3);
      Display.print(">");

      bool edited = false;
      switch(currentEditLine) {
        case 0: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            edited = true;
          }
          if (edited) {
            if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_INVERTED)!=0) {
              Config::ConfigFile.channels[idx].options &= ~CONFIG_CHANNEL_OPT_INVERTED;  
            } else {
              Config::ConfigFile.channels[idx].options |= CONFIG_CHANNEL_OPT_INVERTED;
            }
          }
        }
        break;
        case 1: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            edited = true;
          }
          if (edited) {
            if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_COUPLING)!=0) {
              Config::ConfigFile.channels[idx].options &= ~CONFIG_CHANNEL_OPT_COUPLING;  
            } else {
              Config::ConfigFile.channels[idx].options |= CONFIG_CHANNEL_OPT_COUPLING;
            }
          }
        }
        break;
        case 2: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            edited = true;
          }
          if (edited) {
            if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_DUALRATE)!=0) {
              Config::ConfigFile.channels[idx].options &= ~CONFIG_CHANNEL_OPT_DUALRATE;  
            } else {
              Config::ConfigFile.channels[idx].options |= CONFIG_CHANNEL_OPT_DUALRATE;
            }
          }
        }
        break;
        case 3: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            edited = true;
          }
          if (edited) {
            if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_POWERLAW)!=0) {
              Config::ConfigFile.channels[idx].options &= ~CONFIG_CHANNEL_OPT_POWERLAW;  
            } else {
              Config::ConfigFile.channels[idx].options |= CONFIG_CHANNEL_OPT_POWERLAW;
            }
          }
        }
        break;
        case 4: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            Config::ConfigFile.channels[idx].master_channel += 1;
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            Config::ConfigFile.channels[idx].master_channel -= 1;
            edited = true;
          }
          if (Config::ConfigFile.channels[idx].master_channel<0)
            Config::ConfigFile.channels[idx].master_channel = 0;
          if (Config::ConfigFile.channels[idx].master_channel>=Config::ConfigFile.NBchannels) 
            Config::ConfigFile.channels[idx].master_channel = Config::ConfigFile.NBchannels-1;
        }
        break;
        case 5: {
          if ((currbuttons & BUTTONS_PRESSED::BTN_PLUS)!=0) {
            Config::ConfigFile.channels[master].rate *= 1.01f;
            edited = true;
          }
          if ((currbuttons & BUTTONS_PRESSED::BTN_MINUS)!=0) {
            Config::ConfigFile.channels[master].rate *= (1.0f/(1.01f));
            edited = true;
          }
          if (Config::ConfigFile.channels[master].rate<0.25)
            Config::ConfigFile.channels[master].rate = 0.25;
          if (Config::ConfigFile.channels[master].rate>=4.0) 
            Config::ConfigFile.channels[master].rate = 4.0;
        }
        break;
      }
      
    } break;
  }
  
 /*
  char buff[32];
  for(int i=0; i<4; i++) {
    
    sprintf(buff, "T%d=%4dus", i, (int)(chan_ms[i]));
  
    Serial.println(buff);
    Display.println(buff);
  }
  */
}

int currentDisplayOptionsPage = 0;

void DisplayOptions() {
  Display.setCursor(0,1);
  Display.println("Options:");
 /*
  char buff[32];
  for(int i=0; i<4; i++) {
    
    sprintf(buff, "T%d=%4dus", i, (int)(chan_ms[i]));
  
    Serial.println(buff);
    Display.println(buff);
  }
  */
}

enum STATE_MACHINE : uint8_t
{
  DISPLAY_VALUES = 0,
  DISPLAY_CHANNELS,
  DISPLAY_MODES,
  //DISPLAY_OPTIONS,
  MAX_STATES,
};

uint8_t state = STATE_MACHINE::DISPLAY_VALUES;

void loop() {
  
  ReadValues();
  currbuttons = ReadButtons();
  
  switch(state) {
    case DISPLAY_VALUES:
      DisplayValues();
      break;
    case DISPLAY_CHANNELS:
      DisplayChannels();
      break;
    case DISPLAY_MODES:
      DisplayModes();
      break;
/*    case DISPLAY_OPTIONS:
      DisplayOptions();
      break;
      */
  }

  if ((currbuttons & BUTTONS_PRESSED::BTN_MODE)!=0) {
    state +=1;
    if (state>=STATE_MACHINE::MAX_STATES)
      state = 0;
    Display.clearBody();
  }
  


  lastbuttons = currbuttons;  
}
