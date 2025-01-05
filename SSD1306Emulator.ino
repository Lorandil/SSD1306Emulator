/*
* SSD1306Emulator - a simple emulator to render data intended for an SSD1306 to DVI using PicoDVI
* 2024-07-21 Lorandil
*/
#include "SimpleOLEDRenderer.h"
#include "SimpleOLEDRenderer1Bit.h"
#include "SimpleOLEDRenderer8Bit.h"
#include "VirtualSSD1306.h"

// setup SSD1306 emulation layer
VirtualSSD1306 virtualSSD1306( 128, 64 );

// and rendering class (uncomment exactly one line)
SimpleOLEDRenderer renderer( &virtualSSD1306, 2, 2 );
//SimpleOLEDRenderer1Bit renderer( &virtualSSD1306, 2, 2 );
//SimpleOLEDRenderer8Bit renderer( &virtualSSD1306, 2, 2 );

/*---------------------------------------------------------------------------*/
void setup()
{
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, true );

  Serial.begin( 115200 );
  
  for ( int n = 0; n < 5; n++ )
  {
    Serial.print( F(".") );
    delay( 1000 );
  }
  Serial.println();

  renderer.initScreen();
  
  // start virtual SSD1306
  virtualSSD1306.begin( 0x3C );

  // no overflow so far
  digitalWrite( LED_BUILTIN, false );
}

/*---------------------------------------------------------------------------*/
void loop()
{
  renderer.renderBackground();

  uint32_t count{};

  while ( true )
  {
    virtualSSD1306.processData();

    // render the screen all 1000 cycles
    if ( ( count++ % 1000 ) == 0 )
    {
      virtualSSD1306.performScrolling();
      renderer.renderScreen();
    }
  }
}
