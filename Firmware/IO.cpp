#include "IO.h"

#include "RC_PPMEncoder.h"

input_var_tst Inputs_var_pst[NB_INPUTS] = {0};
input_cfg_tst Inputs_cfg_pst[NB_INPUTS] = {  // pst = pointer to structure
         /* pin,   min_mV, med_mV, max_mV, */
  /*0*/  { A0,        10,   2500,   4990 },  /**< Stick Right Horizontal (Ailerons)      */
  /*1*/  { A1,        10,   2500,   4990 },  /**< Stick Left  Vertical   (Profondeur)    */
  /*2*/  { A2,        10,   2500,   4990 },  /**< Stick Left  Horizontal (Derive)        */
  /*3*/  { A3,        10,   2500,   4990 },  /**< Stick Right Vertical   (Gaz)           */
  /*4*/  { A4,        10,   2500,   4990 },  /**< Trim Right Horizontal (Ailerons)       */
  /*5*/  { A5,        10,   2500,   4990 },  /**< Trim Left  Vertical   (Profondeur)     */
  /*6*/  { A6,        10,   2500,   4990 },  /**< Trim Left  Horizontal (Derive)         */
  /*7*/  { A7,        10,   2500,   4990 },  /**< Trim Right Vertical   (Gaz)            */
  /*8*/  { A8,        10,   2500,   4990 },  /**< Aux 1  */
  /*9*/  { A9,        10,   2500,   4990 },  /**< Aux 2  */
 /*10*/  { VPIN1,      0,      0,      0 },  /**< constant                               */
};


mixers_tst Mixers_pst[NB_MIXERS]={
        /*   out,    input,  curve,          coef,   valid */
 /*0*/  {    0,      0,      curve_expo3_em,   +100,    validity_always_em}, /* Ailerons     */
 /*1*/  {    1,      1,      curve_expo3_em,   +100,    validity_always_em}, /* Profondeur   */
 /*2*/  {    2,      2,      curve_expo3_em,   +100,    validity_always_em}, /* Derive       */
 /*3*/  {    3,      3,      curve_normal_em,  +100,    validity_always_em}, /* Gaz          */
 /*4*/  {    4,     10,      curve_normal_em,  +100,    validity_never_em}, /* dummy        */
 /*5*/  {    5,     10,      curve_normal_em,  +100,    validity_never_em}, /* dummy        */
 /*6*/  {    6,     10,      curve_normal_em,  +100,    validity_never_em}, /* dummy        */
 /*7*/  {    7,     10,      curve_normal_em,  +100,    validity_never_em}, /* dummy        */
 /*8*/  {    8,     10,      curve_normal_em,  +100,    validity_never_em}, /* dummy        */
 /*9*/  {    9,     10,      curve_normal_em,  +100,    validity_never_em}, /* dummy        */
 /*10*/ {    1,      0,      curve_abs_em,      -10,    validity_always_em }, /* dummy        */
 /*11*/ {    1,      3,      curve_positive_em, +50,    validity_always_em }, /* dummy        */
};

float Outputs_pft[NB_OUTPUTS];

uint16_t   Servos_us_pui16[NB_SERVOS];
servos_tst Servos_pst[NB_SERVOS]={
      /*   out,    trim_idx,   trim_coef,  min_us, med_us, max_us */
 /*0*/   { 0,      4,           25,        1100,   1600,   2100},  /* Ailerons 1 */
 /*1*/   { 1,      5,           25,        1100,   1600,   2100},  /* Profondeur */
 /*2*/   { 2,      6,           25,        1100,   1600,   2100},  /* Derive     */
 /*3*/   { 3,      0,            0,        1100,   1600,   2100},  /* Gaz        */
 /*4*/   { 0,      4,           25,        1100,   1600,   2100},  /* Ailerons 2 */
 /*5*/   { 9,      0,            0,        1100,   1600,   2100},  /* na */
 /*6*/   { 9,      0,            0,        1100,   1600,   2100},  /* na */
};


float m_IO_ExpoCurve(float f_In, float f_Expo);

void IO_InitVars(void)
{
}

void IO_InitPins()
{
  // Set all IOs in INPUT mode to avoid burning a pin with a wrong wiring
#if defined(MC2020_MEGA)
  for(int i=0; i<53; i++) {
#else
  for(int i=0; i<12; i++) {
#endif
    pinMode(i, INPUT_PULLUP);
  }

  for(int i=0; i<NB_INPUTS; i++)  {
     if( Inputs_cfg_pst[i].pin_ui8 < VPIN0)
        pinMode(Inputs_cfg_pst[i].pin_ui8, INPUT);
  }
}

uint16_t ButtonsPressed = 0;
uint16_t ButtonsPushed = 0;
uint16_t ButtonsReleased = 0;
uint16_t LastButtonsPressed = 0;

void IO_ReadButtons()
{
  uint16_t buttons = 0;
  uint16_t buttonchanged = 0;

  if (digitalRead(2)==0) buttons |= BUTTONS_ID::BTN_PLUS;
  if (digitalRead(3)==0) buttons |= BUTTONS_ID::BTN_MINUS;
  if (digitalRead(4)==0) buttons |= BUTTONS_ID::BTN_NEXT;
  if (digitalRead(5)==0) buttons |= BUTTONS_ID::BTN_PAGE;
  if (digitalRead(6)==0) buttons |= BUTTONS_ID::BTN_DUAL_RATE;
  if (digitalRead(7)==0) buttons |= BUTTONS_ID::BTN_COUPLING;
  if (digitalRead(8)==0) buttons |= BUTTONS_ID::BTN_OPT1_CH;
  if (digitalRead(9)==0) buttons |= BUTTONS_ID::BTN_OPT2_CH;

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

void IO_InputsProcess(void)
{
   float l_fullx_ft;
   float l_dx_ft;
   uint16_t l_adc_mV_ui16;
   for(int i=0; i<NB_INPUTS; i++)
   {
      if( Inputs_cfg_pst[i].pin_ui8 < VPIN0 ) // if it's a normal pin
      {
         Inputs_var_pst[i].adc_raw_ui16 = analogRead(Inputs_cfg_pst[i].pin_ui8);
         Inputs_var_pst[i].adc_mV_ui16  = ((float)Inputs_var_pst[i].adc_raw_ui16*SCALE_mV_per_ADC);
         if( Inputs_var_pst[i].adc_mV_ui16 < Inputs_cfg_pst[i].med_mV_ui16 )
         {
            l_adc_mV_ui16 = max( Inputs_cfg_pst[i].min_mV_ui16 , Inputs_var_pst[i].adc_mV_ui16 ); // limit to min
            l_dx_ft    = l_adc_mV_ui16 - Inputs_cfg_pst[i].min_mV_ui16; // always positive
            l_fullx_ft = Inputs_cfg_pst[i].med_mV_ui16 - Inputs_cfg_pst[i].min_mV_ui16; // always positive
            Inputs_var_pst[i].val_ft = (l_dx_ft/l_fullx_ft) + (-1.0f);
         }
         else //  Inputs_pst[i].adc_mV_ui16 >= Inputs_pst[i].med_mV_ui16 )
         {
            l_adc_mV_ui16 = min(Inputs_var_pst[i].adc_mV_ui16, Inputs_cfg_pst[i].max_mV_ui16);  // limit to max
            l_dx_ft    = l_adc_mV_ui16 - Inputs_cfg_pst[i].med_mV_ui16; // always positive
            l_fullx_ft = Inputs_cfg_pst[i].max_mV_ui16 - Inputs_cfg_pst[i].med_mV_ui16; // always positive
            Inputs_var_pst[i].val_ft = (l_dx_ft/l_fullx_ft) + (0.0f);
         }
      }
      else // if it's a virtual pin
      {
         switch(Inputs_cfg_pst[i].pin_ui8)
         {
            case VPIN0: Inputs_var_pst[i].val_ft =  0.0f; break;
            case VPIN1: Inputs_var_pst[i].val_ft = +1.0f; break;
            case VPIN2: Inputs_var_pst[i].val_ft = -1.0f; break;
            default:    Inputs_var_pst[i].val_ft =  0.0f; break;
         }
      }
   }
}

void IO_MixersProcess(void)
{
   float l_In_ft;
   float l_Out_ft;
   for(int i=0; i<NB_OUTPUTS; i++)
   {
      Outputs_pft[i] = 0.0f;   /* Outs reinit */
   }

   for(int i=0; i<NB_MIXERS; i++)
   {
      if( Mixers_pst[i].valid_ui8 == validity_never_em ) /* TODO, only the never case is handled for now */
      {
         continue; // skip
      }
      else
      {
         //All other case considered as always!
      }

      // get related input
      // TODO check range of in_idx
      l_In_ft = Inputs_var_pst[Mixers_pst[i].in_idx_ui8].val_ft;

      // apply curve
      switch(Mixers_pst[i].curve_ui8)
      {
         case curve_normal_em: l_Out_ft = l_In_ft;                            break;
         case curve_expo1_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 1.50f ); break;
         case curve_expo2_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 1.75f ); break;
         case curve_expo3_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 2.00f ); break;
         case curve_expo4_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 2.40f ); break;
         case curve_expo5_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 3.00f ); break;

         case curve_abs_em:       l_Out_ft = abs(l_In_ft);                  break;
         case curve_positive_em:  l_Out_ft = ( l_In_ft<0 ? 0 : l_In_ft );   break;
         case curve_negative_em:  l_Out_ft = ( l_In_ft<0 ? l_In_ft : 0 );   break;

         case curve_perso1_em: l_Out_ft = l_In_ft;                            break;
         case curve_perso2_em: l_Out_ft = l_In_ft;                            break;
         case curve_perso3_em: l_Out_ft = l_In_ft;                            break;
         case curve_perso4_em: l_Out_ft = l_In_ft;                            break;
         default:              l_Out_ft = l_In_ft;                            break;
      }

      // apply coef
      l_Out_ft *= (float)Mixers_pst[i].coef_si8;
      l_Out_ft /= 100.0f;

      // Add mixer result to the requested output
      // TODO check range of out_idx
      Outputs_pft[Mixers_pst[i].out_idx_ui8] += l_Out_ft;

      if(Outputs_pft[Mixers_pst[i].out_idx_ui8] > +1.0f)
         Outputs_pft[Mixers_pst[i].out_idx_ui8] = +1.0f;
      if(Outputs_pft[Mixers_pst[i].out_idx_ui8] < -1.0f)
         Outputs_pft[Mixers_pst[i].out_idx_ui8] = -1.0f;
   }
}

void IO_ServosProcess(void)
{
   float l_Out_ft;
   float l_Trim_ft;
   int16_t l_Trim_us_si16;
   uint16_t l_Med_us_ui16;
   uint16_t l_Servo_us_ui16;
   for(int i=0; i<NB_SERVOS; i++)
   {
      /*----- Trim Process -----*/
      l_Trim_ft  = Inputs_var_pst[ Servos_pst[i].trim_idx_ui8 ].val_ft; // get Trim assigned to the servo
      l_Trim_ft *= Servos_pst[i].trim_coef_si8;                     // Apply coef
      l_Trim_ft /= 100.0f;

      l_Trim_us_si16 = Servos_pst[i].max_us_ui16 - Servos_pst[i].min_us_ui16; // Full range
      l_Trim_us_si16*= l_Trim_ft;                                             // get trim in usecond
      l_Med_us_ui16  = Servos_pst[i].med_us_ui16 + l_Trim_us_si16;            // trim is used as an offset to the medium point (TBDiscuss)

      if(l_Med_us_ui16 > Servos_pst[i].max_us_ui16)
         l_Med_us_ui16 = Servos_pst[i].max_us_ui16;
      if(l_Med_us_ui16 < Servos_pst[i].min_us_ui16)
         l_Med_us_ui16 = Servos_pst[i].min_us_ui16;

      /*----- Outputs Calc -----*/
      l_Out_ft  = Outputs_pft[ Servos_pst[i].out_idx_ui8 ];

      if( l_Out_ft > 0.0f )
      {
         l_Servo_us_ui16 = l_Out_ft * (Servos_pst[i].max_us_ui16 - l_Med_us_ui16) + l_Med_us_ui16;
      }
      else
      {
         l_Out_ft *= (-1.0f); // invert sign to get positive l_Servo_us_ui16 (below)
         l_Servo_us_ui16 = l_Out_ft * (l_Med_us_ui16 - Servos_pst[i].min_us_ui16);
         l_Servo_us_ui16 = l_Med_us_ui16 - l_Servo_us_ui16; // (re)-invert and add l_Med_us_ui16 offset
      }
      Servos_us_pui16[i] = l_Servo_us_ui16;
   }
}

void IO_PpmSetChannels( void )
{
   for(int i=0; i<NB_SERVOS; i++)
   {
     ppmEncoder.setChannel(i, Servos_us_pui16[i]);
   }
}

float m_IO_ExpoCurve(float f_In, float f_Expo)
{
   float retVal;

   if( f_In > +1.0f ) f_In = +1.0f;
   if( f_In < -1.0f ) f_In = -1.0f;

   if( f_In > 0.0f )
      retVal =         pow(        f_In, abs(f_Expo) );
   else
      retVal = (-1.0f)*pow((-1.0f)*f_In, abs(f_Expo) );

   return retVal;
}
