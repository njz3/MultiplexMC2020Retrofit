#ifndef ARDUINO_H
#define ARDUINO_H

typedef unsigned char byte;
typedef unsigned char uint8_t;
typedef   signed char int8_t;

typedef   signed short int16_t;
typedef   signed short int16_t;
typedef unsigned short uint16_t;

typedef   signed int int32_t;
typedef unsigned int uint32_t;

#include <string.h>
#include <stdio.h>
#include <math.h>

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


extern uint8_t digitalRead(uint8_t ch);
extern uint16_t analogRead(uint8_t ch);

#define digitalWrite(outputPin, level) // TODO simulation output to setup

#define abs(a)                ( a>0 ? a : -a )
#define max(a,b)              ( a>b ? a :  b )
#define min(a,b)              ( a<b ? a :  b )
#define constrain(v,mi,ma)    ( max( min(v,ma) , mi ) )  // min(v,ma) --> upper limit=ma
                                                         // max(v,mi) --> lower limit=mi
#define INPUT_PULLUP 0  //dummy
#define LOW  0
#define HIGH 1

#define A0     0
#define A1     1
#define A2     2
#define A3     3
#define A4     4
#define A5     5
#define A6     6
#define A7     7
#define A8     8
#define A9     9
#define A10   10
#define A11   11
#define A12   12
#define A13   13
#define A14   14
#define A15   15

#define pinMode(a,b);   // stub

#define cli()           // stub
#define sei()           // stub

#define delay(a)

struct SerialClass
{
   //SerialClass operator!() {return 0;}
   void begin(int pouet) {}
   void print( const char *str) {}
   void println( uint16_t val ) {}
   void println( const char *str ) { print(str); /*print('\n')*/; }
};

extern SerialClass Serial;
#endif // ARDUINO_H
