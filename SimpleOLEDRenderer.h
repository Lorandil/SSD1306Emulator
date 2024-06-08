#pragma once

#include <Arduino.h>
#include "RendererBase.h"
#include "VirtualDisplayBase.h"

class SimpleOLEDRenderer : public RendererBase
{
public:
  SimpleOLEDRenderer( VirtualDisplayBase *pVirtualDisplay );
  ~SimpleOLEDRenderer() override;

  void initScreen() override;
  void renderBackground() override;
  void renderScreen() override;

protected:
  VirtualDisplayBase *m_pVirtualDisplay;
};