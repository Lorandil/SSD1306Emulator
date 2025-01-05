#include <stdint.h>
#pragma once

#include <Arduino.h>
#include "RendererBase.h"
#include "VirtualDisplayBase.h"

class SimpleOLEDRenderer : public RendererBase
{
public:
  SimpleOLEDRenderer( VirtualDisplayBase *pVirtualDisplay, uint16_t scaleX, uint16_t scaleY );

  void initScreen() override;
  void renderBackground() override;
  void renderScreen() override;

protected:
  class DVIGFX16     *m_pDisplay{};
};