#include "SimpleOLEDRenderer8Bit.h"

#include <PicoDVI.h> // Core display & graphics library
#include <Adafruit_GFX.h>

/*--------------------------------------------------------------------------*/
SimpleOLEDRenderer8Bit::SimpleOLEDRenderer8Bit( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY )
 : RendererBase( pVirtualDisplay, scaleX, scaleY )
{
  Serial.println( F("c'tor") );

  // 320x240 8-bit color display (to match common TFT display resolution):
  m_pDisplay = new DVIGFX8( DVI_RES_320x240p60, false, pico_sock_cfg );
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

  // Randomize color palette. First entry is left black, last is set white.
  for (int i=1; i<255; i++) m_pDisplay->setColor(i, random(65536));
  m_pDisplay->setColor(255, 0xFFFF);
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer8Bit::renderBackground()
{
  Serial.println( F("renderBackground()") );

  for ( int y = 0; y < m_pVirtualDisplay->height(); y++ )
  {
    for ( int x = 0; x < m_pVirtualDisplay->width(); x++ )
    {
      m_pDisplay->drawPixel( x, y, 0 );
    }
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer8Bit::renderScreen()
{
  Serial.println( F("renderScreen()") );

  for ( int y = 0; y < m_pVirtualDisplay->height(); y ++ )
  {
    for ( int x = 0; x < m_pVirtualDisplay->width(); x ++ )
    {
      for ( int sy = 0; sy < m_scaleY; sy++ )
      {
        for ( int sx = 0; sx < m_scaleX; sx++ )
        {
          m_pDisplay->drawPixel( x * m_scaleX + sx, y * m_scaleY + sy, m_pVirtualDisplay->getPixel( x, y ) );
        }
      }
    }

    // process the command queue after every 8th line
    if ( ( y & 0x7 ) == 0x7 )
    {
      m_pVirtualDisplay->processData();
    }
  }
}
 