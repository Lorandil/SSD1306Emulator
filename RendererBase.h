#pragma once

#include <Arduino.h>
#include "VirtualDisplayBase.h"

class RendererBase
{
public:
  RendererBase( VirtualDisplayBase *pVirtualDisplay ) {}
  virtual ~RendererBase() = default;

  virtual void initScreen() = 0;
  virtual void renderBackground() {};
  virtual void renderScreen() = 0;
  virtual uint16_t getScreenWidth() { return( m_width ); }
  virtual uint16_t getScreenHeight() { return( m_height ); }

  // optional functions:
  // aspect ration (e.g. 4:3, 16:9, ... )
  virtual void setAspectRation( uint16_t aspectX, uint16_t aspectY ) { m_aspectX = aspectX; m_aspectY = aspectY; }
  virtual void saveScreenshot( unsigned char fileName ) {}

protected:
  uint16_t m_aspectX{1};
  uint16_t m_aspectY{1};
  uint16_t m_width{0};
  uint16_t m_height{0};
};