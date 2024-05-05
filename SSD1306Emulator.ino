/*
* LAB Name: Arduino I2C Slave(Rx)
* Author: Khaled Magdy
* For More Info Visit: www.DeepBlueMbedded.com
*/
#include "SerialHexTools.h"`
#include <Wire.h>
#include "fifo.hpp"
#include "ssd1306commands.h"

#include <PicoDVI.h> // Core display & graphics library

// 320x240 16-bit color display (to match common TFT display resolution):
DVIGFX16 display(DVI_RES_320x240p60, pico_sock_cfg);


SimpleFIFO<uint16_t> fifo;

constexpr uint16_t TYPE_COMMAND = 0x0000;
constexpr uint16_t TYPE_DATA    = 0x8000;
constexpr uint16_t DATA_MASK    = 0x00ff;
constexpr uint16_t TYPE_MASK    = 0x8000;

uint32_t count{};
//uint8_t  columnStartAddressPAM{};
uint8_t  addressingMode{};
uint8_t  segmentRemap{};
uint8_t  columnStartAddress{0};
uint8_t  columnEndAddress{SSD1306Command::DISPLAY_WIDTH - 1};
uint8_t  pageStartAddress{0x00};
uint8_t  pageEndAddress{0x07};
uint8_t  displayOffset{};
uint8_t  displayStartLine{};

// horizontal scrolling
bool     horizontalScrollEnabled{};
uint8_t  horizontalScrollDirection{};
uint8_t  horizontalScrollStartPage{};
uint8_t  horizontalScrollInterval{};
uint8_t  horizontalScrollEndPage{};

// vertical scrolling
bool     verticalScrollEnabled{};
//uint8_t  horizontalScrollDirection{};
//uint8_t  horizontalScrollStartPage{};
//uint8_t  horizontalScrollInterval{};
//uint8_t  horizontalScrollEndPage{};

bool    forceDisplayOn{false};
bool    invertDisplay{false};
// running counters
uint8_t page{};
uint8_t column{};


static uint8_t frameBuffer[1024];
 
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
  renderBackground();

  Serial.println( F("Let's emulate an SSD1306...") );

  Serial.print( F("FIFO fill count = ") ); Serial.println( fifo.fillCount() );
  Serial.print( F("FIFO free count = ") ); Serial.println( fifo.freeCount() );
  Serial.print( F("isEmpty = ") ); Serial.println( fifo.isEmpty() );
  Serial.print( F("isFull = ") ); Serial.println( fifo.isFull() );

  while ( true )
  {

    if ( !fifo.isEmpty() )
    {
      hexdumpResetPositionCount();
      auto value = fifo.readValue();

      if ( ( value & TYPE_MASK ) == TYPE_COMMAND )
      {
        uint8_t command = uint8_t( value );

        Serial.print( F("Command = ") ); printHexToSerial( command ); Serial.print( F(" -> ") );

        if ( command <= SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15 )
        {
          // clear lower nibble
          columnStartAddress &= 0xf0;
          // set lower nibble to lower command nibble
          columnStartAddress |= ( command & 0xf );
          Serial.print( F("SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - low nibble = ") ); Serial.println( command & 0x0f );
        }
        else if ( command <= SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15 )
        {
          // clear upper nibble
          columnStartAddress &= 0x0f;
          // set upper nibble to lower command nibble
          columnStartAddress |= (command & 0xf ) << 4;
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
          page = command & 0x07;
          Serial.print( F("SET_PAGE_START_ADDRESS( ") ); ; printHexToSerial( page ); Serial.println( F(" )" ) );
        }
        else 
        {
          switch( command )
          {
            case SSD1306Command::SET_MEMORY_ADDRESSING_MODE:  // 0x20
            {
              addressingMode = readCommandByte() & 0x03;
              Serial.print( F("SET_MEMORY_ADDRESSING_MODE( ") ); 
              Serial.print( addressingMode == SSD1306Command::HORIZONTAL_ADDRESSING_MODE ? F("horizontal") : ( addressingMode == SSD1306Command::PAGE_ADDRESSING_MODE ? F("page") : F("vertical") ) );
              Serial.println( F(" )") );
              break;
            }
            case SSD1306Command::SET_COLUMN_ADDRESS:  // 0x21
            {
              columnStartAddress = readCommandByte() & 0x7f;
              columnEndAddress = readCommandByte() & 0x7f;
              column = columnStartAddress;
              Serial.print( F("SET_COLUMN_ADDRESS - startAddress = ") ); Serial.print( columnStartAddress );
              Serial.print( F(", endAddress = ") ); Serial.println( columnEndAddress );
              break;
            }
            case SSD1306Command::SET_PAGE_ADDRESS:  // 0x22
            {
              pageStartAddress = readCommandByte() & 0x07;
              pageEndAddress = readCommandByte() & 0x07;
              page = pageStartAddress;
              Serial.print( F("SET_PAGE_ADDRESS - startAddress = ") ); Serial.print( pageStartAddress );
              Serial.print( F(", endAddress = ") ); Serial.println( pageEndAddress );
              break;
            }
            case SSD1306Command::HORIZONTAL_SCROLL_RIGHT_SETUP: // 0x26
            case SSD1306Command::HORIZONTAL_SCROLL_LEFT_SETUP:  // 0x27
            {
              // set scroll direction (0 : right, 1 : left)
              horizontalScrollDirection = command & 0x01;
              // read dummy byte A[7:0] (0x00)
              readCommandByte();
              // read start page B[2:0]
              horizontalScrollStartPage = readCommandByte() & 0x07;
              // read interval C[2:0]
              horizontalScrollInterval = readCommandByte() & 0x07;
              // read end page D[2:0]
              horizontalScrollEndPage = readCommandByte() & 0x07;
              // read dummy byte E[7:0] (0x00)
              readCommandByte();
              // read dummy byte F[7:0] (0xFF)
              readCommandByte();
              Serial.print( horizontalScrollDirection ? F("HORIZONTAL_SCROLL_LEFT_SETUP( startPage = ") : F("HORIZONTAL_SCROLL_RIGHT_SETUP( startPage = ") ); Serial.print( horizontalScrollStartPage );
              Serial.print( F(", endPage = ") ); Serial.print( horizontalScrollEndPage );
              Serial.print( F(", interval = ") ); Serial.print( horizontalScrollInterval );
              Serial.println( F(" )") );
              break;
            }
            case SSD1306Command::DEACTIVATE_SCROLL: // 0x2E
            {
              Serial.println( F("DEACTIVATE_SCROLL") );
              horizontalScrollEnabled = false;
              verticalScrollEnabled = false;
              break;
            }
            case SSD1306Command::ACTIVATE_SCROLL:   // 0x2F
            {
              Serial.println( F("ACTIVATE_SCROLL") );
              horizontalScrollEnabled = true;
              verticalScrollEnabled = true;
              break;
            }
            case SSD1306Command::SET_CONTRAST_CONTROL_FOR_BANK0:  // 0x81
            {
              Serial.print( F("SET_CONTRAST_CONTROL_FOR_BANK0( ") ); printHexToSerial( readCommandByte() ); Serial.println( F(" )" ) );
              break;
            }
            case SSD1306Command::CHARGE_PUMP_SETTING:
            {
              Serial.print( F("CHARGE_PUMP_SETTING( ") ); Serial.print( readCommandByte() == 0x10 ? F("EXTERNALVCC") : F("ENABLE_CHARGE_PUMP") ); Serial.println( F(" )" ) );
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
              Serial.print( F("ENTIRE_DISPLAY_ON( ") ); Serial.print( command == SSD1306Command::ENTIRE_DISPLAY_ON ? F("normal") : F("forceOn") ); Serial.println( F(" )" ) );
              forceDisplayOn = ( command == 0xA5 );
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
              Serial.println( command == SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL ? F("SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL") : F("SET_COM_OUTPUT_SCAN_DIRECTION_FLIPPED") );
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
              hexdumpResetPositionCount();
              Serial.print( F("Command = ") ); printHexToSerial( command ); Serial.print( F(" -> ") );
              Serial.println( F("*** unknown command ***" ) );
              break;
            }
          }
        }
      }
      else
      {
        // write 8 pixels to RAM buffer and increment column and page according to addressing mode
        writePixels( value );
      }
    }

    // render the screen all 1000 cycles
    if ( ( count++ % 1000 ) == 0 )
    {
      if ( horizontalScrollEnabled )
      {
        for ( uint8_t y = horizontalScrollStartPage; y <= horizontalScrollEndPage; y++ )
        {
          if ( horizontalScrollDirection == 1 )
          {
            // scroll left
            uint8_t *dest = &frameBuffer[y * SSD1306Command::DISPLAY_WIDTH];
            uint8_t *src = dest + 1;
            auto lost = *dest;
            memmove( dest, src, SSD1306Command::DISPLAY_WIDTH - 1 );
            frameBuffer[( y + 1 ) * SSD1306Command::DISPLAY_WIDTH - 1] = lost;
          }
          else
          {
            // scroll right
            uint8_t *src = &frameBuffer[y * SSD1306Command::DISPLAY_WIDTH];
            uint8_t *dest = src + 1;
            auto lost = frameBuffer[( y + 1 ) * SSD1306Command::DISPLAY_WIDTH - 1];
            memmove( dest, src, SSD1306Command::DISPLAY_WIDTH -1 );
            *src = lost;
          }
        }
      }

      renderScreen();
    }

    if ( fifo.isOverflow() )
    {
      Serial.println( F("*** FIFO overflow detected!" ) );
     
    }
    if ( fifo.isUnderflow() )
    {
      Serial.println( F("*** FIFO underflow detected!" ) );
    }
  }
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

/*---------------------------------------------------------------------------*/
void writePixels( uint8_t pixels )
{
  // store pixels into frame buffer
  frameBuffer[column + page * SSD1306Command::DISPLAY_WIDTH] = pixels;
  switch( addressingMode )
  {
    case SSD1306Command::HORIZONTAL_ADDRESSING_MODE:
      // advance column
      column++;
      if ( column > columnEndAddress )
      {
        // return to first column and go to next page
        column = columnStartAddress;
        page++;
        if ( page > pageEndAddress )
        {
          page = pageStartAddress;
        }
      }
      break;
    case SSD1306Command::VERTICAL_ADDRESSING_MODE:
      // advance page
      page++;
      if ( page > pageEndAddress )
      {
        // return to first page, go to next column
        page = pageStartAddress;
        column++;
        if ( column >columnEndAddress )
        {
          column = columnStartAddress;
        }
      }
      break;
    case SSD1306Command::PAGE_ADDRESSING_MODE:
    default:
      // limit column to screen width
      column++;
      if ( column > columnEndAddress )
      { 
        column = columnStartAddress;
      }
      break;
  }
}

/*---------------------------------------------------------------------------*/
void renderBackground()
{
  Serial.println( F("renderBackground()") );

  for ( int y = 0; y < display.height(); y++ )
  {
    for ( int x = 0; x < display.width(); x++ )
    {
      display.drawPixel(x, y, y * display.width() + x );
    }
  }
}

/*---------------------------------------------------------------------------*/
// Render the screen in the selected style (just 1:1 for now)
void renderScreen()
{
  //Serial.println( F("renderScreen()") );

  for ( int y = 0; y < 2 * SSD1306Command::DISPLAY_HEIGHT; y++ )
  {
    for ( int x = 0; x < 2 * SSD1306Command::DISPLAY_WIDTH; x++ )
    {
      uint16_t offsetY = y >> 4;
      bool pixelValue = ( ( frameBuffer[( x / 2 ) + offsetY * SSD1306Command::DISPLAY_WIDTH] & ( 1 << ( ( y / 2 ) & 0x07 ) ) ) != 0 ) | forceDisplayOn;
      pixelValue ^= invertDisplay;
      display.drawPixel(x + 32, y + 48, pixelValue ? 0xFFFF : 0x0000 );
    }
  }
}