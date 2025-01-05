#pragma once

#include <Arduino.h>
#include "VirtualDisplayBase.h"

class RendererBase
{
public:
  RendererBase( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY )
  {
    m_pVirtualDisplay = pVirtualDisplay;
    m_scaleX = scaleX;
    m_scaleY = scaleY;
  }
  virtual ~RendererBase() = default;

  virtual void initScreen() = 0;
  virtual void renderBackground() {};
  virtual void renderScreen() = 0;
  virtual uint16_t getScreenWidth() { return( m_width ); }
  virtual uint16_t getScreenHeight() { return( m_height ); }

  // optional functions:
  virtual void saveScreenshot( unsigned char fileName ) {}

protected:
  uint16_t            m_scaleX{1};
  uint16_t            m_scaleY{1};
  uint16_t            m_width{0};
  uint16_t            m_height{0};
  VirtualDisplayBase *m_pVirtualDisplay{};
};