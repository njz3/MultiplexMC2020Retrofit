#pragma once

#include "Config.h"

enum PRINT_MODES : byte {
  PRINT = 0,
  REFRESH = 1
};

void DisplayCurrentValues(PRINT_MODES print_mode);
void DisplayChannels(PRINT_MODES print_mode);
void DisplayChannelsOptions(PRINT_MODES print_mode);
void DisplayPPM(PRINT_MODES print_mode);

void ProcessGUI();
