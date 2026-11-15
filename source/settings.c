#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <orbis/libkernel.h>
#include <orbis/SaveData.h>
#include <orbis/UserService.h>
#include <mini18n.h>

#include "types.h"
#include "menu.h"
#include "saves.h"
#include "common.h"

#define _i18n(str) (str)
#define ORBIS_USER_SERVICE_USER_ID_INVALID	-1

static const char * usb_src[] = {"USB 0", "USB 1", "USB 2", "USB 3", "USB 4", "USB 5", "USB 6", "USB 7", _i18n("Fake USB"), _i18n("Auto-detect"), NULL};

static void usb_callback(int sel);
static void log_callback(int sel);
static void ani_callback(int sel);
static void clearcache_callback(int sel);

menu_option_t menu_options[] = {
	{ .name = _i18n("Background Music"), 
		.options = NULL, 
		.type = APP_OPTION_BOOL, 
		.value = &apollo_config.music, 
		.callback = music_callback 
	},
	{ .name = _i18n("Menu Animations"),
		.options = NULL, 
		.type = APP_OPTION_BOOL, 
		.value = &apollo_config.doAni, 
		.callback = ani_callback 
	},
	{ .name = _i18n("USB Saves Source"),
		.options = usb_src,
		.type = APP_OPTION_LIST,
		.value = &apollo_config.usb_dev,
		.callback = usb_callback
	},
	{ .name = _i18n("Version Update Check"),
		.options = NULL, 
		.type = APP_OPTION_BOOL, 
		.value = &apollo_config.update, 
		.callback = update_callback 
	},
	{ .name = _i18n("Clear Local Cache"),
		.options = NULL, 
		.type = APP_OPTION_CALL,
		.spacer = 1,
		.value = NULL, 
		.callback = clearcache_callback 
	},
	{ .name = _i18n("Enable Debug Log"),
		.options = NULL,
		.type = APP_OPTION_BOOL,
		.value = &apollo_config.dbglog,
		.callback = log_callback 
	},
	{ .name = NULL }
};


void music_callback(int sel)
{
	apollo_config.music = !sel;
}

static void ani_callback(int sel)
{
	apollo_config.doAni = !sel;
}

static void usb_callback(int sel)
{
	apollo_config.usb_dev = sel;
}

static void clearcache_callback(int sel)
{
	LOG("Cleaning folder '%s'...", APOLLO_LOCAL_CACHE);
	clean_directory(APOLLO_LOCAL_CACHE, "");

	show_message("%s\n%s", _("Local cache folder cleaned:"), APOLLO_LOCAL_CACHE);
}

void update_callback(int sel)
{
    apollo_config.update = !sel;

    if (!apollo_config.update)
        return;

	LOG("checking latest version at %s", APOLLO_UPDATE_URL);

	if (!http_download(APOLLO_UPDATE_URL, NULL, APOLLO_LOCAL_CACHE "ver.check", 0))
	{
		LOG("http request to %s failed", APOLLO_UPDATE_URL);
		return;
	}

	char *buffer = readTextFile(APOLLO_LOCAL_CACHE "ver.check");
	if (!buffer)
		return;

	LOG("received %ld bytes", strlen(buffer));

	static const char find[] = "\"name\":\"Lucente Save Tool v";
	const char* start = strstr(buffer, find);
	if (!start)
	{
		LOG("no name found");
		goto end_update;
	}

	LOG("found name");
	start += sizeof(find) - 1;

	char* end = strchr(start, '"');
	if (!end)
	{
		LOG("no end of name found");
		goto end_update;
	}
	*end = 0;
	LOG("latest version is %s", start);

	if (strcasecmp(APOLLO_VERSION, start) == 0)
	{
		LOG("no need to update");
		goto end_update;
	}

	start = strstr(end+1, "\"browser_download_url\":\"");
	if (!start)
		goto end_update;

	start += 24;
	end = strchr(start, '"');
	if (!end)
	{
		LOG("no download URL found");
		goto end_update;
	}

	*end = 0;
	LOG("download URL is %s", start);

	if (show_dialog(DIALOG_TYPE_YESNO, _("New version available! Download update?")))
	{
		char* pkg_path = (dir_exists("/data/pkg") == SUCCESS) ? "/data/pkg/lucente-ps4.pkg" : "/data/lucente-ps4.pkg";
		if (http_download(start, NULL, pkg_path, 1))
			show_message(_("Update downloaded to %s"), pkg_path);
		else
			show_message(_("Download error!"));
	}

end_update:
	free(buffer);
	return;
}

static void log_callback(int sel)
{
	apollo_config.dbglog = !sel;

	if (!apollo_config.dbglog)
	{
		dbglogger_stop();
		show_message(_("Debug Logging Disabled"));
		return;
	}

	dbglogger_init_mode(FILE_LOGGER, APOLLO_PATH "lucente.log", 0);
	show_message("%s\n\n%s", _("Debug Logging Enabled!"), APOLLO_PATH "lucente.log");
}

static int updateSaveParams(const char* mountPath, const char* title, const char* subtitle, const char* details, uint32_t userParam)
{
	OrbisSaveDataParam saveParams;
	OrbisSaveDataMountPoint mount;

	memset(&saveParams, 0, sizeof(OrbisSaveDataParam));
	memset(&mount, 0, sizeof(OrbisSaveDataMountPoint));

	strlcpy(mount.data, mountPath, sizeof(mount.data));
	strlcpy(saveParams.title, title, ORBIS_SAVE_DATA_TITLE_MAXSIZE);
	strlcpy(saveParams.subtitle, subtitle, ORBIS_SAVE_DATA_SUBTITLE_MAXSIZE);
	strlcpy(saveParams.details, details, ORBIS_SAVE_DATA_DETAIL_MAXSIZE);
	saveParams.userParam = userParam;
	saveParams.mtime = time(NULL);

	int32_t setParamResult = sceSaveDataSetParam(&mount, ORBIS_SAVE_DATA_PARAM_TYPE_ALL, &saveParams, sizeof(OrbisSaveDataParam));
	if (setParamResult < 0) {
		LOG("sceSaveDataSetParam error (%X)", setParamResult);
		return 0;
	}

	return (setParamResult == SUCCESS);
}

int save_app_settings(app_config_t* config)
{
	char filePath[256];
	OrbisSaveDataMount2 mount;
	OrbisSaveDataDirName dirName;
	OrbisSaveDataMountResult mountResult;

	memset(&mount, 0x00, sizeof(mount));
	memset(&mountResult, 0x00, sizeof(mountResult));
	strlcpy(dirName.data, "Settings", sizeof(dirName.data));

	mount.userId = apollo_config.user_id;
	mount.dirName = &dirName;
	mount.blocks = ORBIS_SAVE_DATA_BLOCKS_MIN2;
	mount.mountMode = (ORBIS_SAVE_DATA_MOUNT_MODE_CREATE2 | ORBIS_SAVE_DATA_MOUNT_MODE_RDWR | ORBIS_SAVE_DATA_MOUNT_MODE_COPY_ICON);

	LOG("Lucente Save Tool %s v%s - Patch Engine v%s", APOLLO_PLATFORM, APOLLO_VERSION, APOLLO_LIB_VERSION);
	if (sceSaveDataMount2(&mount, &mountResult) < 0) {
		LOG("sceSaveDataMount2 ERROR");
		return 0;
	}

	LOG("Saving Settings...");
	snprintf(filePath, sizeof(filePath), APOLLO_SETTING_PATH "settings.bin", mountResult.mountPathName);
	write_buffer(filePath, (uint8_t*) config, sizeof(app_config_t));

	updateSaveParams(mountResult.mountPathName, "Lucente Save Tool", _("User Settings"), "github.com/leo-recchiette/lucente-save-tool-ps4", 0);
	if (sceSaveDataUmount((void*)&mountResult.mountPathName) < 0)
	{
		LOG("UMOUNT ERROR");
		return 0;
	}

	return 1;
}

int load_app_settings(app_config_t* config)
{
	char filePath[256];
	app_config_t* file_data;
	size_t file_size;
	OrbisSaveDataMount2 mount;
	OrbisSaveDataDirName dirName;
	OrbisSaveDataMountResult mountResult;

	sceUserServiceGetNpAccountId(config->user_id, &config->account_id);
	sceKernelGetOpenPsIdForSystem(config->psid);

	if (sceSaveDataInitialize3(0) != SUCCESS)
	{
		LOG("Failed to initialize save data library");
		return 0;
	}

	memset(&mount, 0x00, sizeof(mount));
	memset(&mountResult, 0x00, sizeof(mountResult));
	strlcpy(dirName.data, "Settings", sizeof(dirName.data));

	mount.userId = apollo_config.user_id;
	mount.dirName = &dirName;
	mount.blocks = ORBIS_SAVE_DATA_BLOCKS_MIN2;
	mount.mountMode = ORBIS_SAVE_DATA_MOUNT_MODE_RDONLY;

	if (sceSaveDataMount2(&mount, &mountResult) < 0) {
		LOG("sceSaveDataMount2 ERROR");
		return 0;
	}

	LOG("Loading Settings...");
	snprintf(filePath, sizeof(filePath), APOLLO_SETTING_PATH "settings.bin", mountResult.mountPathName);

	if (read_buffer(filePath, (uint8_t**) &file_data, &file_size) == SUCCESS && file_size == sizeof(app_config_t))
	{
		file_data->user_id = config->user_id;
		file_data->account_id = config->account_id;
		file_data->psid[0] = config->psid[0];
		file_data->psid[1] = config->psid[1];
		memcpy(config, file_data, file_size);

		LOG("Settings loaded: UserID (%08x) AccountID (%016lX)", config->user_id, config->account_id);
		LOG("PSID: %016lX%016lX", ES64(config->psid[0]), ES64(config->psid[1]));
		free(file_data);
	}

	if (sceSaveDataUmount((void*)&mountResult.mountPathName) < 0)
	{
		LOG("UMOUNT ERROR");
		return 0;
	}

	return 1;
}
