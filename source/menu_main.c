#include <unistd.h>
#include <string.h>
#include <mini18n.h>

#include "orbisPad.h"
#include "saves.h"
#include "menu.h"
#include "menu_gui.h"
#include "libfont.h"
#include "ttf_render.h"
#include "common.h"

extern save_list_t usb_saves;
extern save_list_t user_backup;

extern int close_app;

int menu_options_maxopt = 0;
int * menu_options_maxsel;

int menu_id = 0;												// Menu currently in
int menu_sel = 0;												// Index of selected item (use varies per menu)
int menu_old_sel[TOTAL_MENU_IDS] = { 0 };						// Previous menu_sel for each menu

save_entry_t* selected_entry;									// Save whose cheats are shown
code_entry_t* selected_centry;									// Cheat whose options are shown
int option_index = 0;

void initMenuOptions(void)
{
	menu_options_maxopt = 0;
	while (menu_options[menu_options_maxopt].name)
	{
		menu_options[menu_options_maxopt].name = mini18n(menu_options[menu_options_maxopt].name);
		menu_options_maxopt++;
	}

	menu_options_maxsel = (int *)calloc(1, menu_options_maxopt * sizeof(int));

	for (int i = 0; i < menu_options_maxopt; i++)
	{
		menu_options_maxsel[i] = 0;
		if (menu_options[i].type == APP_OPTION_LIST)
		{
			while (menu_options[i].options[menu_options_maxsel[i]])
			{
				menu_options[i].options[menu_options_maxsel[i]] = mini18n(menu_options[i].options[menu_options_maxsel[i]]);
				menu_options_maxsel[i]++;
			}
		}
	}
}

static void ReloadList(save_list_t* save_list)
{
	init_loading_screen(_("Loading save games..."));

	if (save_list->list)
		UnloadGameList(save_list->list);

	save_list->list = save_list->ReadList();
	stop_loading_screen();
}

static void SetMenu(int id)
{
	switch (menu_id) //leaving menu
	{
		case MENU_CODE_OPTIONS:
			if (apollo_config.doAni)
				Draw_CheatsMenu_Options_Ani_Exit();
			break;
	}

	switch (id) //going to menu
	{
		case MENU_MAIN_SCREEN: //Main Menu
			if (apollo_config.doAni || menu_id == MENU_MAIN_SCREEN) //if load animation
				Draw_MainMenu_Ani();
			break;

		case MENU_USB_SAVES: //Encrypted saves on the USB drive
			// coming back from a save's cheats keeps the list (and the cheats picked so far)
			if (menu_id == MENU_MAIN_SCREEN)
			{
				ReloadList(&usb_saves);
				if (!list_count(usb_saves.list))
				{
					show_message(_("No save-games found"));
					return;
				}
			}
			if (apollo_config.doAni)
				Draw_ToolsMenu_Ani(&usb_saves);
			break;

		case MENU_USER_BACKUP: //User Tools
			ReloadList(&user_backup);
			if (apollo_config.doAni)
				Draw_ToolsMenu_Ani(&user_backup);
			break;

		case MENU_CREDITS: //About Menu
			if (apollo_config.doAni)
				Draw_AboutMenu_Ani();
			break;

		case MENU_SETTINGS: //Options Menu
			if (apollo_config.doAni)
				Draw_OptionsMenu_Ani();
			break;

		case MENU_PATCHES: //Cheats of the selected save
			if (menu_id == MENU_USB_SAVES)
			{
				menu_old_sel[MENU_PATCHES] = 0;
				if (apollo_config.doAni)
					Draw_CheatsMenu_Selection_Ani();
			}
			break;

		case MENU_CODE_OPTIONS: //Options of the selected cheat
			menu_old_sel[MENU_CODE_OPTIONS] = 0;
			if (apollo_config.doAni)
				Draw_CheatsMenu_Options_Ani();
			break;
	}

	menu_old_sel[menu_id] = menu_sel;
	menu_id = id;

	menu_sel = menu_old_sel[menu_id];
}

static void move_selection_back(int game_count, int steps)
{
	menu_sel -= steps;
	if ((menu_sel == -1) && (steps == 1))
		menu_sel = game_count - 1;
	else if (menu_sel < 0)
		menu_sel = 0;
}

static void move_selection_fwd(int game_count, int steps)
{
	menu_sel += steps;
	if ((menu_sel == game_count) && (steps == 1))
		menu_sel = 0;
	else if (menu_sel >= game_count)
		menu_sel = game_count - 1;
}

static void doSaveMenu(save_list_t * save_list)
{
	if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_UP))
		move_selection_back(list_count(save_list->list), 1);

	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_DOWN))
		move_selection_fwd(list_count(save_list->list), 1);

	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CIRCLE))
	{
		SetMenu(MENU_MAIN_SCREEN);
		return;
	}
	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CROSS) && list_count(save_list->list))
	{
		save_entry_t* entry = list_get_item(save_list->list, menu_sel);

		if (entry->type == FILE_TYPE_PS4)
		{
			selected_entry = entry;
			if (!entry->codes)
			{
				init_loading_screen(_("Loading cheats..."));
				ReadCodes(entry);
				stop_loading_screen();
			}

			SetMenu(MENU_PATCHES);
			return;
		}

		if (entry->type == FILE_TYPE_NULL)
		{
			show_message("%s\n%s%s", _("Error Mounting Save!"), entry->path, entry->dir_name);
			return;
		}

		execCommand(entry);

		// the user list shows which accounts are activated
		if (entry->cmd == CMD_ACTIVATE_ACCOUNT)
			ReloadList(save_list);
	}

	Draw_ToolsMenu(save_list, menu_sel, 0xFF);
}

static void doPatchMenu(void)
{
	code_entry_t* code;
	list_node_t* node;

	if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_UP))
		move_selection_back(list_count(selected_entry->codes), 1);

	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_DOWN))
		move_selection_fwd(list_count(selected_entry->codes), 1);

	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_LEFT))
		move_selection_back(list_count(selected_entry->codes), 5);

	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_RIGHT))
		move_selection_fwd(list_count(selected_entry->codes), 5);

	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CIRCLE))
	{
		SetMenu(MENU_USB_SAVES);
		return;
	}
	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CROSS))
	{
		selected_centry = list_get_item(selected_entry->codes, menu_sel);

		if (selected_centry->type == PATCH_COMMAND && selected_centry->codes[0] == CMD_APPLY_CHEATS)
			applyCheats(selected_entry);

		else if (selected_centry->type != PATCH_NULL && selected_centry->type != PATCH_COMMAND &&
			!(selected_centry->flags & APOLLO_CODE_FLAG_DISABLED))
		{
			selected_centry->activated = !selected_centry->activated;

			if (selected_centry->activated)
			{
				// a cheat needs the game's required codes (decrypt, checksum...)
				for (node = list_head(selected_entry->codes); (code = list_get(node)); node = list_next(node))
					if (code->flags & APOLLO_CODE_FLAG_REQUIRED && !(code->flags & APOLLO_CODE_FLAG_DISABLED) && code->options_count == 0)
						code->activated = 1;

				if (selected_centry->options)
				{
					option_index = 0;
					SetMenu(MENU_CODE_OPTIONS);
					return;
				}
			}
		}
	}

	Draw_CheatsMenu_Selection(menu_sel, 0xFFFFFFFF);
}

static void doCodeOptionsMenu(void)
{
	code_entry_t* code = selected_centry;

	if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_UP))
		move_selection_back(list_count(code->options[option_index].opts), 1);

	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_DOWN))
		move_selection_fwd(list_count(code->options[option_index].opts), 1);

	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CIRCLE))
	{
		code->activated = 0;
		SetMenu(MENU_PATCHES);
		return;
	}
	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CROSS))
	{
		code->options[option_index].sel = menu_sel;
		option_index++;

		if (option_index >= code->options_count)
		{
			SetMenu(MENU_PATCHES);
			return;
		}

		menu_sel = 0;
	}

	Draw_CheatsMenu_Options();
}

// The main menu is a row of tiles, with the Settings and About buttons above it
static void doMainMenu(void)
{
	static int last_tile = MAIN_TILE_SAVES;
	save_entry_t cmd = { 0 };

	if (menu_sel < MAIN_MENU_TILES)
		last_tile = menu_sel;

	// Check the pads.
	if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_LEFT))
	{
		if (menu_sel != MAIN_TILE_SAVES && menu_sel != MAIN_BTN_SETTINGS)
			menu_sel--;
	}
	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_RIGHT))
	{
		if (menu_sel != MAIN_MENU_TILES - 1 && menu_sel != MAIN_BTN_ABOUT)
			menu_sel++;
	}
	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_UP))
	{
		if (menu_sel < MAIN_MENU_TILES)
			menu_sel = MAIN_BTN_SETTINGS;
	}
	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_DOWN))
	{
		if (menu_sel >= MAIN_MENU_TILES)
			menu_sel = last_tile;
	}
	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CROSS))
	{
		switch (menu_sel)
		{
			case MAIN_TILE_SAVES:
				SetMenu(MENU_USB_SAVES);
				break;

			case MAIN_TILE_RESIGN:
				cmd.cmd = CMD_RESIGN_IMPORT_FOLDER;
				execCommand(&cmd);
				break;

			case MAIN_TILE_COPY_HDD:
				cmd.cmd = CMD_COPY_ALL_SAVES_HDD;
				execCommand(&cmd);
				break;

			case MAIN_TILE_TOOLS:
				SetMenu(MENU_USER_BACKUP);
				break;

			case MAIN_BTN_SETTINGS:
				SetMenu(MENU_SETTINGS);
				break;

			case MAIN_BTN_ABOUT:
				SetMenu(MENU_CREDITS);
				break;
		}

		drawScene();
		return;
	}

	else if(orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CIRCLE) && show_dialog(DIALOG_TYPE_YESNO, _("Exit to XMB?")))
		close_app = 1;

	Draw_MainMenu();
}

static void doAboutMenu(void)
{
	// Check the pads.
	if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CIRCLE))
	{
		SetMenu(MENU_MAIN_SCREEN);
		return;
	}

	Draw_AboutMenu();
}

static void doOptionsMenu(void)
{
	// Check the pads.
	if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_UP))
		move_selection_back(menu_options_maxopt, 1);

	else if(orbisPadGetButtonHold(ORBIS_PAD_BUTTON_DOWN))
		move_selection_fwd(menu_options_maxopt, 1);

	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CIRCLE))
	{
		save_app_settings(&apollo_config);
		set_ttf_window(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WIN_SKIP_LF);
		SetMenu(MENU_MAIN_SCREEN);
		return;
	}
	else if (orbisPadGetButtonHold(ORBIS_PAD_BUTTON_LEFT))
	{
		if (menu_options[menu_sel].type == APP_OPTION_LIST)
		{
			if (*menu_options[menu_sel].value > 0)
				(*menu_options[menu_sel].value)--;
			else
				*menu_options[menu_sel].value = menu_options_maxsel[menu_sel] - 1;
		}
		else if (menu_options[menu_sel].type == APP_OPTION_INC)
			(*menu_options[menu_sel].value)--;

		if (menu_options[menu_sel].type != APP_OPTION_CALL)
			menu_options[menu_sel].callback(*menu_options[menu_sel].value);
	}
	else if (orbisPadGetButtonHold(ORBIS_PAD_BUTTON_RIGHT))
	{
		if (menu_options[menu_sel].type == APP_OPTION_LIST)
		{
			if (*menu_options[menu_sel].value < (menu_options_maxsel[menu_sel] - 1))
				*menu_options[menu_sel].value += 1;
			else
				*menu_options[menu_sel].value = 0;
		}
		else if (menu_options[menu_sel].type == APP_OPTION_INC)
			*menu_options[menu_sel].value += 1;

		if (menu_options[menu_sel].type != APP_OPTION_CALL)
			menu_options[menu_sel].callback(*menu_options[menu_sel].value);
	}
	else if (orbisPadGetButtonPressed(ORBIS_PAD_BUTTON_CROSS))
	{
		if (menu_options[menu_sel].type == APP_OPTION_BOOL)
			menu_options[menu_sel].callback(*menu_options[menu_sel].value);

		else if (menu_options[menu_sel].type == APP_OPTION_CALL)
			menu_options[menu_sel].callback(0);
	}

	Draw_OptionsMenu();
}

// Resets new frame
void drawScene(void)
{
	switch (menu_id)
	{
		case MENU_MAIN_SCREEN:
			doMainMenu();
			break;

		case MENU_USB_SAVES: //Encrypted saves on the USB drive
			doSaveMenu(&usb_saves);
			break;

		case MENU_PATCHES: //Cheats of the selected save
			doPatchMenu();
			break;

		case MENU_CODE_OPTIONS: //Options of the selected cheat
			doCodeOptionsMenu();
			break;

		case MENU_USER_BACKUP: //User Tools
			doSaveMenu(&user_backup);
			break;

		case MENU_CREDITS: //About Menu
			doAboutMenu();
			break;

		case MENU_SETTINGS: //Options Menu
			doOptionsMenu();
			break;
	}
}
