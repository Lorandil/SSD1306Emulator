/*
* LAB Name: Arduino I2C Slave(Rx)
* Author: Khaled Magdy
* For More Info Visit: www.DeepBlueMbedded.com
*/
#include "SerialHexTools.h"
#include <Wire.h>
#include "fifo.hpp"
#include "ssd1306commands.h"

SimpleFIFO fifo;

static uint32_t lastByteCount{}; 

static char frameBuffer[1024];
 
// SSD1306 decoder
void I2C_RxHandler( int numBytes )
{
  for ( int n = 0; n < numBytes; n++ )
  {
    if ( fifo.isFull() ) { digitalWrite( LED_BUILTIN, true ); }
    fifo.writeByte( uint8_t( Wire.read() ) );
  }
}
 
void setup()
{
  pinMode( LED_BUILTIN, OUTPUT );
  
  digitalWrite( LED_BUILTIN, true );
  Wire.begin(0x3c); // Initialize I2C (Slave Mode: address=0x3c )
  Wire.onReceive(I2C_RxHandler);

  Serial.begin( 115200 );
  
  delay( 5000 );

  digitalWrite( LED_BUILTIN, false );
}

void loop()
{
  uint8_t byteCount{};
  uint8_t columnStartAddressPAM{};
  uint8_t addressingMode{};
  uint8_t columnStartAddress{};
  uint8_t columnEndAddress{};
  uint8_t pageStartAddress{};
  uint8_t pageEndAddress{};

  Serial.println( F("Let's emulate an SSD1306...") );

  while ( true )
  {
    uint8_t command = fifo.readByte();

    switch( command )
    {
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  0: // 0x00
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  1:
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  2:
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  3:
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  4:
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  5:
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  6:
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  7:  
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  8:  
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  9:  
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 10:  
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 11:  
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 12:  
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 13:  
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 14:  
      case SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15:   // 0x0F
      {
        columnStartAddressPAM &= 0xf0;
        columnStartAddressPAM |= command & 0xf;

        Serial.print( F("SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - low nibble = ") ); Serial.println( command & 0x0f );
        break;
      }

      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  0:
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  1:
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  2:
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  3:
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  4:
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  5:
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  6:
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  7:
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  8:  
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE +  9:  
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 10:  
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 11:  
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 12:  
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 13:  
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 14:  
      case SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15:  // 0x1F
      {
        columnStartAddressPAM &= 0x0f;
        columnStartAddressPAM |= (command & 0xf ) << 4;
        Serial.print( F("SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - high nibble = ") ); Serial.println( command & 0x0f );
        break;
      }

      case SSD1306Command::SET_MEMORY_ADDRESSING_MODE:
      {
        addressingMode = fifo.readByte() & 0x03;
        Serial.print( F("SET_MEMORY_ADDRESSING_MODE - mode = ") ); Serial.println( addressingMode );
        break;
      }
      case SSD1306Command::SET_COLUMN_ADDRESS:
      {
        columnStartAddress = fifo.readByte();
        columnEndAddress = fifo.readByte();
        Serial.print( F("SET_COLUMN_ADDRESS - startAddress = ") ); Serial.print( columnStartAddress );
        Serial.print( F(", endAddress = ") ); Serial.print( columnEndAddress );
        break;
      }
      case SSD1306Command::SET_PAGE_ADDRESS:
      {
        pageStartAddress = fifo.readByte();
        pageEndAddress = fifo.readByte();
        Serial.print( F("SET_PAGE_ADDRESS - startAddress = ") ); Serial.print( pageStartAddress );
        Serial.print( F(", endAddress = ") ); Serial.print( pageEndAddress );
        break;
      }
 /*
  SET_DISPLAY_START_LINE                = 0x40, // 0x40 - 0x7F
  SET_CONTRAST_CONTROL_FOR_BANK0        = 0x81,
  SET_SEGMENT_REMAP                     = 0xA0, // 0xA0/0xA1
  ENTIRE_DISPLAY_ON                     = 0xA4, // 0xA4/0xA5
  SET_NORMAL_DISPLAY                    = 0xA6,
  SET_INVERSE_DISPLAY                   = 0xA7,
  SET_MULTIPLEX_RATIO                   = 0xA8, // double byte command???
  SET_DISPLAY_OFF                       = 0xAE,
  SET_DISPLAY_ON                        = 0xAF,
  SET_PAGE_START_ADDRESS                = 0xB0, // 0xB0 - 0xB7
  SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL  = 0xC0,
  SET_COM_OUTPUT_SCAN_DIRECTION_FLIPPED = 0xC8,
  SET_DISPLAY_OFFSET                    = 0xD3, // double byte command
  SET_DISPLAY_CLOCK_DIVIDE_RATIO        = 0xD5, // double byte command???
  SET_PRECHARGE_PERIOD                  = 0xD9, // ???
  SET_COM_PINS_HARDWARE_CONFIGURATION   = 0xDA,
  SET_VCOMH_DESELECT_LEVEL              = 0xDB,
  NOP                                   = 0xE3,
    */

      default:
        break;
    }


    if ( fifo.isOverflow() ) { Serial.print( F("*** Overflow detected! (") ); Serial.print( fifo.getOverflowCount() ); Serial.println( F(" bytes lost) *** ") ); }
    if ( fifo.isUnderflow() ) { Serial.println( F("*** Underflow detected! *** ") ); }
  }
}
