// Support for OLED 128x128 using u8g2 library
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>
#include "Config.h"

#define USE_U8X8
//#define USE_U8G2

class CDisplay {
  private:
    int fontRowSize = 1;
    int col;
    int row;
    char header[20];
    char footer[20];

  public:   
    void begin();
    void loop();

    void cleanup(void);

    void refreshHeader(void);
    void refreshFooter(void);
    
    void draw_bar(uint8_t c, uint8_t is_inverse);
    void draw_ascii_row(uint8_t r, int start);
    void drawString(int col, int row, char *str);
    void draw2x2String(int col, int row, char *str);

    void print(const char *str);
    void print(const String& str);
    void println(const char *str);
    void println(const String& str);
    void setCursor(int col, int row);
    void setSmallFont();
    void setNormalFont();
    void setBigFont();
    void clearAll();
    void clearLine(int row);
    void clearBody();

    int getRow() { return row; }
    int getCol() { return col; }
    int getFontRawSize() { return fontRowSize; }
    void setHeader(const char* str) { strncpy(header, str, sizeof(header)-1); header[16]=0; }
    void setHeader(const String& str);
    void setFooter(const char* str) { strncpy(footer, str, sizeof(footer)-1); footer[16]=0; }
    void setFooter(const String& str);
    
};

extern CDisplay Display;

#endif
