# SSD1306Emulator
 Try to emulate an SSD1306 and display the image data on an DVI monitor using picoDVI

# Known Limitations
The emulation uses the Wire library and thus requires a well formed I2C protocol.
Some versions of highly optimized libraries (e.g. https://github.com/tinusaur/ssd1306xled) 
violate the protocol in a way that works with most displays (no handshake), but sadly not with the Wire library.

The good news is, that there *are* libraries which can be used as a replacement.
The caveat being a higher flash memory foodprint and a slightly reduced transfer speed.
Examples for well working libraries are
  * Adafruit_SSD1306
  * https://github.com/tejashwikalptaru/ssd1306xled

# Modules
The SSD1306 to DVI emulation can be split into three parts

* physical interface layer (at the moment only i2c is supported)

* logic layer (the real emulation part)
  The only currently implemented class is 'VirtualSSD1306'

  The class allocates a frame buffer at one byte per pixel. 
  This outrageous waste of memory allows easy scrolling in all
  directions and also easy pixel access for rendering.

  The most difficult part of this class is to emulate the memory addressing modes.

* display logic (rendering the image)
