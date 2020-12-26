#include "IO.h"

#include "RC_PPMEncoder.h"


input_tst Inputs_pst[NB_INPUTS] = {  // pst = pointer to structure
         /* pin,  raw, mV, min_mV, med_mV, max_mV, value*/
  /*0*/  { PIN_NA, 0,  0,    01,     02,     03,  +1.0f},  /**< constant                               */
  /*1*/  { A3,     0,  0,  1001,   2500,   4000,  -0.1f},  /**< Stick Right Horizontal (Ailerons)      */
  /*2*/  { A0,     0,  0,  1002,   2500,   4000,  -0.2f},  /**< Stick Left  Vertical   (Profondeur)    */
  /*3*/  { A1,     0,  0,  1003,   2500,   4000,   0.0f},  /**< Stick Left  Horizontal (Derive)        */
  /*4*/  { A2,     0,  0,  1004,   2500,   4000,   0.0f},  /**< Stick Right Vertical   (Gaz)           */
  /*5*/  { A6,     0,  0,  1005,   2500,   4000,   0.0f},  /**< Trim Right Horizontal (Ailerons)       */
  /*6*/  { A7,     0,  0,  1006,   2500,   4000,   0.0f},  /**< Trim Left  Vertical   (Profondeur)     */
  /*7*/  { PIN_NA, 0,  0,  1007,   2500,   4000,   0.0f},  /**< Trim Left  Horizontal (Derive)         */
  /*8*/  { PIN_NA, 0,  0,  1008,   2500,   4000,   0.0f},  /**< Trim Right Vertical   (Gaz)            */
  /*9*/  { PIN_NA, 0,  0,  1009,   2500,   4000,   0.0f},  /**< Aux 1  */
  /*10*/ { PIN_NA, 0,  0,  1000,   2500,   4000,   0.0f}   /**< Aux 2  */
};



enum{
   crv_normal_em=0,
   crv_expo1_em,
   crv_expo2_em,
   crv_expo3_em,
   crv_expo4_em,
   crv_expo5_em,
   crv_perso1_em,
   crv_perso2_em,
   crv_perso3_em,
   crv_perso4_em
};

enum{
   val_never_em=0,
   val_always_em,
   crv_bp1_em,
   crv_bp2_em,
   crv_bp3_em,
   crv_bp4_em
};



mixers_tst Mixers_pst[NB_MIXERS]={
        /*   out,    input,  curve,          coef,   valid */
 /*0*/  {    0,      0,      crv_normal_em, +100,    val_always_em}, /* dummy        */
 /*1*/  {    1,      1,      crv_expo3_em,  +100,    val_always_em}, /* Ailerons     */
 /*2*/  {    2,      2,      crv_expo3_em,  +100,    val_always_em}, /* Profondeur   */
 /*3*/  {    3,      3,      crv_expo3_em,  +100,    val_always_em}, /* Derive       */
 /*4*/  {    4,      4,      crv_normal_em, +100,    val_always_em}, /* Gaz          */
 /*5*/  {    5,      0,      crv_normal_em, +100,    val_always_em}, /* dummy        */
 /*6*/  {    6,      0,      crv_normal_em, +100,    val_always_em}, /* dummy        */
 /*7*/  {    7,      0,      crv_normal_em, +100,    val_always_em}, /* dummy        */
 /*8*/  {    8,      0,      crv_normal_em, +100,    val_always_em}, /* dummy        */
 /*9*/  {    9,      0,      crv_normal_em, +100,    val_always_em}, /* dummy        */
 /*10*/ {   10,      0,      crv_normal_em, +100,    val_always_em}, /* dummy        */
 /*11*/ {   11,      0,      crv_normal_em, +100,    val_always_em}, /* dummy        */
};


float Outputs_pft[NB_OUTPUTS];


uint16_t   Servos_us_pui16[NB_SERVOS];
servos_tst Servos_pst[NB_SERVOS]={
      /*   out,    trim_idx,   trim_coef,  min_us, med_us, max_us */
 /*0*/   { 1,      5,           25,        1100,   1600,   2100},  /* Ailerons 1 */
 /*1*/   { 2,      6,           25,        1100,   1600,   2100},  /* Profondeur */
 /*2*/   { 3,      7,           25,        1100,   1600,   2100},  /* Derive     */
 /*3*/   { 4,      0,            0,        1100,   1600,   2100},  /* Gaz        */
 /*4*/   { 1,      5,           25,        1100,   1600,   2100},  /* Ailerons 2 */
 /*5*/   { 0,      0,            0,        1100,   1600,   2100},  /* na */
 /*6*/   { 1,      1,           25,        1100,   1600,   2100},  /* na */
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
     if( Inputs_pst[i].pin_ui8 != PIN_NA)
        pinMode(Inputs_pst[i].pin_ui8, INPUT);
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
      if( Inputs_pst[i].pin_ui8 != PIN_NA )
      {
         Inputs_pst[i].adc_raw_ui16 = analogRead(Inputs_pst[i].pin_ui8);
         Inputs_pst[i].adc_mV_ui16  = ((float)Inputs_pst[i].adc_raw_ui16*SCALE_mV_per_ADC);
         if( Inputs_pst[i].adc_mV_ui16 < Inputs_pst[i].med_mV_ui16 )
         {
            l_adc_mV_ui16 = max( Inputs_pst[i].min_mV_ui16 , Inputs_pst[i].adc_mV_ui16 ); // limit to min
            l_dx_ft    = l_adc_mV_ui16 - Inputs_pst[i].min_mV_ui16; // always positive
            l_fullx_ft = Inputs_pst[i].med_mV_ui16 - Inputs_pst[i].min_mV_ui16; // always positive
            Inputs_pst[i].val_ft = (l_dx_ft/l_fullx_ft) + (-1.0f);
         }
         else //  Inputs_pst[i].adc_mV_ui16 >= Inputs_pst[i].med_mV_ui16 )
         {
            l_adc_mV_ui16 = min(Inputs_pst[i].adc_mV_ui16, Inputs_pst[i].max_mV_ui16);  // limit to max
            l_dx_ft    = l_adc_mV_ui16 - Inputs_pst[i].med_mV_ui16; // always positive
            l_fullx_ft = Inputs_pst[i].max_mV_ui16 - Inputs_pst[i].med_mV_ui16; // always positive
            Inputs_pst[i].val_ft = (l_dx_ft/l_fullx_ft) + (0.0f);
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
      if( Mixers_pst[i].valid_ui8 == val_never_em ) /* TODO, only the never case is handled for now */
      {
         break; // skip
      }
      else
      {
         //All other case considered as always!
      }

      // get related input
      l_In_ft = Inputs_pst[Mixers_pst[i].in_idx_ui8].val_ft;

      // apply curve
      switch(Mixers_pst[i].curve_ui8)
      {
         case crv_normal_em: l_Out_ft = l_In_ft;                            break;
         case crv_expo1_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 1.50f ); break;
         case crv_expo2_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 1.75f ); break;
         case crv_expo3_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 2.00f ); break;
         case crv_expo4_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 2.40f ); break;
         case crv_expo5_em:  l_Out_ft =  m_IO_ExpoCurve( l_In_ft , 3.00f ); break;
         case crv_perso1_em: l_Out_ft = l_In_ft;                            break;
         case crv_perso2_em: l_Out_ft = l_In_ft;                            break;
         case crv_perso3_em: l_Out_ft = l_In_ft;                            break;
         case crv_perso4_em: l_Out_ft = l_In_ft;                            break;
         default:            l_Out_ft = l_In_ft;                            break;
      }

      // apply coef
      l_Out_ft *= (float)Mixers_pst[i].coef_si8;
      l_Out_ft /= 100.0f;

      // Add mixer result to the requested output
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
      l_Trim_ft  = Inputs_pst[ Servos_pst[i].trim_idx_ui8 ].val_ft; // get Trim assigned to the servo
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
   if( f_In > -1.0f ) f_In = -1.0f;

   if( f_In > 0.0f )
      retVal =         pow(        f_In, abs(f_Expo) );
   else
      retVal = (-1.0f)*pow((-1.0f)*f_In, abs(f_Expo) );

   return retVal;
}
