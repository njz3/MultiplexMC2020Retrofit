#include "GUI.h"
#include "IO.h"
#include "Display.h"
#include "RC_PPMEncoder.h"
#include "Resources.h"
#include "Config.h"


int currentEditLine = 0;

int currentDisplayValuesPage = 0;
int currentDisplayValuesMode = 0;

typedef char Name_Type[6]; // caution /0 takes one char  (so max string is 5char long)

Name_Type Inputs_Names[NB_INPUTS]={
      "V0",
      "M1","M2","M3","M4",
      "T1","T2","T3","T4",
      "A5","A6"
};

Name_Type Outputs_Names[NB_OUTPUTS]={
      "xA", "xB", "xC", "xD", "xE", "xF",
      "xG", "xH", "xI", "xJ", "xK", "xL"
};

Name_Type Servos_Names[NB_SERVOS]={
   "S1", "S2", "S3", "S4", "S5", "S6", "S7"
};

Name_Type Mixers_Names[NB_MIXERS]={
      "X01", "X02", "X03", "X04", "X05", "X06",
      "X07", "X08", "X09", "X10", "X11", "X12"
};

Name_Type Curves_Names[curve_max_em]={
      "NRML",
      "EXP1", "EXP2", "EXP3", "EXP4", "EXP5",
      "ABS ", "POS ", "NEG ",
      "PER1", "PER2", "PER3", "PER4"
};


#define md_ChangeUInt16_Adc_mV(pVal)      ChangeUInt16(pVal, gd_STEP_TUNING_Up_mV, gd_STEP_TUNING_Down_mV, gd_MIN_mV, gd_MAX_mV)
#define md_ChangeUInt16_us(pVal)          ChangeUInt16(pVal, gd_STEP_TUNING_Up_us, gd_STEP_TUNING_Down_us, gd_MIN_us, gd_MAX_us)
#define md_ChangeUInt16_Index(pVal,max)   ChangeUInt16(pVal, 1, 1, 0, max)
#define md_ChangeUInt8_Index( pVal,max)   ChangeUInt8( pVal, 1,    0, max)

bool ChangeUInt16(uint16_t *pVal, uint16_t stp_up, uint16_t stp_down, uint16_t min, uint16_t max) {
   bool edited = false;
   if (pVal==NULL)
      return edited;

   if( IS_PRESSED(BUTTONS_ID::BTN_PLUS) )
   {
      if(*pVal != max) // if not already equal to max
      {
         edited = true;
         if(*pVal < max-stp_up) // precheck overflow (avoid bug if max >= 32768 - step)
            *pVal += stp_up;
         else
            *pVal = max;
      }
   }
   else if( IS_PRESSED(BUTTONS_ID::BTN_MINUS) )
   {
      if(*pVal != min) // if not already equal to min
      {
         edited = true;
         if( min+stp_down < *pVal)  // precheck underflow
            *pVal -= stp_down;
         else
            *pVal = min;
      }
   }
  return edited;
}

bool ChangeUInt8(uint8_t *pVal, uint8_t stp, uint8_t min, uint8_t max) {
   bool edited = false;
   if (pVal==NULL)
      return edited;

   if( IS_PRESSED(BUTTONS_ID::BTN_PLUS) )
   {
      if(*pVal != max) // if not already equal to max
      {
         edited = true;
         if(*pVal < max-stp) // precheck overflow (avoid bug if max >= 256 - step)
            *pVal += stp;
         else
            *pVal = max;
      }
   }
   else if( IS_PRESSED(BUTTONS_ID::BTN_MINUS) )
   {
      if(*pVal != min) // if not already equal to min
      {
         edited = true;
         if( min+stp < *pVal)
            *pVal -= stp;
         else
            *pVal = min;
      }
   }
  return edited;
}

bool ChangeInt8(int8_t *pVal, int8_t stp, int8_t min, int8_t max) {
   bool edited = false;
   if (pVal==NULL)
      return edited;

   if( IS_PRESSED(BUTTONS_ID::BTN_PLUS) )
   {
      if(*pVal != max) // if not already equal to max
      {
         edited = true;
         if(*pVal < max-stp) // precheck overflow (avoid bug if max >= 256 - step)
            *pVal += stp;
         else
            *pVal = max;
      }
   }
   else if( IS_PRESSED(BUTTONS_ID::BTN_MINUS) )
   {
      if(*pVal != min) // if not already equal to min
      {
         edited = true;
         if( min+stp < *pVal)
            *pVal -= stp;
         else
            *pVal = min;
      }
   }
  return edited;
}

bool ChangeBitUInt8(uint8_t *pVal, uint8_t bit) {
  bool edited = false;
  if (IS_PUSHED(BUTTONS_ID::BTN_PLUS | BUTTONS_ID::BTN_MINUS)) {
    edited = true;
  }
  if (edited) {
    if ((*pVal & bit)!=0) {
      *pVal &= ~bit;
    } else {
      *pVal |= bit;
    }
  }
  return edited;
}

struct display_line
{
  public:
  const char *fixed;
  byte offset;
  byte row;

  display_line(const byte row, const char *fxd): fixed(fxd) {
    this->row = row;
    if (fixed!=NULL) {
      offset = strlen(fixed);
    } else {
      offset = 0;
    }
  };
  virtual ~display_line(){};
  virtual void print_fixed() const {
    if (fixed!=NULL) {
#ifdef HEADER_FOOTER
       Display.setCursor(0, 2*row+1);
#else
       Display.setCursor(0, 2*row);
#endif // HEADER_FOOTER

      Display.print(fixed);
      refresh_notfixed();
    }
  };
  virtual void refresh_notfixed() const {
#ifdef HEADER_FOOTER
     Display.setCursor(offset, 2*row+1);
#else
     Display.setCursor(offset, 2*row);
#endif // HEADER_FOOTER

  };
};

struct display_line_uint16 : public display_line
{
  public:
  const char *format;
  uint16_t *pval;

  display_line_uint16(const byte row, const char *fxd, const char *fmt, uint16_t *pval): display_line(row, fxd), format(fmt), pval(pval) { };
  virtual ~display_line_uint16(){ };

  virtual void refresh_notfixed() const override  {
    if (format!=NULL) {
      display_line::refresh_notfixed();
      char buf[20];
      sprintf(buf, format, *pval);
      Display.print(buf);
    }
  };
};

struct display_line_uint8 : public display_line
{
  public:
  const char *format;
  uint8_t *pval;

  display_line_uint8(const byte row, const char *fxd, const char *fmt, uint8_t *pval): display_line(row, fxd), format(fmt), pval(pval) { };
  virtual ~display_line_uint8(){ };

  virtual void refresh_notfixed() const override  {
    if (format!=NULL) {
      display_line::refresh_notfixed();
      char buf[20];
      sprintf(buf, format, *pval);
      Display.print(buf);
    }
  };
};

struct display_line_uint8_names : public display_line
{
  public:
  const char *format;
  uint8_t *pval;
  Name_Type *names;

  display_line_uint8_names(const byte row, const char *fxd, const char *fmt, uint8_t *pval, Name_Type *names):
     display_line(row, fxd), format(fmt), pval(pval), names(names) { };
  virtual ~display_line_uint8_names(){ };

  virtual void refresh_notfixed() const override  {
    if (format!=NULL) {
      display_line::refresh_notfixed();
      char buf[20];
      sprintf(buf, format, names[*pval]);
      Display.print(buf);
    }
  };
};

struct display_line_int8 : public display_line
{
  public:
  const char *format;
  int8_t *pval;

  display_line_int8(const byte row, const char *fxd, const char *fmt, int8_t *pval): display_line(row, fxd), format(fmt), pval(pval) { };
  virtual ~display_line_int8(){ };

  virtual void refresh_notfixed() const override  {
    if (format!=NULL) {
      display_line::refresh_notfixed();
      char buf[20];
      sprintf(buf, format, *pval);
      Display.print(buf);
    }
  };
};

struct display_line_ft100 : public display_line
{
  public:
  const char *format;
  float *pval;

  display_line_ft100(const byte row, const char *fxd, const char *fmt, float *pval): display_line(row, fxd), format(fmt), pval(pval) { };
  virtual ~display_line_ft100(){ };

  virtual void refresh_notfixed() const override  {
    if (format!=NULL) {
      display_line::refresh_notfixed();
      char buf[20];
      int i = (*pval)*100;
      sprintf(buf, format, i);
      Display.print(buf);
    }
  };
};

struct display_line_str : public display_line
{
  public:
  const char *format;
  const char *str;

  display_line_str(const byte row, const char *fxd, const char *fmt, const char *strg): display_line(row, fxd), format(fmt), str(strg) { };
  virtual ~display_line_str(){ };

  virtual void refresh_notfixed() const override {
    if (format!=NULL) {
      display_line::refresh_notfixed();
      char buf[20];
      sprintf(buf, format, str);
      Display.print(buf);
    }
  };
};

struct ScreenBody
{
  public:
#ifdef HEADER_FOOTER
   const static int MAX_BODY_LINES = 7;
#else
  const static int MAX_BODY_LINES = 8;
#endif // HEADER_FOOTER
  display_line* Lines[MAX_BODY_LINES] = {0};

  void Delete() {
  for (int i=0; i<(int)(sizeof(Lines)/sizeof(Lines[0])); i++) {
      if (Lines[i]!=NULL) {
        delete Lines[i];
        Lines[i] = NULL;
      }
    }
  };

  void SetCursor(byte col, byte row) {
#ifdef HEADER_FOOTER
     Display.setCursor(col, 2*row+1);
#else
     Display.setCursor(col, 2*row);
#endif // HEADER_FOOTER

  };

  void Print(const char *str) {
    Display.print(str);
  };

  void PrintAllFixed() {
    for(int row=0; row<MAX_BODY_LINES; row++) {
      if (Lines[row]!=NULL)
        Lines[row]->print_fixed();
      else
#ifdef HEADER_FOOTER
         Display.clearLine(2*row+1);
#else
      Display.clearLine(2*row);
#endif // HEADER_FOOTER

    }
  };

  void RefreshAllNotFixed() {
    for(int row=0; row<MAX_BODY_LINES; row++) {
      if (Lines[row]!=NULL) {
        Lines[row]->refresh_notfixed();
      }
    }
  };
};

ScreenBody Body;

void EditLineCursor( int min, int max )
{
   if( IS_PUSHED(BUTTONS_ID::BTN_NEXT) )
   {
      Body.SetCursor(0, currentEditLine);
      Body.Print(" ");
      currentEditLine += 1;
   }
   if (currentEditLine < min || max < currentEditLine ) // if out of range reinit to min
   {
      currentEditLine = min;
   }
   Body.SetCursor(0, currentEditLine);
   Body.Print(">");
}

uint8_t g_CalibInput_select_ui8;
uint16_t g_ProcessGUI_Cnt=0;
void MakeDisplay_CalibInput( )
{
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line_uint8_names( 0, " Calib. Input " , mStr2Char , &g_CalibInput_select_ui8, Inputs_Names );
   Body.Lines[1] = (display_line*)new display_line_ft100(       1, "   Val= ", mStrValue3Pct, &Inputs_var_pst[g_CalibInput_select_ui8].val_ft);
   Body.Lines[2] = (display_line*)new display_line_uint16(      2, "   Val= ", mStrValue4mV,  &Inputs_var_pst[g_CalibInput_select_ui8].adc_mV_ui16);
   Body.Lines[3] = (display_line*)new display_line_uint16(      3, "   Min= ", mStrValue4mV,  &Inputs_cfg_pst[g_CalibInput_select_ui8].min_mV_ui16);
   Body.Lines[4] = (display_line*)new display_line_uint16(      4, "   Med= ", mStrValue4mV,  &Inputs_cfg_pst[g_CalibInput_select_ui8].med_mV_ui16);
   Body.Lines[5] = (display_line*)new display_line_uint16(      5, "   Max= ", mStrValue4mV,  &Inputs_cfg_pst[g_CalibInput_select_ui8].max_mV_ui16);
   Body.Lines[6] = (display_line*)new display_line_uint16(      6, "        ", mStrValue05d,  &g_ProcessGUI_Cnt);
}

uint8_t g_CalibServo_select_ui8;
void MakeDisplay_CalibServo( )
{
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line_uint8_names( 0, " Calib. Servo "  , mStr2Char , &g_CalibServo_select_ui8, Servos_Names );
   Body.Lines[1] = (display_line*)new display_line_uint16(      1, "   Val= ", mStrValue4us,  &Servos_us_pui16[g_CalibServo_select_ui8]);
   Body.Lines[2] = (display_line*)new display_line_uint16(      2, "   Min= ", mStrValue4us, &Servos_pst[g_CalibServo_select_ui8].min_us_ui16);
   Body.Lines[3] = (display_line*)new display_line_uint16(      3, "   Med= ", mStrValue4us, &Servos_pst[g_CalibServo_select_ui8].med_us_ui16);
   Body.Lines[4] = (display_line*)new display_line_uint16(      4, "   Max= ", mStrValue4us, &Servos_pst[g_CalibServo_select_ui8].max_us_ui16);
   Body.Lines[5] = (display_line*)new display_line_uint8_names( 5, "   Source = ", mStr2Char, &Servos_pst[g_CalibServo_select_ui8].out_idx_ui8, Outputs_Names);
   Body.Lines[6] = (display_line*)new display_line_uint8_names( 6, "   TrInput= ", mStr2Char,   &Servos_pst[g_CalibServo_select_ui8].trim_idx_ui8, Inputs_Names );
   Body.Lines[7] = (display_line*)new display_line_int8(        7, "   TrCoef=", mStrValue3Pct, &Servos_pst[g_CalibServo_select_ui8].trim_coef_si8);
}

uint8_t g_CalibMixer_select_ui8;
void MakeDisplay_CalibMixer( )
{
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line_uint8_names( 0, " Calib Mixer " , mStr2Char, &g_CalibMixer_select_ui8 , Mixers_Names);
   Body.Lines[1] = (display_line*)new display_line_uint8_names( 1, "   Input = ", mStr2Char,     &Mixers_pst[g_CalibMixer_select_ui8].in_idx_ui8, Inputs_Names );
   Body.Lines[2] = (display_line*)new display_line_uint8_names( 2, "   Courbe= ", mStr2Char,     &Mixers_pst[g_CalibMixer_select_ui8].curve_ui8 , Curves_Names );
   Body.Lines[3] = (display_line*)new display_line_int8(        3, "   Coef  = ", mStrValue3Pct, &Mixers_pst[g_CalibMixer_select_ui8].coef_si8);
   Body.Lines[4] = (display_line*)new display_line_uint8(       4, "   Valid = ", mStrValue2d,   &Mixers_pst[g_CalibMixer_select_ui8].valid_ui8);
   Body.Lines[5] = (display_line*)new display_line_uint8_names( 5, "   MixOut= ", mStr2Char,     &Mixers_pst[g_CalibMixer_select_ui8].out_idx_ui8, Outputs_Names);
}

void Edit_CalibInput( )
{
   switch( currentEditLine )
   {
      case 3:  md_ChangeUInt16_Adc_mV( &Inputs_cfg_pst[g_CalibInput_select_ui8].min_mV_ui16 ); break;
      case 4:  md_ChangeUInt16_Adc_mV( &Inputs_cfg_pst[g_CalibInput_select_ui8].med_mV_ui16 ); break;
      case 5:  md_ChangeUInt16_Adc_mV( &Inputs_cfg_pst[g_CalibInput_select_ui8].max_mV_ui16 ); break;
      default: break;
   }
}

void Edit_CalibServo( )
{
   switch( currentEditLine )
   {
      case 2:  md_ChangeUInt16_Adc_mV( &Servos_pst[g_CalibServo_select_ui8].min_us_ui16 ); break;
      case 3:  md_ChangeUInt16_Adc_mV( &Servos_pst[g_CalibServo_select_ui8].med_us_ui16 ); break;
      case 4:  md_ChangeUInt16_Adc_mV( &Servos_pst[g_CalibServo_select_ui8].max_us_ui16 ); break;
      case 5:  ChangeUInt8(  &Servos_pst[g_CalibServo_select_ui8].out_idx_ui8  , 1 ,   0 , NB_OUTPUTS-1 ); break;
      case 6:  ChangeUInt8(  &Servos_pst[g_CalibServo_select_ui8].trim_idx_ui8 , 1 ,   0 , NB_INPUTS-1  ); break;
      case 7:  ChangeInt8(   &Servos_pst[g_CalibServo_select_ui8].trim_coef_si8, 5 , -95 , 95); break;
      default: break;
   }
}

void Edit_CalibMixer( )
{
   switch( currentEditLine )
   {
      case 1:  ChangeUInt8(  &Mixers_pst[g_CalibMixer_select_ui8].in_idx_ui8  , 1 ,    0 , NB_INPUTS-1        );     break;
      case 2:  ChangeUInt8(  &Mixers_pst[g_CalibMixer_select_ui8].curve_ui8   , 1 ,    0 , curve_max_em-1     );     break;
      case 3:  ChangeInt8(   &Mixers_pst[g_CalibMixer_select_ui8].coef_si8    , 1 , -120 , 120                );     break;
      case 4:  ChangeUInt8(  &Mixers_pst[g_CalibMixer_select_ui8].valid_ui8   , 1 ,    0 , validity_max_em-1  );     break;
      case 5:  ChangeUInt8(  &Mixers_pst[g_CalibMixer_select_ui8].out_idx_ui8 , 1 ,    0 , NB_OUTPUTS-1       );     break;
   }
}

void MakeDisplay_ShowInputs( )
{
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line( 0, "Affich. Inputs" );

   Body.Lines[1] = (display_line*)new display_line( 1, "V0....% M1....%" );
   Body.Lines[2] = (display_line*)new display_line( 2, "M2....% M3....%" );
   Body.Lines[3] = (display_line*)new display_line( 3, "M4....% T1....%" );
   Body.Lines[4] = (display_line*)new display_line( 4, "T2....% T3....%" );
   Body.Lines[5] = (display_line*)new display_line( 5, "T4....% A5....%" );
   Body.Lines[6] = (display_line*)new display_line( 6, "A6....%" );
}

void MakeDisplay_ShowOutputs( )
{
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line( 0, "Affich. MixOuts" );
   Body.Lines[1] = (display_line*)new display_line( 1, "xA....% xB....%" );
   Body.Lines[2] = (display_line*)new display_line( 2, "xC....% xD....%" );
   Body.Lines[3] = (display_line*)new display_line( 3, "xE....% xF....%" );
   Body.Lines[4] = (display_line*)new display_line( 4, "xG....% xH....%" );
   Body.Lines[5] = (display_line*)new display_line( 5, "xI....% xJ....%" );
   Body.Lines[6] = (display_line*)new display_line( 6, "xK....% xL....%" );
}

void MakeDisplay_ShowServos( )
{
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line( 0, "Affich. Servos" );
   Body.Lines[1] = (display_line*)new display_line( 1, "S1 +...% ....us" );
   Body.Lines[2] = (display_line*)new display_line( 2, "S2" );
   Body.Lines[3] = (display_line*)new display_line( 3, "S3" );
   Body.Lines[4] = (display_line*)new display_line( 4, "S4" );
   Body.Lines[5] = (display_line*)new display_line( 5, "S5" );
   Body.Lines[6] = (display_line*)new display_line( 6, "S6" );
   Body.Lines[7] = (display_line*)new display_line( 7, "S7" );
}

void Update_ShowInputs( )
{
   char buf[8];
   int val;
   for( int i=0 ; i<NB_INPUTS ; i++ )
   {
      val =  Inputs_var_pst[i].val_ft * 100.0f;
      sprintf(buf, mStrValue3Pct, val );

      if( i&1 ) // if odd
         Display.setCursor( 8+2, (i/2)*2+2 );
      else // if even
         Display.setCursor(  2, (i/2)*2+2 );

      Display.print(buf);
   }
}

void Update_ShowOutputs( )
{
   char buf[8];
   int val;
   for( int i=0 ; i<NB_OUTPUTS ; i++ )
   {
      val =  Outputs_pft[i] * 100.0f;
      sprintf(buf, mStrValue3Pct, val);

      if( i&1 ) // if odd
         Display.setCursor( 8+2, (i/2)*2+2 );
      else // if even
         Display.setCursor(  2, (i/2)*2+2 );

      Display.print(buf);
   }
}

void Update_ShowServos( )
{
   char buf[8];
   int val;
   for( int i=0 ; i<NB_SERVOS ; i++ )
   {
      val =  Outputs_pft[Servos_pst[i].out_idx_ui8] * 100.0f;
      sprintf(buf, mStrValue3Pct,val);
      Display.setCursor( 3, i*2+2 );
      Display.print(buf);

      sprintf(buf, mStrValue4us, Servos_us_pui16[i]);
      Display.setCursor( 9, i*2+2 );
      Display.print(buf);
   }
}

void MakeDisplay_ShowSetting( )
{
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line( 0, " Setting" );
   Body.Lines[1] = (display_line*)new display_line( 1, "  Mem0 Load" );
   Body.Lines[2] = (display_line*)new display_line( 2, "  Mem0 Save" );
}

void Edit_Setting( )
{
   switch( currentEditLine )
   {
      case 1:
         if( IS_PUSHED(BUTTONS_ID::BTN_PLUS) )
         {
            Display.setCursor( 12, currentEditLine*2 );
            if( 0 == Config::LoadConfigFromEEPROM() )
               Display.print("Ok");
            else
               Display.print("Nok");
         }
         break;

      case 2:
         if( IS_PUSHED(BUTTONS_ID::BTN_PLUS) )
         {
            Config::SaveConfigToEEPROM();
            Display.setCursor( 12, currentEditLine*2 );
            Display.print("Done");
         }
         break;

      default:
         break;
   }
}


enum PAGES_en : uint8_t
{
   PAGE_INIT=0,
   PAGE_SHOW_INPUTS,
   PAGE_SHOW_OUTPUTS,
   PAGE_SHOW_SERVOS,
   PAGE_CALIB_INPUTS,
   PAGE_CALIB_SERVOS,
   PAGE_CALIB_MIXERS,
   PAGE_SETTING,
   PAGE_MAX
};

uint8_t g_Page = PAGE_INIT;

void ProcessGUI()
{
   g_ProcessGUI_Cnt++;

   if( IS_PUSHED(BUTTONS_ID::BTN_PAGE) || g_Page == PAGE_INIT )
   {
      Display.clearBody();
      if (++g_Page >= PAGE_MAX)
         g_Page = 1;

      switch( g_Page )
      {
         case PAGE_SHOW_INPUTS:
            MakeDisplay_ShowInputs();
            break;

         case PAGE_SHOW_OUTPUTS:
            MakeDisplay_ShowOutputs();
            break;

         case PAGE_SHOW_SERVOS:
            MakeDisplay_ShowServos();
            break;

         case PAGE_CALIB_INPUTS:
            g_CalibInput_select_ui8 = 0;
            MakeDisplay_CalibInput();
            break;

         case PAGE_CALIB_SERVOS:
            g_CalibServo_select_ui8=0;
            MakeDisplay_CalibServo();
            break;

         case PAGE_CALIB_MIXERS:
            g_CalibMixer_select_ui8=0;
            MakeDisplay_CalibMixer();
            break;

         case PAGE_SETTING:
            MakeDisplay_ShowSetting();
            break;
      }
      currentEditLine = 0;
      Body.PrintAllFixed();
   }


   switch( g_Page )
   {
      case PAGE_SHOW_INPUTS:
         Update_ShowInputs();
         break;

      case PAGE_SHOW_OUTPUTS:
         Update_ShowOutputs();
         break;

      case PAGE_SHOW_SERVOS:
         Update_ShowServos();
         break;

      case PAGE_CALIB_INPUTS:
         if( currentEditLine == 0 ) /* if the 1st line */
         {
            if( md_ChangeUInt8_Index( &g_CalibInput_select_ui8 , NB_INPUTS-1) )
               MakeDisplay_CalibInput();
         }
         else
         {
            Edit_CalibInput();
         }
         EditLineCursor(0,5);
         break;

      case PAGE_CALIB_SERVOS:
         if( currentEditLine == 0 ) /* if the 1st line */
         {
            if( md_ChangeUInt8_Index( &g_CalibServo_select_ui8 , NB_SERVOS-1) )
               MakeDisplay_CalibServo();
         }
         else
         {
            Edit_CalibServo();
         }
         EditLineCursor(0,7);
         break;

      case PAGE_CALIB_MIXERS:
         if( currentEditLine == 0 ) /* if the 1st line */
         {
            if( md_ChangeUInt8_Index( &g_CalibMixer_select_ui8 , NB_MIXERS-1) )
               MakeDisplay_CalibMixer();
         }
         else
         {
            Edit_CalibMixer();
         }
         EditLineCursor(0,5);
         break;

      case PAGE_SETTING:
         Edit_Setting();
         EditLineCursor(1,2);
         break;
   }

   Body.RefreshAllNotFixed();
}
