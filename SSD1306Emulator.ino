/*
* LAB Name: Arduino I2C Slave(Rx)
* Author: Khaled Magdy
* For More Info Visit: www.DeepBlueMbedded.com
*/
#include "SerialHexTools.h"
#include <Wire.h>
#include "fifo.hpp"
#include "ssd1306commands.h"

#include <PicoDVI.h> // Core display & graphics library

// 320x240 16-bit color display (to match common TFT display resolution):
DVIGFX16 display(DVI_RES_320x240p60, pico_sock_cfg);


SimpleFIFO<uint16_t> fifo;

constexpr uint16_t TYPE_COMMAND = 0x0000;
constexpr uint16_t TYPE_DATA    = 0x8000;
constexpr uint16_t DATA_MASK   = 0x00ff;
constexpr uint16_t TYPE_MASK   = 0x8000;

static char frameBuffer[1024];
 
/*---------------------------------------------------------------------------*/
// SSD1306 decoder
void I2C_RxHandler( int numBytes )
{
  if ( numBytes != 0 )
  {
    // command or data received?
    bool isCommand = !( Wire.read() );
    {
      if ( isCommand )
      {
        while( Wire.available() )
        {
          fifo.writeValue( uint8_t( Wire.read() ) | TYPE_COMMAND );
        }
      }
      else
      {
        while( Wire.available() )
        {
          fifo.writeValue( uint8_t( Wire.read() ) | TYPE_DATA );
        }
      }
    }
    
    // signal if FIFO detected an overflow
    if ( fifo.isOverflow() ) { digitalWrite( LED_BUILTIN, true ); }
  }
}
 
/*---------------------------------------------------------------------------*/
void setup()
{
  pinMode( LED_BUILTIN, OUTPUT );
  
  digitalWrite( LED_BUILTIN, true );
  Wire.begin(0x3c); // Initialize I2C (Slave Mode: address=0x3c )
  Wire.onReceive(I2C_RxHandler);

  Serial.begin( 115200 );
  
  if (!display.begin()) { // Blink LED if insufficient RAM
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }
  
  delay( 5000 );

  fifo.clear();

  // no overflow so far
  digitalWrite( LED_BUILTIN, false );
}

/*---------------------------------------------------------------------------*/
void loop()
{
  for ( int y = 0; y < display.height(); y++ )
  {
    for ( int x = 0; x < 256; x++ )
    {
      display.drawPixel(x + 32, y, y * display.width() + x );
    }
  }



  uint8_t count{};
  uint8_t columnStartAddressPAM{};
  uint8_t addressingMode{};
  uint8_t segmentRemap{};
  uint8_t columnStartAddress{};
  uint8_t columnEndAddress{};
  uint8_t pageStartAddress{};
  uint8_t pageEndAddress{};
  uint8_t displayOffset{};
  uint8_t displayStartLine{};
  uint8_t scrollMode{};

  Serial.println( F("Let's emulate an SSD1306...") );

  Serial.print( F("FIFO fill count = ") ); Serial.println( fifo.fillCount() );
  Serial.print( F("FIFO free count = ") ); Serial.println( fifo.freeCount() );
  Serial.print( F("isEmpty = ") ); Serial.println( fifo.isEmpty() );
  Serial.print( F("isFull = ") ); Serial.println( fifo.isFull() );

  while ( count++ < 65536 )
  {
    hexdumpResetPositionCount();

    while( fifo.isEmpty() );

    auto value = fifo.readValue();

    if ( ( value & TYPE_MASK ) == TYPE_COMMAND )
    {
      uint8_t command = uint8_t( value );

      Serial.print( F("Command = ") ); printHexToSerial( command ); Serial.print( F(" -> ") );

      if ( command <= SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15 )
      {
        columnStartAddressPAM &= 0xf0;
        columnStartAddressPAM |= command & 0xf;
        Serial.print( F("SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - low nibble = ") ); Serial.println( command & 0x0f );
      }
      else if ( command <= SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15 )
      {
          columnStartAddressPAM &= 0x0f;
          columnStartAddressPAM |= (command & 0xf ) << 4;
          Serial.print( F("SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - high nibble = ") ); Serial.println( command & 0x0f );
      }
      else if (    ( command >= SSD1306Command::SET_DISPLAY_START_LINE )
                && ( command <= SSD1306Command::SET_DISPLAY_START_LINE + 0x3F )
              )
      {
          displayStartLine = command & 0x3F;
          Serial.print( F("SET_DISPLAY_START_LINE( ") ); ; printHexToSerial( displayStartLine ); Serial.println( F(" )" ) ); 
      }
      else if (    ( command >= SSD1306Command::SET_PAGE_START_ADDRESS )        // 0xB0
                && ( command <= SSD1306Command::SET_PAGE_START_ADDRESS + 0x07 ) // 0xB7
              )
      {
        
      }
      else 
      {
        switch( command )
        {
          case SSD1306Command::SET_MEMORY_ADDRESSING_MODE:  // 0x20
          {
            addressingMode = readCommandByte() & 0x03;
            Serial.print( F("SET_MEMORY_ADDRESSING_MODE( ") ); 
            Serial.print( addressingMode == 0x00 ? F("horizontal") : ( addressingMode == 0x01 ? F("page") : F("vertical") ) );
            Serial.println( F(" )") );
            break;
          }
          case SSD1306Command::SET_COLUMN_ADDRESS:  // 0x21
          {
            columnStartAddress = readCommandByte();
            columnEndAddress = readCommandByte();
            Serial.print( F("SET_COLUMN_ADDRESS - startAddress = ") ); Serial.print( columnStartAddress );
            Serial.print( F(", endAddress = ") ); Serial.println( columnEndAddress );
            break;
          }
          case SSD1306Command::SET_PAGE_ADDRESS:  // 0x22
          {
            pageStartAddress = readCommandByte();
            pageEndAddress = readCommandByte();
            Serial.print( F("SET_PAGE_ADDRESS - startAddress = ") ); Serial.print( pageStartAddress );
            Serial.print( F(", endAddress = ") ); Serial.println( pageEndAddress );
            break;
          }
          case SSD1306Command::DEACTIVATE_SCROLL: // 0x2E
          {
            Serial.println( F("DEACTIVATE_SCROLL") );
            scrollMode = 0;
            break;
          }
          case SSD1306Command::SET_CONTRAST_CONTROL_FOR_BANK0:  // 0x81
          {
            Serial.print( F("SET_CONTRAST_CONTROL_FOR_BANK0( ") ); printHexToSerial( readCommandByte() ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::CHARGE_PUMP_SETTING:
          {
            uint8_t mode = readCommandByte();
            Serial.print( F("CHARGE_PUMP_SETTING( ") ); Serial.print( mode == 0x10 ? F("EXTERNALVCC") : F("ENABLE_CHARGE_PUMP") ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_SEGMENT_REMAP:
          {
            Serial.println( F("SET_SEGMENT_REMAP( 0 )") );
            segmentRemap = 0;
            break;
          }
          case SSD1306Command::SET_SEGMENT_REMAP + 1:
          {
            Serial.println( F("SET_SEGMENT_REMAP( 127 )") );
            segmentRemap = 127;
            break;
          }
          case SSD1306Command::ENTIRE_DISPLAY_ON:     // 0xA4
          case SSD1306Command::ENTIRE_DISPLAY_ON + 1: // 0xA5
          {
            // no idea what's the difference...
            Serial.print( F("ENTIRE_DISPLAY_ON( ") ); Serial.print( command == SSD1306Command::ENTIRE_DISPLAY_ON ? F("normal") : F("always") ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_NORMAL_DISPLAY:    // 0xA6
          case SSD1306Command::SET_INVERSE_DISPLAY:   // 0xA7
          {
            // no idea what's the difference...
            Serial.println( command == SSD1306Command::SET_NORMAL_DISPLAY ? F("SET_NORMAL_DISPLAY") : F("SET_INVERSE_DISPLAY") );
            break;
          }
          case SSD1306Command::SET_MULTIPLEX_RATIO: // 0xA8
          {
            Serial.print( F("SET_MULTIPLEX_RATIO( ") ); printHexToSerial( readCommandByte() ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_DISPLAY_OFF: // 0xAE
          case SSD1306Command::SET_DISPLAY_ON:  // 0xAF
          {
            Serial.println( command == SSD1306Command::SET_DISPLAY_OFF ? F("SET_DISPLAY_OFF") : F("SET_DISPLAY_ON") );
            break;
          }
          case SSD1306Command::SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL:  // 0xC0
          case SSD1306Command::SET_COM_OUTPUT_SCAN_DIRECTION_FLIPPED: // 0xC8
          {
            Serial.println( command == SET_DISPLAY_OFF ? F("SET_DISPLAY_OFF") : F("SET_DISPLAY_ON") );
            break;
          }
          case SSD1306Command::SET_DISPLAY_OFFSET:              // 0xD3
          {
            // read oofset parameter
            displayOffset = readCommandByte();
            Serial.print( F("SET_DISPLAY_OFFSET( ") ); printHexToSerial( displayOffset ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_DISPLAY_CLOCK_DIVIDE_RATIO:  // 0xD5
          {
            Serial.print( F("SET_DISPLAY_CLOCK_DIVIDE_RATIO( ") ); printHexToSerial( readCommandByte() ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_COM_PINS_HARDWARE_CONFIGURATION: // 0xDA,
          {
            Serial.print( F("SET_COM_PINS_HARDWARE_CONFIGURATION( ") ); printHexToSerial( readCommandByte() ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_PRECHARGE_PERIOD: // 0xD9
          {
            Serial.print( F("SET_PRECHARGE_PERIOD( ") ); printHexToSerial( readCommandByte() ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_VCOMH_DESELECT_LEVEL: // 0xDB
          {
            Serial.print( F("SET_VCOMH_DESELECT_LEVEL( ") ); printHexToSerial( readCommandByte() ); Serial.println( F(" )" ) );
            break;
          }
          case SSD1306Command::NOP: // 0xE3
          {
            Serial.println( F("NOP") );
            break;
          }
          default:
          {
            Serial.println( F("*** unknown command ***" ) );
            break;
          }
        }
      }
    }
    else
    {
      Serial.print( F("  received data byte ") ); printHexToSerial( uint8_t( value ) ); Serial.println();
    }

    //if ( fifo.isOverflow() ) { Serial.print( F("*** Overflow detected! (") ); Serial.print( fifo.getOverflowCount() ); Serial.println( F(" bytes lost) *** ") ); }
    //if ( fifo.isUnderflow() ) { Serial.println( F("*** Underflow detected! *** ") ); }
  }

  while( true );
}

/*---------------------------------------------------------------------------*/
uint8_t readCommandByte()
{
  if ( !fifo.isEmpty() )
  {
    auto value = fifo.readValue();

    if ( ( value & TYPE_MASK ) == TYPE_COMMAND )
    {
      return( uint8_t( value ) );
    }
    Serial.println( F("*** Command expected!") );
  }
  else
  {
    Serial.println( F("*** Command underflow detected!") );
  }

  return( 0 );
}

/*---------------------------------------------------------------------------*/
uint8_t readDataByte()
{
  if ( !fifo.isEmpty() )
  {
    auto value = fifo.readValue();

    if ( ( value & TYPE_MASK ) == TYPE_DATA )
    {
      return( uint8_t( value ) );
    }
    Serial.println( F("*** Data expected!") );
  }
  else
  {
    Serial.println( F("*** Data underflow detected!") );
  }

  return( 0 );
}