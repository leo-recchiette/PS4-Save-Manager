#include <string.h>
#include <threads.h>
#include <unistd.h>
#include <stdio.h>

#define STBI_ASSERT(x)
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "types.h"
#include "libfont.h"
#include "menu.h"

#include <dbglogger.h>
#define LOG dbglogger_log

// Main menu layout (design: 1920x1080)
#define MM_MARGIN			96
#define MM_HEADER_Y			56
#define MM_HEADER_H			88
#define MM_HEADLINE_Y		236
#define MM_TILE_SIZE		300
#define MM_TILE_GAP			40
#define MM_TILE_Y			500
#define MM_TILE_ON_SIZE		622		// focused tile image: the tile at MM_TILE_ON_SCALE plus its shadow
#define MM_TILE_ON_SCALE	1.14f
#define MM_TILE_DIM_SCALE	0.96f	// the other tiles, while one is focused
#define MM_FOOTER_Y			954
#define MM_LINE_COLOR		0x24262CFF

// glow behind the tiles: one hue per tile, the last one for the header buttons
static const u32 mm_glow_color[MAIN_MENU_TILES + 1] = { 0xC6961200, 0x37B88000, 0x53A3F200, 0xAC89E800, 0x53A3F200 };

// animation state: 0 = idle, 1 = focused
static float mm_focus[MAIN_MENU_TILES];
static float mm_dim;


int LoadMenuTexture(const char* path, int idx)
{
	int d;

	LOG("Loading '%s'", path);
	if (menu_textures[idx].texture)
		SDL_DestroyTexture(menu_textures[idx].texture);

	menu_textures[idx].size = 0;
	menu_textures[idx].texture = NULL;
	menu_textures[idx].buffer = (uint32_t*) stbi_load(path, &menu_textures[idx].width, &menu_textures[idx].height, &d, STBI_rgb_alpha);

	if (!menu_textures[idx].buffer)
	{
		LOG("Error Loading texture (%s)!", path);
		return 0;
	}

	// GLES2 supports RGBA8888 only
	for (d = 0; d < menu_textures[idx].width * menu_textures[idx].height; d++)
		menu_textures[idx].buffer[d] = ES32(menu_textures[idx].buffer[d]);

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(menu_textures[idx].buffer, menu_textures[idx].width, menu_textures[idx].height, 32, 4 * menu_textures[idx].width,
												0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

	menu_textures[idx].texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_FreeSurface(surface);
	stbi_image_free(menu_textures[idx].buffer);

	menu_textures[idx].size = menu_textures[idx].width * menu_textures[idx].height * 4;
	menu_textures[idx].buffer = NULL;
	return 1;
}

// draw one background color in virtual 2D coordinates
void DrawBackground2D(u32 rgba)
{
	SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

	SDL_SetRenderDrawColor(renderer, RGBA_R(rgba), RGBA_G(rgba), RGBA_B(rgba), RGBA_A(rgba));
	SDL_RenderFillRect(renderer, &rect);
}

static void _drawListBackground(int off, int icon)
{
	if (icon == cat_cheats_png_index)
		DrawTexture(&menu_textures[help_png_index], off + MENU_ICON_OFF, help_png_y, 0, (SCREEN_WIDTH - 75) - off - MENU_ICON_OFF, help_png_h, 0xFFFFFF00 | 0xFF);
	else if (icon != cat_about_png_index)
		DrawTexture(&menu_textures[help_png_index], help_png_x, help_png_y, 0, help_png_w, help_png_h, 0xFFFFFF00 | 0xFF);
}

void DrawHeader_Ani(int icon, const char * hdrTitle, const char * headerSubTitle, u32 rgba, u32 bgrgba, int ani, int div)
{
	u8 icon_a = (u8)(((ani * 2) > 0xFF) ? 0xFF : (ani * 2));
	char headerTitle[44];
	snprintf(headerTitle, sizeof(headerTitle), "%.40s%s", hdrTitle, (strlen(hdrTitle) > 40 ? "..." : ""));

	//------------ Backgrounds
	
	//Background
	DrawBackgroundTexture(0, (u8)bgrgba);

	_drawListBackground(0, icon);

	//header mini icon
	DrawTextureCenteredX(&menu_textures[icon], MENU_ICON_OFF - 20, 32, 0, 96, 96, 0xffffff00 | icon_a);

	//header title string
	SetFontColor(rgba | icon_a, 0);
	SetFontSize(APP_FONT_SIZE_TITLE);
	DrawString(MENU_ICON_OFF + 40, 31, headerTitle);

	//header sub title string
	if (headerSubTitle)
	{
		int width = (SCREEN_WIDTH - 75) - (MENU_ICON_OFF + MENU_TITLE_OFF + WidthFromStr(headerTitle)) - 30;
		SetFontSize(APP_FONT_SIZE_SUBTITLE);
		char * tName = strdup(headerSubTitle);
		while (WidthFromStr(tName) > width)
		{
			tName[strlen(tName) - 1] = 0;
		}
		SetFontAlign(FONT_ALIGN_RIGHT);
		DrawString(SCREEN_WIDTH - 75, 35, tName);
		free(tName);
		SetFontAlign(FONT_ALIGN_LEFT);
	}
}

void DrawHeader(int icon, int xOff, const char * hdrTitle, const char * headerSubTitle, u32 rgba, u32 bgrgba, int mode)
{
	char headerTitle[44];
	snprintf(headerTitle, sizeof(headerTitle), "%.40s%s", hdrTitle, (strlen(hdrTitle) > 40 ? "..." : ""));

	//Background
	DrawBackgroundTexture(xOff, (u8)bgrgba);

	_drawListBackground(xOff, icon);

	//header mini icon
	//header title string
	SetFontColor(rgba, 0);
	if (mode)
	{
		DrawTextureCenteredX(&menu_textures[icon], xOff + MENU_ICON_OFF - 12, 40, 0, 64, 64, 0xffffffff);
		SetFontSize(APP_FONT_SIZE_SUBTITLE);
		DrawString(xOff + MENU_ICON_OFF + 40, 35, headerTitle);
	}
	else
	{
		DrawTextureCenteredX(&menu_textures[icon], xOff + MENU_ICON_OFF - 20, 32, 0, 96, 96, 0xffffffff);
		SetFontSize(APP_FONT_SIZE_TITLE);
		DrawString(xOff + MENU_ICON_OFF + 40, 31, headerTitle);
	}

	//header sub title string
	if (headerSubTitle)
	{
		int width = (SCREEN_WIDTH - 75) - (MENU_ICON_OFF + MENU_TITLE_OFF + WidthFromStr(headerTitle)) - 30;
		SetFontSize(APP_FONT_SIZE_SUBTITLE);
		char * tName = strdup(headerSubTitle);
		while (WidthFromStr(tName) > width)
		{
			tName[strlen(tName) - 1] = 0;
		}
		SetFontAlign(FONT_ALIGN_RIGHT);
		DrawString(SCREEN_WIDTH - 75, 35, tName);
		free(tName);
		SetFontAlign(FONT_ALIGN_LEFT);
	}
}

void DrawBackgroundTexture(int x, u8 alpha)
{
	DrawTexture(&menu_textures[bgimg_png_index], x, 0, 0, SCREEN_WIDTH - x, SCREEN_HEIGHT, 0xFFFFFF00 | alpha);
}

void DrawTexture(png_texture* tex, int x, int y, int z, int w, int h, u32 rgba)
{
	SDL_Rect dest = {
		.x = x,
		.y = y,
		.w = w,
		.h = h,
	};

	SDL_SetTextureAlphaMod(tex->texture, RGBA_A(rgba));
	SDL_RenderCopy(renderer, tex->texture, NULL, &dest);
}

void DrawTextureCentered(png_texture* tex, int x, int y, int z, int w, int h, u32 rgba)
{
	x -= w / 2;
	y -= h / 2;

	DrawTexture(tex, x, y, z, w, h, rgba);
}

void DrawTextureCenteredX(png_texture* tex, int x, int y, int z, int w, int h, u32 rgba)
{
	x -= w / 2;

	DrawTexture(tex, x, y, z, w, h, rgba);
}

void drawSplashLogo(int mode)
{
	int ani, max;

	if (mode > 0)
	{
		ani = 0;
		max = MENU_ANI_MAX;
	}
	else
	{
		ani = MENU_ANI_MAX;
		max = 0;
	}

	for (; ani != max; ani += mode)
	{
		// clear the current display buffer
		SDL_RenderClear(renderer);
		DrawBackground2D(APP_BG_COLOR);
		
		//------------ Backgrounds
		int logo_a_t = ((ani < 0x20) ? 0 : ((ani - 0x20)*3));
		if (logo_a_t > 0xFF)
			logo_a_t = 0xFF;
		u8 logo_a = (u8)logo_a_t;

		//App name
		DrawTextureCentered(&menu_textures[logo_png_index], SCREEN_WIDTH/2, SCREEN_HEIGHT /2, 0, menu_textures[logo_png_index].width, menu_textures[logo_png_index].height, 0xFFFFFF00 | logo_a);

		//flush and flip
		SDL_RenderPresent(renderer);
	}
}

void drawEndLogo(void)
{
	SDL_Rect rect = {
		.x = 0,
		.w = SCREEN_WIDTH,
	};

	for (rect.h = 0; rect.h <= SCREEN_HEIGHT/2; rect.h += 3)
	{
		// clear the current display buffer
		SDL_RenderClear(renderer);
		DrawBackground2D(APP_BG_COLOR);

		//App name
		DrawTextureCentered(&menu_textures[logo_png_index], SCREEN_WIDTH/2, SCREEN_HEIGHT /2, 0, menu_textures[logo_png_index].width, menu_textures[logo_png_index].height, 0xFFFFFF00 | 0xFF);

		rect.y = 0;
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
		SDL_RenderFillRect(renderer, &rect);

		rect.y = SCREEN_HEIGHT - rect.h;
		SDL_RenderFillRect(renderer, &rect);

		//flush and flip
		SDL_RenderPresent(renderer);
	}
}

static float approach(float value, float target)
{
	value += (target - value) * 0.3f;
	return ((target - value) < 0.01f && (value - target) < 0.01f) ? target : value;
}

static void drawImage(int idx, int x, int y, uint8_t alpha)
{
	DrawTexture(&menu_textures[idx], x, y, 0, menu_textures[idx].width, menu_textures[idx].height, 0xFFFFFF00 | alpha);
}

// A tile cross-fades from its idle image to its focused one while it grows
static void drawTile(int i, uint8_t alpha)
{
	float t = mm_focus[i];
	float scale = (1.0f - (1.0f - MM_TILE_DIM_SCALE) * mm_dim) * (1.0f - t) + MM_TILE_ON_SCALE * t;
	int x0 = (SCREEN_WIDTH - (MAIN_MENU_TILES * MM_TILE_SIZE + (MAIN_MENU_TILES - 1) * MM_TILE_GAP)) / 2;
	int cx = x0 + i * (MM_TILE_SIZE + MM_TILE_GAP) + MM_TILE_SIZE / 2;
	int cy = MM_TILE_Y + MM_TILE_SIZE / 2;
	int size;

	if (t < 1.0f)
	{
		size = (int)(MM_TILE_SIZE * scale);
		DrawTextureCentered(&menu_textures[mm_tile0_off_png_index + i], cx, cy, 0, size, size, 0xFFFFFF00 | alpha);
	}

	if (t > 0.0f)
	{
		size = (int)(MM_TILE_ON_SIZE * scale / MM_TILE_ON_SCALE);
		DrawTextureCentered(&menu_textures[mm_tile0_on_png_index + i], cx, cy, 0, size, size, 0xFFFFFF00 | (uint8_t)(alpha * t));
	}
}

static void _draw_MainMenu(uint8_t alpha)
{
	int i;
	int on_tile = (menu_sel < MAIN_MENU_TILES);
	u32 glow = mm_glow_color[on_tile ? menu_sel : MAIN_MENU_TILES];
	SDL_Rect line = { MM_MARGIN, MM_HEADER_Y + MM_HEADER_H, SCREEN_WIDTH - MM_MARGIN * 2, 1 };

	DrawBackground2D(APP_BG_COLOR);

	//Glow, in the colour of the focused tile
	SDL_SetTextureColorMod(menu_textures[mm_glow_png_index].texture, RGBA_R(glow), RGBA_G(glow), RGBA_B(glow));
	DrawTexture(&menu_textures[mm_glow_png_index], 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0xFFFFFF00 | alpha);

	//Header: app name, Settings and About buttons
	drawImage(mm_header_png_index, MM_MARGIN, MM_HEADER_Y, alpha);
	drawImage((menu_sel == MAIN_BTN_SETTINGS) ? mm_hbtns_settings_png_index : (menu_sel == MAIN_BTN_ABOUT) ? mm_hbtns_about_png_index : mm_hbtns_none_png_index,
		SCREEN_WIDTH - MM_MARGIN - menu_textures[mm_hbtns_none_png_index].width, MM_HEADER_Y, alpha);

	if (alpha == 0xFF)
	{
		SDL_SetRenderDrawColor(renderer, RGBA_R(MM_LINE_COLOR), RGBA_G(MM_LINE_COLOR), RGBA_B(MM_LINE_COLOR), 0xFF);
		SDL_RenderFillRect(renderer, &line);
	}

	drawImage(mm_headline_png_index, MM_MARGIN, MM_HEADLINE_Y, alpha);

	//Tiles: the focused one is drawn last, above its neighbours
	mm_dim = approach(mm_dim, on_tile ? 1.0f : 0.0f);
	for (i = 0; i < MAIN_MENU_TILES; i++)
		mm_focus[i] = approach(mm_focus[i], (menu_sel == i) ? 1.0f : 0.0f);

	for (i = 0; i < MAIN_MENU_TILES; i++)
		if (menu_sel != i)
			drawTile(i, alpha);

	if (on_tile)
		drawTile(menu_sel, alpha);

	//Button hints
	drawImage(mm_footer_png_index, SCREEN_WIDTH - MM_MARGIN - menu_textures[mm_footer_png_index].width, MM_FOOTER_Y, alpha);
}

void Draw_MainMenu_Ani(void)
{
	for (int alpha = 0; alpha < 0xFF; alpha += 15)
	{
		SDL_RenderClear(renderer);
		_draw_MainMenu(alpha);
		SDL_RenderPresent(renderer);
	}
}

void Draw_MainMenu(void)
{
	_draw_MainMenu(0xFF);
}
