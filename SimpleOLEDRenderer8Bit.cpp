#include "SimpleOLEDRenderer8Bit.h"

#include <PicoDVI.h> // Core display & graphics library

/*--------------------------------------------------------------------------*/
SimpleOLEDRenderer8Bit::SimpleOLEDRenderer8Bit( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY )
 : RendererBase( pVirtualDisplay, scaleX, scaleY )
{
  Serial.println( F("c'tor") );

  // 640x240 8-bit color display (to match common TFT display resolution):
  m_pDisplay = new DVIGFX8( DVI_RES_640x240p60, false, pico_sock_cfg );
  if ( !m_pDisplay )
  {
    Serial.println( F("*** Failed to create DVIGFX8!") );
    // this is bad!
    panic();
  }
  m_width = m_pDisplay->width();
  m_height = m_pDisplay->height();
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer8Bit::initScreen()
{
  Serial.println( F("initScreen()") );

  if (!m_pDisplay->begin()) { // Blink LED if insufficient RAM
    Serial.println( F("*** insufficient RAM!") );
    panic();
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer8Bit::renderBackground()
{
  Serial.println( F("renderBackground()") );

  /*
  for ( int y = 0; y < m_pDisplay->height(); y++ )
  {
    for ( int x = 0; x < m_pDisplay->width(); x++ )
    {
      m_pDisplay->drawPixel(x, y, y * m_pDisplay->width() + x );
    }
  }
  */
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer8Bit::renderScreen()
{
  Serial.println( F("renderScreen()") );
/*  
#ifdef _PACKED_PIXELS  
  auto frameBuffer = m_pVirtualDisplay->getFrameBuffer();

  auto forceDisplayOn = m_pVirtualDisplay->forceDisplayOn();
  auto invertDisplay = m_pVirtualDisplay->invertDisplay();
  auto displayOn = m_pVirtualDisplay->displayOn();

  for ( int y = 0; y < 2 * SSD1306Command::DISPLAY_HEIGHT; y++ )
  {
    for ( int x = 0; x < 2 * SSD1306Command::DISPLAY_WIDTH; x++ )
    {
      uint16_t offsetY = y >> 4;
      bool pixelValue = ( ( frameBuffer[( x / 2 ) + offsetY * SSD1306Command::DISPLAY_WIDTH] & ( 1 << ( ( y / 2 ) & 0x07 ) ) ) != 0 ) | forceDisplayOn;
      pixelValue ^= invertDisplay;
      pixelValue &= displayOn;
      m_pDisplay->drawPixel(x + 32, y + 48, pixelValue ? 0xFFFF : 0x0000 );
    }
  }
#else
  for ( uint8_t y = 0; y < m_pVirtualDisplay->height(); y++ )
  {
    for ( uint8_t x = 0; x < m_pVirtualDisplay->width(); x++ )
    {
      // get pixel with optional effects (inverted, display on/off, forced on)
      auto pixelValue = m_pVirtualDisplay->getPixel( x, y );
      // RGB565
      uint16_t color = ( 0x0f << 11 ) + ( 0x3f << 5 ) + ( 0x1f ); // cyan
      //uint16_t color = ( 0x1f << 11 ) + ( 0x3f << 5 ) + ( 0x00 ); // yellow
      //uint16_t color = 0xffff;                                    // white
      m_pDisplay->drawPixel( 2 * x + 32, 2 * y + 48, pixelValue ? color : 0x0000 );
      m_pDisplay->drawPixel( 2 * x + 32, 2 * y + 49, pixelValue ? color : 0x0000 );
      m_pDisplay->drawPixel( 2 * x + 33, 2 * y + 48, pixelValue ? color : 0x0000 );
      m_pDisplay->drawPixel( 2 * x + 33, 2 * y + 49, pixelValue ? color : 0x0000 );
    }
  }
#endif
*/
}
 