// Support for OLED 128x128 using u8g2 library
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <U8g2lib.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

// Graphics mode
// Rotation = U8G2_R1 for 90°
//U8G2_SSD1327_MIDAS_128X128_2_HW_I2C u8g2(/*rotation, [reset [, clock, data]]) [page buffer, size = 256 bytes]*/);

// Text mode
U8X8_SSD1327_MIDAS_128X128_HW_I2C u8x8(/*[reset [, clock, data]]*/);

class CDisplay {
  public:

    void begin();

  private:

};

extern CDisplay Display;

#endif
