#pragma once

#include "Config.h"

#define NB_CST       (2)
#define NB_STICKS    (4)
#define NB_TRIMS     (4)
#define NB_AUXS      (2)

#define VPIN0   (0xF0)  // Virtual Pin
#define VPIN1   (0xF1)  // Virtual Pin
#define VPIN2   (0xF2)  // Virtual Pin


#define NB_INPUTS   (NB_CST+NB_STICKS+NB_TRIMS+NB_AUXS)
#define NB_MIXERS   (16)
#define NB_OUTPUTS  (12)
#define NB_SERVOS   (7)

enum{
   curve_normal_em=0,
   curve_expo1_em,
   curve_expo2_em,
   curve_expo3_em,
   curve_expo4_em,
   curve_expo5_em,
   curve_abs_em,
   curve_positive_em,
   curve_negative_em,
   curve_perso1_em,
   curve_perso2_em,
   curve_perso3_em,
   curve_perso4_em,
   curve_max_em
};

enum{
   validity_never_em=0,
   validity_always_em,
   validity_SwA0_em,
   validity_SwA1_em,
   validity_SwB0_em,
   validity_SwB1_em,
   validity_max_em
};

enum BUTTONS_ID : uint16_t
{
  BTN_PLUS  = (1<<0),
  BTN_MINUS = (1<<1),
  BTN_NEXT  = (1<<2),
  BTN_PAGE  = (1<<3),
  BTN_SWA   = (1<<4),
  BTN_SWB   = (1<<5),
  BTN_SWC   = (1<<6),
  BTN_SWD   = (1<<7),
};

typedef struct {
   uint8_t  pin_ui8;
   uint16_t min_mV_ui16;
   uint16_t med_mV_ui16;
   uint16_t max_mV_ui16;
}input_cfg_tst; // tst = type struct

typedef struct {
   uint16_t adc_raw_ui16;
   uint16_t adc_mV_ui16;
   float    val_ft;         /**< [ -1.0f : 1.0f ] */
}input_var_tst; // tst = type struct


typedef struct {
   uint8_t  out_idx_ui8;
   uint8_t  in_idx_ui8;
   uint8_t  curve_ui8;
   int8_t  coef_si8;
   uint8_t  valid_ui8;
}mixers_tst; // tst = type struct

typedef struct {
   uint8_t  out_idx_ui8;
   uint8_t  trim_idx_ui8;
   int8_t  trim_coef_si8;
   uint16_t min_us_ui16;
   uint16_t med_us_ui16;
   uint16_t max_us_ui16;
}servos_tst; // tst = type struct

extern input_var_tst Inputs_var_pst[NB_INPUTS];
extern input_cfg_tst Inputs_cfg_pst[NB_INPUTS];
extern mixers_tst Mixers_pst[NB_MIXERS];
extern float Outputs_pft[NB_OUTPUTS];
extern uint16_t   Servos_us_pui16[NB_SERVOS];
extern servos_tst Servos_pst[NB_SERVOS];

extern uint16_t ButtonsPressed;
extern uint16_t ButtonsPushed;
extern uint16_t ButtonsReleased;
extern uint16_t LastButtonsPressed;

void IO_InitPins();

void IO_ReadButtons();

#define IS_PUSHED(btn) ((ButtonsPushed & (btn))!=0)
#define IS_PRESSED(btn) ((ButtonsPressed & (btn))!=0)
#define IS_RELEASED(btn) ((ButtonsReleased & (btn))!=0)

void IO_InputsProcess(void);
void IO_MixersProcess(void);
void IO_ServosProcess(void);
void IO_PpmSetChannels(void);
