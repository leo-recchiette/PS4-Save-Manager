/*
   Text drawing on top of the TTF renderer (ttf_fonts.c).
   Originally TINY3D font library / (c) 2010 Hermes <www.elotrolado.net>
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "types.h"
#include "libfont.h"
#include "menu.h"
#include "ttf_render.h"

static struct
{
	int sx, sy;
	u32 color, bkcolor;
	int align;
} font_datas = { 8, 8, 0xffffffff, 0, FONT_ALIGN_LEFT };

typedef struct t_special_char
{
	char value;
	short fy;
	float scale;
	png_texture* image;
} special_char;

static special_char special_chars[MAX_SPECIAL_CHARS];
static int special_char_index = 0;


static special_char* GetSpecialCharFromValue(const char value)
{
	for (int x = 0; x < special_char_index; x++)
		if (special_chars[x].value == value)
			return &(special_chars[x]);

	return NULL;
}

void SetFontSize(int sx, int sy)
{
	font_datas.sx = (sx < 8) ? 8 : sx;
	font_datas.sy = (sy < 8) ? 8 : sy;
}

void SetFontColor(u32 color, u32 bkcolor)
{
	font_datas.color   = color;
	font_datas.bkcolor = bkcolor;
}

void SetFontAlign(int mode)
{
	font_datas.align = mode;
}

void RegisterSpecialCharacter(char value, short fy, float scale, png_texture* image)
{
	if (value == 0 || image->texture == NULL || image->size == 0)
		return;

	if (GetSpecialCharFromValue(value) || special_char_index >= MAX_SPECIAL_CHARS)
		return;

	special_chars[special_char_index].value = value;
	special_chars[special_char_index].fy = fy;
	special_chars[special_char_index].scale = scale;
	special_chars[special_char_index].image = image;
	special_char_index++;
}

int WidthFromStr(const char * str)
{
	return width_ttf_string(str, font_datas.sx, font_datas.sy);
}

// Draws the image registered for a control character; returns its width
static int draw_icon(int x, int y, char c)
{
	special_char* schr = GetSpecialCharFromValue(c);
	if (!schr)
		return 0;

	float dx = (float)font_datas.sx / (float)schr->image->width;
	float dy = (float)font_datas.sy / (float)schr->image->height;
	float min = (dx > dy ? dy : dx);

	dx = (min * schr->scale * schr->image->width);
	dy = (min * schr->scale * schr->image->height);

	if (schr->fy)
		y += schr->fy;
	else
		y += (int)((font_datas.sy - dy) / 2);

	DrawTexture(schr->image, x, y, 0, (int)dx, (int)dy, 0xFFFFFF00 | RGBA_A(font_datas.color));

	return (int)dx;
}

float DrawString(float x, float y, const char *str)
{
	switch (font_datas.align)
	{
	case FONT_ALIGN_SCREEN_CENTER:
		x = (SCREEN_WIDTH - WidthFromStr(str)) / 2;
		break;

	case FONT_ALIGN_RIGHT:
		x -= WidthFromStr(str);
		break;

	case FONT_ALIGN_CENTER:
		x -= WidthFromStr(str)/2;
		break;

	default:
		break;
	}

	return display_ttf_string((int)x, (int)y, str, font_datas.color, font_datas.bkcolor, font_datas.sx, font_datas.sy+4, &draw_icon);
}

float DrawFormatString(float x, float y, char *format, ...)
{
	char buff[4096];
	va_list opt;

	va_start(opt, format);
	vsnprintf(buff, sizeof(buff), format, opt);
	va_end(opt);

	return DrawString(x, y, buff);
}
