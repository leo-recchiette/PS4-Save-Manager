#include <apollo.h>
#include <dbglogger.h>
#define LOG dbglogger_log

#define APOLLO_PATH				"/data/ps4-save-signer/"
#define APOLLO_SANDBOX_PATH		"/data/ps4-save-signer/mount/%s/"

#ifdef APOLLO_ENABLE_LOGGING
#define APOLLO_APP_PATH			"/data/ps4-save-signer/debug/"
#define APOLLO_SETTING_PATH		"/mnt/sandbox/LOAD00044_000%s/"
#else
#define APOLLO_APP_PATH			"/mnt/sandbox/PSSS00001_000/app0/assets/"
#define APOLLO_SETTING_PATH		"/mnt/sandbox/PSSS00001_000%s/"
#endif

#define APOLLO_DATA_PATH		APOLLO_PATH "data/"
#define APOLLO_LOCAL_CACHE		APOLLO_PATH "cache/"
#define APOLLO_UPDATE_URL		"https://api.github.com/repos/leo-recchiette/PS4-Save-Manager/releases/latest"

#define MAX_USB_DEVICES         8
#define USB_PATH                "/mnt/usb%d/"
#define FAKE_USB_PATH           "/data/fakeusb/"
#define SAVES_DB_PATH           "/system_data/savedata/%08x/db/user/savedata.db"

#define IMPORT_PATH_USB         "PS4/IMPORT/"
#define ENC_SAVES_PATH_USB      "PS4/SAVEDATA/"
#define IMPORT_TMP_PATH         APOLLO_PATH "tmp/import/"

#define SAVES_PATH_HDD          "/user/home/%08x/savedata/"
#define SAVE_ICON_PATH_HDD      "/user/home/%08x/savedata_meta/user/"


enum cmd_code_enum
{
    CMD_CODE_NULL,
    CMD_RESIGN_IMPORT_FOLDER,
    CMD_COPY_ALL_SAVES_HDD,
    CMD_ACTIVATE_ACCOUNT,
    CMD_APPLY_CHEATS,
};

// Save flags
#define SAVE_FLAG_OWNER         2
#define SAVE_FLAG_LOCKED        256
#define SAVE_FLAG_PS4           512

enum save_type_enum
{
    FILE_TYPE_NULL,
    FILE_TYPE_MENU,
    FILE_TYPE_PS4,
};

// Special characters drawn as icons (values 10 and 13 are \n and \r)
enum char_flag_enum
{
    CHAR_TAG_NULL,
    CHAR_TAG_APPLY = 6,
    CHAR_TAG_OWNER,
    CHAR_TAG_LOCKED,
    CHAR_RES_LF = 10,
    CHAR_RES_CR = 13,
    CHAR_TAG_WARNING = 15,
    CHAR_BTN_X = 16,
    CHAR_BTN_O = 19,
};

enum code_type_enum
{
    PATCH_NULL,
    PATCH_GAMEGENIE = APOLLO_CODE_GAMEGENIE,
    PATCH_BSD = APOLLO_CODE_BSD,
    PATCH_PYTHON = APOLLO_CODE_PYTHON,
    PATCH_COMMAND,
};

typedef struct save_entry
{
    char * name;
    char * title_id;
    char * path;
    char * dir_name;
    uint32_t blocks;
    uint16_t flags;
    uint16_t type;
    uint8_t cmd;            // menu entries: command to run (cmd_code_enum)
    uint8_t arg;            // menu entries: command argument
    list_t * codes;         // saves: cheats loaded by ReadCodes()
} save_entry_t;

typedef struct
{
    list_t * list;
    const char* title;
    uint8_t id;
    list_t* (*ReadList)(void);
} save_list_t;

// Menu lists
list_t * ReadUsbList(void);
list_t * ReadUserToolsList(void);
void UnloadGameList(list_t * list);
int ReadCodes(save_entry_t * save);

// Save scanning
list_t * ReadUsbSaves(const char* usb_path);
char * readTextFile(const char * path);
void update_usb_path(char *path);

int http_init(void);
void http_end(void);
int http_download(const char* url, const char* filename, const char* local_dst, int show_progress);


int show_dialog(int dialog_type, const char * format, ...);
int osk_dialog_get_text(const char* title, char* text, uint32_t size);
void init_progress_bar(const char* msg);
void update_progress_bar(uint64_t progress, const uint64_t total_size, const char* msg);
void end_progress_bar(void);
#define show_message(...)	show_dialog(DIALOG_TYPE_OK, __VA_ARGS__)

int init_loading_screen(const char* message);
void stop_loading_screen(void);

void execCommand(const save_entry_t* entry);
void applyCheats(save_entry_t* entry);
int extract_zip(const char* zip_file, const char* dest_path);

void* open_sqlite_db(const char* db_path);
int save_sqlite_db(void* db, const char* db_path);

int regMgr_GetUserName(int userNumber, char* outString);
int regMgr_GetAccountId(int userNumber, uint64_t* psnAccountId);
int regMgr_SetAccountId(int userNumber, uint64_t* psnAccountId);

int orbis_SaveUmount(const char* mountPath);
int orbis_SaveMount(const save_entry_t *save, uint32_t mode, char* mountPath);
int orbis_UpdateSaveParams(const save_entry_t* save, const char* title, const char* subtitle, const char* details, uint32_t up);

const char* get_fw_by_pfskey_ver(int key_ver);
