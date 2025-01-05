#include <sys/_stdint.h>
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

  if ( !m_pDisplay->begin() ) { // Blink LED if insufficient RAM
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer1Bit::renderBackground()
{
  Serial.println( F("renderBackground()") );

  // just a simple black background
  m_pDisplay->fillScreen( 1 );
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer1Bit::renderScreen()
{
  // Draw random lines
  m_pDisplay->drawLine(random(m_pDisplay->width()), random(m_pDisplay->height()), // Start X,Y
                   random(m_pDisplay->width()), random(m_pDisplay->height()), // End X,Y
                   random(2)); // Color (0 or 1)

  return;

  int16_t offsetX = max( 0, ( m_pDisplay->width() - m_pVirtualDisplay->width() * m_scaleX ) / 2 );
  int16_t offsetY = max( 0, ( m_pDisplay->height() - m_pVirtualDisplay->height() * m_scaleY ) / 2 );

  Serial.print( F("renderScreen( offsetX = ") ); Serial.print( offsetX ); Serial.print( F(", offsetY = ") ); Serial.print( offsetY ); Serial.println( F(" )") );
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);

  for ( int16_t y = 0; y < m_pVirtualDisplay->height(); y++ )
  {
    for ( int16_t x = 0; x < m_pVirtualDisplay->width(); x++ )
    {
      // get pixel with optional effects (inverted, display on/off, forced on)
      auto pixelValue = m_pVirtualDisplay->getPixel( x, y );

      int16_t sy = 0;
      //for ( int16_t sy = 0; sy < m_scaleY; sy++ )
      {
        //for ( int16_t sx = 0; sx < m_scaleX; sx++ )
        int16_t sx = 0;
        {
          m_pDisplay->drawPixel( m_scaleX * x + sx + offsetX, m_scaleY * y + sy + offsetY, pixelValue ? 1 : 0 );
        }
      }
    }
  }
}
 