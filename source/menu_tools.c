#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <mini18n.h>

#include "saves.h"
#include "menu.h"
#include "menu_gui.h"
#include "libfont.h"

#define UTF8_CHAR_GROUP     "\xe2\x97\x86"
#define UTF8_CHAR_ITEM      "\xe2\x94\x97"
#define CHAR_ICON_ALERT     "\x0F"

static void DrawScrollBar(int selIndex, int max, int y_inc, int xOff, u8 alpha)
{
    int game_y = help_png_y;
    int maxPerPage = (SCREEN_HEIGHT - (game_y * 2)) / y_inc;
    float pLoc = (float)selIndex / (float)max;
    int yTotal = SCREEN_HEIGHT - (game_y * 2);
    
    if (max < maxPerPage)
        return;
    
    if (idle_time > 80)
    {
        int dec = (idle_time - 80) * 4;
        if (dec > alpha)
            dec = alpha;
        alpha -= dec;
    }
    
    SetFontAlign(FONT_ALIGN_LEFT);
    
    //Draw box
	DrawTextureCenteredX(&menu_textures[scroll_bg_png_index], xOff, game_y, 0, menu_textures[scroll_bg_png_index].width, yTotal, 0xffffff00 | alpha);
    
    //Draw cursor
	DrawTextureCenteredX(&menu_textures[scroll_lock_png_index], xOff, (int)((float)yTotal * pLoc) + game_y, 0, menu_textures[scroll_lock_png_index].width, maxPerPage * 2, 0xffffff00 | alpha);
}

static u8 CalculateAlphaList(int curIndex, int selIndex, int max)
{
    int mult = ((float)0xFF / (float)max * 7) / 4;
    int dif = (selIndex - curIndex);
    if (dif < 0)
        dif *= -1;
    int alpha = (0xFF - (dif * mult));
    return (u8)(alpha < 0 ? 0 : alpha);
}


/*
 * List of commands
 */
static void DrawGameList(int selIndex, list_t * games, u8 alpha)
{
    SetFontSize(APP_FONT_SIZE_SELECTION);
    
    list_node_t *node;
    save_entry_t *item;
    int game_y = help_png_y, y_inc = APP_LINE_OFFSET;
    int maxPerPage = (SCREEN_HEIGHT - (game_y * 2) - 30) / y_inc;
    
    int x = selIndex - (maxPerPage / 2);
    int max = maxPerPage + selIndex;
    
    if (max > list_count(games))
        max = list_count(games);
    
    node = list_head(games);
    for (int i = 0; i < x; i++)
        node = list_next(node);
    
    for (; x < max; x++)
    {
        if (x >= 0 && node)
        {
			item = list_get(node);
			u8 a = ((alpha * CalculateAlphaList(x, selIndex, maxPerPage)) / 0xFF);

            if (!a)
                goto skip_draw;

            SetFontColor(APP_FONT_COLOR | a, 0);
			if (item->name)
			{
				char * nBuffer = strdup(item->name);
				int game_name_width = 0;
				int max_width = item->title_id ? ((SCREEN_WIDTH - 40) - (MENU_ICON_OFF * 5)) : (SCREEN_WIDTH - 200);
				while ((game_name_width = WidthFromStr(nBuffer)) > 0 && (MENU_ICON_OFF + (MENU_TITLE_OFF * 1) + game_name_width) > max_width)
					nBuffer[strlen(nBuffer) - 1] = '\0';
				DrawString(MENU_ICON_OFF + (MENU_TITLE_OFF * 1), game_y, nBuffer);
				free(nBuffer);
			}
			if (item->title_id)
				DrawString((SCREEN_WIDTH - 40) - (MENU_ICON_OFF * 5), game_y, item->title_id);

			// saves already signed for the current account
			if (item->flags & SAVE_FLAG_OWNER)
				DrawString(SCREEN_WIDTH - (MENU_ICON_OFF * 3), game_y, (char[]) {' ', CHAR_TAG_OWNER, 0});
skip_draw:
			node = list_next(node);
        }
        
        if (x == selIndex)
        {
            DrawTexture(&menu_textures[mark_line_png_index], 0, game_y, 0, SCREEN_WIDTH, menu_textures[mark_line_png_index].height * 2, 0xFFFFFF00 | alpha);
            DrawTextureCenteredX(&menu_textures[mark_arrow_png_index], MENU_ICON_OFF - 20, game_y, 0, (2 * y_inc) / 3, y_inc + 2, 0xFFFFFF00 | alpha);
        }
        
        game_y += y_inc;
    }
    
    DrawScrollBar(selIndex, list_count(games), y_inc, SCREEN_WIDTH - 125, alpha);
}

/*
 * Cheats Code Options Menu
 */
static void DrawOptions(option_entry_t* option, u8 alpha, int y_inc, int selIndex)
{
    option_value_t* optval;

    if (!option->opts)
        return;
    
    int c = 0, yOff = 80;
    
    int maxPerPage = (SCREEN_HEIGHT - (yOff * 2)) / y_inc;
    int startDrawX = selIndex - (maxPerPage / 2);
    int max = maxPerPage + startDrawX;
    
    SetFontSize(APP_FONT_SIZE_SELECTION);

    for (c = startDrawX; c < max; c++)
    {
        if (c >= 0 && c < list_count(option->opts))
        {
            SetFontColor(APP_FONT_COLOR | ((alpha * CalculateAlphaList(c, selIndex, maxPerPage)) / 0xFF), 0);
            
            optval = list_get_item(option->opts, c);
            if (optval->name)
                DrawString(MENU_SPLIT_OFF + MENU_ICON_OFF + 20, yOff, optval->name);
            
            //Selector
            if (c == selIndex)
            {
                DrawTexture(&menu_textures[mark_line_png_index], MENU_SPLIT_OFF, yOff, 0, SCREEN_WIDTH - MENU_SPLIT_OFF, menu_textures[mark_line_png_index].height * 2, 0xFFFFFF00 | alpha);
            }
        }
        yOff += y_inc;
    }
}

void Draw_CheatsMenu_Options_Ani_Exit(void)
{
	int div = 12, ani = 0, left = MENU_SPLIT_OFF;
	for (ani = MENU_ANI_MAX - 1; ani >= 0; ani--)
	{
		SDL_RenderClear(renderer);
		DrawBackground2D(APP_BG_COLOR);

		u8 icon_a = (u8)((int)(((SCREEN_WIDTH - left) / SCREEN_WIDTH) * 255.0));
		left = MENU_SPLIT_OFF + ((MENU_ANI_MAX - ani) * div * 3);
		if (left > SCREEN_WIDTH)
			left = SCREEN_WIDTH;

		u8 rgbVal = 0xFF;
		rgbVal -= (u8)((SCREEN_WIDTH - left) / div);
		if (rgbVal < 0xD0)
			rgbVal = 0xD0;
		Draw_CheatsMenu_Selection(menu_old_sel[MENU_PATCHES], (rgbVal << 24) | (rgbVal << 16) | (rgbVal << 8) | 0xFF);

		DrawTexture(&menu_textures[edit_shadow_png_index], left - (menu_textures[edit_shadow_png_index].width * 1) + 1, 0, 0, menu_textures[edit_shadow_png_index].width, SCREEN_HEIGHT, icon_a);
		DrawHeader(cat_cheats_png_index, left, selected_centry->name, _("Options"), APP_FONT_TITLE_COLOR | icon_a, 0xffffffff, 1);

		SDL_RenderPresent(renderer);

		if (left == SCREEN_WIDTH)
			return;
	}
}

void Draw_CheatsMenu_Options_Ani(void)
{
	int div = 12, ani = 0, left = SCREEN_WIDTH;
    for (ani = 0; ani < MENU_ANI_MAX; ani++)
    {
		SDL_RenderClear(renderer);
		DrawBackground2D(APP_BG_COLOR);
        
		u8 icon_a = (u8)(((ani * 4 + 0x40) > 0xFF) ? 0xFF : (ani * 4 + 0x40));
		left = SCREEN_WIDTH - (ani * div * 3);
		if (left < MENU_SPLIT_OFF)
			left = MENU_SPLIT_OFF;
		
        
		u8 rgbVal = 0xFF;
		rgbVal -= (u8)((SCREEN_WIDTH - left) / div);
		if (rgbVal < 0xD0)
			rgbVal = 0xD0;
		Draw_CheatsMenu_Selection(menu_sel, (rgbVal << 24) | (rgbVal << 16) | (rgbVal << 8) | 0xFF);

		DrawTexture(&menu_textures[edit_shadow_png_index], left - (menu_textures[edit_shadow_png_index].width * 1) + 1, 0, 0, menu_textures[edit_shadow_png_index].width, SCREEN_HEIGHT, icon_a);
		DrawHeader(cat_cheats_png_index, left, selected_centry->name, _("Options"), APP_FONT_TITLE_COLOR | icon_a, 0xffffffff, 1);
        
		u8 game_a = (u8)(icon_a < 0x8F ? 0 : icon_a);
		DrawOptions(&selected_centry->options[option_index], game_a, APP_LINE_OFFSET, menu_old_sel[MENU_CODE_OPTIONS]);
        DrawScrollBar(menu_old_sel[MENU_CODE_OPTIONS], list_count(selected_centry->options[option_index].opts), APP_LINE_OFFSET, SCREEN_WIDTH - 125, game_a);
        
		SDL_RenderPresent(renderer);
        
		if ((SCREEN_WIDTH - (ani * div * 3)) < (MENU_SPLIT_OFF / 2))
            return;
    }
}

void Draw_CheatsMenu_Options(void)
{
	//------------ Backgrounds

	Draw_CheatsMenu_Selection(menu_old_sel[MENU_PATCHES], 0xD0D0D0FF);

	DrawTexture(&menu_textures[edit_shadow_png_index], MENU_SPLIT_OFF - (menu_textures[edit_shadow_png_index].width * 1) + 1, 0, 0, menu_textures[edit_shadow_png_index].width, SCREEN_HEIGHT, 0x000000FF);
	DrawHeader(cat_cheats_png_index, MENU_SPLIT_OFF, selected_centry->name, _("Options"), APP_FONT_TITLE_COLOR | 0xFF, 0xffffffff, 1);

	DrawOptions(&selected_centry->options[option_index], 0xFF, APP_LINE_OFFSET, menu_sel);
	DrawScrollBar(menu_sel, list_count(selected_centry->options[option_index].opts), APP_LINE_OFFSET, SCREEN_WIDTH - 125, 0xFF);
}

static void DrawCheatsList(int selIndex, save_entry_t* game, u8 alpha)
{
    SetFontSize(APP_FONT_SIZE_SELECTION);
    
    list_node_t *node;
    code_entry_t *code;
    option_value_t *optval;
    int game_y = help_png_y, y_inc = APP_LINE_OFFSET;
    int maxPerPage = (SCREEN_HEIGHT - (game_y * 2) - 30) / y_inc;
    
    int x = selIndex - (maxPerPage / 2);
    int max = maxPerPage + selIndex;
    
    if (max > list_count(game->codes))
        max = list_count(game->codes);
    
    node = list_head(game->codes);
    for (int i = 0; i < x; i++)
        node = list_next(node);

    for (; x < max; x++)
    {
        if (x >= 0 && node)
        {
			code = list_get(node);
			u8 a = (u8)((alpha * CalculateAlphaList(x, selIndex, maxPerPage)) / 0xFF);

            if (!a)
                goto skip_code;

            SetFontColor(APP_FONT_COLOR | a, 0);

            const char *group = "";
            if (code->flags & APOLLO_CODE_FLAG_PARENT) group = UTF8_CHAR_GROUP " ";
            if (code->flags & APOLLO_CODE_FLAG_CHILD)  group = " " UTF8_CHAR_ITEM " ";
            const char *alert = (code->flags & APOLLO_CODE_FLAG_ALERT) ? CHAR_ICON_ALERT : "";

            float dx = DrawFormatString(MENU_ICON_OFF + (MENU_TITLE_OFF * 3), game_y, "%s%s%s", group, alert, code->name);

            if (code->activated)
            {
				DrawTexture(&menu_textures[cheat_png_index], MENU_ICON_OFF, game_y, 0, (MENU_TITLE_OFF * 3) - 15, y_inc + 2, 0xFFFFFF00 | a);

                SetFontSize((int)(y_inc * 1.0), (int)(y_inc * 0.6));
                SetFontAlign(FONT_ALIGN_CENTER);
				SetFontColor(APP_FONT_TAG_COLOR | a, 0);
                DrawString(MENU_ICON_OFF + ((MENU_TITLE_OFF * 3) - 15) / 2, game_y + 5, "select");
                SetFontAlign(FONT_ALIGN_LEFT);
                SetFontSize(APP_FONT_SIZE_SELECTION);
                
                if (code->options_count > 0 && code->options)
                {
                    for (int od = 0; od < code->options_count; od++)
                    {
                        optval = list_get_item(code->options[od].opts, code->options[od].sel);
                        if (code->options[od].sel >= 0 && optval && optval->name)
                        {
                            //Allocate option
                            char * option = calloc(1, strlen(optval->name) + 4);

                            //If first print "(NAME", else add to list of names ", NAME"
                            sprintf(option, (od == 0) ? " (%s" : ", %s", optval->name);
                            
                            //If it's the last one then end the list
                            if (od == (code->options_count - 1))
                                option[strlen(option)] = ')';
                            
							SetFontColor(APP_FONT_COLOR | a, 0);
                            dx = DrawString(dx, game_y, option);
                            
                            free (option);
                        }
                    }
                }
            }
skip_code:
            node = list_next(node);
        }
        
        if (x == selIndex)
        {
            //Draw selection bar
            DrawTexture(&menu_textures[mark_line_png_index], 0, game_y, 0, SCREEN_WIDTH, menu_textures[mark_line_png_index].height * 2, 0xFFFFFF00 | alpha);
            DrawTextureCenteredX(&menu_textures[mark_arrow_png_index], MENU_ICON_OFF - 20, game_y, 0, (2 * y_inc) / 3, y_inc + 2, 0xFFFFFF00 | alpha);
        }
        
        game_y += y_inc;
    }
    
    DrawScrollBar(selIndex, list_count(game->codes), y_inc, SCREEN_WIDTH - 125, alpha);
}

void Draw_CheatsMenu_Selection_Ani(void)
{
    int ani = 0;
    for (ani = 0; ani < MENU_ANI_MAX; ani++)
    {
        SDL_RenderClear(renderer);
        DrawBackground2D(APP_BG_COLOR);
        
        u8 icon_a = (u8)(((ani * 2) > 0xFF) ? 0xFF : (ani * 2));
        
		DrawHeader_Ani(cat_sav_png_index, selected_entry->name, selected_entry->title_id, APP_FONT_TITLE_COLOR, 0xffffffff, ani, 12);

        int _game_a = (int)(icon_a - (MENU_ANI_MAX / 2)) * 2;
        if (_game_a > 0xFF)
            _game_a = 0xFF;
        u8 game_a = (u8)(_game_a < 0 ? 0 : _game_a);
        DrawCheatsList(menu_old_sel[MENU_PATCHES], selected_entry, game_a);
        
        SDL_RenderPresent(renderer);
        
        if (_game_a == 0xFF)
            return;
    }
}

void Draw_CheatsMenu_Selection(int menuSel, u32 rgba)
{
	DrawHeader(cat_sav_png_index, 0, selected_entry->name, selected_entry->title_id, APP_FONT_TITLE_COLOR | 0xFF, rgba, 0);
    DrawCheatsList(menuSel, selected_entry, (u8)rgba);
}

static int get_icon_id(int type)
{
    switch (type)
    {
        case MENU_USB_SAVES:
            return cat_usb_png_index;
        default:
            return cat_bup_png_index;
    }
}

/*
 * List menus (USB saves, User Tools)
 */
void Draw_ToolsMenu_Ani(save_list_t * list)
{
    for (int ani = 0; ani < MENU_ANI_MAX; ani++)
    {
        SDL_RenderClear(renderer);
        DrawBackground2D(APP_BG_COLOR);
        
        u8 icon_a = (u8)(((ani * 2) > 0xFF) ? 0xFF : (ani * 2));
        
        DrawHeader_Ani(get_icon_id(list->id), list->title, (list->id == MENU_USB_SAVES) ? _("Saves") : _("Tools"), APP_FONT_TITLE_COLOR, 0xffffffff, ani, 12);
        
        int _game_a = (int)(icon_a - (MENU_ANI_MAX / 2)) * 2;
        if (_game_a > 0xFF)
            _game_a = 0xFF;
        u8 game_a = (u8)(_game_a < 0 ? 0 : _game_a);
        DrawGameList(menu_old_sel[list->id], list->list, game_a);
        
        SDL_RenderPresent(renderer);
        
        if (_game_a == 0xFF)
            return;
    }
}

void Draw_ToolsMenu(save_list_t * list, int menuSel, u8 alpha)
{
    DrawHeader(get_icon_id(list->id), 0, list->title, (list->id == MENU_USB_SAVES) ? _("Saves") : _("Tools"), APP_FONT_TITLE_COLOR | 0xFF, 0xffffff00 | alpha, 0);
    DrawGameList(menuSel, list->list, alpha);
}
