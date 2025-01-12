#include <sys/_stdint.h>
#include "SimpleOLEDRenderer1Bit.h"

#include <PicoDVI.h> // Core display & graphics library

/*--------------------------------------------------------------------------*/
SimpleOLEDRenderer1Bit::SimpleOLEDRenderer1Bit( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY )
 : RendererBase( pVirtualDisplay, scaleX, scaleY )
{
  Serial.println( F("c'tor") );

  // 640x480 1-bit color display (to match common TFT display resolution):
  m_pDisplay = new DVIGFX1( DVI_RES_640x480p60, false, pico_sock_cfg, VREG_VOLTAGE_1_20 );
  if ( !m_pDisplay )
  {
    Serial.println( F("*** Failed to create DVIGFX1!") );
    // this is bad!
    panic();
  }
  m_width = m_pDisplay->width();
  m_height = m_pDisplay->height();
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer1Bit::initScreen()
{
  Serial.println( F("initScreen()") );

  if ( !m_pDisplay->begin() ) { // Blink LED if insufficient RAM
    Serial.println( F("*** insufficient RAM!") );
    panic();
  }

  // Randomize color palette. First entry is left black, last is set white.
  //for (int i=1; i<255; i++) m_pDisplay->setColor(i, random(65536));
  //m_pDisplay->setColor(255, 0xFFFF);

  Serial.print( F("renderScreen( width = ") ); Serial.print( m_pDisplay->width() ); Serial.print( F(", height = ") ); Serial.print( m_pDisplay->height() ); Serial.println( F(" )") );
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer1Bit::renderBackground()
{
  Serial.println( F("renderBackground()") );

  // just a simple black background
  m_pDisplay->fillScreen( 0 );
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer1Bit::renderScreen()
{
  // Draw random lines
  //m_pDisplay->drawLine( random( m_width ), random( m_height ), // Start X,Y
  //                      random( m_width ), random( m_height ), // End X,Y
  //                      random(2) ); // Color (0 or 1)

  int16_t offsetX = max( 0, ( m_width - m_pVirtualDisplay->width() * m_scaleX ) / 2 );
  int16_t offsetY = max( 0, ( m_height - m_pVirtualDisplay->height() * m_scaleY ) / 2 );

  //Serial.print( F("renderScreen( offsetX = ") ); Serial.print( offsetX ); Serial.print( F(", offsetY = ") ); Serial.print( offsetY ); Serial.println( F(" )") );

  for ( int16_t y = 0; y < m_pVirtualDisplay->height(); y++ )
  {
    for ( int16_t x = 0; x < m_pVirtualDisplay->width(); x++ )
    {
      // get pixel with optional effects (inverted, display on/off, forced on)
      auto pixelValue = m_pVirtualDisplay->getPixel( x, y );

      //for ( int16_t sy = 0; sy < 2; sy++ )
      {
        int sy = 0;
        int sx = 0;
        //for ( int16_t sx = 0; sx < 2; sx++ )
        {
          int px = m_scaleX * x + sx + offsetX;
          int py = m_scaleY * y + sy + offsetY;
          //Serial.print( F("drawPixel( x = ") ); Serial.print( px ); Serial.print( F(", y = ") ); Serial.print( py ); Serial.println( F(" )") );
          m_pDisplay->drawPixel(px, py, pixelValue ? 1 : 0 );
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
 