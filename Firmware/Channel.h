//Taken from Christopher Schirner
// https://github.com/schinken/PPMEncoder
#pragma once

#include "Config.h"

class CChannel {
  public:
    uint16_t min_us = 1000;
    uint16_t med_us = 1600;
    uint16_t max_us = 2200;

    void setPeriod(uint16_t value) { };
    void setPercent(uint8_t percent) { };

};
