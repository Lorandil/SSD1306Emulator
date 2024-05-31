/*
* LAB Name: Arduino I2C Slave(Rx)
* Author: Khaled Magdy
* For More Info Visit: www.DeepBlueMbedded.com
*/
#include "SimpleOLEDRenderer.h"
#include "VirtualSSD1306.h"

// setup SSD1306 emualtion layer
VirtualSSD1306 virtualSSD1306( 128, 64 );

// and rendering class
SimpleOLEDRenderer renderer( &virtualSSD1306 );

/*---------------------------------------------------------------------------*/
void setup()
{
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, true );

  Serial.begin( 115200 );
  
  renderer.initScreen();
  
  delay( 2000 );

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
