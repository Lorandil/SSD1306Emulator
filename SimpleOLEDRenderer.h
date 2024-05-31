#pragma once

#include <Arduino.h>
#include "VirtualSSD1306.h"

class SimpleOLEDRenderer
{
public:
  SimpleOLEDRenderer( VirtualSSD1306 *pVirtualSSD1306 );
  virtual ~SimpleOLEDRenderer();

  virtual void initScreen();
  virtual void renderBackground();
  virtual void renderScreen();

protected:
  VirtualSSD1306 *m_pVirtualSSD1306;
};