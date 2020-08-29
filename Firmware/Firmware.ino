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
  delay(500);
  
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
#endif

  delay(500);
  Serial.println("Starting PPM...");
  Display.println("Starting PPM");
  
  // Nombre de voies
  ppmEncoder.begin(OUTPUT_PIN, NB_CHANNELS);

  delay(500);
  Display.println("#chan=" + String(NB_CHANNELS));
  Display.println("Output=D" + String(OUTPUT_PIN));
  Display.println("Version=" VERSION);
  
    
  delay(4000);
  Display.cleanup();
  Display.setNormalFont();
}

const float SCALE_mV_per_ADC = (5000.0f/1023.0f);
const float SCALE_ADC_per_mV = (1023.0f/5000.0f);
const int MIN_MANCHE_mV = 1800;
const int MAX_MANCHE_mV = 3200;
const int MIN_MANCHE_ADC = (int)(MIN_MANCHE_mV*SCALE_ADC_per_mV);
const int MAX_MANCHE_ADC = (int)(MAX_MANCHE_mV*SCALE_ADC_per_mV);

void loop() {
  Serial.println("Reading ");
 
  int adc_vals[16];
  int chan_adc[16];
  int chan_ms[16];
  int nb_adc = 0;
  
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
  
  Serial.println("Constrain manche");
  // Special manche (course limitée)
  for(int i=0; i<4; i++) {
    chan_adc[i] = constrain(adc_vals[i],MIN_MANCHE_ADC, MAX_MANCHE_ADC);
    chan_ms[i] = map(chan_adc[i], MIN_MANCHE_ADC, MAX_MANCHE_ADC, PPMEncoder::MIN, PPMEncoder::MAX);
    // Set PPM value
    ppmEncoder.setChannel(i, chan_ms[i]);
  }
  
  Serial.println("Constrain autre ");
  // Autre potars
  for(int i=4; i<min(NB_CHANNELS, nb_adc); i++) {
    chan_adc[i] = constrain(adc_vals[i], 0, 1023);
    chan_ms[i] = map(chan_adc[i], 0, 1023, PPMEncoder::MIN, PPMEncoder::MAX);
    // Set PPM value
    ppmEncoder.setChannel(i, chan_ms[i]);
  }

  Serial.print("Values ");
  Serial.print(MIN_MANCHE_mV);
  Serial.print(" ");
  Serial.println(MAX_MANCHE_mV);
  
  Display.setCursor(0,1);
  Display.println("Values:");
 
  char buff[32];
  for(int i=0; i<4; i++) {
    //dtostrf(vals[0], 2, 2, buff);
    int btn = digitalRead(2);
    if (btn!=0)
      sprintf(buff, "A%d=%4dus", i, (int)(chan_ms[i]));
    else
      sprintf(buff, "A%d=%4dmV", i, (int)(adc_vals[i]*SCALE_mV_per_ADC));
  
    Serial.println(buff);
    Display.println(buff);
  }
  
  delay(10);
}
