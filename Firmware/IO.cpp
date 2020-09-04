#include "IO.h"

#include "RC_PPMEncoder.h"

void InitIOs()
{
  // Set all IOs in INPUT mode to avoid burning a pin with a wrong wiring
#if defined(MC2020_MEGA)
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
  // A4/A5 used for I2C
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
#ifdef MC2020_MEGA
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A8, INPUT);
  pinMode(A9, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);
  pinMode(A12, INPUT);
  pinMode(A13, INPUT);
  pinMode(A14, INPUT);
  pinMode(A15, INPUT);
#endif
}

uint16_t ButtonsPressed = 0;
uint16_t ButtonsPushed = 0;
uint16_t ButtonsReleased = 0;

uint16_t LastButtonsPressed = 0;

void ReadButtons()
{
  uint16_t buttons = 0;
  uint16_t buttonchanged = 0;
  
  if (digitalRead(2)==0) buttons |= BUTTONS_ID::BTN_PLUS;
  if (digitalRead(3)==0) buttons |= BUTTONS_ID::BTN_MINUS;
  if (digitalRead(4)==0) buttons |= BUTTONS_ID::BTN_NEXT;
  if (digitalRead(5)==0) buttons |= BUTTONS_ID::BTN_PAGE;
  if (digitalRead(6)==0) buttons |= BUTTONS_ID::BTN_OPT1;
  if (digitalRead(7)==0) buttons |= BUTTONS_ID::BTN_OPT2;

  LastButtonsPressed = ButtonsPressed;
  ButtonsPressed = buttons;
  // Detect low->high or high->low transitions
  buttonchanged = ButtonsPressed^LastButtonsPressed;
  if (buttonchanged!=0) {
    Serial.print("B=");
    Serial.println(buttonchanged);
    // Detect low->high transitions
    ButtonsPushed = buttonchanged & ButtonsPressed;
    // Detect high->low transitions
    ButtonsReleased = buttonchanged & LastButtonsPressed;
  } else {
    // No changes
    ButtonsPushed = 0;
    ButtonsReleased = 0;
  }
}

int adc_mv[MAX_ADC];
int chan_mv[MAX_CHANNELS];
int chan_ms[MAX_CHANNELS];
int chan_pct[MAX_CHANNELS];
int nb_adc = 0;

void ReadValues() {
  nb_adc = 0;
  int adc_vals[MAX_ADC];
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
  if (nb_adc>MAX_ADC) {
    Serial.println("Error MAX ADC!");
  }
  for(int i=0; i<nb_adc; i++) {
    adc_mv[i] = (int)((float)adc_vals[i]*SCALE_mV_per_ADC);
  }
  
}

void ProcessValues() {
  for(int i=0; i<Config::ConfigFile.NBchannels; i++) {
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

    float us_to_pct = 100.0f/((float)(Config::ConfigFile.channels[i].max_us - Config::ConfigFile.channels[i].min_us));
    chan_pct[i] = (int)((chan_ms[i]-Config::ConfigFile.channels[i].min_us)*us_to_pct);

    // Set PPM value
    ppmEncoder.setChannel(i, chan_ms[i]);
  }
}


  
