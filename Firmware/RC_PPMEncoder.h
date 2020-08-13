//Taken from Christopher Schirner
// https://github.com/schinken/PPMEncoder

#ifndef _PPMEncoder_h_
#define _PPMEncoder_h_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define PPM_DEFAULT_CHANNELS (8)

#define PPM_INTERVAL_LENGTH_uS (300)
#define PPM_FRAME_LENGTH_uS (20000)

enum PPM_STATE : uint8_t
{
  WAITING_START_FRAME = 0,
  PULSING_CHANNEL,
  WAITING_INTERVAL,
};

class PPMEncoder {
  public:
    static const uint16_t MAX_CHANNEL = 9;
    static const uint16_t MIN = 1000;
    static const uint16_t MAX = 2000;

    void setChannel(uint8_t channel, uint16_t value);
    void setChannelPercent(uint8_t channel, uint8_t percent);

    void begin(uint8_t pin);
    void begin(uint8_t pin, uint8_t ch);

    void interrupt();
    void setPinLevel(int level);

  private:
    int16_t channels[MAX_CHANNEL+1];
    uint16_t elapsedUs;

    uint8_t numChannels;
    uint8_t currentChannel;
    byte outputPin;
    uint8_t state;
};

extern PPMEncoder ppmEncoder;

#endif
