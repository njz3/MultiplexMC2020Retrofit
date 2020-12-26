#include "GUI.h"
#include "IO.h"
#include "Display.h"
#include "RC_PPMEncoder.h"
#include "Resources.h"


int currentEditLine = 0;

int currentDisplayValuesPage = 0;
int currentDisplayValuesMode = 0;

bool ChangeUInt16(uint16_t *pVal, uint16_t stp, uint16_t min, uint16_t max) {
   bool edited = false;
   if (pVal==NULL)
      return edited;

   if( IS_PRESSED(BUTTONS_ID::BTN_PLUS) )
   {
      if(*pVal != max) // if not already equal to max
      {
         edited = true;
         if(*pVal < max-stp) // precheck overflow (avoid bug if max >= 32768 - step)
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
         if( min+stp < *pVal)  // precheck underflow
            *pVal -= stp;
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
   if (currentEditLine > max)
   {
      currentEditLine = min;
   }
   Body.SetCursor(0, currentEditLine);
   Body.Print(">");
}

uint16_t g_CalibInput_select;
uint16_t g_ProcessGUI_Cnt=0;
void MakeDisplay_CalibInput( uint16_t input )
{
   g_CalibInput_select = constrain( input , 0 , NB_INPUTS-1);
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line_uint16(  0, " Calib. Input "  , mStrValue2d , &g_CalibInput_select);
   Body.Lines[1] = (display_line*)new display_line_ft100(   1, "   Val= ", mStrValue3Pct, &Inputs_pst[g_CalibInput_select].val_ft);
   Body.Lines[2] = (display_line*)new display_line_uint16(  2, "   Val= ", mStrValue4mV,  &Inputs_pst[g_CalibInput_select].adc_mV_ui16);
   Body.Lines[3] = (display_line*)new display_line_uint16(  3, "   Min= ", mStrValue4mV,  &Inputs_pst[g_CalibInput_select].min_mV_ui16);
   Body.Lines[4] = (display_line*)new display_line_uint16(  4, "   Med= ", mStrValue4mV,  &Inputs_pst[g_CalibInput_select].med_mV_ui16);
   Body.Lines[5] = (display_line*)new display_line_uint16(  5, "   Max= ", mStrValue4mV,  &Inputs_pst[g_CalibInput_select].max_mV_ui16);
   Body.Lines[6] = (display_line*)new display_line_uint16(  6, "          ", mStrValue05d,  &g_ProcessGUI_Cnt);
}

uint16_t g_CalibServo_select;
void MakeDisplay_CalibServo( uint16_t servo )
{
   g_CalibServo_select = constrain( servo , 0 ,  NB_SERVOS-1);
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line_uint16(  0, " Calib. Servo "  , mStrValue2d , &g_CalibServo_select);
   Body.Lines[1] = (display_line*)new display_line_uint16(  1, "   Val= ", mStrValue4us,  &Servos_us_pui16[g_CalibServo_select]);
   Body.Lines[2] = (display_line*)new display_line_uint16(  2, "   Min= ", mStrValue4us, &Servos_pst[g_CalibServo_select].min_us_ui16);
   Body.Lines[3] = (display_line*)new display_line_uint16(  3, "   Med= ", mStrValue4us, &Servos_pst[g_CalibServo_select].med_us_ui16);
   Body.Lines[4] = (display_line*)new display_line_uint16(  4, "   Max= ", mStrValue4us, &Servos_pst[g_CalibServo_select].max_us_ui16);
   Body.Lines[5] = (display_line*)new display_line_uint8(   5, "   SrcOut = ", mStrValue2d, &Servos_pst[g_CalibServo_select].out_idx_ui8);
   Body.Lines[6] = (display_line*)new display_line_uint8(   6, "   TrInput= ", mStrValue2d, &Servos_pst[g_CalibServo_select].trim_idx_ui8);
   Body.Lines[7] = (display_line*)new display_line_int8(    7, "   TrCoef=", mStrValue3Pct, &Servos_pst[g_CalibServo_select].trim_coef_si8);
}


enum PAGES_en : uint8_t
{
   PAGE_INIT=0,
   PAGE_CALIB_INPUTS,
   PAGE_CALIB_SERVOS,
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
         case PAGE_CALIB_INPUTS:
            g_CalibInput_select = 0;
            MakeDisplay_CalibInput(g_CalibInput_select);
            break;

         case PAGE_CALIB_SERVOS:
            g_CalibServo_select=0;
            MakeDisplay_CalibServo(g_CalibServo_select);
            break;
      }
      Body.PrintAllFixed();
   }

   if( currentEditLine == 0 ) /* if the 1st line */
   {
      switch( g_Page )
      {
         case PAGE_CALIB_INPUTS:
            if( ChangeUInt16( &g_CalibInput_select , 1 , 0 , NB_INPUTS-1) )
               MakeDisplay_CalibInput(g_CalibInput_select);
            break;

         case PAGE_CALIB_SERVOS:
            if( ChangeUInt16( &g_CalibServo_select , 1 , 0 , NB_SERVOS-1) )
               MakeDisplay_CalibServo(g_CalibServo_select);
            break;
      }
   }

   switch( g_Page )
   {
      case PAGE_CALIB_INPUTS:
         EditLineCursor(0,5);
         break;

      case PAGE_CALIB_SERVOS:
         EditLineCursor(0,7);
         break;
   }

   Body.RefreshAllNotFixed();
}
