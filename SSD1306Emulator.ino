/*
* SSD1306Emulator - a simple emulator to render data intended for an SSD1306 to DVI using PicoDVI
* 2024-07-21 Lorandil
*/
#include "SimpleOLEDRenderer.h"
#include "VirtualSSD1306.h"

//#define DIAGNOSTIC_BOOT

// setup SSD1306 emulation layer
VirtualSSD1306 virtualSSD1306( 128, 64, true );

// declare global rendering class pointer
RendererBase *pRenderer{};

/*---------------------------------------------------------------------------*/
void setup()
{
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, true );

  Serial.begin( 115200 );

#ifdef DIAGNOSTIC_BOOT
  // give the Pico and the OS some time to establish a serial connection
  for ( int n = 0; n < 2; n++ )
  {
    Serial.print( F(".") );
    delay( 1000 );
  }
  Serial.println();
#else
  // start virtual SSD1306 early
  virtualSSD1306.begin( SSD1306_I2C_DEFAULT_ADDRESS );
#endif

  // create a render class
  pRenderer = new SimpleOLEDRenderer( &virtualSSD1306, 2, 2 );

  // no overflow so far
  digitalWrite( LED_BUILTIN, false );

  // screen initilization
  pRenderer->initScreen();

#ifdef DIAGNOSTIC_BOOT
  // start virtual SSD1306 later, so that serial output will be visible
  virtualSSD1306.begin( SSD1306_I2C_DEFAULT_ADDRESS );
#endif
}

/*---------------------------------------------------------------------------*/
void loop()
{
  pRenderer->renderBackground();

  uint32_t count{};

  while ( true )
  {
    virtualSSD1306.processData();

    // render the screen all 1000 cycles
    if ( ( count++ % 1000 ) == 0 )
    {
      virtualSSD1306.performScrolling();
      pRenderer->renderScreen();
    }
  }
}
