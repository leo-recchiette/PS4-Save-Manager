# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Apollo Save Tool for PS4: a homebrew C app (OpenOrbis SDK + SDL2) to manage save games on a jailbroken PS4: decrypt/resign/patch PS4 saves, trophies, offline account activation, and PS1/PS2 VMC and Dreamcast VMU memory-card images. Technical wiki: https://deepwiki.com/bucanero/apollo-ps4/

## Build

There are no tests and no linter. A successful build is the only automated check (CI: `.github/workflows/build.yml`).

```sh
make              # build eboot.bin -> pkg.gp4 -> IV0000-APOL00004_00-APOLLO0000000PS4.pkg
make DEBUGLOG=1   # defines APOLLO_ENABLE_LOGGING (see below)
make createzip    # download latest patch pack to assets/misc/appdata.zip (bundled into the pkg)
make clean
```

- Requires `OO_PS4_TOOLCHAIN` pointing at the OpenOrbis toolchain, with every library in the Makefile `LIBS` line installed into it (apollo-lib, dbglogger, mbedTLS 2.16.12, curl, libzip, SDL2 (PS4 port), libjbc, libs3m, SQLite, mxml, mini18n, libunrar, libun7zip). `build.yml` is the authoritative step-by-step recipe for building and installing them (CI uses LLVM 12).
- On macOS the Makefile hardcodes clang/ld.lld at `/usr/local/opt/llvm/bin/` (Intel Homebrew path); Linux uses `clang`/`ld.lld` from PATH.
- All `source/*.c` files are compiled automatically (wildcard), with `-Iinclude`; objects go to `build/x64/Debug/`.
- `sce_sys/param.sfo` is regenerated from Makefile metadata (`TITLE`, `VERSION`, `TITLE_ID`, `CONTENT_ID`).

### Debug builds

`DEBUGLOG=1` changes runtime paths (`include/saves.h`): assets are loaded from `/data/apollo/debug/` instead of the app sandbox, and the splash/intro animations are skipped. The intent is running `eboot.bin` through a PS4Load listener (`export PS3LOAD=tcp:<ps4-ip>`). `LOG()` (`dbglogger_log`) output goes to UDP multicast `239.255.0.100:30000`:

```sh
socat udp4-recv:30000,ip-add-membership=239.255.0.100:0.0.0.0 -
```

In release builds, logging only happens if the user enables the debug-log setting (written to `/data/apollo/apollo.log`).

## Architecture

### Main loop and menus
`source/main.c` initializes SDL, system modules, the jailbreak (`orbis_jbc.c`; the app exits if it fails), localization, and settings, then runs a single loop: `orbisPadUpdate()` → `drawScene()` → `SDL_RenderPresent()`. `drawScene()` in `menu_main.c` dispatches on the global `menu_id` (`enum menu_screen_ids` in `include/menu.h`) to a `do*Menu()` function, which handles input and calls a matching `Draw_*` function in `menu_*.c`. Menu navigation uses `SetMenu()` plus the globals `menu_sel`, `menu_old_sel[]`, `last_menu_id[]`, `selected_entry`, and `selected_centry`.

### Save lists → entries → codes
- Each browsable source (HDD, USB, trophies, online DB, user tools, PS1 VMC, PS2 VMC, DC VMU) is a `save_list_t` defined in `main.c`. It holds three callbacks: `UpdatePath`, `ReadList` (builds a `list_t` of `save_entry_t`), and `ReadCodes` (builds the actions for one entry). All of them share the generic `doSaveMenu()`.
- The `Read*List` / `Read*Codes` implementations live in `source/saves.c`. An entry's `codes` list holds `code_entry_t` items (type from apollo-lib) of two kinds:
  - **Commands** (`PATCH_COMMAND`), built with `_createCmdCode()`. `code->codes[0]` is a `cmd_code_enum` value (`include/saves.h`), and an optional `codes[1]` byte is an argument, typically the USB device index. Options attached with `_createOptions()` produce sub-choices.
  - **Cheats** parsed by apollo-lib (`apollo_load_code_list`) from `/data/apollo/data/<TITLE_ID>.savepatch` (Save Wizard / BSD format), plus SFO patches and trophy lock/unlock.
- Activating a command calls `execCodeCommand()` in `source/exec_cmd.c`, a big `switch` on `codecmd[0]` that calls static helpers in the same file. Cheats are applied through `apply_cheat_patches()` → `apollo_apply_code()`, with `orbis_host_callback` supplying host values such as account ID and PSID.
- **Adding a new action** usually means: a new enum value in `cmd_code_enum`, a `_createCmdCode(...)` entry in the relevant `Read*Codes` in `saves.c`, a `case` in `execCodeCommand`, and new translatable strings (see Localization).

### Save mounting
HDD saves and encrypted (`SAVE_FLAG_LOCKED`) USB saves must be mounted with `orbis_SaveMount()` before access. Code then temporarily replaces `save->path` with `APOLLO_SANDBOX_PATH` (`/data/apollo/mount/<mount>/`), and must restore the path and call `orbis_SaveUmount()` afterwards. `ReadCodes` and `execCodeCommand` both follow this pattern. All on-console paths (USB, fake USB `/data/fakeusb/`, HDD, DB locations) are `#define`s in `include/saves.h`.

### Memory-card images
- PS1: `ps1card.c` (port of MemcardRex).
- PS2: `mcio.c` (port of ps3mca-tool; handles ECC and non-ECC images).
- Dreamcast: `dccard.c` (card image), `dcvmu.c` (VMU filesystem), `dcsave.c` (DCI/VMI/VMS formats).

Each module keeps **one card open at a time in static globals**, so a VMC must be opened before its saves are listed or modified. Patching a save inside a VMC dumps the file to a temp path, applies the patch, then injects the file back (`_vmc_dump_file` / `_vmc_inject_file` in `exec_cmd.c`; `vmc_read_save_file` / `vmc_write_save_file` in `saves.c`).

Related format code:
- `psv_ps2.c`: MAX/CBS/XPS/PSU → PSV conversion.
- `psv_resign.c`: PSV/VMP signing, plus `psv_verify()`.
- `ps2icon.c` / `ps2render.c`: PS2 3D icon rendering.

### System databases
`sqlite_db.c` edits PS4 system SQLite DBs (`app.db`, `savedata.db`, trophy DB). Each DB is loaded into memory through the custom `memvfs.c` VFS, modified, then written back with `sqlite3_memvfs_dump()`.

### External libraries
The patch engine, the code/list types (`code_entry_t`, `list_t`, `option_entry_t`), and the format parsing for patch files live in **apollo-lib** (`#include <apollo.h>`), a separate repo. Changes to the patch language or code parsing belong there, not here.

## Localization

- User-facing strings are wrapped in `_("...")`, a mini18n runtime lookup.
- Static initializers (e.g. `menu_options[]` in `settings.c`) use `_i18n("...")`, a no-op marker for extraction. Those strings are translated later through `mini18n()` in `initMenuOptions()`.
- The template is `docs/translate.po` (Poedit, keywords `_;_i18n`, base path `source/`). Translations are `assets/misc/lang_<code>.po` (12 languages). At runtime, `/data/apollo/data/lang_<code>.po` overrides the bundled file.
- When adding or changing strings, update `docs/translate.po` **and** every `assets/misc/lang_*.po`, as past commits do.

## Releases

To bump the version, update all three of:
- `VERSION` in `Makefile` (format `02.40`)
- `APOLLO_VERSION` in `include/settings.h` (format `"2.4.0"`)
- `CHANGELOG.md`

A change to `APOLLO_VERSION` makes the app re-extract the bundled `appdata.zip` into `/data/apollo/data/` on first launch.
