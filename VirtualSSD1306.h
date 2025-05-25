#pragma once

#include <Arduino.h>

#define SIMPLE_FIFO_SIZE 2048

#define VIRTUAL_SSD1306_RECEIVE_FIFO_BUFFER_SIZE 1056 // 1024*8 pixels + some reserve bytes ;)

#define SSD1306_I2C_DEFAULT_ADDRESS 0x3C

#include "fifo.hpp"
#include "ssd1306commands.h"
#include "VirtualDisplayBase.h"

static SimpleFIFO<uint16_t> m_fifo;

class VirtualSSD1306 : public VirtualDisplayBase
{
  enum Direction
  {
    left  = 1,
    right = 2,
    up    = 4,
    down  = 8
  };

  enum DataTypes
  {
    TYPE_COMMAND = 0x0000,
    TYPE_DATA    = 0x8000,
    DATA_MASK    = 0x00ff,
    TYPE_MASK    = 0x8000
  };

public:
  VirtualSSD1306( uint16_t width = 128, uint16_t height = 64, bool enableDebugOutput = false );
  virtual ~VirtualSSD1306();

  void             begin( uint8_t i2cAddress = SSD1306_I2C_DEFAULT_ADDRESS ) override;

  uint8_t          getPixel( uint8_t x, uint8_t y ) override;
  uint8_t         *getFrameBuffer() override { return( m_pFrameBuffer ); }
  void             processData() override;
  void             printDebugInfo() override;

  virtual void     invertDisplay( bool invertFlag ) { m_invertDisplay = invertFlag ? 0xff : 0x00; }
  virtual uint8_t  invertDisplay() { return( m_invertDisplay ); }
  virtual void     forceDisplayOn( bool allPixelsOn ) { m_forceDisplayOn = allPixelsOn ? 0xff : 0x00; }
  virtual uint8_t  forceDisplayOn() { return( m_forceDisplayOn ); }
  virtual void     displayOn( bool displayOn ) { m_displayOn = displayOn ? 0xff : 0x00; }
  virtual uint8_t  displayOn() { return( m_displayOn ); }
  virtual void     performScrolling();

protected:
  static  void     i2cRxHandler( int numBytes );
  virtual uint8_t  readCommandByte();
  virtual uint8_t  readDataByte();
  virtual void     writePixels( uint8_t pixels );
  virtual void     scrollHorizontal();
  virtual void     scrollVertical();

protected:
  uint8_t          m_addressingMode{};
  uint8_t          m_segmentRemap{};
  uint8_t          m_columnStartAddress{0};
  uint8_t          m_columnEndAddress{SSD1306Command::DISPLAY_WIDTH - 1};
  uint8_t          m_pageStartAddress{0x00};
  uint8_t          m_pageEndAddress{0x07};
  uint8_t          m_displayOffset{};
  uint8_t          m_displayStartLine{};

  // scrolling
  uint32_t         m_scrollTimer{};

  // scrolling
  bool             m_scrollEnabled{};
  uint8_t          m_horizontalScrollDirection{};
  uint8_t          m_scrollStartPage{};
  uint8_t          m_scrollEndPage{};
  uint8_t          m_scrollInterval{};
  uint8_t          m_verticalScrollOffset{};
  uint8_t          m_verticalTopFixedLines{};
  uint8_t          m_verticalScrollAreaLines{};


  // display modifiers
  uint8_t          m_displayOn{0xff};
  uint8_t          m_forceDisplayOn{0x00};
  uint8_t          m_invertDisplay{0x00};

  // running counters for writing pixels
  uint8_t          m_page{};
  uint8_t          m_column{};

  constexpr static uint32_t scrollingTimerInterval[] = { 5, 64, 12, 256, 3, 4, 25, 2 };

  uint8_t         *m_pFrameBuffer{};
  uint8_t         *m_pScrollBuffer{};
};