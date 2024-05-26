/*
* LAB Name: Arduino I2C Slave(Rx)
* Author: Khaled Magdy
* For More Info Visit: www.DeepBlueMbedded.com
*/
#include "VirtualSSD1306.h"

#include <PicoDVI.h> // Core display & graphics library

// 320x240 16-bit color display (to match common TFT display resolution):
DVIGFX16 display(DVI_RES_320x240p60, pico_sock_cfg);

// setup SSD1306 emualtion layer
VirtualSSD1306 virtualSSD1306( 128, 64 );

/*---------------------------------------------------------------------------*/
void setup()
{
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, true );

  Serial.begin( 115200 );
  
  if (!display.begin()) { // Blink LED if insufficient RAM
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }
  
  delay( 2000 );

  // start virtual SSD1306
  virtualSSD1306.begin( 0x3C );

  // no overflow so far
  digitalWrite( LED_BUILTIN, false );
}

/*---------------------------------------------------------------------------*/
void loop()
{
  renderBackground();

  uint32_t count{};

  while ( true )
  {
    virtualSSD1306.processData();

    // render the screen all 1000 cycles
    if ( ( count++ % 1000 ) == 0 )
    {
      virtualSSD1306.performScrolling();
      renderScreen();
    }
  }
}

/*---------------------------------------------------------------------------*/
void renderBackground()
{
  Serial.println( F("renderBackground()") );

  for ( int y = 0; y < display.height(); y++ )
  {
    for ( int x = 0; x < display.width(); x++ )
    {
      display.drawPixel(x, y, y * display.width() + x );
    }
  }
}

/*---------------------------------------------------------------------------*/
// Render the screen in the selected style (just 1:1 for now)
void renderScreen()
{
  auto frameBuffer = virtualSSD1306.getFrameBuffer();

  auto forceDisplayOn = virtualSSD1306.forceDisplayOn();
  auto invertDisplay = virtualSSD1306.invertDisplay();
  auto displayOn = virtualSSD1306.displayOn();

  for ( int y = 0; y < 2 * SSD1306Command::DISPLAY_HEIGHT; y++ )
  {
    for ( int x = 0; x < 2 * SSD1306Command::DISPLAY_WIDTH; x++ )
    {
      uint16_t offsetY = y >> 4;
      bool pixelValue = ( ( frameBuffer[( x / 2 ) + offsetY * SSD1306Command::DISPLAY_WIDTH] & ( 1 << ( ( y / 2 ) & 0x07 ) ) ) != 0 ) | forceDisplayOn;
      pixelValue ^= invertDisplay;
      pixelValue &= displayOn;
      display.drawPixel(x + 32, y + 48, pixelValue ? 0xFFFF : 0x0000 );
    }
  }
}