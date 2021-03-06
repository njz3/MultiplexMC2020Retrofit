#include "Display.h"




#if defined(USE_U8X8)
// Text mode
#include <U8g2lib.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#ifndef TEST_MEM
//U8X8_SSD1327_MIDAS_128X128_HW_I2C u8x8(/*[reset [, clock, data]]*/);
U8X8_SSD1327_MIDAS_128X128_HW_I2C u8x8(U8X8_PIN_NONE);
#endif

#elif defined(USE_U8G2)

// Graphics mode
// Rotation = U8G2_R1 for 90°
//U8G2_SSD1327_MIDAS_128X128_2_HW_I2C u8g2(/*rotation, [reset [, clock, data]]) [page buffer, size = 256 bytes]*/);
U8G2_SSD1327_MIDAS_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); /* Uno: A4=SDA, A5=SCL, add "u8g2.setBusClock(400000);" into setup() for speedup if possible */
#endif

#define SDA_PIN (SDA)
#define SCL_PIN (SCL)

// Singleton
CDisplay Display;


void CDisplay::begin() {

// Does not seem to be useful
/*
#if defined(ARDUINO_AVR_MEGA2560)
  pinMode(SDA, OUTPUT);
  pinMode(SCL, OUTPUT);
  digitalWrite(SDA,HIGH);
  digitalWrite(SCL,HIGH);
#elif defined(ARDUINO_AVR_NANO)
  // Nano
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);
#endif
*/

#ifndef TEST_MEM
#if defined(USE_U8X8)
  u8x8.setBusClock(400000);
  u8x8.setI2CAddress(0x78);
  u8x8.begin();
#elif defined(USE_U8G2)
  u8g2.setBusClock(400000);
  u8g2.setI2CAddress(0x78);
  u8g2.begin();
#endif
#endif

#ifndef TEST_MEM
  strncpy(header, "  MC2020  V" VERSION "  ", sizeof(header));
#if defined(ARDUINO_AVR_MEGA2560)
  strncpy(footer, "   MEGA 2560    ", sizeof(footer));
#elif defined(ARDUINO_AVR_NANO)
  strncpy(footer, "      NANO      ", sizeof(footer));
#endif
#endif
}



void CDisplay::cleanup(void)
{
#ifndef TEST_MEM
  u8x8.clear();

#ifdef HEADER_FOOTER
  setSmallFont();
  u8x8.inverse();
  u8x8.print(header);
  u8x8.setCursor(0,15);
  u8x8.print(footer);
#endif // HEADER_FOOTER
  u8x8.noInverse();
  setNormalFont();
  u8x8.setCursor(0,1);
#endif // TEST_MEM
  col = 0;
  row = 1;
}

void CDisplay::refreshHeader()
{
#ifdef HEADER_FOOTER
  #ifndef TEST_MEM
  setSmallFont();
  u8x8.setCursor(0,0);
  u8x8.inverse();
  u8x8.print(header);
  u8x8.noInverse();

  setNormalFont();
  u8x8.setCursor(col, row);
  #endif
#endif // HEADER_FOOTER
}

void CDisplay::refreshFooter()
{
#ifdef HEADER_FOOTER
  #ifndef TEST_MEM
  setSmallFont();
  u8x8.setCursor(0,15);
  u8x8.inverse();
  u8x8.print(footer);
  u8x8.noInverse();

  setNormalFont();
  u8x8.setCursor(col, row);
  #endif
#endif // HEADER_FOOTER
}

void CDisplay::draw_bar(uint8_t c, uint8_t is_inverse)
{
  #ifndef TEST_MEM
  uint8_t r;
  u8x8.setInverseFont(is_inverse);
  for( r = 0; r < u8x8.getRows(); r++ )
  {
    u8x8.setCursor(c, r);
    u8x8.print(" ");
  }
  #endif
}

void CDisplay::draw_ascii_row(uint8_t r, int start)
{
  #ifndef TEST_MEM
  int a;
  uint8_t c;
  for( c = 0; c < u8x8.getCols(); c++ )
  {
    u8x8.setCursor(c,r);
    a = start + c;
    if ( a <= 255 )
      u8x8.write(a);
  }
  #endif
}

void CDisplay::print(const char *str)
{
  #ifndef TEST_MEM
  u8x8.print(str);
  #endif
  col+= strlen(str);
  if (col>15) col = 0;
}
void CDisplay::print(const int val)
{
  char buf[10];
  sprintf(buf, "%d", val);
  print(buf);
}
/*
void CDisplay::print(const String& str)
{
  char buf[20];
  str.toCharArray(buf, 20);
  buf[16]=0;
  print(buf);
}
*/
void CDisplay::println(const char *str)
{
  print(str);
  col = 0;
  row = (row+2); // Increase and loop back
  if (row>14) row = 1;
  #ifndef TEST_MEM
  u8x8.setCursor(col, row);
  #endif
}
void CDisplay::println(const int val)
{
  char buf[10];
  sprintf(buf, "%d", val);
  println(buf);
}
/*
void CDisplay::println(const String& str)
{
  char buf[20];
  str.toCharArray(buf, 20);
  buf[16]=0;
  println(buf);
}
*/
/*
void CDisplay::setHeader(const String& str) {
  char buf[20];
  str.toCharArray(buf, 20);
  buf[16]=0;
  setHeader(buf);
}

void CDisplay::setFooter(const String& str) {
  char buf[20];
  str.toCharArray(buf, 20);
  buf[16]=0;
  setFooter(buf);
}
*/
void CDisplay::setCursor(int col, int row)
{
  this->col = col;
  this->row = row;
#ifndef TEST_MEM
  u8x8.setCursor(col, row);
#endif
}

void CDisplay::clearAll()
{
#ifndef TEST_MEM
  u8x8.clear();
  setCursor(0, 0);
#endif
}

void CDisplay::clearLine(int row)
{
#ifndef TEST_MEM
  u8x8.clearLine(row);
  if (fontRowSize==2) {
    u8x8.clearLine(row+1);
  }
  if (fontRowSize==3) {
    u8x8.clearLine(row+2);
  }
#endif
}
void CDisplay::clearBody()
{
#ifndef TEST_MEM
#ifdef HEADER_FOOTER
  for(int row=1; row<15; row++)
#else
  for(int row=0; row<16; row++)
#endif
  {
    u8x8.clearLine(row);
  }
#endif
}

void CDisplay::drawString(int col, int row, char *str)
{
#ifndef TEST_MEM
  u8x8.drawString(col, row, str);
#endif
}
void CDisplay::draw2x2String(int col, int row, char *str)
{
#ifndef TEST_MEM
  u8x8.draw2x2String(col, row, str);
#endif
}

void CDisplay::setSmallFont()
{
#ifndef TEST_MEM
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
#endif
  fontRowSize = 1;
}
void CDisplay::setNormalFont()
{
#ifndef TEST_MEM
  u8x8.setFont(u8x8_font_8x13B_1x2_r);
#endif
  fontRowSize = 2;
}
void CDisplay::setBigFont()
{
   //u8x8.setFont(u8x8_font_lucasarts_scumm_subtitle_o_2x2_r  );
   fontRowSize = 2;
}

/*
void CDisplay::loop(void)
{
  int i;
  uint8_t c, r, d;
  cleanup();
  u8x8.print("github.com/");
  u8x8.setCursor(0,2);
  u8x8.print("olikraus/u8g2");
  delay(2000);
  u8x8.setCursor(0,3);
  u8x8.print("Tile size:");
  u8x8.print((int)u8x8.getCols());
  u8x8.print("x");
  u8x8.print((int)u8x8.getRows());

  delay(2000);

  cleanup();
  for( i = 19; i > 0; i-- )
  {
    u8x8.setCursor(3,2);
    u8x8.print(i);
    u8x8.print("  ");
    delay(150);
  }

  draw_bar(0, 1);
  for( c = 1; c < u8x8.getCols(); c++ )
  {
    draw_bar(c, 1);
    draw_bar(c-1, 0);
    delay(50);
  }
  draw_bar(u8x8.getCols()-1, 0);

  cleanup();
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  for( d = 0; d < 8; d ++ )
  {
    for( r = 1; r < u8x8.getRows(); r++ )
    {
      draw_ascii_row(r, (r-1+d)*u8x8.getCols() + 32);
    }
    delay(400);
  }

  draw_bar(u8x8.getCols()-1, 1);
  for( c = u8x8.getCols()-1; c > 0; c--)
  {
    draw_bar(c-1, 1);
    draw_bar(c, 0);
    delay(50);
  }
  draw_bar(0, 0);

  cleanup();
  u8x8.drawString(0, 2, "Small");
  u8x8.draw2x2String(0, 5, "Scale Up");
  delay(3000);

  cleanup();
  u8x8.drawString(0, 2, "Small");
  u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
  u8x8.drawString(0, 5, "2x2 Font");
  delay(3000);

  cleanup();
  u8x8.drawString(0, 1, "3x6 Font");
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  for(i = 0; i < 100; i++ )
  {
    u8x8.setCursor(0, 2);
    u8x8.print(i);      // Arduino Print function
    delay(10);
  }
  for(i = 0; i < 100; i++ )
  {
    u8x8.drawString(0, 2, u8x8_u16toa(i, 5)); // U8g2 Build-In functions
    delay(10);
  }

  cleanup();
  u8x8.drawString(0, 2, "Weather");
  u8x8.setFont(u8x8_font_open_iconic_weather_4x4);
  for(c = 0; c < 6; c++ )
  {
    u8x8.drawGlyph(0, 4, '@'+c);
    delay(300);
  }


  cleanup();
  u8x8.print("print \\n\n");
  delay(500);
  u8x8.println("println");
  delay(500);
  u8x8.println("done");
  delay(1500);

  cleanup();
  u8x8.fillDisplay();
  for( r = 0; r < u8x8.getRows(); r++ )
  {
    u8x8.clearLine(r);
    delay(100);
  }
  delay(1000);

}
*/
