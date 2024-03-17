#pragma once

enum SSD1306Command
{
  // fundamental commands
  SET_LOWER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE  = 0x00, // 0x00 - 0x0F
  SET_HIGHER_COLUMN_START_ADDRESS_FOR_PAGE_ADRESSING_MODE = 0x10, // 0x10 - 0x1F
  SET_MEMORY_ADDRESSING_MODE            = 0x20, // double byte command
  SET_COLUMN_ADDRESS                    = 0x21,
  SET_PAGE_ADDRESS                      = 0x22,
  SET_DISPLAY_START_LINE                = 0x40, // 0x40 - 0x7F
  SET_CONTRAST_CONTROL_FOR_BANK0        = 0x81,
  SET_SEGMENT_REMAP                     = 0xA0, // 0xA0/0xA1
  ENTIRE_DISPLAY_ON                     = 0xA4, // 0xA4/0xA5
  SET_NORMAL_DISPLAY                    = 0xA6,
  SET_INVERSE_DISPLAY                   = 0xA7,
  SET_MULTIPLEX_RATIO                   = 0xA8, // double byte command???
  SET_DISPLAY_OFF                       = 0xAE,
  SET_DISPLAY_ON                        = 0xAF,
  SET_PAGE_START_ADDRESS                = 0xB0, // 0xB0 - 0xB7
  SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL  = 0xC0,
  SET_COM_OUTPUT_SCAN_DIRECTION_FLIPPED = 0xC8,
  SET_DISPLAY_OFFSET                    = 0xD3, // double byte command
  SET_DISPLAY_CLOCK_DIVIDE_RATIO        = 0xD5, // double byte command???
  SET_PRECHARGE_PERIOD                  = 0xD9, // ???
  SET_COM_PINS_HARDWARE_CONFIGURATION   = 0xDA,
  SET_VCOMH_DESELECT_LEVEL              = 0xDB,
  NOP                                   = 0xE3,

  // graphic acceleration commands
  HORIZONTAL_SCROLL_RIGHT_SETUP                      = 0x26, // 0x26, 0x00, <startPage>, <interval>, <endPage>
  HORIZONTAL_SCROLL_LEFT_SETUP                       = 0x27, // 0x27, 0x00, <startPage>, <interval>, <endPage>
  CONTINOUS_VERTICAL_UP_AND_HORIZONTAL_SCOLL_SETUP   = 0x29, // 0x29, 0x00, <startPage>, <interval>, <endPage>, <verticalScrollingOffset>
  CONTINOUS_VERTICAL_DOWN_AND_HORIZONTAL_SCOLL_SETUP = 0x2A, // 0x2A, 0x00, <startPage>, <interval>, <endPage>, <verticalScrollingOffset>
  DEACTIVATE_SCROLL                                  = 0x2E, 
  ACTIVATE_SCROLL                                    = 0x2F,
  SET_VERTICAL_SCROLL_AREA                           = 0xA3, // three byte command

};