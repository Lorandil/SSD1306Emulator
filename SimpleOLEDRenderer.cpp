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
  auto pcbColor = m_pDisplay->color565( 32, 32, 64 );
  auto pinColor = m_pDisplay->color565( 128, 128, 128 );
  auto textColor = m_pDisplay->color565( 255,255,255 );

  m_pDisplay->fillScreen( pcbColor );
  m_pDisplay->setTextColor( textColor );
  m_pDisplay->setTextSize( 1 );

  char strings[][4] = {"GND", "VCC", "SCK", "SDA"};

  for ( int pin = 0; pin < 4; pin++ )
  {
    m_pDisplay->fillCircle( 80 + 48 *pin, 20, 12, pinColor );
    m_pDisplay->setCursor( 72 + 48 * pin, 36 );
    m_pDisplay->print( strings[pin] );
  }
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
void SimpleOLEDRenderer::renderScreen()
{
#ifdef _PACKED_PIXELS  
  auto frameBuffer = m_pVirtualDisplay->getFrameBuffer();

  auto forceDisplayOn = m_pVirtualDisplay->forceDisplayOn();
  auto invertDisplay = m_pVirtualDisplay->invertDisplay();
  auto displayOn = m_pVirtualDisplay->displayOn();

  for ( int y = 0; y < 2 * m_pVirtualDisplay->height(); y++ )
  {
    for ( int x = 0; x < 2 * m_pVirtualDisplay->width(); x++ )
    {
      uint16_t offsetY = y >> 4;
      bool pixelValue = ( ( frameBuffer[( x / 2 ) + offsetY * m_pVirtualDisplay->width()] & ( 1 << ( ( y / 2 ) & 0x07 ) ) ) != 0 ) | forceDisplayOn;
      pixelValue ^= invertDisplay;
      pixelValue &= displayOn;
      m_pDisplay->drawPixel(x + 32, y + 48, pixelValue ? 0xFFFF : 0x0000 );
    }

    // process the command queue after every 16th line
    if ( ( y & 0x1f ) == 0x1f )
    {
      m_pVirtualDisplay->processData();
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
    
     // process the command queue
    m_pVirtualDisplay->processData();
 }
#endif
}
 