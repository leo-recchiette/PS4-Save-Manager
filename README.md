# Lucente Save Tool (PS4)

**Lucente Save Tool** is a PlayStation 4 homebrew app for encrypted save games on a USB drive: it resigns whole save packs for your account, copies them to the console, and applies cheats to them.

It installs with its own Title ID (`LUCE00001`) and data folder (`/data/lucente/`), so it can sit next to other save tools.

# Features

* **Bulk resign of a save folder:** reads every sub-folder of `PS4/IMPORT/` on the USB drive, resigns each encrypted save for the current account, and copies it to `PS4/SAVEDATA/<account-id>/` keeping the original folder structure.
* **Copy all USB saves to the HDD:** every encrypted save in `PS4/SAVEDATA/<account-id>/<TITLE_ID>/` is copied to the console and resigned for the current user.
* **Cheats on USB saves:** select an encrypted save in **USB Saves**, pick the cheats from its save patch, and apply them; the save is also resigned for the current user.
* **Offline account activation:** give an Account ID to a PS4 user that never signed in to PSN.
* **Localization support:** Chinese, English, French, German, Greek, Hungarian, Italian, Japanese, Korean, Portuguese, Russian, Spanish.

Copies work on encrypted saves only, and always on all the saves found: to copy a single save, leave only that one in the source folder.

# Bulk resign

An encrypted PS4 save is a pair of files: the save image and its `.bin` key file (96 bytes), inside a folder named after the game's Title ID.

1. Copy your save pack to `PS4/IMPORT/<GAME NAME>/` on the USB drive. Any folder depth works; archives must be extracted first.
2. Select **Bulk Resign** on the main screen.
3. Confirm. The resigned copies are written to `PS4/SAVEDATA/<account-id>/`, with the same sub-folders:

```
PS4/IMPORT/<GAME NAME>/#3. Hats Off!/CUSA09171/SAVESERVICE-LINE-0-3Slot
PS4/IMPORT/<GAME NAME>/#3. Hats Off!/CUSA09171/SAVESERVICE-LINE-0-3Slot.bin
        ↓
PS4/SAVEDATA/<account-id>/<GAME NAME>/#3. Hats Off!/CUSA09171/SAVESERVICE-LINE-0-3Slot
PS4/SAVEDATA/<account-id>/<GAME NAME>/#3. Hats Off!/CUSA09171/SAVESERVICE-LINE-0-3Slot.bin
```

The files in `PS4/IMPORT/` are not modified.

> [!IMPORTANT]
> The PS4 (and the **Copy to HDD** command) only reads saves from `PS4/SAVEDATA/<account-id>/<TITLE_ID>/`. To use a save, move its Title ID folder (e.g. `CUSA09171`) directly under `PS4/SAVEDATA/<account-id>/`.

The current user needs an Account ID. If the account is not activated, use **Tools** first.

# Cheats

Cheats come from save patch files (Save Wizard / Bruteforce Save Data format), one per game: `/data/lucente/data/<TITLE_ID>.savepatch`. The pkg bundles the [Apollo patch pack](https://github.com/bucanero/apollo-patches) and unpacks it on the first run of each version; a file with the same name placed in that folder by hand replaces the bundled one.

1. Put the save in `PS4/SAVEDATA/<account-id>/<TITLE_ID>/` on the USB drive.
2. Open **USB Saves** and select the save.
3. Select the cheats; the ones the game always needs (decrypt, checksum) are selected automatically.
4. Select **Apply Changes & Resign**. The save on the USB drive is modified in place: keep a copy of the original.

# Setup instructions

Download the `.pkg` from the [latest release][app_latest], or from a build in the **[Actions](../../actions)** tab, and install it on your PlayStation 4.
On first run, the application will detect and setup the required user settings.

## Data folders

| PS4 | Folder |
|-----|--------|
| **Save packs to resign** | `/mnt/usbX/PS4/IMPORT/` |
| **Encrypted saves** | `/mnt/usbX/PS4/SAVEDATA/<account-id>/<TITLE_ID>/` |
| **External saves (HDD)** | `/data/fakeusb/` can be used in place of a USB drive, with the same folders. |

# Usage

 - On the main screen, move <kbd>LEFT</kbd>/<kbd>RIGHT</kbd> and press ![X button](https://github.com/bucanero/pkgi-ps3/raw/master/data/CROSS.png) to pick **USB Saves**, **Bulk Resign**, **Copy to HDD** or **Tools**; press <kbd>UP</kbd> to reach **Settings** and **About**.
   **Bulk Resign** and **Copy to HDD** start right away, and ask for confirmation before writing anything.
 - In a list, move <kbd>UP</kbd>/<kbd>DOWN</kbd> to select a save or a tool, and press ![X button](https://github.com/bucanero/pkgi-ps3/raw/master/data/CROSS.png) to open the save's cheats or to run the tool.
 - Press ![O button](https://github.com/bucanero/pkgi-ps3/raw/master/data/CIRCLE.png) to go back.

# Credits

Lucente Save Tool is based on [Apollo Save Tool](https://github.com/bucanero/apollo-ps4) by [Bucanero](https://github.com/bucanero).

## Acknowledgments

* [Dnawrkshp](https://github.com/Dnawrkshp/): [Artemis PS3](https://github.com/Dnawrkshp/ArtemisPS3)
* [hzh](https://github.com/hzhreal)/[Team-Alua](https://github.com/Team-Alua/cecie.nim): [vsh-utils](https://github.com/hzhreal/PS4-vsh-utils)
* [flatz](https://github.com/flatz): [SFO tools](https://github.com/bucanero/pfd_sfo_tools/)
* [aldostools](https://aldostools.org/): [Bruteforce Save Data](https://bruteforcesavedata.forumms.net/)
* [Manrope](https://github.com/sharanda/manrope) font (OFL) and [Material Symbols](https://fonts.google.com/icons) (Apache 2.0)

## Translators

Translations come from the Apollo Save Tool project: [Akela](https://x.com/Akela_1979) (Russian), Algol (French), [Bucanero](https://github.com/bucanero) (Spanish), [TheheroGAC](https://x.com/TheheroGAC) (Italian), [yyoossk](https://x.com/Cloud0835) (Japanese), [Phoenixx1202](https://github.com/Phoenixx1202) (Portuguese), [Gabor Sari](https://github.com/oldman63) (Hungarian), SpyroMancer (Greek), [SoftwareRat](https://github.com/SoftwareRat) (German), [孙中磊 Sun Zhonglei](https://github.com/1987Lr1987) (Chinese)

# Building

You need to have installed:

- [Open Orbis SDK](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain/)
- [Apollo](https://github.com/bucanero/apollo-lib) library (save patch engine)
- [mbedTLS](https://github.com/Mbed-TLS/mbedtls/releases/tag/v2.16.12) library
- [cURL](https://github.com/bucanero/oosdk_libraries/tree/master/curl-7.64.1) library
- [libZip](https://github.com/bucanero/oosdk_libraries/tree/master/libzip-1.9.2) library
- [SDL2](https://github.com/PacBrew/SDL/tree/ps4) library
- [libJbc](https://github.com/bucanero/ps4-libjbc) library
- [libSQLite](https://github.com/bucanero/libSQLite-ps4) library
- [mini18n](https://github.com/bucanero/mini18n) library
- [dbglogger](https://github.com/bucanero/dbglogger) library

`.github/workflows/build.yml` builds and installs all of them step by step.

Run `make` to create a release build. Run `make createzip` first to bundle the latest save patches (cheats) in the `.pkg`.

You can also set the `PS3LOAD` environment variable to your PS4 IP address: `export PS3LOAD=tcp:x.x.x.x`.
This will allow you to use a [ps3load client](https://github.com/bucanero/ps4load/tree/main/client) and send the `eboot.bin` directly to the [PS4Load listener](https://github.com/bucanero/ps4load).

To enable debug logging, build Lucente Save Tool with `make DEBUGLOG=1`. The application will send debug messages to
UDP multicast address `239.255.0.100:30000`. To receive them you can use [socat][] on your computer:

```sh
$ socat udp4-recv:30000,ip-add-membership=239.255.0.100:0.0.0.0 -
```

# License

Lucente Save Tool is based on [Apollo Save Tool](https://github.com/bucanero/apollo-ps4/) (PS4) - Copyright (C) 2020-2026 [Damian Parrino](https://twitter.com/dparrino)

This program is free software: you can redistribute it and/or modify
it under the terms of the [GNU General Public License][app_license] as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

[socat]: http://www.dest-unreach.org/socat/
[app_latest]: https://github.com/leo-recchiette/lucente-save-tool-ps4/releases/latest
[app_license]: LICENSE
