# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

PS4 Save Manager for PS4: a homebrew C app (OpenOrbis SDK + SDL2) for a jailbroken PS4 that works on **encrypted** USB saves: resign every save under `PS4/IMPORT/` into `PS4/SAVEDATA/<account-id>/` (keeping the folder structure), copy all USB saves to the HDD, apply cheats (save patches) to a USB save, and offline account activation. It started as a fork of Apollo Save Tool (bucanero/apollo-ps4) and was cut down to these features.

## Build

There are no tests and no linter. A successful build is the only automated check (CI: `.github/workflows/build.yml`).

```sh
make              # build eboot.bin -> pkg.gp4 -> IV0000-LUCE00001_00-LUCENTE000000PS4.pkg
make createzip    # download the save patch pack to assets/misc/appdata.zip (bundled into the pkg)
make DEBUGLOG=1   # defines APOLLO_ENABLE_LOGGING (see below)
make clean
```

- Requires `OO_PS4_TOOLCHAIN` pointing at the OpenOrbis toolchain, with every library in the Makefile `LIBS` line installed into it (apollo-lib, dbglogger, mbedTLS 2.16.12, curl, libzip, SDL2 (PS4 port), libjbc, SQLite, mini18n). `build.yml` is the authoritative step-by-step recipe for building and installing them (CI uses LLVM 12).
- On macOS the Makefile hardcodes clang/ld.lld at `/usr/local/opt/llvm/bin/` (Intel Homebrew path); Linux uses `clang`/`ld.lld` from PATH.
- All `source/*.c` files are compiled automatically (wildcard), with `-Iinclude`; objects go to `build/x64/Debug/`. Every file under `assets/` is packed into the pkg.
- `sce_sys/param.sfo` is regenerated from Makefile metadata (`TITLE`, `VERSION`, `TITLE_ID`, `CONTENT_ID`).

### Debug builds

`DEBUGLOG=1` changes runtime paths (`include/saves.h`): assets are loaded from `/data/lucente/debug/` instead of the app sandbox, and the splash/intro animations are skipped. The intent is running `eboot.bin` through a PS4Load listener (`export PS3LOAD=tcp:<ps4-ip>`). `LOG()` (`dbglogger_log`) output goes to UDP multicast `239.255.0.100:30000`:

```sh
socat udp4-recv:30000,ip-add-membership=239.255.0.100:0.0.0.0 -
```

In release builds, logging only happens if the user enables the debug-log setting (written to `/data/lucente/lucente.log`).

## Architecture

### Main loop and menus
`source/main.c` initializes SDL, system modules, the jailbreak (`orbis_jbc.c`; the app exits if it fails), localization, and settings, then runs a single loop: `orbisPadUpdate()` → `drawScene()` → `SDL_RenderPresent()`. `drawScene()` in `menu_main.c` dispatches on the global `menu_id` (`enum menu_screen_ids` in `include/menu.h`).

The main screen (`doMainMenu()` in `menu_main.c`, `_draw_MainMenu()` in `draw.c`) is a row of tiles plus the Settings/About header buttons; `menu_sel` is an `enum main_menu_items` value (`include/menu.h`). **USB Saves** and **Tools** open a list, **Bulk Resign** and **Copy to HDD** run their command at once through `execCommand()`.

### Theme and pre-rendered images
- The whole main menu is pre-rendered PNGs (`assets/images/mm_*.png`): header, headline, footer, and each tile in its idle (`mm_tileN_off`, 300 px) and focused state (`mm_tileN_on`, 622 px = the tile at 1.14x plus its shadow). The code only positions, scales and cross-fades them (`drawTile()`), and tints `mm_glow.png` with the focused tile's colour. **Main-menu text is baked into the images, in English**: changing a label, a tile or its order means regenerating the PNGs, and keeping `enum main_menu_items`, the `mm_tile*` texture order and `mm_glow_color[]` in step.
- The images were rendered with headless Chrome from an HTML page that reuses the CSS of the design mock-up (`Lucente Main Menu.html`, variant 4a: Manrope + Material Symbols Rounded, tile hues 85/160/250/300 in `oklch(0.8 0.12 h)`). The other screens use the same palette through regenerated `cat_*`, `tag_*`, `help.png` (panel), `mark_*`, `scroll_*`, `opt_*` images and `APP_BG_COLOR`.
- Run-time text goes through `DrawString()` (`libfont.c`) → `display_ttf_string()` (`ttf_fonts.c`): glyphs are cached at one size and drawn **white, or black when the colour's RGB is 0**, with alpha; grey text is white with a lower alpha. Font slot 0 is Manrope SemiBold (Latin only); the system fonts and Nanum cover the other scripts.

### Lists → saves and tools
- **USB Saves** and **Tools** are `save_list_t` (defined in `main.c`) whose `ReadList` callback (`ReadUsbList`, `ReadUserToolsList` in `saves.c`) returns a `list_t` of `save_entry_t`, drawn by `menu_tools.c`.
- **USB Saves** lists the encrypted saves found by `ReadUsbSaves()` (mounts each `PS4/SAVEDATA/<account>/<TITLE_ID>/<save>` to read its `param.sfo`; unreadable ones get `FILE_TYPE_NULL`). It is rebuilt when entered from the main screen, not when coming back from a save's cheats. The USB root comes from `update_usb_path()` in `main.c` (Settings → USB source, or auto-detect).
- **Tools** entries are commands: `type == FILE_TYPE_MENU`, `cmd` a `cmd_code_enum` value (`include/saves.h`) and `arg` its argument (the user number for account activation); X calls `execCommand()` in `source/exec_cmd.c`, a `switch` on `entry->cmd`.
- **Adding a command** means: a new `cmd_code_enum` value, a `case` in `execCommand`, an entry in `ReadUserToolsList` or a main-menu tile, and new translatable strings (see Localization).

### Cheats
- Selecting a save calls `ReadCodes()` (`saves.c`): it fills `save->codes` with an **Apply Changes & Resign** command (`PATCH_COMMAND`, `codes[0] == CMD_APPLY_CHEATS`) and the cheats parsed by apollo-lib (`apollo_load_code_list`) from `/data/lucente/data/<TITLE_ID>.savepatch`, with the save mounted so cheats aimed at `*` file masks get the matching files as options (`get_file_entries`).
- `MENU_PATCHES` (`doPatchMenu` in `menu_main.c`) toggles `code->activated`; activating a cheat also activates the game's `APOLLO_CODE_FLAG_REQUIRED` codes, and cheats with options go through `MENU_CODE_OPTIONS`.
- `applyCheats()` (`exec_cmd.c`) mounts the save, runs `apollo_apply_code()` on each active code (`orbis_host_callback` supplies account ID, PSID, etc.), patches `param.sfo` for the current user, and unmounts. The USB save is modified in place.
- The patch pack (`assets/misc/appdata.zip`, from `make createzip`) is unpacked into `/data/lucente/data/` by `main.c` whenever `APOLLO_VERSION` differs from the `app_ver` stored in the settings.

### Save mounting
Encrypted saves are mounted with `orbis_SaveMount()` (`saves.c`, on top of `sd.c`) into `APOLLO_SANDBOX_PATH` (`/data/lucente/mount/<save>/`) and must be released with `orbis_SaveUmount()`. `SAVE_FLAG_LOCKED` mounts a save image + `.bin` sealed key from `save->path`; without it the HDD copy (`/user/home/<user>/savedata/<TITLE_ID>/sdimg_<save>`) is used, and `ORBIS_SAVE_DATA_MOUNT_MODE_CREATE2` creates it (plus its `savedata.db` row) if missing. Resigning = patching `sce_sys/param.sfo` with `patch_sfo()` (`sfo.c`). All on-console paths are `#define`s in `include/saves.h`.

### System databases
`sqlite_db.c` opens a system SQLite DB (e.g. `savedata.db`) in memory through the custom `memvfs.c` VFS and writes it back with `save_sqlite_db()`.

### External libraries
The save patch engine, the code/list types (`code_entry_t`, `option_entry_t`, `list_t`) and helpers such as `read_buffer()`/`write_buffer()` come from **apollo-lib** (`#include <apollo.h>`), a separate repo. Changes to the patch language belong there.

## Localization

- User-facing strings are wrapped in `_("...")`, a mini18n runtime lookup.
- Static initializers (e.g. `menu_options[]` in `settings.c`) use `_i18n("...")`, a no-op marker for extraction. Those strings are translated later through `mini18n()` in `initMenuOptions()`.
- The template is `docs/translate.po` (Poedit, keywords `_;_i18n`, base path `source/`). Translations are `assets/misc/lang_<code>.po` (12 languages). At runtime, `/data/lucente/data/lang_<code>.po` overrides the bundled file.
- When adding or changing strings, update `docs/translate.po` **and** every `assets/misc/lang_*.po`.

## Releases

To bump the version, update all three of:
- `VERSION` in `Makefile` (format `02.40`)
- `APOLLO_VERSION` in `include/settings.h` (format `"2.4.0"`)
- `CHANGELOG.md`

A change to `APOLLO_VERSION` makes the app re-extract the bundled `appdata.zip` on first launch.
