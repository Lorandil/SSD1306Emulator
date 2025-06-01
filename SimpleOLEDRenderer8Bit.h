#pragma once

#include <Arduino.h>
#include "RendererBase.h"
#include "VirtualDisplayBase.h"

class SimpleOLEDRenderer8Bit : public RendererBase
{
public:
  SimpleOLEDRenderer8Bit( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY );

  void initScreen() override;
  void renderBackground() override;
  void renderScreen() override;

protected:
  class DVIGFX8      *m_pDisplay{};
};