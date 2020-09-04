#pragma once

#include "Config.h"

enum BUTTONS_ID : uint16_t
{
  BTN_PLUS  = (1<<0),
  BTN_MINUS = (1<<1),
  BTN_NEXT  = (1<<2),
  BTN_PAGE  = (1<<3),
  BTN_OPT1  = (1<<4),
  BTN_OPT2  = (1<<5),
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

extern int adc_mv[MAX_ADC];
extern int chan_mv[MAX_CHANNELS];
extern int chan_ms[MAX_CHANNELS];
extern int chan_pct[MAX_CHANNELS];
extern int nb_adc;

void ReadValues();
void ProcessValues();
