/*
   Text drawing on top of the TTF renderer (ttf_fonts.c).
   Originally TINY3D font library / (c) 2010 Hermes <www.elotrolado.net>
*/

#ifndef LIBFONT_H
#define LIBFONT_H

#include "menu.h"

#define MAX_SPECIAL_CHARS	0x20

#define FONT_ALIGN_LEFT            0
#define FONT_ALIGN_SCREEN_CENTER   1
#define FONT_ALIGN_RIGHT           2
#define FONT_ALIGN_CENTER          3

// character size in pixels
void SetFontSize(int sx, int sy);

// RGBA colour; the glyphs are drawn white, or black when the RGB part is 0
void SetFontColor(u32 color, u32 bkcolor);

// alignment of DrawString (FONT_ALIGN_*)
void SetFontAlign(int mode);

// draw an image in place of a control character (< 32)
void RegisterSpecialCharacter(char value, short fy, float scale, png_texture* image);

// width of a string at the current size
int WidthFromStr(const char * str);

// draw a string; returns the X after the last character
float DrawString(float x, float y, const char *str);
float DrawFormatString(float x, float y, char *format, ...);

#endif
