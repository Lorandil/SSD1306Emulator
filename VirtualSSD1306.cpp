#include <Wire.h>
#include "VirtualSSD1306.h"
#include "ssd1306commands.h"
#include "SerialHexTools.h"

/*--------------------------------------------------------------------------*/
VirtualSSD1306::VirtualSSD1306( uint8_t width /*= 128*/, uint8_t height /*= 64*/ ) : m_width( width ), m_height( height )
{
  // Allocate frame buffer
  // The data is stored as one byte per pixel, making the data handling a lot easier on the cost of (max. 7kB) RAM
  m_pFrameBuffer = (uint8_t *) malloc( width * height * 8 );

  // initalize frame buffer to '0x00'
  memset( m_pFrameBuffer, 0x00, width * height * 8 );
}

/*--------------------------------------------------------------------------*/
VirtualSSD1306::~VirtualSSD1306()
{
  // this is probably never going to happen ;)
  free( m_pFrameBuffer );
}

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::begin( uint8_t i2cAddress )
{
  // clear FIFO
  m_fifo.clear();

  Serial.println( F("Let's emulate an SSD1306...") );

  Serial.print( F("FIFO fill count = ") ); Serial.println( m_fifo.fillCount() );
  Serial.print( F("FIFO free count = ") ); Serial.println( m_fifo.freeCount() );
  Serial.print( F("isEmpty = ") ); Serial.println( m_fifo.isEmpty() );
  Serial.print( F("isFull = ") ); Serial.println( m_fifo.isFull() );

  // initialize I2C
  Wire.begin( i2cAddress );
  // setup I2C handler
  Wire.onReceive( i2cRxHandler );
}

/*--------------------------------------------------------------------------*/
// image data is stored as one pixel per byte
uint8_t VirtualSSD1306::getPixel( uint8_t x, uint8_t y )
{
  return( m_pFrameBuffer[x + y * m_width] );
}

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::i2cRxHandler( int numBytes )
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
          m_fifo.writeValue( uint8_t( Wire.read() ) | TYPE_COMMAND );
        }
      }
      else
      {
        while( Wire.available() )
        {
          m_fifo.writeValue( uint8_t( Wire.read() ) | TYPE_DATA );
        }
      }
    }
    
    // signal if FIFO detected an overflow
    if ( m_fifo.isOverflow() ) { digitalWrite( LED_BUILTIN, true ); }
  }
}

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::processData()
{
  if ( !m_fifo.isEmpty() )
  {
    hexdumpResetPositionCount();
    auto value = m_fifo.readValue();

    if ( ( value & TYPE_MASK ) == TYPE_COMMAND )
    {
      uint8_t command = uint8_t( value );

      Serial.print( F("Command = ") ); printHexToSerial( command ); Serial.print( F(" -> ") );

      if ( command <= SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15 )
      {
        // clear lower nibble
        m_columnStartAddress &= 0xf0;
        // set lower nibble to lower command nibble
        m_columnStartAddress |= ( command & 0xf );
        Serial.print( F("SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - low nibble = ") ); Serial.println( command & 0x0f );
      }
      else if ( command <= SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15 )
      {
        // clear upper nibble
        m_columnStartAddress &= 0x0f;
        // set upper nibble to lower command nibble
        m_columnStartAddress |= (command & 0xf ) << 4;
        Serial.print( F("SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - high nibble = ") ); Serial.println( command & 0x0f );
      }
      else if (    ( command >= SSD1306Command::SET_DISPLAY_START_LINE )
                && ( command <= SSD1306Command::SET_DISPLAY_START_LINE + 0x3F )
              )
      {
        m_displayStartLine = command & 0x3F;
        Serial.print( F("SET_DISPLAY_START_LINE( ") ); ; printHexToSerial( m_displayStartLine ); Serial.println( F(" )" ) ); 
      }
      else if (    ( command >= SSD1306Command::SET_PAGE_START_ADDRESS )        // 0xB0
                && ( command <= SSD1306Command::SET_PAGE_START_ADDRESS + 0x07 ) // 0xB7
              )
      {
        m_page = command & 0x07;
        Serial.print( F("SET_PAGE_START_ADDRESS( ") ); ; printHexToSerial( m_page ); Serial.println( F(" )" ) );
      }
      else 
      {
        switch( command )
        {
          case SSD1306Command::SET_MEMORY_ADDRESSING_MODE:  // 0x20
          {
            m_addressingMode = readCommandByte() & 0x03;
            Serial.print( F("SET_MEMORY_ADDRESSING_MODE( ") ); 
            Serial.print( m_addressingMode == SSD1306Command::HORIZONTAL_ADDRESSING_MODE ? F("horizontal") : ( m_addressingMode == SSD1306Command::PAGE_ADDRESSING_MODE ? F("page") : F("vertical") ) );
            Serial.println( F(" )") );
            break;
          }
          case SSD1306Command::SET_COLUMN_ADDRESS:  // 0x21
          {
            m_columnStartAddress = readCommandByte() & 0x7f;
            m_columnEndAddress = readCommandByte() & 0x7f;
            m_column = m_columnStartAddress;
            Serial.print( F("SET_COLUMN_ADDRESS - startAddress = ") ); Serial.print( m_columnStartAddress );
            Serial.print( F(", endAddress = ") ); Serial.println( m_columnEndAddress );
            break;
          }
          case SSD1306Command::SET_PAGE_ADDRESS:  // 0x22
          {
            m_pageStartAddress = readCommandByte() & 0x07;
            m_pageEndAddress = readCommandByte() & 0x07;
            m_page = m_pageStartAddress;
            Serial.print( F("SET_PAGE_ADDRESS - startAddress = ") ); Serial.print( m_pageStartAddress );
            Serial.print( F(", endAddress = ") ); Serial.println( m_pageEndAddress );
            break;
          }
          case SSD1306Command::HORIZONTAL_SCROLL_RIGHT_SETUP: // 0x26
          case SSD1306Command::HORIZONTAL_SCROLL_LEFT_SETUP:  // 0x27
          {
            // set scroll direction (0 : right, 1 : left)
            m_horizontalScrollDirection = command & 0x01;
            // read dummy byte A[7:0] (0x00)
            readCommandByte();
            // read start m_page B[2:0]
            m_horizontalScrollStartPage = readCommandByte() & 0x07;
            // read interval C[2:0]
            m_horizontalScrollInterval = scrollingTimerInterval[readCommandByte() & 0x07];
            // read end m_page D[2:0]
            m_horizontalScrollEndPage = readCommandByte() & 0x07;
            // read dummy byte E[7:0] (0x00)
            readCommandByte();
            // read dummy byte F[7:0] (0xFF)
            readCommandByte();
            Serial.print( m_horizontalScrollDirection ? F("HORIZONTAL_SCROLL_LEFT_SETUP( startPage = ") : F("HORIZONTAL_SCROLL_RIGHT_SETUP( startPage = ") ); Serial.print( m_horizontalScrollStartPage );
            Serial.print( F(", endPage = ") ); Serial.print( m_horizontalScrollEndPage );
            Serial.print( F(", interval = ") ); Serial.print( m_horizontalScrollInterval );
            Serial.println( F(" )") );
            break;
          }
          case SSD1306Command::DEACTIVATE_SCROLL: // 0x2E
          {
            Serial.println( F("DEACTIVATE_SCROLL") );
            m_horizontalScrollEnabled = false;
            m_verticalScrollEnabled = false;
            break;
          }
          case SSD1306Command::ACTIVATE_SCROLL:   // 0x2F
          {
            Serial.println( F("ACTIVATE_SCROLL") );
            m_horizontalScrollEnabled = true;
            m_verticalScrollEnabled = true;
            m_scrollTimer = 0;
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
            m_segmentRemap = 0;
            break;
          }
          case SSD1306Command::SET_SEGMENT_REMAP + 1:
          {
            Serial.println( F("SET_SEGMENT_REMAP( 127 )") );
            m_segmentRemap = 127;
            break;
          }
          case SSD1306Command::ENTIRE_DISPLAY_ON:     // 0xA4
          case SSD1306Command::ENTIRE_DISPLAY_ON + 1: // 0xA5
          {
            // no idea what's the difference...
            Serial.print( F("ENTIRE_DISPLAY_ON( ") ); Serial.print( command == SSD1306Command::ENTIRE_DISPLAY_ON ? F("normal") : F("forceOn") ); Serial.println( F(" )" ) );
            forceDisplayOn( command == 0xA5 );
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
            displayOn( command == SSD1306Command::SET_DISPLAY_ON );
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
            m_displayOffset = readCommandByte();
            Serial.print( F("SET_DISPLAY_OFFSET( ") ); printHexToSerial( m_displayOffset ); Serial.println( F(" )" ) );
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
      // write 8 pixels to RAM buffer and increment m_column and m_page according to addressing mode
      writePixels( value );
    }
  }

  // check for overflow/underflow
  if ( m_fifo.isOverflow() )
  {
    Serial.println( F("*** FIFO overflow detected!" ) );
    
  }
  if ( m_fifo.isUnderflow() )
  {
    Serial.println( F("*** FIFO underflow detected!" ) );
  }
}

/*---------------------------------------------------------------------------*/
uint8_t VirtualSSD1306::readCommandByte()
{
  if ( !m_fifo.isEmpty() )
  {
    auto value = m_fifo.readValue();

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
uint8_t VirtualSSD1306::readDataByte()
{
  if ( !m_fifo.isEmpty() )
  {
    auto value = m_fifo.readValue();

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

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::writePixels( uint8_t pixels )
{
  // store pixels into frame buffer
  m_pFrameBuffer[m_column + m_page * m_width] = pixels;

  switch( m_addressingMode )
  {
    case SSD1306Command::HORIZONTAL_ADDRESSING_MODE:
      // advance m_column
      m_column++;
      if ( m_column > m_columnEndAddress )
      {
        // return to first m_column and go to next m_page
        m_column = m_columnStartAddress;
        m_page++;
        if ( m_page > m_pageEndAddress )
        {
          m_page = m_pageStartAddress;
        }
      }
      break;
    case SSD1306Command::VERTICAL_ADDRESSING_MODE:
      // advance m_page
      m_page++;
      if ( m_page > m_pageEndAddress )
      {
        // return to first m_page, go to next m_column
        m_page = m_pageStartAddress;
        m_column++;
        if ( m_column > m_columnEndAddress )
        {
          m_column = m_columnStartAddress;
        }
      }
      break;
    case SSD1306Command::PAGE_ADDRESSING_MODE:
    default:
      // limit m_column to screen width
      m_column++;
      if ( m_column > m_columnEndAddress )
      { 
        m_column = m_columnStartAddress;
      }
      break;
  }
}

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::performScrolling()
{
  // perform scrolling if required
  if ( m_horizontalScrollEnabled )
  {
    if ( ( m_scrollTimer % m_horizontalScrollInterval ) == 0 )
    {
      scrollHorizontal();
    }
  }

  if ( m_verticalScrollEnabled )
  {
    if ( ( m_scrollTimer % m_verticalScrollInterval ) == 0 )
    {
      scrollVertical();
    }
  }

  // another one finished
  m_scrollTimer++;
}

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::scrollHorizontal()
{
  auto frameBuffer = getFrameBuffer();

  for ( uint8_t y = m_horizontalScrollStartPage; y <= m_horizontalScrollEndPage; y++ )
  {
    if ( m_horizontalScrollDirection == 1 )
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

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::scrollVertical()
{
}