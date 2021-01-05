#include "Resources.h"

#undef RSTR
#define RSTR const char*

RSTR mBoot          = "Booting";
RSTR mConfigureIOs  = "Configure IOs";
RSTR mResetPinOn    = "RESET! (Pin D12)";
RSTR mLoadConfig    = "Loading conf";
RSTR mWrongConfig   = "Bad CRC! Reset";
RSTR mPPMStarted    = "PPM Started";
RSTR mNChanEq       = "#chan=";
RSTR mOutputPinEq   = "Output=D";
RSTR mVersionEq     = "Version=" VERSION;

RSTR mReading       = "Reading";

RSTR mSaveConfig    = "SAVING!!!";

RSTR mFooterValues  = "Values p.%d    ";
RSTR mStrValue2d    = "%2d";
RSTR mStr2Char      = "%.2s";
RSTR mStr3Char      = "%.3s";
RSTR mStr4Char      = "%.4s";
RSTR mStrValue05d   = "%05d";
RSTR mStrValue3Pct  = "%+4d%%";
RSTR mStrValue4mV   = "%4dmV";
RSTR mStrValue4us   = "%4dus";
RSTR mStrValue5us   = "%5dus";

RSTR mFooterChan    = "Channel %s    ";
RSTR mFooterChanOpt = "Options %s    ";
RSTR mFooterConfPPM = "Configure PPM ";

RSTR mSaveAll       = " SAVE ALL?";
RSTR mSaving        = ">SAVING...";
