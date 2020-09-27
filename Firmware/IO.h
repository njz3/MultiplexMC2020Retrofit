#pragma once

#include "Config.h"

enum BUTTONS_ID : uint16_t
{
  BTN_PLUS  = (1<<0),
  BTN_MINUS = (1<<1),
  BTN_NEXT  = (1<<2),
  BTN_PAGE  = (1<<3),
  BTN_DUAL_RATE = (1<<4),
  BTN_COUPLING  = (1<<5),
  BTN_OPT1_CH  = (1<<6),
  BTN_OPT2_CH  = (1<<7),
};


extern uint16_t ButtonsPressed;
extern uint16_t ButtonsPushed;
extern uint16_t ButtonsReleased;
extern uint16_t LastButtonsPressed;

void InitIOs();

void ReadButtons();

#define IS_PUSHED(btn) ((ButtonsPushed & (btn))!=0)
#define IS_PRESSED(btn) ((ButtonsPressed & (btn))!=0)
#define IS_RELEASED(btn) ((ButtonsReleased & (btn))!=0)

extern int16_t adc_mv[MAX_ADC];
extern int16_t chan_mv[MAX_CHANNELS];
extern int16_t chan_ms[MAX_CHANNELS];
extern int16_t chan_pct[MAX_CHANNELS];
extern int nb_adc;

void ReadValues();
void ProcessValues();
