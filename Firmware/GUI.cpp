#include "GUI.h"
#include "IO.h"
#include "Display.h"
#include "RC_PPMEncoder.h"
#include "Resources.h"


int currentEditLine = 0;

int currentDisplayValuesPage = 0;
int currentDisplayValuesMode = 0;

bool ChangeInt16(int16_t *pVal, int16_t stp, int16_t min, int16_t max) {
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
      Display.setCursor(0, 2*row+1);
      Display.print(fixed);
      refresh_notfixed();
    }
  };
  virtual void refresh_notfixed() const {
    Display.setCursor(offset, 2*row+1);
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
  const static int MAX_BODY_LINES = 7;
  display_line* Lines[MAX_BODY_LINES] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

  void Delete() {
  for (int i=0; i<(int)(sizeof(Lines)/sizeof(Lines[0])); i++) {
      if (Lines[i]!=NULL) {
        delete Lines[i];
        Lines[i] = NULL;
      }
    }
  };

  void SetCursor(byte col, byte row) {
    Display.setCursor(col, 2*row+1);
  };

  void Print(const char *str) {
    Display.print(str);
  };

  void PrintAllFixed() {
    for(int row=0; row<MAX_BODY_LINES; row++) {
      if (Lines[row]!=NULL)
        Lines[row]->print_fixed();
      else
        Display.clearLine(2*row+1);
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

void MakeDisplayCurrentValuesPage()
{
#if 0 // TODO screen
  char buff[20];
  sprintf(buff, mFooterValues, currentDisplayValuesPage);
  Display.setFooter(buff);
  Display.refreshFooter();

  Body.Delete();
  for (byte i=0; i<ScreenBody::MAX_BODY_LINES; i++) {
    int idx = (currentDisplayValuesPage*7) + i;
    if (idx<Config::ConfigFile.NBchannels) {
      switch(currentDisplayValuesMode) {
        case 0:
        default:
        Body.Lines[i] = (display_line*)new display_line_uint16(i, Config::ConfigFile.channels[idx].name, mStrValue3Pct, &chan_pct[idx]);
        break;
        case 1:
        Body.Lines[i] = (display_line*)new display_line_uint16(i, Config::ConfigFile.channels[idx].name, mStrValue4mV, &adc_mv[idx]);
        break;
        case 2:
        Body.Lines[i] = (display_line*)new display_line_uint16(i, Config::ConfigFile.channels[idx].name, mStrValue4mV, &chan_mv[idx]);
        break;
        case 3:
        Body.Lines[i] = (display_line*)new display_line_uint16(i, Config::ConfigFile.channels[idx].name, mStrValue4us, &chan_us[idx]);
        break;
      }
    }
  }
#endif
}

void DisplayCurrentValues(PRINT_MODES print_mode) {

  if (IS_PUSHED(BUTTONS_ID::BTN_NEXT)) {
    currentDisplayValuesPage += 1;
    int nb_pages = (Config::ConfigFile.NBchannels-1)/7;
    if (currentDisplayValuesPage>nb_pages)
      currentDisplayValuesPage = 0;
    MakeDisplayCurrentValuesPage();
    print_mode=PRINT_MODES::PRINT;
  }
  if (IS_PUSHED(BUTTONS_ID::BTN_PLUS)) {
    currentDisplayValuesMode += 1;
    if (currentDisplayValuesMode>=4)
      currentDisplayValuesMode = 3;
    MakeDisplayCurrentValuesPage();
    print_mode=PRINT_MODES::PRINT;
  }
  if (IS_PUSHED(BUTTONS_ID::BTN_MINUS)) {
    currentDisplayValuesMode -= 1;
    if (currentDisplayValuesMode<0)
      currentDisplayValuesMode = 0;
    MakeDisplayCurrentValuesPage();
    print_mode=PRINT_MODES::PRINT;
  }

  if (print_mode==PRINT_MODES::PRINT) {
    MakeDisplayCurrentValuesPage();
    Body.PrintAllFixed();
  } else {
    Body.RefreshAllNotFixed();
  }
}

int16_t currentDisplayChannelPage = 0;

void MakeDisplayChannelPage()
{
#if 0 // TODO screen
  char buff[20];
  sprintf(buff, mFooterChan, Config::ConfigFile.channels[currentDisplayChannelPage].name);
  Display.setFooter(buff);
  Display.refreshFooter();

  Body.Delete();
  Body.Lines[0] = (display_line*)new display_line_str(0, " Name: ", "%s  ", Config::ConfigFile.channels[currentDisplayChannelPage].name);
  switch(currentDisplayValuesMode) {
    case 0:
    default:
    Body.Lines[1] = (display_line*)new display_line_uint16(1, " P=", mStrValue3Pct, &chan_pct[currentDisplayChannelPage]);
    break;
    case 1:
    Body.Lines[1] = (display_line*)new display_line_uint16(1, " A=", mStrValue4mV, &adc_mv[currentDisplayChannelPage]);
    break;
    case 2:
    Body.Lines[1] = (display_line*)new display_line_uint16(1, " C=", mStrValue4mV, &chan_mv[currentDisplayChannelPage]);
    break;
    case 3:
    Body.Lines[1] = (display_line*)new display_line_uint16(1, " T=", mStrValue4us, &chan_us[currentDisplayChannelPage]);
    break;
  }
  Body.Lines[2] = (display_line*)new display_line_uint16(2, " Min.A=", mStrValue4mV, &Config::ConfigFile.channels[currentDisplayChannelPage].min_mV);
  Body.Lines[3] = (display_line*)new display_line_uint16(3, " Max.A=", mStrValue4mV, &Config::ConfigFile.channels[currentDisplayChannelPage].max_mV);
  Body.Lines[4] = (display_line*)new display_line_uint16(4, " TrimA=", mStrValue4mV, &Config::ConfigFile.channels[currentDisplayChannelPage].trim_mV);
  Body.Lines[5] = (display_line*)new display_line_uint16(5, " Min.T=", mStrValue4us, &Config::ConfigFile.channels[currentDisplayChannelPage].min_us);
  Body.Lines[6] = (display_line*)new display_line_uint16(6, " Max.T=", mStrValue4us, &Config::ConfigFile.channels[currentDisplayChannelPage].max_us);
  //Body.Lines[6] = (display_line*)new display_line_int(6, " TrimT=", mStrValue4us, &Config::ConfigFile.channels[currentDisplayChannelPage].trim_us);
#endif // 0
}

void DisplayChannels(PRINT_MODES print_mode) {
  if (IS_PUSHED(BUTTONS_ID::BTN_NEXT)) {
    Body.SetCursor(0, currentEditLine);
    Body.Print(" ");
    currentEditLine += 1;
  }
  if (currentEditLine>=7) {
    currentEditLine = 0;
  }
  if (currentDisplayChannelPage>=Config::ConfigFile.NBchannels) {
    currentDisplayChannelPage = Config::ConfigFile.NBchannels-1;
    print_mode=PRINT_MODES::PRINT;
  }

  if (print_mode==PRINT_MODES::PRINT) {
    MakeDisplayChannelPage();
    Body.PrintAllFixed();
  } else {
    Body.RefreshAllNotFixed();
  }

  int idx = currentDisplayChannelPage;
  Body.SetCursor(0, currentEditLine);
  Body.Print(">");
  bool edited = false;
  switch(currentEditLine) {
    case 0: {
      edited = ChangeInt16(&currentDisplayChannelPage,
        1,
        0,
        Config::ConfigFile.NBchannels-1);
    }
    break;
    case 1: {
      if (IS_PUSHED(BUTTONS_ID::BTN_PLUS)) {
        currentDisplayValuesMode += 1;
        if (currentDisplayValuesMode>=4)
          currentDisplayValuesMode = 3;
        edited = true;
      }if (IS_PUSHED(BUTTONS_ID::BTN_MINUS)) {
        currentDisplayValuesMode -= 1;
        if (currentDisplayValuesMode<0)
          currentDisplayValuesMode = 0;
        edited = true;
      }
    }
    break;
    case 2: {
      ChangeInt16(&Config::ConfigFile.channels[idx].min_mV,
        STEP_TUNING_mV,
        0,
        Config::ConfigFile.channels[idx].max_mV);
    }
    break;
    case 3: {
      ChangeInt16(&Config::ConfigFile.channels[idx].max_mV,
        STEP_TUNING_mV,
        Config::ConfigFile.channels[idx].min_mV,
        5000);
    }
    break;
    case 4: {
      ChangeInt16(&Config::ConfigFile.channels[idx].trim_mV,
        STEP_TUNING_mV,
        Config::ConfigFile.channels[idx].min_mV,
        Config::ConfigFile.channels[idx].max_mV);
    }
    break;
    case 5: {
      ChangeInt16(&Config::ConfigFile.channels[idx].min_us,
        STEP_TUNING_us,
        0,
        Config::ConfigFile.channels[idx].max_us);
    }
    break;
    case 6: {
      ChangeInt16(&Config::ConfigFile.channels[idx].max_us,
        STEP_TUNING_us,
        Config::ConfigFile.channels[idx].min_us,
        5000);
    }
    break;
    case 7: {
      ChangeInt16(&Config::ConfigFile.channels[idx].trim_us,
        STEP_TUNING_us,
        -((Config::ConfigFile.channels[idx].min_us+Config::ConfigFile.channels[idx].max_us)>>1),
        (Config::ConfigFile.channels[idx].min_us+Config::ConfigFile.channels[idx].max_us)>>1);
    }
    break;
    /*
    case 7: {
      if (IS_PUSHED(BUTTONS_ID::BTN_PLUS)) {
        Config::ConfigFile.channels[idx].min_mV += STEP_TUNING_mV;
        Config::ConfigFile.channels[idx].max_mV += STEP_TUNING_mV;
      }
      if (IS_PUSHED(BUTTONS_ID::BTN_MINUS)) {
        Config::ConfigFile.channels[idx].min_mV -= STEP_TUNING_mV;
        Config::ConfigFile.channels[idx].max_mV -= STEP_TUNING_mV;
      }
      if (Config::ConfigFile.channels[idx].min_mV<0)
        Config::ConfigFile.channels[idx].min_mV = 0;
      if (Config::ConfigFile.channels[idx].max_mV>5000)
        Config::ConfigFile.channels[idx].max_mV = 5000;
      if (Config::ConfigFile.channels[idx].min_mV>Config::ConfigFile.channels[idx].max_mV)
        Config::ConfigFile.channels[idx].min_mV = Config::ConfigFile.channels[idx].max_mV;
      if (Config::ConfigFile.channels[idx].max_mV<Config::ConfigFile.channels[idx].min_mV)
        Config::ConfigFile.channels[idx].max_mV = Config::ConfigFile.channels[idx].min_mV;
    }
    break;*/

  }
  if (edited) {
    MakeDisplayChannelPage();
    Body.PrintAllFixed();
  }
}


void MakeDisplayChannelsOptionsPage()
{
  char buff[20];
  sprintf(buff, mFooterChanOpt, Config::ConfigFile.channels[currentDisplayChannelPage].name);
  Display.setFooter(buff);
  Display.refreshFooter();

  Body.Delete();
  Body.Lines[0] = (display_line*)new display_line_str(0, " Name: ", "%s  ", Config::ConfigFile.channels[currentDisplayChannelPage].name);
  Body.Lines[1] = (display_line*)new display_line(1, " Signe ");
  Body.Lines[2] = (display_line*)new display_line(2, " Coup. ");
  Body.Lines[3] = (display_line*)new display_line(3, " Dual  ");
  Body.Lines[4] = (display_line*)new display_line(4, " Ramp  ");
  int master = Config::ConfigFile.channels[currentDisplayChannelPage].master_channel;
  Body.Lines[5] = (display_line*)new display_line_str(5, " Mastr ", "%s  ", Config::ConfigFile.channels[master].name);
  Body.Lines[6] = (display_line*)new display_line(6, " Rate  ");
}


void DisplayChannelsOptions(PRINT_MODES print_mode) {
  if (IS_PUSHED(BUTTONS_ID::BTN_NEXT)) {
    Body.SetCursor(0, currentEditLine);
    Body.Print(" ");
    currentEditLine += 1;
  }
  if (currentEditLine>=7) {
    currentEditLine = 0;
  }
  if (currentDisplayChannelPage>=Config::ConfigFile.NBchannels) {
    currentDisplayChannelPage = Config::ConfigFile.NBchannels-1;
    print_mode=PRINT_MODES::PRINT;
  }

  int idx = currentDisplayChannelPage;
  if (print_mode==PRINT_MODES::PRINT) {
    MakeDisplayChannelsOptionsPage();
    Body.PrintAllFixed();
  } else {
    Body.RefreshAllNotFixed();

    Body.Lines[1]->refresh_notfixed();
    if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_INVERTED)!=0)
      Display.print("Invert");
    else
      Display.print("Normal");

    Body.Lines[2]->refresh_notfixed();
    if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_COUPLING)!=0)
      Display.print("Yes");
    else
      Display.print("No ");

    Body.Lines[3]->refresh_notfixed();
    if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_DUALRATE)!=0)
      Display.println("Yes");
    else
      Display.println("No ");

    Body.Lines[4]->refresh_notfixed();
    if ((Config::ConfigFile.channels[idx].options & CONFIG_CHANNEL_OPT_POWERLAW)!=0)
      Display.println("Power ");
    else
      Display.println("Linear");

    Body.Lines[6]->refresh_notfixed();
    Display.print((int)(Config::ConfigFile.channels[idx].rate*1000.0));
  }

  Body.SetCursor(0, currentEditLine);
  Body.Print(">");

  bool edited = false;
  switch(currentEditLine) {
    case 0: {
      edited = ChangeInt16(&currentDisplayChannelPage, 1, 0, Config::ConfigFile.NBchannels-1);
    }
    break;
    case 1: {
      edited = ChangeBitUInt8(&Config::ConfigFile.channels[idx].options, CONFIG_CHANNEL_OPT_INVERTED);
    }
    break;
    case 2: {
      edited = ChangeBitUInt8(&Config::ConfigFile.channels[idx].options, CONFIG_CHANNEL_OPT_COUPLING);
    }
    break;
    case 3: {
      edited = ChangeBitUInt8(&Config::ConfigFile.channels[idx].options, CONFIG_CHANNEL_OPT_DUALRATE);
    }
    break;
    case 4: {
      edited = ChangeBitUInt8(&Config::ConfigFile.channels[idx].options, CONFIG_CHANNEL_OPT_POWERLAW);
    }
    break;
    case 5: {
      edited = ChangeUInt8(&Config::ConfigFile.channels[idx].master_channel, 1, 0, Config::ConfigFile.NBchannels-1);
    }
    break;
    case 6: {
      if (IS_PRESSED(BUTTONS_ID::BTN_PLUS)) {
        Config::ConfigFile.channels[idx].rate *= 1.02f;
        edited = true;
      }
      if (IS_PRESSED(BUTTONS_ID::BTN_MINUS)) {
        Config::ConfigFile.channels[idx].rate *= (1.0f/(1.02f));
        edited = true;
      }
      if (Config::ConfigFile.channels[idx].rate<0.25)
        Config::ConfigFile.channels[idx].rate = 0.25;
      if (Config::ConfigFile.channels[idx].rate>=4.0)
        Config::ConfigFile.channels[idx].rate = 4.0;
    }
    break;
  }

  if (edited) {
    MakeDisplayChannelsOptionsPage();
    Body.PrintAllFixed();
  }
}

void MakePPMPage()
{
#if 0
  Display.setFooter(mFooterConfPPM);
  Display.refreshFooter();

  Body.Delete();
  Body.Lines[0] = (display_line*)new display_line_uint16(0, " #Chan=", mStrValue2d , &Config::ConfigFile.NBchannels);
  Body.Lines[1] = (display_line*)new display_line_uint16(1, " Frame=", mStrValue5us, &Config::ConfigFile.frame_length_us);
  Body.Lines[2] = (display_line*)new display_line_uint16(2, " Inter=", mStrValue5us, &Config::ConfigFile.interval_us);
  Body.Lines[3] = (display_line*)new display_line_uint16(3, " Min.T=", mStrValue5us, &Config::ConfigFile.min_pulse_us);
  Body.Lines[4] = (display_line*)new display_line_uint16(4, " Max.T=", mStrValue5us, &Config::ConfigFile.max_pulse_us);
  Body.Lines[5] = (display_line*)new display_line(5, mSaveAll);
#endif
}

void DisplayPPM(PRINT_MODES print_mode) {

  if (IS_PUSHED(BUTTONS_ID::BTN_NEXT)) {
    Body.SetCursor(0, currentEditLine);
    Body.Print(" ");
    currentEditLine += 1;
  }
  if (currentEditLine>=6) {
    currentEditLine = 0;
  }

  if (print_mode==PRINT_MODES::PRINT) {
    MakePPMPage();
    Body.PrintAllFixed();
  } else {
    Body.RefreshAllNotFixed();
  }

  Body.SetCursor(0, currentEditLine);
  Body.Print(">");

  bool edited = false;
  switch(currentEditLine) {
    case 0: {
      edited = ChangeInt16(&Config::ConfigFile.NBchannels,
        1, 1, MAX_CHANNELS);
    }
    break;

    case 1: {
      edited = ChangeInt16(&Config::ConfigFile.frame_length_us,
        STEP_TUNING_us<<1, 500, 32000);
    }
    break;
    case 2: {
      edited = ChangeInt16(&Config::ConfigFile.interval_us,
        STEP_TUNING_us, 100, 1000);
    }
    break;
   case 3: {
      edited = ChangeInt16(&Config::ConfigFile.min_pulse_us,
        STEP_TUNING_us, Config::ConfigFile.interval_us, Config::ConfigFile.max_pulse_us);
    }
    break;
    case 4: {
      edited = ChangeInt16(&Config::ConfigFile.max_pulse_us,
        STEP_TUNING_us, Config::ConfigFile.min_pulse_us, Config::ConfigFile.frame_length_us);
    }
    break;
    case 5: {
      if (IS_PUSHED(BUTTONS_ID::BTN_PLUS | BUTTONS_ID::BTN_MINUS)) {
        Body.SetCursor(0, currentEditLine);
        Body.Print(mSaving);
        Config::SaveConfigToEEPROM();
        Body.PrintAllFixed();
      }
    }
    break;
  }
  if (edited) {
    // Nombre de voies
    ppmEncoder.setNbChannel(Config::ConfigFile.NBchannels);
    ppmEncoder.PPM_INTERVAL_LENGTH_us = Config::ConfigFile.frame_length_us;
    ppmEncoder.PPM_FRAME_LENGTH_us = Config::ConfigFile.interval_us;
    ppmEncoder.MIN_us = Config::ConfigFile.min_pulse_us;
    ppmEncoder.MAX_us = Config::ConfigFile.max_pulse_us;
  }

}


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
   g_CalibInput_select = constrain( input , 0 , 5); //todo limit hardcoded
   Body.Delete();
   Body.Lines[0] = (display_line*)new display_line_uint16(  0, " Calib. Input "  , mStrValue2d , &g_CalibInput_select);
   Body.Lines[1] = (display_line*)new display_line_ft100(   1, "   Val= ", mStrValue3Pct, &Inputs_pst[g_CalibInput_select].val_ft);
   Body.Lines[2] = (display_line*)new display_line_uint16(  2, "   Val= ", mStrValue4mV,  &Inputs_pst[g_CalibInput_select].adc_mV_ui16);
   Body.Lines[3] = (display_line*)new display_line_uint16(  3, "   Min= ", mStrValue4mV,  &Inputs_pst[g_CalibInput_select].min_mV_ui16);
   Body.Lines[4] = (display_line*)new display_line_uint16(  4, "   Med= ", mStrValue4mV,  &Inputs_pst[g_CalibInput_select].med_mV_ui16);
   Body.Lines[5] = (display_line*)new display_line_uint16(  5, "   Max= ", mStrValue4mV,  &Inputs_pst[g_CalibInput_select].max_mV_ui16);
   Body.Lines[6] = (display_line*)new display_line_uint16(  6, "          ", mStrValue05d,  &g_ProcessGUI_Cnt);
}


enum PAGES_en : uint8_t
{
   PAGE_INIT=0,
   PAGE_CALIB_INPUTS,
   PAGE_MAX
};

uint8_t g_Page = PAGE_INIT;

void ProcessGUI()
{
   g_ProcessGUI_Cnt++;
   if( IS_PUSHED(BUTTONS_ID::BTN_PAGE) || g_Page == PAGE_INIT )
   {
      if (++g_Page >= PAGE_MAX)
         g_Page = 1;
      Display.clearBody();

      MakeDisplay_CalibInput(1);
      Body.PrintAllFixed();
   }

   EditLineCursor(0,5);
   Body.RefreshAllNotFixed();

   switch (g_Page)
   {
      case PAGE_CALIB_INPUTS:

         break;
   }
}
