#include "SimpleOLEDRenderer.h"

#include <PicoDVI.h> // Core display & graphics library
#include <string>
#include <vector>

/*--------------------------------------------------------------------------*/
SimpleOLEDRenderer::SimpleOLEDRenderer( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY )
 : RendererBase( pVirtualDisplay, scaleX, scaleY )
{
  // 320x240 16-bit color display (to match common TFT display resolution):
  m_pDisplay = new DVIGFX16( DVI_RES_320x240p60, pico_sock_cfg );
  if ( !m_pDisplay )
  {
    Serial.println( F("*** Failed to create DVIGFX16!") );
    // this is bad!
    panic();
  }
  m_width = m_pDisplay->width();
  m_height = m_pDisplay->height();
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer::initScreen()
{
  Serial.println( F("initScreen()") );

  if ( !m_pDisplay->begin() ) { // Blink LED if insufficient RAM
    Serial.println( F("*** insufficient RAM!") );
    panic();
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer::renderBackground()
{
  auto black = m_pDisplay->color565( 0, 0, 0 );
  auto pcbColor = m_pDisplay->color565( 32, 32, 64 );
  auto padColor = m_pDisplay->color565( 128, 128, 128 );
  auto pinColor = m_pDisplay->color565( 192, 192, 192 );
  auto textColor = m_pDisplay->color565( 255,255,255 );

  m_pDisplay->fillScreen( pcbColor );
  m_pDisplay->setTextColor( textColor );
  m_pDisplay->setTextSize( 1 );

  char strings[][4] = {"GND", "VCC", "SCK", "SDA"};

  for ( int pin = 0; pin < 4; pin++ )
  {
    m_pDisplay->fillCircle( 80 + 48 *pin, 24, 12, padColor );
    m_pDisplay->fillCircle( 81 + 48 *pin, 25, 4, black );
    m_pDisplay->fillCircle( 80 + 48 *pin, 24, 4, pinColor );
    m_pDisplay->setCursor( 72 + 48 * pin, 40 );
    m_pDisplay->print( strings[pin] );
  }
}

/*--------------------------------------------------------------------------*/
void SimpleOLEDRenderer::renderScreen()
{
  // center the output window
  int cx = ( m_width - m_scaleX * m_pVirtualDisplay->width() ) / 2;
  int cy = ( m_height - m_scaleY * m_pVirtualDisplay->height() ) / 2;

  for ( uint8_t y = 0; y < m_pVirtualDisplay->height(); y++ )
  {
    for ( uint8_t x = 0; x < m_pVirtualDisplay->width(); x++ )
    {
      // get pixel with optional effects (inverted, display on/off, forced on)
      auto pixelValue = m_pVirtualDisplay->getPixel( x, y );
      // RGB565
      //uint16_t foreground = m_pDisplay->color565( 0, 255, 255 ); // cyan
      uint16_t foreground = m_pDisplay->color565( 255, 255, 0 ); // yellow
      //uint16_t foreground = m_pDisplay->color565( 255, 255, 255 ); // white
      uint16_t background = m_pDisplay->color565( 0, 0, 0 ); // black

      // draw scaled pixel
      for ( int sy = 0; sy < m_scaleY; sy++ )
      {
        for ( int sx = 0; sx < m_scaleX; sx++ )
        {
          m_pDisplay->drawPixel( cx + x * m_scaleX + sx, cy + y * m_scaleY + sy, pixelValue ? foreground : background );
        }
      }
    }
    
     // process the command queue
    m_pVirtualDisplay->processData();
  }
}