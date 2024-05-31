#include "SimpleOLEDRenderer.h"

#include <PicoDVI.h> // Core display & graphics library

// 320x240 16-bit color display (to match common TFT display resolution):
DVIGFX16 display(DVI_RES_320x240p60, pico_sock_cfg);

/*--------------------------------------------------------------------------*/
SimpleOLEDRenderer::SimpleOLEDRenderer( VirtualSSD1306 *pVirtualSSD1306 ) : m_pVirtualSSD1306( pVirtualSSD1306 )
{
}

/*--------------------------------------------------------------------------*/
SimpleOLEDRenderer::~SimpleOLEDRenderer() 
{
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer::initScreen()
{
  if (!display.begin()) { // Blink LED if insufficient RAM
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer::renderBackground()
{
  for ( int y = 0; y < display.height(); y++ )
  {
    for ( int x = 0; x < display.width(); x++ )
    {
      display.drawPixel(x, y, y * display.width() + x );
    }
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer::renderScreen()
{
#ifdef _PACKED_PIXELS  
  auto frameBuffer = m_pVirtualSSD1306->getFrameBuffer();

  auto forceDisplayOn = m_pVirtualSSD1306->forceDisplayOn();
  auto invertDisplay = m_pVirtualSSD1306->invertDisplay();
  auto displayOn = m_pVirtualSSD1306->displayOn();

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
#else
  for ( uint8_t y = 0; y < m_pVirtualSSD1306->height(); y++ )
  {
    for ( uint8_t x = 0; x < m_pVirtualSSD1306->width(); x++ )
    {
      // get pixel with optional effects (inverted, display on/off, forced on)
      auto pixelValue = m_pVirtualSSD1306->getPixel( x, y );
      // RGB565
      uint16_t color = ( 0x0f << 11 ) + ( 0x3f << 5 ) + ( 0x1f ); // cyan
      //uint16_t color = ( 0x1f << 11 ) + ( 0x3f << 5 ) + ( 0x00 ); // yellow
      //uint16_t color = 0xffff;                                    // white
      display.drawPixel( 2 * x + 32, 2 * y + 48, pixelValue ? color : 0x0000 );
      display.drawPixel( 2 * x + 32, 2 * y + 49, pixelValue ? color : 0x0000 );
      display.drawPixel( 2 * x + 33, 2 * y + 48, pixelValue ? color : 0x0000 );
      display.drawPixel( 2 * x + 33, 2 * y + 49, pixelValue ? color : 0x0000 );
    }
  }
#endif
}
 