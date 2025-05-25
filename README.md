## SSD1306Emulator

When I discovered Luke Wren's groundbreaking PicoDVI project (https://github.com/Wren6991/PicoDVI), I was impressed beyond words.

After I got my DVIsock boards delivered and working, I thought it would be quite cool to connect a standard monitor to an ATtiny85 instead of having to use a tiny SSD1306 0.96"OLED.

And some time later, here I am with the SSD1306Emulator project, which allows exactly that:
![ATtiny85 output on DVI Monitor](https://github.com/Lorandil/SSD1306Emulator/blob/main/images/Full_setup_at_work.jpg)
<br>
I2C output from an ATtiny85 displayed on a DVI Monitor with genuine 0.96" display for size reference... *(for convenience using a cheap HDMI grabber)*

## Features

* Plug-in replacement - remove the SDD1306 and connect four wires to the Pi Pico (and maybe add pull ups and voltage level adjustments)
* DVI output using DVIsock
* Complete emulation of an SSD1306 OLED display with 128x64 pixels (other resolution might follow)
* Support for all relevant SSD1306 commands (including scrolling)
* Can be configured to work on any i2c address (default address is 0x3C)

## Wiring

![ATtiny85 output on DVI Monitor](https://github.com/Lorandil/SSD1306Emulator/blob/main/images/TinyJoypad_connected_to_SSD1306Emulator.jpg)
<br>
TinyJoypad connected to DVI. <br> *Normally I would have connected the display pads, but I damaged the traces on the PCB when removing a broken panel, so I had to resort to the 8 pin connector.*

| Pi Pico (i2c slave) | i2c master          |
|:--------------------|:--------------------|
| 3V3(OUT) (pin 36)   | maybe VCC (if 3.3V) |
| GND (e.g. pin 38)   | GND                 |
| I2C0 SCL (pin 7)    | SCL                 |
| I2C0 SDA (pin 6)    | SDA                 |

Remember to add pull ups to SCL and SDA!

## Implementation
The SSD1306 to DVI emulation is split into three parts

### Physical interface layer (at the moment only i2c is supported)
This 'layer' is the Pi Pico's i2c hardware interface which is handled by the Wire library. Voltage level adjustment might be required if connection e.g. a 5V controller.


### Logic layer (the real emulation part)
The only currently implemented class is `VirtualSSD1306`

The class allocates a frame buffer at one byte per pixel. 
This 'outrageous waste of memory' allows easy scrolling in all
directions and also easy pixel access for rendering.
 
The most tricky part of this class was to emulate the scrolling.
As there is no sensible way to control the scrolling (it depends on the display's screen refresh rate, which is - as far as I know - out of the user's control),
it is pretty useless except when you just want to scroll a static text around the screen (as in the Adafruit demo). It's working nonetheless ;)

### Display logic (rendering the image)
The rendering is impented as a separated class and can easily enhanced for visual effects ;)

## Known Limitations
The emulation uses the Wire library and thus requires a well formed I2C protocol.
Some versions of highly optimized libraries violate the protocol in a way that works with most displays
(narrow timings, no handshake, transfer size limitations, ...), but sadly not always with the Wire library.

The good news is, that there *are* libraries which can be used as a replacement.
The caveat being a higher flash memory foodprint and a slightly reduced transfer speed.
Examples for well working libraries are
  * Adafruit_SSD1306
  * https://github.com/tejashwikalptaru/ssd1306xled

Because of the relatively low resolution of the 16Bit DVI output (320x240), the aspect ratio can only be adjusted coarsely, so it might be necessary to adjust the monitor/TV's scaling to get proper results (i.e. square pixels).

The SSD1306's with 128x64 pixels has 2:1 image format, so it's not possible to get a full screen output without hefty distortion. In practical use, the active area (zoom factor of 2 resulting in 256x192 pixels) is about 43% of the screen area.


## Known Issues
At the moment only the 16Bit render is fully working (`SimpleOLEDRenderer`), the 8Bit and 1Bit versions don't work correctly for unknown reasons *(ok, because I do something wrong...)*

## Future Ideas
It would be useful to define a pin that could be used to start the data sender after the Pi Pico is up and running.

Working 8Bit and 1Bit modes with higher resolution could help reducing the aspect ratio problem.

## Credits
Last but not least, credits to all those giants whose work made this project possible (in no specific order)
* Luke Wren (https://github.com/Wren6991/PicoDVI, https://github.com/Wren6991/Pico-DVI-Sock)
* Earle Philhower, III (https://github.com/earlephilhower/arduino-pico)
* Adafruit
* Arduino
* everyone I should have mentioned
* ...
