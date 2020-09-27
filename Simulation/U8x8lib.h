#ifndef U8X8LIB_H
#define U8X8LIB_H

#define U8X8_PIN_NONE   0

#define u8x8_font_amstrad_cpc_extended_r NULL
#define u8x8_font_8x13B_1x2_r NULL

class U8X8
{
   protected:

   public:
      uint8_t tx=0, ty=0;
      char lines[16][16] = {0};

      U8X8(void) { return;  };
      bool begin(void) { return 1; }
      void clear(void) {};
      void setCursor(uint8_t x, uint8_t y) { tx = x; ty = y; }
      void print(const char *str) {
         int len = strlen(str);
         len = min(len,16);
         for( int i=0 ; i<len ; i++ ) {
           lines[ty][tx++] = str[i];
           tx %= 16;
         }
      };
      void clearDisplay(void) {
         for( int i=0 ; i<16 ; i++ )
            clearLine(i);
         tx = 0;
         ty = 0;
      };

      void setFont(const uint8_t *font_8x8) {};
      void setInverseFont(uint8_t value) {};
      size_t write(uint8_t v) {
         lines[ty][tx++] = v;
         tx %= 16;
         return 0;
      };
      void clearLine(uint8_t line) {
         for( int x=0 ; x<16 ; x++ )
            lines[line][x]=0;
         tx = 0;
         ty = line;
      };
      void drawString(uint8_t x, uint8_t y, const char *s) {};
      void draw2x2String(uint8_t x, uint8_t y, const char *s) {};

      uint8_t getCols(void) { return 16;  };
      uint8_t getRows(void) { return 16;  };
      void inverse(void) {};
      void noInverse(void) {};
      void setBusClock(uint32_t clock_speed) {};
      void setI2CAddress(uint8_t adr) {};
};

class U8X8_SSD1327_MIDAS_128X128_HW_I2C : public U8X8 {
  public: U8X8_SSD1327_MIDAS_128X128_HW_I2C(uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE) : U8X8() {
  }
};


#endif // U8X8LIB_H
