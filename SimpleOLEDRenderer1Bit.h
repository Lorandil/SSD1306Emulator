#pragma once

#include <Arduino.h>
#include "RendererBase.h"
#include "VirtualDisplayBase.h"

class SimpleOLEDRenderer1Bit : public RendererBase
{
public:
  SimpleOLEDRenderer1Bit( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY );

  void initScreen() override;
  void renderBackground() override;
  void renderScreen() override;

protected:
  class DVIGFX1      *m_pDisplay{};
};