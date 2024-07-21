#pragma once

#include <Arduino.h>
#include "RendererBase.h"
#include "VirtualDisplayBase.h"

class SimpleOLEDRenderer8Bit : public RendererBase
{
public:
  SimpleOLEDRenderer8Bit( VirtualDisplayBase *pVirtualDisplay );

  void initScreen() override;
  void renderBackground() override;
  void renderScreen() override;

protected:
  VirtualDisplayBase *m_pVirtualDisplay{};
  class DVIGFX8      *m_pDisplay{};
};