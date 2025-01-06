#pragma once

#include <Arduino.h>

class VirtualDisplayBase
{
public:
  VirtualDisplayBase( uint16_t width, uint16_t height, bool enableDebugOutput= false ) : m_width( width ), m_height( height ), m_enableDebugOutput( enableDebugOutput ) {}
  virtual ~VirtualDisplayBase() = default;

  virtual void      begin( uint8_t i2cAddress ) = 0;

  virtual uint16_t  width() { return( m_width ); }
  virtual uint16_t  height() { return( m_height ); }
  virtual uint8_t   getPixel( uint8_t x, uint8_t y ) = 0;
  virtual uint8_t  *getFrameBuffer() = 0;
  virtual void      processData() = 0;

  template <typename T> void DebugOutput( T value )
  {
    if ( m_enableDebugOutput )
    {
      Serial.print( value );
    }
  }
  template <typename T> void DebugOutputLn( T value )
  {
    if ( m_enableDebugOutput )
    {
      Serial.println( value );
    }
  }

protected:
  uint16_t          m_width{};
  uint16_t          m_height{};

  // debug output
  bool              m_enableDebugOutput{};
};