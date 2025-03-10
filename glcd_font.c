
#include "platform.h"
#include "glcd_font.h"



const uint8_t g_font16x12[10][24] = {
    {0x00,0x00,0xF0,0x3F,0x7C,0xF0,0xFC,0xF0,0xBC,0xF1,0x3C,0xF3,0x3C,0xF6,0x3C,0xFC,0x3C,0xF8,0xF0,0x3F,0x00,0x00,0x00,0x00},	// 0x30
    {0x00,0x00,0x00,0x03,0x00,0x0F,0x00,0xFF,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0xF0,0xFF,0x00,0x00,0x00,0x00},	// 0x31
    {0x00,0x00,0xC0,0x3F,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,0xC0,0x03,0x00,0x0F,0x00,0x3C,0xF0,0xF0,0xF0,0xFF,0x00,0x00,0x00,0x00},	// 0x32
    {0x00,0x00,0xC0,0x3F,0xF0,0xF0,0xF0,0x00,0xF0,0x00,0xC0,0x0F,0xF0,0x00,0xF0,0x00,0xF0,0xF0,0xC0,0x3F,0x00,0x00,0x00,0x00},	// 0x33
    {0x00,0x00,0xF0,0x00,0xF0,0x03,0xF0,0x0F,0xF0,0x3C,0xF0,0xF0,0xFC,0xFF,0xF0,0x00,0xF0,0x00,0xFC,0x03,0x00,0x00,0x00,0x00},	// 0x34
    {0x00,0x00,0xF0,0xFF,0x00,0xF0,0x00,0xF0,0x00,0xF0,0xC0,0xFF,0xF0,0x00,0xF0,0x00,0xF0,0xF0,0xC0,0x3F,0x00,0x00,0x00,0x00},	// 0x35
    {0x00,0x00,0xC0,0x0F,0x00,0x3C,0x00,0xF0,0x00,0xF0,0xC0,0xFF,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xC0,0x3F,0x00,0x00,0x00,0x00},	// 0x36
    {0x00,0x00,0xFC,0xFF,0x3C,0xF0,0x3C,0xF0,0x3C,0x00,0xF0,0x00,0xC0,0x03,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x00,0x00,0x00},	// 0x37
    {0x00,0x00,0xC0,0x3F,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xC0,0x3F,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xC0,0x3F,0x00,0x00,0x00,0x00},	// 0x38
    {0x00,0x00,0xC0,0x3F,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x3F,0xC0,0x03,0xC0,0x03,0x00,0x0F,0x00,0x3F,0x00,0x00,0x00,0x00},	// 0x39
};


const uint8_t g_font6x8[][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},  // 0x20
    {0x04,0x0E,0x0E,0x04,0x04,0x00,0x04,0x00},  // 0x21
    {0x1B,0x1B,0x12,0x00,0x00,0x00,0x00,0x00},  // 0x22
    {0x00,0x0A,0x1F,0x0A,0x0A,0x1F,0x0A,0x00},  // 0x23
    {0x08,0x0E,0x10,0x0C,0x02,0x1C,0x04,0x00},  // 0x24
    {0x19,0x19,0x02,0x04,0x08,0x13,0x13,0x00},  // 0x25
    {0x08,0x14,0x14,0x08,0x15,0x12,0x0D,0x00},  // 0x26
    {0x0C,0x0C,0x08,0x00,0x00,0x00,0x00,0x00},  // 0x27
    {0x04,0x08,0x08,0x08,0x08,0x08,0x04,0x00},  // 0x28
    {0x08,0x04,0x04,0x04,0x04,0x04,0x08,0x00},  // 0x29
    {0x00,0x0A,0x0E,0x1F,0x0E,0x0A,0x00,0x00},  // 0x2A
    {0x00,0x04,0x04,0x1F,0x04,0x04,0x00,0x00},  // 0x2B
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x08},  // 0x2C
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00},  // 0x2D
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00},  // 0x2E
    {0x00,0x01,0x02,0x04,0x08,0x10,0x00,0x00},  // 0x2F
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E,0x00},  // 0x30
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E,0x00},  // 0x31
    {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F,0x00},  // 0x32
    {0x0E,0x11,0x01,0x0E,0x01,0x11,0x0E,0x00},  // 0x33
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02,0x00},  // 0x34
    {0x1F,0x10,0x10,0x1E,0x01,0x11,0x0E,0x00},  // 0x35
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E,0x00},  // 0x36
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08,0x00},  // 0x37
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E,0x00},  // 0x38
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C,0x00},  // 0x39
    {0x00,0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00},  // 0x3A
    {0x00,0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x08},  // 0x3B
    {0x02,0x04,0x08,0x10,0x08,0x04,0x02,0x00},  // 0x3C
    {0x00,0x00,0x1F,0x00,0x00,0x1F,0x00,0x00},  // 0x3D
    {0x08,0x04,0x02,0x01,0x02,0x04,0x08,0x00},  // 0x3E
    {0x0E,0x11,0x01,0x06,0x04,0x00,0x04,0x00},  // 0x3F
    {0x0E,0x11,0x17,0x15,0x17,0x10,0x0E,0x00},  // 0x40
    {0x0E,0x11,0x11,0x11,0x1F,0x11,0x11,0x00},  // 0x41
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E,0x00},  // 0x42
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E,0x00},  // 0x43
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E,0x00},  // 0x44
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F,0x00},  // 0x45
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10,0x00},  // 0x46
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F,0x00},  // 0x47
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11,0x00},  // 0x48
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E,0x00},  // 0x49
    {0x01,0x01,0x01,0x01,0x11,0x11,0x0E,0x00},  // 0x4A
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11,0x00},  // 0x4B
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0x00},  // 0x4C
    {0x11,0x1B,0x15,0x11,0x11,0x11,0x11,0x00},  // 0x4D
    {0x11,0x19,0x15,0x13,0x11,0x11,0x11,0x00},  // 0x4E
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E,0x00},  // 0x4F
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10,0x00},  // 0x50
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D,0x00},  // 0x51
    {0x1E,0x11,0x11,0x1E,0x12,0x11,0x11,0x00},  // 0x52
    {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E,0x00},  // 0x53
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04,0x00},  // 0x54
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E,0x00},  // 0x55
    {0x11,0x11,0x11,0x11,0x11,0x0A,0x04,0x00},  // 0x56
    {0x11,0x11,0x15,0x15,0x15,0x15,0x0A,0x00},  // 0x57
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11,0x00},  // 0x58
    {0x11,0x11,0x11,0x0A,0x04,0x04,0x04,0x00},  // 0x59
    {0x1E,0x02,0x04,0x08,0x10,0x10,0x1E,0x00},  // 0x5A
    {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E,0x00},  // 0x5B
    {0x00,0x10,0x08,0x04,0x02,0x01,0x00,0x00},  // 0x5C
    {0x0E,0x02,0x02,0x02,0x02,0x02,0x0E,0x00},  // 0x5D
    {0x04,0x0A,0x11,0x00,0x00,0x00,0x00,0x00},  // 0x5E
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F},  // 0x5F
    {0x0C,0x0C,0x04,0x00,0x00,0x00,0x00,0x00},  // 0x60
    {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F,0x00},  // 0x61
    {0x10,0x10,0x1E,0x11,0x11,0x11,0x1E,0x00},  // 0x62
    {0x00,0x00,0x0E,0x11,0x10,0x11,0x0E,0x00},  // 0x63
    {0x01,0x01,0x0F,0x11,0x11,0x11,0x0F,0x00},  // 0x64
    {0x00,0x00,0x0E,0x11,0x1E,0x10,0x0E,0x00},  // 0x65
    {0x06,0x08,0x08,0x1E,0x08,0x08,0x08,0x00},  // 0x66
    {0x00,0x00,0x0F,0x11,0x11,0x0F,0x01,0x0E},  // 0x67
    {0x10,0x10,0x1C,0x12,0x12,0x12,0x12,0x00},  // 0x68
    {0x04,0x00,0x04,0x04,0x04,0x04,0x06,0x00},  // 0x69
    {0x02,0x00,0x06,0x02,0x02,0x02,0x12,0x0C},  // 0x6A
    {0x10,0x10,0x12,0x14,0x18,0x14,0x12,0x00},  // 0x6B
    {0x04,0x04,0x04,0x04,0x04,0x04,0x06,0x00},  // 0x6C
    {0x00,0x00,0x1A,0x15,0x15,0x11,0x11,0x00},  // 0x6D
    {0x00,0x00,0x1C,0x12,0x12,0x12,0x12,0x00},  // 0x6E
    {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E,0x00},  // 0x6F
    {0x00,0x00,0x1E,0x11,0x11,0x11,0x1E,0x10},  // 0x70
    {0x00,0x00,0x0F,0x11,0x11,0x11,0x0F,0x01},  // 0x71
    {0x00,0x00,0x16,0x09,0x08,0x08,0x1C,0x00},  // 0x72
    {0x00,0x00,0x0E,0x10,0x0E,0x01,0x0E,0x00},  // 0x73
    {0x00,0x08,0x1E,0x08,0x08,0x0A,0x04,0x00},  // 0x74
    {0x00,0x00,0x12,0x12,0x12,0x16,0x0A,0x00},  // 0x75
    {0x00,0x00,0x11,0x11,0x11,0x0A,0x04,0x00},  // 0x76
    {0x00,0x00,0x11,0x11,0x15,0x1F,0x0A,0x00},  // 0x77
    {0x00,0x00,0x12,0x12,0x0C,0x12,0x12,0x00},  // 0x78
    {0x00,0x00,0x12,0x12,0x12,0x0E,0x04,0x18},  // 0x79
    {0x00,0x00,0x1E,0x02,0x0C,0x10,0x1E,0x00},  // 0x7A
    {0x06,0x08,0x08,0x18,0x08,0x08,0x06,0x00},  // 0x7B
    {0x04,0x04,0x04,0x00,0x04,0x04,0x04,0x00},  // 0x7C
    {0x0C,0x02,0x02,0x03,0x02,0x02,0x0C,0x00},  // 0x7D
    {0x0A,0x14,0x00,0x00,0x00,0x00,0x00,0x00},  // 0x7E
    {0x04,0x0E,0x1B,0x11,0x11,0x1F,0x00,0x00},  // 0x7F
};

const uint8_t g_font7x9[][9] = {
  {0x1E, 0x23, 0x25, 0x29, 0x31, 0x21, 0x31, 0x1E}, // 0
  {0x04, 0x1C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F}, // 1
  {0x1E, 0x21, 0x01, 0x01, 0x0E, 0x10, 0x20, 0x3F}, // 2
  {0x1E, 0x21, 0x01, 0x1E, 0x01, 0x01, 0x21, 0x1E}, // 3
  {0x02, 0x06, 0x0A, 0x12, 0x22, 0x3F, 0x02, 0x02}, // 4
  {0x3F, 0x20, 0x20, 0x3E, 0x01, 0x01, 0x21, 0x1E}, // 5
  {0x1E, 0x21, 0x20, 0x3E, 0x21, 0x21, 0x21, 0x1E}, // 6
  {0x3F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08, 0x08}, // 7
  {0x1E, 0x21, 0x21, 0x1E, 0x21, 0x21, 0x21, 0x1E}, // 8
  {0x1E, 0x21, 0x21, 0x21, 0x1F, 0x01, 0x21, 0x1E}, // 9
};

const uint8_t g_font32x48[10][192] = {
  {
    0xF0,0xFF,0xFF,0x0F,0xF8,0xFF,0xFF,0x1F,0xFC,0xFF,0xFF,0x3F,0xFE,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x03,0x80,0xFF,0xFF,0x01,0x00,0xFF,0xFF,0x03,0x00,0xFE,0x7F,0x07,0x00,0xFC,0x3F,0x0E,0x00,0xFC,0x3F,0x1C,0x00,0xFC,
    0x3F,0x38,0x00,0xFC,0x3F,0x70,0x00,0xFC,0x3F,0xE0,0x00,0xFC,0x3F,0xC0,0x01,0xFC,0x3F,0x80,0x03,0xFC,0x3F,0x00,0x07,0xFC,0x3F,0x00,0x0E,0xFC,0x3F,0x00,0x1C,0xFC,0x3F,0x00,0x38,0xFC,0x3F,0x00,0x70,0xFC,0x3F,0x00,0xE0,0xFC,0x3F,0x00,0xC0,0xFD,
    0x3F,0x00,0x80,0xFF,0x3F,0x00,0x00,0xFF,0x3F,0x00,0x00,0xFE,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,
    0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x7F,0x00,0x00,0xFE,0xFF,0x00,0x00,0xFF,0xFF,0x01,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x7F,0xFC,0xFF,0xFF,0x3F,0xF8,0xFF,0xFF,0x1F,0xF0,0xFF,0xFF,0x0F
  }, //0x30
  {
    0x00,0xE0,0x07,0x00,0x00,0xE0,0x0F,0x00,0x00,0xE0,0x1F,0x00,0x00,0xE0,0x3F,0x00,0x00,0xE0,0x7F,0x00,0x00,0xE0,0xFF,0x00,0x00,0xE0,0xFF,0x01,0x00,0xE0,0xFF,0x03,0x00,0xE0,0xFF,0x07,0x00,0xE0,0xFF,0xFF,0x00,0xE0,0xFF,0xFF,0x00,0xE0,0xFF,0xFF,
    0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,
    0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,
    0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0x00,0xE0,0x07,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
  }, //0x31
  {
    0xF0,0xFF,0xFF,0x0F,0xF8,0xFF,0xFF,0x1F,0xFC,0xFF,0xFF,0x3F,0xFE,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0x7F,0x00,0x00,0xFE,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,
    0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
    0xFF,0xFF,0x0F,0x00,0xFF,0xFF,0x1F,0x00,0xFE,0xFF,0x3F,0x00,0xFc,0xFF,0x7F,0x00,0xF8,0xFF,0xFF,0x00,0xF0,0xFF,0xFF,0x01,0x00,0x00,0xFC,0x03,0x00,0x00,0xF8,0x07,0x00,0x00,0xF0,0x0F,0x00,0x00,0xE0,0x1F,0x00,0x00,0xC0,0x3F,0x00,0x00,0x80,0x7F,
    0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
  }, // 0x32
  {
     0xF0,0xFF,0xFF,0x0F,0xF8,0xFF,0xFF,0x1F,0xFC,0xFF,0xFF,0x3F,0xFE,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFE,0x7F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xF8,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,
     0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x00,0xFC,0xFF,0xFF,0x00,0xF8,0xFF,0xFF,0x00,0xF8,0xFF,0xFF,0x00,0xFC,0xFF,0xFF,0x00,
     0xFE,0xFF,0xFF,0x00,0xFF,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,
     0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x7F,0x00,0x00,0xFE,0xFF,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x7F,0xFC,0xFF,0xFF,0x3F,0xF8,0xFF,0xFF,0x1F,0xF0,0xFF,0xFF,0x0F
  }, // 0x33
    {
    0xF0,0xFF,0x01,0x00,0xF0,0xFF,0x01,0x00,0xF0,0xFF,0x03,0x00,0xF0,0xFF,0x03,0x00,0xF0,0xC3,0x07,0x00,0xF0,0xC3,0x07,0x00,0xF0,0x83,0x0F,0x00,0xF0,0x83,0x0F,0x00,0xF0,0x03,0x1F,0x00,0xF0,0x03,0x1F,0x00,0xF0,0x03,0x3E,0x00,0xF0,0x03,0x3E,0x00,
    0xF0,0x03,0x7C,0x00,0xF0,0x03,0x7C,0x00,0xF0,0x03,0xF8,0x00,0xF0,0x03,0xF8,0x00,0xF0,0x03,0xF0,0x01,0xF0,0x03,0xF0,0x01,0xF0,0x03,0xE0,0x03,0xF0,0x03,0xE0,0x03,0xF0,0x03,0xC0,0x07,0xF0,0x03,0xC0,0x07,0xF0,0x03,0x80,0x0F,0xF0,0x03,0x80,0x0F,
    0xF0,0x03,0x00,0x1F,0xF0,0x03,0x00,0x1F,0xF0,0x03,0x00,0x3E,0xF0,0x03,0x00,0x3E,0xF0,0x03,0x00,0x7C,0xF0,0x03,0x00,0x7C,0xF0,0x03,0x00,0xF8,0xF0,0x03,0x00,0xF8,0xF0,0x03,0x00,0xF0,0xF0,0x03,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x03,0x00,0x00,0xF0,0x03,0x00,0x00,0xF0,0x03,0x00,0x00,0xF0,0x03,0x00,0x00,0xF0,0x03,0x00,0x00,0xF0,0x03,0x00,0x00,0xF0,0x03,0x00,0x00,0xF0,0x03,0x00,0x00
  }, //0x34
  {
     0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,
     0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0xF0,0xFF,0xFF,0xFF,0xF8,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,
     0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,
     0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x7F,0x00,0x00,0xFE,0xFF,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x7F,0xFC,0xFF,0xFF,0x3F,0xF8,0xFF,0xFF,0x1F,0xF0,0xFF,0xFF,0x0F
  }, //0X35
  {
     0xF0,0xFF,0xFF,0x0F,0xF8,0xFF,0xFF,0x1F,0xFC,0xFF,0xFF,0x3F,0xFE,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,0x00,0xFF,0x1F,0x00,0x00,0xFE,0x0F,0x00,0x00,0xFC,0x0F,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,
     0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0xFE,0xF0,0xFF,0xFF,0xFF,0xF8,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
     0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x80,0xFF,0xFF,0x00,0x00,0xFF,0x7F,0x00,0x00,0xFE,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,
     0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x7F,0x00,0x00,0xFE,0xFF,0x00,0x00,0xFF,0xFF,0x01,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x7F,0xFC,0xFF,0xFF,0x3F,0xF8,0xFF,0xFF,0x1F,0xF0,0xFF,0xFF,0x0F,
  }, //0x36
  {
     0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0xFC,0x01,0x00,0x00,0xFC,0x01,0x00,0x00,
     0xF8,0x03,0x00,0x00,0xF8,0x03,0x00,0x00,0xF0,0x07,0x00,0x00,0xF0,0x07,0x00,0x00,0xE0,0x0F,0x00,0x00,0xE0,0x0F,0x00,0x00,0xC0,0x1F,0x00,0x00,0xC0,0x1F,0x00,0x00,0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x7F,0x00,0x00,
     0x00,0xFE,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0xFC,0x01,0x00,0x00,0xFC,0x01,0x00,0x00,0xF8,0x03,0x00,0x00,0xF8,0x03,0x00,0x00,0xF0,0x07,0x00,0x00,0xF0,0x07,0x00,0x00,0xE0,0x0F,0x00,0x00,0xE0,0x0F,0x00,0x00,0xC0,0x1F,0x00,0x00,0xC0,0x1F,0x00,
     0x00,0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0xFC,0x01,0x00,0x00,0xFC,0x01,0x00,0x00,0xF8,0x03,0x00,0x00,0xF8,0x03,0x00,0x00,0xF0,0x07,0x00,0x00,0xF0,0x07,
  }, // 0X37
  {
     0xF0,0xFF,0xFF,0x0F,0xF8,0xFF,0xFF,0x1F,0xFC,0xFF,0xFF,0x3F,0xFE,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x80,0xFF,0xFF,0x00,0x00,0xFF,0x7F,0x00,0x00,0xFE,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,
     0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x7F,0x00,0x00,0xFE,0xFF,0x00,0x00,0xFF,0xFF,0x01,0x80,0xFF,0xFE,0xFF,0xFF,0x7F,0xFC,0xFF,0xFF,0x3F,0xF8,0xFF,0xFF,0x1F,0xF8,0xFF,0xFF,0x1F,
     0xFC,0xFF,0xFF,0x3F,0xFE,0xFF,0xFF,0x7F,0xFF,0x01,0x80,0xFF,0xFF,0x00,0x00,0xFF,0x7F,0x00,0x00,0xFE,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,
     0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x7F,0x00,0x00,0xFE,0xFF,0x00,0x00,0xFF,0xFF,0x01,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x7F,0xFC,0xFF,0xFF,0x3F,0xF8,0xFF,0xFF,0x1F,0xF0,0xFF,0xFF,0x0F
  }, // 0x38
  {
     0xF0,0xFF,0xFF,0x0F,0xF8,0xFF,0xFF,0x1F,0xFC,0xFF,0xFF,0x3F,0xFE,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x80,0xFF,0xFF,0x00,0x00,0xFF,0x7F,0x00,0x00,0xFE,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,
     0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x3F,0x00,0x00,0xFC,0x7F,0x00,0x00,0xFE,0xFF,0x00,0x00,0xFF,0xFF,0x01,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
     0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0x3F,0xFF,0xFF,0xFF,0x1F,0xFF,0xFF,0xFF,0x0F,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,
     0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x3F,0x00,0x00,0xF8,0x7F,0x00,0x00,0xF8,0xFF,0x00,0x00,0xFC,0xFF,0x01,0x00,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0x7F,0xFC,0xFF,0xFF,0x3F,0xF8,0xFF,0xFF,0x1F,0xF0,0xFF,0xFF,0x0F
  }, // 0x39
};

const uint8_t g_font24x32[10][96] = {

  {
    0xF8,0xFF,0x1F,0xFC,0xFF,0x3F,0xFE,0xFF,0x7F,0xFF,0xFF,0xFF,0x3F,0x00,0xFC,0x7F,0x00,0xF8,0xEF,0x00,0xF0,0xCF,0x01,0xF0,
    0x8F,0x03,0xF0,0x0F,0x07,0xF0,0x0F,0x0E,0xF0,0x0F,0x1C,0xF0,0x0F,0x38,0xF0,0x0F,0x70,0xF0,0x0F,0xE0,0xF0,0x0F,0xC0,0xF1,
    0x0F,0x80,0xF3,0x0F,0x00,0xF7,0x0F,0x00,0xFE,0x0F,0x00,0xFC,0x0F,0x00,0xF8,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,
    0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xF8,0x3F,0x00,0xFC,0xFF,0xFF,0xFF,0xFE,0xFF,0x7F,0xFC,0xFF,0x3F,0xF8,0xFF,0x1F
  }, // 0x30
  {
    0x00,0x3C,0x00,0x00,0x7C,0x00,0x00,0xFC,0x00,0x00,0xFC,0x01,0x00,0xFC,0x03,0x00,0xFC,0x0F,0x00,0xFC,0x0F,0x00,0x3C,0x00,
    0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,
    0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,
    0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0xF8,0xFF,0x1F,0xF8,0xFF,0x1F,0xF8,0xFF,0x1F,0xF8,0xFF,0x1F
  }, // 0x31
  {
    0xF8,0xFF,0x1F,0xFC,0xFF,0x3F,0xFE,0xFF,0x7F,0xFF,0xFF,0xFF,0x3F,0x00,0xFC,0x1F,0x00,0xF8,0x0F,0x00,0xF0,0x0F,0x00,0xF0,
    0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x3F,0x00,0x00,
    0xFF,0x00,0x00,0xFE,0x07,0x00,0xF8,0x3F,0x00,0xE0,0xFF,0x01,0x00,0xFF,0x0F,0x00,0xF8,0x7F,0x00,0xC0,0xFF,0x00,0x00,0xFE,
    0x00,0x00,0xF8,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
  }, // 0x32
  {
    0xF8,0xFF,0x0F,0xFC,0xFF,0x1F,0xFE,0xFF,0x3F,0xFF,0xFF,0x7F,0x3F,0x00,0x7C,0x3F,0x00,0x78,0x0F,0x00,0x70,0x0F,0x00,0x00,
    0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x1F,0x00,0x00,0x3F,0x00,0x00,0xFE,0xFF,0x03,0xFC,0xFF,0x03,0xFC,0xFF,0x03,
    0xFE,0xFF,0x03,0x3F,0x00,0x00,0x1F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,
    0x0F,0x00,0x00,0x0F,0x00,0xE0,0x1F,0x00,0xF0,0x3F,0x00,0xF8,0xFF,0xFF,0xFF,0xFE,0xFF,0x7F,0xFC,0xFF,0x3F,0xF8,0xFF,0x1F
  }, // 0x33
  {
    0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xF0,0x1F,0x00,0xF0,0x3C,0x00,0xF0,0x3C,0x00,0xF0,0x78,0x00,0xF0,0x78,0x00,
    0xF0,0xF0,0x00,0xF0,0xF0,0x00,0xF0,0xE0,0x01,0xF0,0xE0,0x01,0xF0,0xC0,0x03,0xF0,0xC0,0x03,0xF0,0x80,0x07,0xF0,0x80,0x07,
    0xF0,0x00,0x0F,0xF0,0x00,0x0F,0xF0,0x00,0x1E,0xF0,0x00,0x1E,0xF0,0x00,0x3C,0xF0,0x00,0x3C,0xF0,0x00,0x78,0xF0,0x00,0x78,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00
  }, // 0x34
  {
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,
    0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0xFC,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3F,0x00,0x00,
    0x1F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,
    0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xF0,0x3F,0x00,0xF8,0xFF,0xFF,0xFF,0xFE,0xFF,0x7F,0xFC,0xFF,0x3F,0xF8,0xFF,0x1F
  }, // 0x35
  {
    0xF8,0xFF,0x1F,0xFC,0xFF,0x3F,0xFE,0xFF,0x7F,0xFF,0xFF,0xFF,0x3F,0x00,0xFC,0x1F,0x00,0xF8,0x0F,0x00,0xF0,0x00,0x00,0xF0,
    0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF8,0x00,0x00,0xFC,0xF8,0xFF,0xFF,0xFC,0xFF,0xFF,0xFE,0xFF,0xFF,
    0xFF,0xFF,0xFF,0x3F,0x00,0xFC,0x1F,0x00,0xF8,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,
    0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xFC,0x3F,0x00,0xFC,0xFF,0xFF,0xFF,0xFE,0xFF,0x7F,0xFC,0xFF,0x3F,0xF8,0xFF,0x1F
  }, // 0x36
  {
    0xFF,0xFF,0x3F,0xFF,0xFF,0x3F,0xFF,0xFF,0x3F,0xFF,0xFF,0x3F,0x3F,0x00,0x00,0x3F,0x00,0x00,0x3E,0x00,0x00,0x3E,0x00,0x00,
    0x7C,0x00,0x00,0x7C,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0xF0,0x01,0x00,0xF0,0x01,0x00,0xE0,0x03,0x00,0xE0,0x03,0x00,
    0xC0,0x07,0x00,0xC0,0x07,0x00,0x80,0x0F,0x00,0x80,0x0F,0x00,0x00,0x1F,0x00,0x00,0x1F,0x00,0x00,0x3E,0x00,0x00,0x3E,0x00,
    0x00,0x7C,0x00,0x00,0x7C,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0xF0,0x01,0x00,0xF0,0x01,0x00,0xE0,0x03,0x00,0xE0,0x03
  }, // 0x37
  {
    0xF8,0xFF,0x1F,0xFC,0xFF,0x3F,0xFE,0xFF,0x7F,0xFF,0xFF,0xFF,0x3F,0x00,0xFC,0x1F,0x00,0xF8,0x0F,0x00,0xF0,0x0F,0x00,0xF0,
    0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xF8,0x3F,0x00,0xFC,0xFE,0xFF,0x7F,0xFC,0xFF,0x3F,0xFC,0xFF,0x3F,
    0xFE,0xFF,0x7F,0x3F,0x00,0xFC,0x1F,0x00,0xF8,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,
    0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xF8,0x3F,0x00,0xFC,0xFF,0xFF,0xFF,0xFE,0xFF,0x7F,0xFC,0xFF,0x3F,0xF8,0xFF,0x1F
  }, // 0x38
  {
    0xF8,0xFF,0x1F,0xFC,0xFF,0x3F,0xFE,0xFF,0x7F,0xFF,0xFF,0xFF,0x3F,0x00,0xFC,0x1F,0x00,0xF8,0x0F,0x00,0xF0,0x0F,0x00,0xF0,
    0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xF8,0x3F,0x00,0xFC,0xFF,0xFF,0xFF,
    0xFF,0xFF,0x7F,0xFF,0xFF,0x3F,0xFF,0xFF,0x1F,0x3F,0x00,0x00,0x1F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,0x0F,0x00,0x00,
    0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xF8,0x3F,0x00,0xFC,0xFF,0xFF,0xFF,0xFE,0xFF,0x7F,0xFC,0xFF,0x3F,0xF8,0xFF,0x1F
  }, // 0x39
};

font_t font32x48 = {
    .width = 32,
    .height = 48,
    .startCharacter = '0',
    .endCharacter = '9',
    .content = (uint8_t*)&g_font32x48,
};


font_t font24x32 = {
    .width = 24,
    .height = 32,
    .startCharacter = '0',
    .endCharacter = '9',
    .content = (uint8_t*)&g_font24x32,
};


font_t font7x9 = {
    .width = 7,
    .height = 9,
    .startCharacter = '0',
    .endCharacter = '9',
    .content = (uint8_t*)&g_font7x9,
};


font_t font16x12 = {
    .width = 16,
    .height = 12,
    .startCharacter = '0',
    .endCharacter = '9',
    .content = (uint8_t*)&g_font16x12,
};

font_t font6x8 = {
    .width = 6,
    .height = 8,
    .startCharacter = ' ',
    .endCharacter = '~',
    .content = (uint8_t*)&g_font6x8,
};
