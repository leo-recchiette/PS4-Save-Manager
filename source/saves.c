#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <orbis/SaveData.h>
#include <sqlite3.h>
#include <mini18n.h>

#include "saves.h"
#include "common.h"
#include "sfo.h"
#include "settings.h"
#include "util.h"
#include "sd.h"

#define UTF8_CHAR_STAR		"\xE2\x98\x85"

#define CHAR_ICON_SIGN		"\x06"
#define CHAR_ICON_WARN		"\x0F"


int orbis_SaveUmount(const char* mountPath)
{
	char mountDir[256];

	snprintf(mountDir, sizeof(mountDir), APOLLO_SANDBOX_PATH, mountPath);
	int umountErrorCode = umountSave(mountDir, 0, 0);

	if (umountErrorCode < 0)
	{
		LOG("UMOUNT_ERROR (%X)", umountErrorCode);
		notify_popup(NOTIFICATION_ICON_BAN, _("Warning! Save couldn't be unmounted!"));
	}

	rmdir(mountDir);
	return (umountErrorCode == SUCCESS);
}

int orbis_SaveMount(const save_entry_t *save, uint32_t mount_mode, char* mount_path)
{
	char mountDir[256];
	char keyPath[256];
	char volumePath[256];

	snprintf(mountDir, sizeof(mountDir), APOLLO_SANDBOX_PATH, save->dir_name);
	if (mkdirs(mountDir) < 0)
	{
		LOG("ERROR: can't create '%s'", mountDir);
		return 0;
	}

	if (mount_mode & SAVE_FLAG_LOCKED)
	{
		snprintf(keyPath, sizeof(keyPath), "%s%s.bin", save->path, save->dir_name);
		snprintf(volumePath, sizeof(volumePath), "%s%s", save->path, save->dir_name);
	}
	else
	{
		snprintf(keyPath, sizeof(keyPath), SAVES_PATH_HDD "%s/%s.bin", apollo_config.user_id, save->title_id, save->dir_name);
		snprintf(volumePath, sizeof(volumePath), SAVES_PATH_HDD "%s/sdimg_%s", apollo_config.user_id, save->title_id, save->dir_name);
	}

	if ((mount_mode & ORBIS_SAVE_DATA_MOUNT_MODE_CREATE2) && (file_exists(keyPath) != SUCCESS))
	{
		sqlite3 *db;
		char *query, dbpath[256];

		LOG("Creating save '%s'...", keyPath);
		mkdirs(volumePath);
		if (createSave(volumePath, keyPath, save->blocks) < 0)
		{
			LOG("ERROR: can't create '%s'", keyPath);
			return 0;
		}

		snprintf(dbpath, sizeof(dbpath), SAVES_DB_PATH, apollo_config.user_id);
		if ((db = open_sqlite_db(dbpath)) == NULL)
			return 0;

		query = sqlite3_mprintf("INSERT INTO savedata(title_id, dir_name, main_title, sub_title, detail, tmp_dir_name, is_broken, user_param, blocks, free_blocks, size_kib, mtime, fake_broken, account_id, user_id, faked_owner, cloud_icon_url, cloud_revision, game_title_id) "
			"VALUES (%Q, %Q, '', '', '', '', 0, 0, %d, %d, %d, strftime('%%Y-%%m-%%dT%%H:%%M:%%S.00Z', CURRENT_TIMESTAMP), 0, %ld, %d, 0, '', 0, %Q);",
			save->title_id, save->dir_name, save->blocks, save->blocks, (save->blocks*32), apollo_config.account_id, apollo_config.user_id, save->title_id);

		if (sqlite3_exec(db, query, NULL, NULL, NULL) != SQLITE_OK)
		{
			LOG("Error inserting '%s': %s", save->title_id, sqlite3_errmsg(db));
			sqlite3_free(query);
			sqlite3_close(db);
			return 0;
		}

		save_sqlite_db(db, dbpath);
		sqlite3_free(query);
		sqlite3_close(db);
	}

	int mountErrorCode = mountSave(volumePath, keyPath, mountDir);
	if (mountErrorCode < 0)
	{
		LOG("ERROR (%X): can't mount '%s/%s'", mountErrorCode, save->title_id, save->dir_name);
		rmdir(mountDir);
		return 0;
	}

	LOG("'%s/%s' mountPath (%s)", save->title_id, save->dir_name, mountDir);
	strlcpy(mount_path, save->dir_name, ORBIS_SAVE_DATA_DIRNAME_DATA_MAXSIZE);

	return 1;
}

int orbis_UpdateSaveParams(const save_entry_t* save, const char* title, const char* subtitle, const char* details, uint32_t userParam)
{
	sqlite3 *db;
	char* query, dbpath[256];

	snprintf(dbpath, sizeof(dbpath), SAVES_DB_PATH, apollo_config.user_id);
	db = open_sqlite_db(dbpath);
	if (!db)
		return 0;

	LOG("Updating %s ...", save->title_id);
	query = sqlite3_mprintf("UPDATE savedata SET (main_title, sub_title, detail, user_param)="
		"(%Q, %Q, %Q, %ld) WHERE (title_id=%Q AND dir_name=%Q);",
		title, subtitle, details, userParam, save->title_id, save->dir_name);

	if (sqlite3_exec(db, query, NULL, NULL, NULL) != SQLITE_OK)
	{
		LOG("Failed to execute query: %s", sqlite3_errmsg(db));
		sqlite3_free(query);
		sqlite3_close(db);
		return 0;
	}

	save_sqlite_db(db, dbpath);
	sqlite3_free(query);
	sqlite3_close(db);

	return 1;
}

/*
 * Checks to see if a ends with b (case insensitive)
 * Return: pointer if true, NULL if false
 */
static char* endsWith(const char * a, const char * b)
{
	int al = strlen(a), bl = strlen(b);

	if (al < bl)
		return NULL;

	a += (al - bl);
	while (*a)
		if (toupper(*a++) != toupper(*b++)) return NULL;

	return (char*) (a - bl);
}

/*
 * Reads the contents of a text file into a new buffer
 */
char * readTextFile(const char * path)
{
	FILE *f = fopen(path, "rb");

	if (!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (fsize <= 0)
	{
		fclose(f);
		return NULL;
	}

	char * string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;

	return string;
}

static save_entry_t* _createSaveEntry(uint16_t flag, const char* icon, const char* name)
{
	save_entry_t* entry = (save_entry_t *)calloc(1, sizeof(save_entry_t));
	entry->flags = flag;
	asprintf(&entry->name, "%s%s", icon, name);

	return entry;
}

static code_entry_t* _createCmdCode(uint8_t type, const char* icon, const char* name, char code)
{
	code_entry_t* entry = (code_entry_t *)calloc(1, sizeof(code_entry_t));
	entry->type = type;
	asprintf(&entry->codes, "%c", code);
	asprintf(&entry->name, "%s%s", icon, name);

	return entry;
}

static void _walk_dir_list(const char* startdir, const char* inputdir, const char* mask, list_t* list)
{
	char fullname[256];
	struct dirent *dirp;
	int len = strlen(startdir);
	DIR *dp = opendir(inputdir);

	if (!dp) {
		LOG("Failed to open input directory: '%s'", inputdir);
		return;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if ((strcmp(dirp->d_name, ".")  == 0) || (strcmp(dirp->d_name, "..") == 0) || (strcmp(dirp->d_name, "sce_sys") == 0))
			continue;

		snprintf(fullname, sizeof(fullname), "%s%s", inputdir, dirp->d_name);

		if (dirp->d_type == DT_DIR)
		{
			strcat(fullname, "/");
			_walk_dir_list(startdir, fullname, mask, list);
		}
		else if (wildcard_match_icase(dirp->d_name, mask))
			list_append(list, strdup(fullname+len));
	}
	closedir(dp);
}

/*
 * Files of the mounted save matching a cheat's target mask (e.g. "data*.bin"),
 * offered as options when the cheat is selected
 */
static option_entry_t* get_file_entries(const char* save_path, const char* mask)
{
	char *filename;
	list_t* file_list;
	list_node_t* node;
	option_value_t* optval;
	option_entry_t* opt;

	if (dir_exists(save_path) != SUCCESS)
		return NULL;

	LOG("Loading filenames {%s} from '%s'...", mask, save_path);

	file_list = list_alloc();
	_walk_dir_list(save_path, save_path, mask, file_list);

	if (!list_count(file_list))
	{
		asprintf(&filename, CHAR_ICON_WARN " --- %s%s --- " CHAR_ICON_WARN, save_path, mask);
		list_append(file_list, filename);
	}

	opt = (option_entry_t*) calloc(1, sizeof(option_entry_t));
	opt->sel = -1;
	opt->opts = list_alloc();

	for (node = list_head(file_list); (filename = list_get(node)); node = list_next(node))
	{
		optval = malloc(sizeof(option_value_t));
		optval->name = filename;
		optval->value = strdup(mask);
		list_append(opt->opts, optval);
	}

	list_free(file_list);

	return opt;
}

/*
 * Cheats of an encrypted USB save, from APOLLO_DATA_PATH/<TITLE_ID>.savepatch
 */
int ReadCodes(save_entry_t * save)
{
	list_node_t* node;
	code_entry_t * code;
	char filePath[256];
	char mount[ORBIS_SAVE_DATA_DIRNAME_DATA_MAXSIZE];
	char * buffer;
	char * tmp;

	save->codes = list_alloc();
	list_append(save->codes, _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Apply Changes & Resign"), CMD_APPLY_CHEATS));

	snprintf(filePath, sizeof(filePath), APOLLO_DATA_PATH "%s.savepatch", save->title_id);
	if ((buffer = readTextFile(filePath)) == NULL)
	{
		list_append(save->codes, _createCmdCode(PATCH_NULL, CHAR_ICON_WARN " ", _("No cheats found for this game"), CMD_CODE_NULL));
		return list_count(save->codes);
	}

	// the save is mounted so cheats can list the files they apply to
	if (!orbis_SaveMount(save, SAVE_FLAG_LOCKED, mount))
	{
		free(buffer);
		list_append(save->codes, _createCmdCode(PATCH_NULL, CHAR_ICON_WARN " ", _("Error Mounting Save!"), CMD_CODE_NULL));
		return list_count(save->codes);
	}
	tmp = save->path;
	asprintf(&save->path, APOLLO_SANDBOX_PATH, mount);

	code = _createCmdCode(PATCH_NULL, "", "", CMD_CODE_NULL);
	free(code->name);
	asprintf(&code->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("Cheats"));
	list_append(save->codes, code);

	node = list_tail(save->codes);
	LOG("Loading BSD codes '%s'...", filePath);
	apollo_load_code_list(buffer, save->codes, &get_file_entries, save->path);
	free(buffer);

	// Disable codes meant for other saves of the same game
	for (node = list_next(node); (code = list_get(node)); node = list_next(node))
		if (strchr(code->file, '\\') != NULL && code->file[0] != '~')
		{
			buffer = strdup(code->file);
			strchr(buffer, '\\')[0] = 0;
			if(!wildcard_match_icase(save->dir_name, buffer))
			{
				LOG("(%s) Disabled code '%s'", buffer, code->name);
				code->flags |= (APOLLO_CODE_FLAG_ALERT | APOLLO_CODE_FLAG_DISABLED);
			}
			free(buffer);
		}

	LOG("Loaded %zu codes", list_count(save->codes));

	orbis_SaveUmount(mount);
	free(save->path);
	save->path = tmp;

	return list_count(save->codes);
}

/*
 * Menu lists
 */
// USB Saves: the encrypted saves on the USB drive (selecting one opens its cheats)
list_t * ReadUsbList(void)
{
	char usb_path[256];

	update_usb_path(usb_path);
	if (!usb_path[0])
		return list_alloc();

	return ReadUsbSaves(usb_path);
}

list_t * ReadUserToolsList(void)
{
	char tmp[128];
	char userName[0x20];
	uint64_t account;
	save_entry_t *item;
	list_t *list = list_alloc();

	LOG("Getting Users...");
	for (int i = 1; i <= 16; i++)
	{
		regMgr_GetUserName(i, userName);
		if (!userName[0])
			continue;

		account = 0;
		regMgr_GetAccountId(i, &account);
		snprintf(tmp, sizeof(tmp), "%c %s %s (%016lx)", account ? CHAR_TAG_LOCKED : CHAR_TAG_OWNER, _("Activate Offline Account"), userName, account);

		// users that already have an Account ID can't be changed
		item = _createSaveEntry(0, "", tmp);
		item->type = FILE_TYPE_MENU;
		item->cmd = account ? CMD_CODE_NULL : CMD_ACTIVATE_ACCOUNT;
		item->arg = i;
		list_append(list, item);

		LOG("ID %d = '%s' (%lx)", i, userName, account);
	}

	return list;
}

void UnloadGameList(list_t * list)
{
	list_node_t *node;
	save_entry_t *item;

	for (node = list_head(list); (item = list_get(node)); node = list_next(node))
	{
		free(item->name);
		free(item->path);
		free(item->dir_name);
		free(item->title_id);

		if (item->codes)
			apollo_free_code_list(item->codes, list_head(item->codes));

		free(item);
	}

	list_free(list);
}

/*
 * Encrypted saves in <userPath>/<TITLE_ID>/<save> + <save>.bin
 */
static void read_usb_encrypted_saves(const char* userPath, list_t *list)
{
	DIR *d, *d2;
	struct dirent *dir, *dir2;
	save_entry_t *item, tmp;
	char savePath[256];
	char mount[ORBIS_SAVE_DATA_DIRNAME_DATA_MAXSIZE];

	d = opendir(userPath);

	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_DIR || dir->d_name[0] == '.')
			continue;

		snprintf(savePath, sizeof(savePath), "%s%s", userPath, dir->d_name);
		d2 = opendir(savePath);

		if (!d2)
			continue;

		LOG("Reading %s...", savePath);

		while ((dir2 = readdir(d2)) != NULL)
		{
			// skip sealed keys and hidden files (macOS "._*" resource forks)
			if (dir2->d_type != DT_REG || dir2->d_name[0] == '.' || endsWith(dir2->d_name, ".bin"))
				continue;

			snprintf(savePath, sizeof(savePath), "%s%s/", userPath, dir->d_name);
			tmp.path = savePath;
			tmp.title_id = dir->d_name;
			tmp.dir_name = dir2->d_name;
			if (!orbis_SaveMount(&tmp, SAVE_FLAG_LOCKED, mount))
			{
				// unreadable save: listed so it's counted as an error
				snprintf(savePath, sizeof(savePath), "%s/%s", dir->d_name, dir2->d_name);
				item = _createSaveEntry(SAVE_FLAG_PS4 | SAVE_FLAG_LOCKED, CHAR_ICON_WARN " ", savePath);
				item->type = FILE_TYPE_NULL;
				item->dir_name = strdup(dir2->d_name);
				asprintf(&item->path, "%s%s/", userPath, dir->d_name);
				asprintf(&item->title_id, "%.9s", dir->d_name);
				list_append(list, item);
				continue;
			}

			snprintf(savePath, sizeof(savePath), APOLLO_SANDBOX_PATH "sce_sys/param.sfo", mount);
			LOG("Reading %s...", savePath);

			sfo_context_t* sfo = sfo_alloc();
			if (sfo_read(sfo, savePath) < 0) {
				LOG("Unable to read from '%s'", savePath);
				sfo_free(sfo);
				orbis_SaveUmount(mount);
				continue;
			}

			snprintf(savePath, sizeof(savePath), "%s - %s", sfo_get_param_value(sfo, "MAINTITLE"), sfo_get_param_value(sfo, "SUBTITLE"));
			item = _createSaveEntry(SAVE_FLAG_PS4 | SAVE_FLAG_LOCKED, "", savePath);
			item->type = FILE_TYPE_PS4;
			item->dir_name = strdup(dir2->d_name);
			asprintf(&item->path, "%s%s/", userPath, dir->d_name);
			asprintf(&item->title_id, "%.9s", dir->d_name);

			uint64_t* int_data = (uint64_t*) sfo_get_param_value(sfo, "ACCOUNT_ID");
			if (int_data && (apollo_config.account_id == *int_data))
				item->flags |= SAVE_FLAG_OWNER;

			int_data = (uint64_t*) sfo_get_param_value(sfo, "SAVEDATA_BLOCKS");
			item->blocks = (*int_data);

			sfo_free(sfo);
			orbis_SaveUmount(mount);

			LOG("[%s] F(%X) name '%s'", item->title_id, item->flags, item->name);
			list_append(list, item);
		}
		closedir(d2);
	}

	closedir(d);
}

/*
 * Encrypted saves on the USB drive: <usb>/PS4/SAVEDATA/<account-id>/<TITLE_ID>/
 */
list_t * ReadUsbSaves(const char* usb_path)
{
	DIR *d;
	struct dirent *dir;
	char path[256];
	list_t *list = list_alloc();

	snprintf(path, sizeof(path), "%s" ENC_SAVES_PATH_USB, usb_path);
	d = opendir(path);

	if (!d)
		return list;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_DIR || dir->d_name[0] == '.')
			continue;

		snprintf(path, sizeof(path), "%s" ENC_SAVES_PATH_USB "%s/", usb_path, dir->d_name);
		read_usb_encrypted_saves(path, list);
	}

	closedir(d);
	return list;
}
