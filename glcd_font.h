#ifndef FONT16X12_H
#define FONT16X12_H

#include "platform.h"

typedef struct
{
    uint8_t width;
    uint8_t height;
    uint16_t startCharacter;
    uint16_t endCharacter;
    uint8_t* content;
} font_t;

extern font_t font6x8;
extern font_t font7x9;
extern font_t font16x12;
extern font_t font32x48;
extern font_t font24x32;

extern const unsigned char vietnamese_vowel_table[];

#endif  /* FONT16X12_H */
