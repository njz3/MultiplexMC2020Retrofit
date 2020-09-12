//Taken from Christopher Schirner
// https://github.com/schinken/PPMEncoder
#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
// Arduino Framework
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define PPM_DEFAULT_CHANNELS (8)


enum PPM_STATE : uint8_t
{
  WAITING_START_FRAME = 0,
  PULSING_CHANNEL,
  WAITING_INTERVAL,
};

class PPMEncoder {
  public:
    static const uint16_t MAX_CHANNEL = 16;
    
    static uint16_t PPM_INTERVAL_LENGTH_us;
    static uint16_t PPM_FRAME_LENGTH_us;
    static uint16_t MIN_us;
    static uint16_t MED_us;
    static uint16_t MAX_us;

    void setNbChannel(uint8_t numChannels);
    void setChannel(uint8_t channel, uint16_t value);
    void setChannelPercent(uint8_t channel, uint8_t percent);
    
    void begin(uint8_t pin);
    void begin(uint8_t pin, uint8_t ch);

    void setPinLevel(int level);

    void interrupt();

  private:

    int16_t channels[MAX_CHANNEL+1];
    uint16_t elapsed_us;

    uint8_t numChannels;
    uint8_t currentChannel;
    byte outputPin;
    uint8_t state;
};

extern PPMEncoder ppmEncoder;
