#include <Wire.h>
#include "VirtualSSD1306.h"
#include "ssd1306commands.h"
#include "SerialHexTools.h"

//#define STOP_ON_ERROR

/*--------------------------------------------------------------------------*/
VirtualSSD1306::VirtualSSD1306( uint16_t width /*= 128*/, uint16_t height /*= 64*/, bool enableDebugOutput ) : VirtualDisplayBase( width, height, enableDebugOutput )
{
  // Allocate frame buffer
  // The data is stored as one byte per pixel, making the data handling a lot easier on the cost of (max. 7kB) RAM
  m_pFrameBuffer = (uint8_t *) malloc( width * height );
  // another buffer for vertical scrolling - for simplicity's sake we use a full size buffer (thank me later)
  m_pScrollBuffer = (uint8_t *) malloc( width * height );

  // default values for vertical scroll area
  m_verticalTopFixedLines = 0;
  m_verticalScrollAreaLines = 64;

  // initalize frame buffer to '0x00'
  memset( m_pFrameBuffer, 0x00, width * height );
}

/*--------------------------------------------------------------------------*/
VirtualSSD1306::~VirtualSSD1306()
{
  // this is probably never going to happen ;)
  free( m_pFrameBuffer );
  free( m_pScrollBuffer );
}

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::begin( uint8_t i2cAddress )
{
  // clear FIFO
  m_fifo.clear();

  Serial.print( F("Let's emulate an SSD1306 on address 0x") ); Serial.print( i2cAddress, HEX ); Serial.println( F("...") );
  printDebugInfo();

  // increase FIFO buffer size if supported by Wire library
#ifdef WIRE_HAS_BUFFER_SIZE 
  Serial.print(F("Trying to set i2c receive buffer size to ")); Serial.println( VIRTUAL_SSD1306_RECEIVE_FIFO_BUFFER_SIZE );
  auto newSize = Wire.setBufferSize( VIRTUAL_SSD1306_RECEIVE_FIFO_BUFFER_SIZE );
  if ( newSize != VIRTUAL_SSD1306_RECEIVE_FIFO_BUFFER_SIZE )
  {
    Serial.print( F("*** setBufferSize() failed and returned a buffer size of ") ); Serial.print( newSize ); Serial.println( F(" bytes instead!") );
  }
#endif

  // initialize I2C
  Wire.begin( i2cAddress );
  // setup I2C handler
  Wire.onReceive( i2cRxHandler );
}

/*--------------------------------------------------------------------------*/
// Image data is stored as one pixel per byte
// The following effects are supported:
// - forced display on
// - display inversion
// - display on/off
uint8_t VirtualSSD1306::getPixel( uint8_t x, uint8_t y )
{
  auto pixelValue = m_pFrameBuffer[x + y * m_width] | m_forceDisplayOn;
  pixelValue ^= m_invertDisplay;
  pixelValue &= m_displayOn;
  return( pixelValue );
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

      DebugOutput( F("Command = ") ); if( m_enableDebugOutput ) { printHexToSerial( command ); } DebugOutput( F(" -> ") );

      if ( command <= SSD1306Command::SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15 )
      {
        // clear lower nibble
        m_columnStartAddress &= 0xf0;
        // set lower nibble to lower command nibble
        m_columnStartAddress |= ( command & 0xf );
        DebugOutput( F("SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - low nibble = ") ); DebugOutputLn( command & 0x0f );
      }
      else if ( command <= SSD1306Command::SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE + 15 )
      {
        // clear upper nibble
        m_columnStartAddress &= 0x0f;
        // set upper nibble to lower command nibble
        m_columnStartAddress |= (command & 0xf ) << 4;
        DebugOutput( F("SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE - high nibble = ") ); DebugOutputLn( command & 0x0f );
      }
      else if (    ( command >= SSD1306Command::SET_DISPLAY_START_LINE )
                && ( command <= SSD1306Command::SET_DISPLAY_START_LINE + 0x3F )
              )
      {
        m_displayStartLine = command & 0x3F;
        DebugOutput( F("SET_DISPLAY_START_LINE( ") ); ; if( m_enableDebugOutput ) { printHexToSerial( m_displayStartLine ); } DebugOutputLn( F(" )" ) ); 
      }
      else if (    ( command >= SSD1306Command::SET_PAGE_START_ADDRESS )        // 0xB0
                && ( command <= SSD1306Command::SET_PAGE_START_ADDRESS + 0x07 ) // 0xB7
              )
      {
        m_page = command & 0x07;
        DebugOutput( F("SET_PAGE_START_ADDRESS( ") ); if( m_enableDebugOutput ) { printHexToSerial( m_page ); } DebugOutputLn( F(" )" ) );
      }
      else 
      {
        switch( command )
        {
          case SSD1306Command::SET_MEMORY_ADDRESSING_MODE:  // 0x20
          {
            m_addressingMode = readCommandByte() & 0x03;
            DebugOutput( F("SET_MEMORY_ADDRESSING_MODE( ") );
            DebugOutput( ( m_addressingMode == SSD1306Command::HORIZONTAL_ADDRESSING_MODE ) ? F("horizontal") : ( m_addressingMode == SSD1306Command::PAGE_ADDRESSING_MODE ? F("page") : F("vertical") ) );
            DebugOutput( F(" )") );

            // reset start and end values on addressing mode change
            m_page = 0;
            m_pageStartAddress = 0;
            m_pageEndAddress = ( m_height >> 3 ) - 1;
            m_column = 0;
            m_columnStartAddress = 0;
            m_columnEndAddress = m_width - 1;

            //DebugOutput( F(", columnStart = ") ); DebugOutput( m_columnStartAddress );
            //DebugOutput( F(", columnEnd = ") ); DebugOutput( m_columnEndAddress );
            //DebugOutput( F(", pageStart = ") ); DebugOutput( m_pageStartAddress );
            //DebugOutput( F(", pageEnd = ") ); DebugOutputLn( m_pageEndAddress );
            break;
          }
          case SSD1306Command::SET_COLUMN_ADDRESS:          // 0x21
          {
            m_columnStartAddress = readCommandByte() & 0x7f;
            m_columnEndAddress = readCommandByte() & 0x7f;
            m_column = m_columnStartAddress;
            DebugOutput( F("SET_COLUMN_ADDRESS - startAddress = ") ); DebugOutput( m_columnStartAddress );
            DebugOutput( F(", endAddress = ") ); DebugOutputLn( m_columnEndAddress );
            break;
          }
          case SSD1306Command::SET_PAGE_ADDRESS:            // 0x22
          {
            m_pageStartAddress = readCommandByte() & 0x07;
            m_pageEndAddress = readCommandByte() & 0x07;
            m_page = m_pageStartAddress;
            DebugOutput( F("SET_PAGE_ADDRESS - startAddress = ") ); DebugOutput( m_pageStartAddress );
            DebugOutput( F(", endAddress = ") ); DebugOutputLn( m_pageEndAddress );
            break;
          }
          case SSD1306Command::HORIZONTAL_SCROLL_RIGHT_SETUP:                        // 0x26
          case SSD1306Command::HORIZONTAL_SCROLL_LEFT_SETUP:                         // 0x27
          {
            // set scroll direction (0 : right, 1 : left)
            m_horizontalScrollDirection =  ( command == SSD1306Command::HORIZONTAL_SCROLL_LEFT_SETUP );
            // read dummy byte A[7:0] (0x00)
            readCommandByte();
            // read start m_page B[2:0]
            m_scrollStartPage = readCommandByte() & 0x07;
            // read interval C[2:0]
            m_scrollInterval = scrollingTimerInterval[readCommandByte() & 0x07];
            // read end m_page D[2:0]
            m_scrollEndPage = readCommandByte() & 0x07;
            // read dummy byte E[7:0] (0x00)
            m_verticalScrollOffset = readCommandByte();
            // read dummy byte F[7:0] (0xFF)
            readCommandByte();

            switch( command )
            {
              case SSD1306Command::HORIZONTAL_SCROLL_RIGHT_SETUP:
                DebugOutput( F("HORIZONTAL_SCROLL_RIGHT_SETUP") ); 
                break;
              case SSD1306Command::HORIZONTAL_SCROLL_LEFT_SETUP:
              default:
                DebugOutput( F("HORIZONTAL_SCROLL_LEFT_SETUP") );
                break;
            }
            DebugOutput( F("( startPage = ") ); DebugOutput( m_scrollStartPage ); DebugOutput( F(", endPage = ") ); DebugOutput( m_scrollEndPage );
            DebugOutput( F(", interval = ") ); DebugOutput( m_scrollInterval ); DebugOutput( F(" frames, offset = ") ); DebugOutput( m_verticalScrollOffset );
            DebugOutputLn( F(" )") );

            // safety check - ensure startpage <= endpage
            if ( m_scrollStartPage > m_scrollEndPage )
            {
              m_scrollStartPage = m_scrollEndPage;
              DebugOutput( F("*** fixed invalid start page - new m_scrollStartPage = ") ); DebugOutputLn( m_scrollStartPage );
            #ifdef STOP_ON_ERROR
              DebugOutputLn( F("<STOPPED>") );
              while(1);
            #endif
            }
            break;
          }
          case SSD1306Command::CONTINOUS_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL_SETUP: // 0x29
          case SSD1306Command::CONTINOUS_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL_SETUP:  // 0x2A
          {
            // set scroll direction (0 : right, 1 : left)
            m_horizontalScrollDirection = ( command == SSD1306Command::CONTINOUS_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL_SETUP );
            // read dummy byte A[7:0] (0x00)
            readCommandByte();
            // read start m_page B[2:0]
            m_scrollStartPage = readCommandByte() & 0x07;
            // read interval C[2:0]
            m_scrollInterval = scrollingTimerInterval[readCommandByte() & 0x07];
            // read end m_page D[2:0]
            m_scrollEndPage = readCommandByte() & 0x07;
            // read dummy byte E[7:0] (0x00)
            m_verticalScrollOffset = readCommandByte();

            switch( command )
            {
              case SSD1306Command::CONTINOUS_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL_SETUP:
                DebugOutput( F("CONTINOUS_VERTICAL_AND_RIGHT_HORIZONTAL_SCOLL_SETUP") );
                break;
              case SSD1306Command::CONTINOUS_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL_SETUP:
              default:
                DebugOutput( F("CONTINOUS_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL_SETUP") );
                break;
            }
            DebugOutput( F("( startPage = ") ); DebugOutput( m_scrollStartPage ); DebugOutput( F(", endPage = ") ); DebugOutput( m_scrollEndPage );
            DebugOutput( F(", interval = ") ); DebugOutput( m_scrollInterval ); DebugOutput( F(" frames, offset = ") ); DebugOutput( m_verticalScrollOffset );
            DebugOutputLn( F(" )") );
            // safety check - ensure startpage <= endpage
            if ( m_scrollStartPage > m_scrollEndPage )
            {
              m_scrollStartPage = m_scrollEndPage;
              DebugOutput( F("*** fixed invalid start page - new m_scrollStartPage = ") ); DebugOutputLn( m_scrollStartPage );
            }
            break;
          }
          case SSD1306Command::DEACTIVATE_SCROLL: // 0x2E
          {
            DebugOutputLn( F("DEACTIVATE_SCROLL") );
            m_scrollEnabled = false;
            break;
          }
          case SSD1306Command::ACTIVATE_SCROLL:   // 0x2F
          {
            DebugOutputLn( F("ACTIVATE_SCROLL") );
            m_scrollEnabled = true;
            m_scrollTimer = 0;
            break;
          }
          case SSD1306Command::SET_CONTRAST_CONTROL_FOR_BANK0:  // 0x81
          {
            DebugOutput( F("SET_CONTRAST_CONTROL_FOR_BANK0( ") ); if( m_enableDebugOutput ) { printHexToSerial( readCommandByte() ); } DebugOutputLn( F(" )" ) );
            break;
          }
          case SSD1306Command::CHARGE_PUMP_SETTING:
          {
            DebugOutput( F("CHARGE_PUMP_SETTING( ") ); DebugOutput( readCommandByte() == 0x10 ? F("EXTERNALVCC") : F("ENABLE_CHARGE_PUMP") ); DebugOutputLn( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_SEGMENT_REMAP:
          {
            DebugOutputLn( F("SET_SEGMENT_REMAP( 0 )") );
            m_segmentRemap = 0;
            break;
          }
          case SSD1306Command::SET_SEGMENT_REMAP + 1:
          {
            DebugOutputLn( F("SET_SEGMENT_REMAP( 127 )") );
            m_segmentRemap = 127;
            break;
          }
          case SSD1306Command::SET_VERTICAL_SCROLL_AREA:  // 0xA3
          {
            m_verticalTopFixedLines = readCommandByte() % m_height;
            m_verticalScrollAreaLines = readCommandByte() % m_height;
            DebugOutput( F("SET_VERTICAL_SCROLL_AREA( topFixedLines = ") ); DebugOutput( m_verticalTopFixedLines );
            DebugOutput( F(", scrollAreaLines = ") ); DebugOutput( m_verticalScrollAreaLines );
            DebugOutputLn( F(" )") );
            // sanity check: fixed + scroll <= m_height
            if ( m_verticalTopFixedLines + m_verticalScrollAreaLines > m_height )
            {
              DebugOutputLn( F("*** Invalid scrolling area definition - scrolling disabled!") );
            #ifdef STOP_ON_ERROR
              DebugOutputLn( F("<STOPPED>") );
              while(1);
            #endif
              // no scrolling at all!
              m_verticalTopFixedLines = 0;
              m_verticalScrollAreaLines = 0;
            }
            break;
          }
          case SSD1306Command::ENTIRE_DISPLAY_ON:     // 0xA4
          case SSD1306Command::ENTIRE_DISPLAY_ON + 1: // 0xA5
          {
            // no idea what's the difference...
            DebugOutput( F("ENTIRE_DISPLAY_ON( ") ); DebugOutput( command == SSD1306Command::ENTIRE_DISPLAY_ON ? F("normal") : F("forceOn") ); DebugOutputLn( F(" )" ) );
            forceDisplayOn( command == 0xA5 );
            break;
          }
          case SSD1306Command::SET_NORMAL_DISPLAY:    // 0xA6
          case SSD1306Command::SET_INVERSE_DISPLAY:   // 0xA7
          {
            // no idea what's the difference...
            DebugOutputLn( command == SSD1306Command::SET_NORMAL_DISPLAY ? F("SET_NORMAL_DISPLAY") : F("SET_INVERSE_DISPLAY") );
            break;
          }
          case SSD1306Command::SET_MULTIPLEX_RATIO: // 0xA8
          {
            DebugOutput( F("SET_MULTIPLEX_RATIO( ") ); if( m_enableDebugOutput ) { printHexToSerial( readCommandByte() ); } DebugOutputLn( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_DISPLAY_OFF: // 0xAE
          case SSD1306Command::SET_DISPLAY_ON:  // 0xAF
          {
            displayOn( command == SSD1306Command::SET_DISPLAY_ON );
            DebugOutputLn( command == SSD1306Command::SET_DISPLAY_OFF ? F("SET_DISPLAY_OFF") : F("SET_DISPLAY_ON") );
            break;
          }
          case SSD1306Command::SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL:  // 0xC0
          case SSD1306Command::SET_COM_OUTPUT_SCAN_DIRECTION_FLIPPED: // 0xC8
          {
            DebugOutputLn( command == SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL ? F("SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL") : F("SET_COM_OUTPUT_SCAN_DIRECTION_FLIPPED") );
            break;
          }
          case SSD1306Command::SET_DISPLAY_OFFSET:              // 0xD3
          {
            // read oofset parameter
            m_displayOffset = readCommandByte();
            DebugOutput( F("SET_DISPLAY_OFFSET( ") ); if( m_enableDebugOutput ) { printHexToSerial( m_displayOffset ); } DebugOutputLn( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_DISPLAY_CLOCK_DIVIDE_RATIO:  // 0xD5
          {
            DebugOutput( F("SET_DISPLAY_CLOCK_DIVIDE_RATIO( ") ); if( m_enableDebugOutput ) { printHexToSerial( readCommandByte() ); } DebugOutputLn( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_COM_PINS_HARDWARE_CONFIGURATION: // 0xDA,
          {
            DebugOutput( F("SET_COM_PINS_HARDWARE_CONFIGURATION( ") ); if( m_enableDebugOutput ) { printHexToSerial( readCommandByte() ); } DebugOutputLn( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_PRECHARGE_PERIOD: // 0xD9
          {
            DebugOutput( F("SET_PRECHARGE_PERIOD( ") ); if( m_enableDebugOutput ) { printHexToSerial( readCommandByte() ); } DebugOutputLn( F(" )" ) );
            break;
          }
          case SSD1306Command::SET_VCOMH_DESELECT_LEVEL: // 0xDB
          {
            DebugOutput( F("SET_VCOMH_DESELECT_LEVEL( ") ); if( m_enableDebugOutput ) { printHexToSerial( readCommandByte() ); } DebugOutputLn( F(" )" ) );
            break;
          }
          case SSD1306Command::NOP: // 0xE3
          {
            DebugOutputLn( F("NOP") );
            break;
          }
          default:
          {
            hexdumpResetPositionCount();
            Serial.print( F("Command = ") ); printHexToSerial( command );  Serial.print( F(" -> ") );
            Serial.println( F("*** unknown command ***" ) );
          #ifdef STOP_ON_ERROR
            DebugOutputLn( F("<STOPPED>") );
            while(1);
          #endif
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

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::printDebugInfo()
{
  Serial.print( F("VirtualSSD1306( width = ") ); Serial.print( m_width ); Serial.print( F(", height = ") ); Serial.print( m_height ); Serial.println( F(" )") );
  Serial.print( F("  FIFO fill count = ") ); Serial.println( m_fifo.fillCount() );
  Serial.print( F("  FIFO free count = ") ); Serial.println( m_fifo.freeCount() );
  Serial.print( F("  isEmpty = ") ); Serial.println( m_fifo.isEmpty() );
  Serial.print( F("  isFull = ") ); Serial.println( m_fifo.isFull() );
}

/*---------------------------------------------------------------------------*/
uint8_t VirtualSSD1306::readCommandByte()
{
  auto value = m_fifo.readValue();

  if ( ( value & TYPE_MASK ) == TYPE_COMMAND )
  {
    return( uint8_t( value ) );
  }
  Serial.println( F("*** Command expected!") );

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
  // less efficient when storing, but allows much easier handling for scrolling and rendering
  for ( uint8_t y = 0; y < 8; y++ )
  {
    // store 0xff when pixel set and 0x00 otherwise
    m_pFrameBuffer[uint16_t( m_column ) + uint16_t( m_page * 8 + y ) * uint16_t( m_width) ] = ( pixels & ( 1 << y ) ? 0xff : 0x00 );
  }

  //Serial.print( F("Wrote value = ") ); Serial.print( pixels );
  //Serial.print( F("at column = ") ); Serial.print( m_column );
  //Serial.print( F(", page = ") ); Serial.println( m_page );

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
  if ( m_scrollEnabled )
  {
    if ( ( m_scrollTimer % m_scrollInterval ) == 0 )
    {
      scrollHorizontal();
      scrollVertical();
    }
  }

  // another one finished
  m_scrollTimer++;
}

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::scrollHorizontal()
{
  uint8_t startLine = m_scrollStartPage * 8;
  uint8_t endLine = ( m_scrollEndPage + 1 ) * 8;

  Serial.print( F("Horizontal scolling, direction = ") ); Serial.print( m_horizontalScrollDirection );
  Serial.print( F(", startPage = ") ); Serial.print( m_scrollStartPage );
  Serial.print( F(", endPage = ") ); Serial.println( m_scrollEndPage );


  for ( uint8_t y = startLine; y < endLine; y++ )
  {
    if ( m_horizontalScrollDirection == 1 )
    {
      // scroll left
      uint8_t *dest = &m_pFrameBuffer[y * m_width];
      uint8_t *src = dest + 1;
      auto lost = *dest;
      memmove( dest, src, m_width - 1 );
      m_pFrameBuffer[( y + 1 ) * m_width - 1] = lost;
    }
    else
    {
      // scroll right
      uint8_t *src = &m_pFrameBuffer[y * SSD1306Command::DISPLAY_WIDTH];
      uint8_t *dest = src + 1;
      auto lost = m_pFrameBuffer[( y + 1 ) * SSD1306Command::DISPLAY_WIDTH - 1];
      memmove( dest, src, m_width -1 );
      *src = lost;
    }
  }
}

/*--------------------------------------------------------------------------*/
void VirtualSSD1306::scrollVertical()
{
  if ( m_verticalScrollOffset != 0 )
  {
    // copy full frame buffer to scrolling buffer
    memcpy( m_pScrollBuffer, m_pFrameBuffer, m_width * m_height );

    for ( uint8_t y = 0; y < m_verticalScrollAreaLines; y++ )
    {
      uint8_t targetLine = y + m_verticalScrollOffset;
      if ( targetLine >= m_verticalScrollAreaLines )
      {
        targetLine = ( targetLine % m_verticalScrollAreaLines );
      }
      auto *dest = &m_pFrameBuffer[( targetLine + m_verticalTopFixedLines ) * m_width];
      auto *src = &m_pScrollBuffer[( y + m_verticalTopFixedLines ) * m_width];
      memcpy( dest, src, m_width );
    }
  }
}