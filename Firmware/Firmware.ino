// Arduino Mega
#include "RC_PPMEncoder.h"

// Use D10 on mega 2560
#define OUTPUT_PIN (10)

void setup() {
  
  Serial.begin(1000000);
  while(!Serial) {  ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("Configuring IOs...");
  // Set all IOs in INPUT mode to avoid burning a pin with a wrong wiring
  for(int i=0; i<53; i++) {
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
  pinMode(A8, INPUT);
  pinMode(A9, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);
  pinMode(A12, INPUT);
  pinMode(A13, INPUT);
  pinMode(A14, INPUT);
  pinMode(A15, INPUT);
  
  Serial.println("Starting PPM...");
  ppmEncoder.begin(OUTPUT_PIN, 7);
}

void loop() {
  int vals[16];
  int i=0;
  vals[i++] = analogRead(A0);
  vals[i++] = analogRead(A1);
  vals[i++] = analogRead(A2);
  vals[i++] = analogRead(A3);
  vals[i++] = analogRead(A4);
  vals[i++] = analogRead(A5);
  vals[i++] = analogRead(A6);
  vals[i++] = analogRead(A7);
  /*vals[i++] = analogRead(A8);
  vals[i++] = analogRead(A9);
  vals[i++] = analogRead(A10);
  vals[i++] = analogRead(A11);
  vals[i++] = analogRead(A12);
  vals[i++] = analogRead(A13);
  vals[i++] = analogRead(A14);
  vals[i++] = analogRead(A15);
  */
  Serial.println("Values read");
  for(i=0; i<7; i++) {
    int chanval = map(vals[i], 0, 1023, PPMEncoder::MIN, PPMEncoder::MAX);
    // Set PPM value
    ppmEncoder.setChannel(i, chanval);
  }
  
  delay(10);
}
