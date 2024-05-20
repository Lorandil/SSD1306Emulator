# SSD1306Emulator
 Try to emulate an SSD1306 and display the image data on an DVI monitor using picoDVI

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
