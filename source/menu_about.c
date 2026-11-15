#include <unistd.h>
#include <string.h>
#include <mini18n.h>

#include "saves.h"
#include "menu.h"
#include "menu_gui.h"
#include "libfont.h"

static char user_id_str[9] = "00000000";
static char psid_str[] = "0000000000000000 0000000000000000";
static char account_id_str[] = "0000000000000000";

const char * menu_about_strings[] = { "Based on", "Apollo Save Tool",
									"Bucanero", "Original developer",
									"Berion", "GUI design",
									"Dnawrkshp", "Artemis code",
									NULL };

const char * menu_about_strings_project[] = { "User ID", user_id_str,
											"Account ID", account_id_str,
											"Console PSID", psid_str,
											NULL };

static void _setIdValues()
{
	// set to display the PSID on the About menu
	snprintf(psid_str, sizeof(psid_str), "%016lX %016lX", ES64(apollo_config.psid[0]), ES64(apollo_config.psid[1]));
	snprintf(user_id_str, sizeof(user_id_str), "%08x", apollo_config.user_id);
	snprintf(account_id_str, sizeof(account_id_str), "%016lx", apollo_config.account_id);
}

static void _draw_AboutMenu(u8 alpha)
{
	int cnt = 0;
	u8 alp2 = ((alpha*2) > 0xFF) ? 0xFF : (alpha * 2);
	png_texture* logo = &menu_textures[logo_png_index];

	//------------- About Menu Contents
	DrawTextureCenteredX(logo, SCREEN_WIDTH/2, 170, 0, logo->width * 2/3, logo->height * 2/3, 0xFFFFFF00 | alp2);

	SetFontSize(APP_FONT_SIZE_SELECTION);
	SetFontAlign(FONT_ALIGN_SCREEN_CENTER);
	SetFontColor(APP_FONT_COLOR | (alpha * 0x8C / 0xFF), 0);
	DrawString(0, 290, "PlayStation 4 version");

	for (cnt = 0; menu_about_strings[cnt] != NULL; cnt += 2)
	{
		SetFontAlign(FONT_ALIGN_RIGHT);
		SetFontColor(APP_FONT_COLOR | alpha, 0);
		DrawString((SCREEN_WIDTH / 2) - 20, 370 + (cnt * 24), menu_about_strings[cnt]);

		SetFontAlign(FONT_ALIGN_LEFT);
		SetFontColor(APP_FONT_COLOR | (alpha * 0x8C / 0xFF), 0);
		DrawString((SCREEN_WIDTH / 2) + 20, 370 + (cnt * 24), menu_about_strings[cnt + 1]);
	}

	DrawTexture(&menu_textures[help_png_index], help_png_x, 620, 0, help_png_w, 280, 0xFFFFFF00 | alp2);

	SetFontAlign(FONT_ALIGN_SCREEN_CENTER);
	SetFontColor(APP_FONT_COLOR | alpha, 0);
	SetFontSize(APP_FONT_SIZE_DESCRIPTION);
	DrawString(0, 650, _("Console details:"));
	SetFontSize(APP_FONT_SIZE_SELECTION);

	for (cnt = 0; menu_about_strings_project[cnt] != NULL; cnt += 2)
	{
		SetFontAlign(FONT_ALIGN_RIGHT);
		SetFontColor(APP_FONT_COLOR | (alpha * 0x8C / 0xFF), 0);
		DrawString((SCREEN_WIDTH / 2) - 10, 725 + (cnt * 24), menu_about_strings_project[cnt]);

		SetFontAlign(FONT_ALIGN_LEFT);
		SetFontColor(APP_FONT_COLOR | alpha, 0);
		DrawString((SCREEN_WIDTH / 2) + 10, 725 + (cnt * 24), menu_about_strings_project[cnt + 1]);
	}

	SetFontAlign(FONT_ALIGN_LEFT);
}

void Draw_AboutMenu_Ani(void)
{
	_setIdValues();

	for (int ani = 0; ani < MENU_ANI_MAX; ani++)
	{
		SDL_RenderClear(renderer);
		DrawBackground2D(APP_BG_COLOR);

		DrawHeader_Ani(cat_about_png_index, _("About"), "v" APOLLO_VERSION, APP_FONT_TITLE_COLOR, 0xffffffff, ani, 12);

		//------------- About Menu Contents

		int rate = (0x100 / (MENU_ANI_MAX - 0x60));
		u8 about_a = (u8)((((ani - 0x60) * rate) > 0xFF) ? 0xFF : ((ani - 0x60) * rate));
		if (ani < 0x60)
			about_a = 0;

		_draw_AboutMenu(about_a);

		SDL_RenderPresent(renderer);

		if (about_a == 0xFF)
			return;
	}
}

void Draw_AboutMenu(void)
{
	_setIdValues();
	DrawHeader(cat_about_png_index, 0, _("About"), "v" APOLLO_VERSION, APP_FONT_TITLE_COLOR | 0xFF, 0xffffffff, 0);
	_draw_AboutMenu(0xFF);
}
