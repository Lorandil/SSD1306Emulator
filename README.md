# SSD1306Emulator
 Try to emulate an SSD1306 and display the image data on an DVI monitor using picoDVI

# Modules
The SSD1306 to DVI emulation can be split into three parts

* physical interface layer (at the moment only i2c is supported)

* logic layer (the real emulation part)
The only currently implemented class is 'VirtualSSD1306'

* display logic (rendering the image)
