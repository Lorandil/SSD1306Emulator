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
};