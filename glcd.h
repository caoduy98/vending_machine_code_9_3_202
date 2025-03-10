
#ifndef GLCD_H
#define GLCD_H

#include "platform.h"
#include "glcd_font.h"

/* LCD Dimension */
#define GLCD_WIDTH      128
#define GLCD_HEIGHT     64

/* Color code */
#define WHITE   1
#define BLACK   0


void GLcd_Init(int dataPin, int clockPin, int csPin);
void GLcd_ClearScreen(int color);
void GLcd_SetFont(font_t* font);
void GLcd_DrawPoint(int x, int y, int color);
void GLcd_DrawChar(char c, int x, int y, int color);
void GLcd_DrawCharUni(const uint8_t* image, int x, int y, uint8_t w, uint8_t h, int color);
void GLcd_DrawString(const char* str, int x, int y, int color);
void GLcd_DrawStringUni(const uint16_t* s, int x1, int y1, int color);
void GLcd_DrawLine(int x0, int y0, int x1, int y1, int color);
void GLcd_DrawRect(int x, int y, int width, int height, int color);
void GLcd_DrawCircle(int x, int y, int radius, int color);
void GLcd_FillRect(int x, int y, int width, int height, int color);
void GLcd_DrawImage(const uint8_t* img);
void GLcd_DrawBitmap(const uint8_t *image, int x, int y);

/* Get the length in pixel of a string with the current font */
int  GLcd_MeasureString(const char* str);
int  GLcd_MeasureStringUni(const uint16_t* str);
int  GLcd_CalculateXCenter(const uint16_t* s);

/* Draw the offscreen buffer to the LCD
 * All of the above functions just do drawing on the offscreen buffer.
 * In order to draw on the LCD, user have to call this function after calling those above functions
 * This example will draw a string and a line on the LCD
 *     GLcd_DrawString("Hello", 0, 0, WHITE);   // Just draw on the offscreen buffer. The string does not appear on the LCD yet
 *     GLcd_DrawLine(10, 10, 50, 50, WHITE);    // Just draw on the offscreen buffer. The line does not appear on the LCD yet
 *     GLcd_Flush();    // Draw the above string and line on to the LCD
 */
void GLcd_Flush(void);
void GLcd_FlushRegion(int y, int height);

#endif /* GLCD_H */
