#ifndef __ARTEMIS_MENU_H__
#define __ARTEMIS_MENU_H__

#include <SDL2/SDL.h>

#include "types.h"
#include "settings.h"

// SDL window and software renderer
extern SDL_Window* window;
extern SDL_Renderer* renderer;

enum menu_screen_ids
{
	MENU_MAIN_SCREEN,		/* 0 - Main Menu */
	MENU_USB_SAVES,			/* 1 - USB tools and saves */
	MENU_USER_BACKUP,		/* 2 - User Tools */
	MENU_SETTINGS,			/* 3 - Options Menu */
	MENU_CREDITS,			/* 4 - About Menu */
	MENU_PATCHES,			/* 5 - Cheats of one save */
	MENU_CODE_OPTIONS,		/* 6 - Options of one cheat */
	TOTAL_MENU_IDS
};

//Textures
enum texture_index
{
	bgimg_png_index,
	logo_png_index,

	// main menu (pre-rendered from the design)
	mm_header_png_index,
	mm_headline_png_index,
	mm_hbtns_none_png_index,
	mm_hbtns_settings_png_index,
	mm_hbtns_about_png_index,
	mm_footer_png_index,
	mm_glow_png_index,
	mm_tile0_off_png_index,
	mm_tile1_off_png_index,
	mm_tile2_off_png_index,
	mm_tile3_off_png_index,
	mm_tile0_on_png_index,
	mm_tile1_on_png_index,
	mm_tile2_on_png_index,
	mm_tile3_on_png_index,

	cat_about_png_index,
	cat_opt_png_index,
	cat_usb_png_index,
	cat_bup_png_index,
	cat_sav_png_index,
	cat_cheats_png_index,
	cheat_png_index,
	edit_shadow_png_index,
	tag_warning_png_index,
	tag_lock_png_index,
	tag_own_png_index,
	tag_apply_png_index,
	footer_ico_circle_png_index,
	footer_ico_cross_png_index,
	opt_off_png_index,
	opt_on_png_index,
	help_png_index,
	scroll_bg_png_index,
	scroll_lock_png_index,
	mark_arrow_png_index,
	mark_line_png_index,

	TOTAL_MENU_TEXTURES
};

// Main menu items: the tiles, then the header buttons
enum main_menu_items
{
	MAIN_TILE_SAVES,
	MAIN_TILE_RESIGN,
	MAIN_TILE_COPY_HDD,
	MAIN_TILE_TOOLS,
	MAIN_BTN_SETTINGS,
	MAIN_BTN_ABOUT,
	MAIN_MENU_ITEMS,
	MAIN_MENU_TILES = MAIN_BTN_SETTINGS
};

#define RGBA_R(c)		(uint8_t)((c & 0xFF000000) >> 24)
#define RGBA_G(c)		(uint8_t)((c & 0x00FF0000) >> 16)
#define RGBA_B(c)		(uint8_t)((c & 0x0000FF00) >> 8)
#define RGBA_A(c)		(uint8_t) (c & 0x000000FF)

#define DIALOG_TYPE_OK						0
#define DIALOG_TYPE_YESNO					1

#define APP_BG_COLOR						0x0F1013FF

#define APP_FONT_COLOR						0xFFFFFF00
#define APP_FONT_TAG_COLOR					0x00000000
#define APP_FONT_TITLE_COLOR				0xFFFFFF00
#define APP_FONT_SIZE_TITLE					84, 72
#define APP_FONT_SIZE_SUBTITLE				68, 60
#define APP_FONT_SIZE_SELECTION				56, 48
#define APP_FONT_SIZE_DESCRIPTION			54, 48
#define APP_LINE_OFFSET						40

#define SCREEN_WIDTH						1920
#define SCREEN_HEIGHT						1080

//Asset sizes
#define help_png_x							80
#define help_png_y							150
#define help_png_w							1730
#define help_png_h							800

typedef struct t_png_texture
{
	uint32_t *buffer;
	int width;
	int height;
	u32 size;
	SDL_Texture *texture;
} png_texture;

extern png_texture * menu_textures;				// png_texture array for main menu, initialized in LoadTexture

extern int idle_time;							// Set by readPad

extern int menu_id;
extern int menu_sel;
extern int menu_old_sel[]; 

extern struct save_entry * selected_entry;
extern struct code_entry * selected_centry;
extern int option_index;

extern void DrawBackground2D(u32 rgba);
extern void DrawTexture(png_texture* tex, int x, int y, int z, int w, int h, u32 rgba);
extern void DrawTextureCentered(png_texture* tex, int x, int y, int z, int w, int h, u32 rgba);
extern void DrawTextureCenteredX(png_texture* tex, int x, int y, int z, int w, int h, u32 rgba);
extern void DrawHeader(int icon, int xOff, const char * headerTitle, const char * headerSubTitle, u32 rgba, u32 bgrgba, int mode);
extern void DrawHeader_Ani(int icon, const char * headerTitle, const char * headerSubTitle, u32 rgba, u32 bgrgba, int ani, int div);
extern void DrawBackgroundTexture(int x, u8 alpha);
extern void Draw_MainMenu(void);
extern void Draw_MainMenu_Ani(void);
int LoadMenuTexture(const char* path, int idx);
void initMenuOptions(void);

void drawScene(void);
void drawSplashLogo(int m);
void drawEndLogo(void);

int load_app_settings(app_config_t* config);
int save_app_settings(app_config_t* config);

int initialize_jbc(void);
void terminate_jbc(void);
int initVshDataMount(void);
int get_max_pfskey_ver(void);

#endif
