#include "SimpleOLEDRenderer1Bit.h"

#include <PicoDVI.h> // Core display & graphics library

/*--------------------------------------------------------------------------*/
SimpleOLEDRenderer1Bit::SimpleOLEDRenderer1Bit( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY )
 : RendererBase( pVirtualDisplay, scaleX, scaleY )
{
  Serial.println( F("c'tor") );

  // 640x240 8-bit color display (to match common TFT display resolution):
  m_pDisplay = new DVIGFX1( DVI_RES_640x480p60, false, pico_sock_cfg );
  if ( !m_pDisplay )
  {
    Serial.println( F("*** Failed to create DVIGFX1!") );
    // this is bad!
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }
  m_width = m_pDisplay->width();
  m_height = m_pDisplay->height();
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer1Bit::initScreen()
{
  Serial.println( F("initScreen()") );

  return;

  if (!m_pDisplay->begin()) { // Blink LED if insufficient RAM
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer1Bit::renderBackground()
{
  Serial.println( F("renderBackground()") );

return;
  for ( int y = 0; y < m_pDisplay->height(); y++ )
  {
    for ( int x = 0; x < m_pDisplay->width(); x++ )
    {
      m_pDisplay->drawPixel(x, y, y * m_pDisplay->width() + x );
    }
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer1Bit::renderScreen()
{
  Serial.println( F("renderScreen()") );
  return;
  
  for ( uint8_t y = 0; y < m_pVirtualDisplay->height(); y++ )
  {
    for ( uint8_t x = 0; x < m_pVirtualDisplay->width(); x++ )
    {
      // get pixel with optional effects (inverted, display on/off, forced on)
      auto pixelValue = m_pVirtualDisplay->getPixel( x, y );
      // monochrome
      uint16_t color = 1;
      m_pDisplay->drawPixel( 2 * x + 32, 2 * y + 48, pixelValue ? color : 0x0000 );
      m_pDisplay->drawPixel( 2 * x + 32, 2 * y + 49, pixelValue ? color : 0x0000 );
      m_pDisplay->drawPixel( 2 * x + 33, 2 * y + 48, pixelValue ? color : 0x0000 );
      m_pDisplay->drawPixel( 2 * x + 33, 2 * y + 49, pixelValue ? color : 0x0000 );
    }
  }
}
 