# Lucente Save Tool (PS4)

**Lucente Save Tool** is a fork of [Apollo Save Tool](https://github.com/bucanero/apollo-ps4) for the PlayStation 4, trimmed down to one job: resigning save-game packs in bulk.

It installs next to the original Apollo Save Tool (different Title ID, `LUCE00001`, and its own data folder, `/data/lucente/`).

# Features

* **Bulk resign of a save folder:** reads every sub-folder of `PS4/IMPORT/` on the USB drive, resigns each encrypted save for the current account, and copies it to `PS4/SAVEDATA/<account-id>/` keeping the original folder structure.
* **Save management:** list, resign, patch (Save Wizard / Bruteforce Save Data cheats), import, and export PS4 saves on USB and HDD.
* **Account activation:** create fake Account IDs and generate offline PS4 activations.
* **Archive extraction:** RAR, Zip, and 7z.
* **Localization support:** Chinese, English, French, German, Greek, Hungarian, Italian, Japanese, Korean, Portuguese, Russian, Spanish.

Removed from the original app: trophies, the online save database, PS1/PS2/Dreamcast memory cards, App.db management, and network tools.

# Bulk resign

An encrypted PS4 save is a pair of files: the save image and its `.bin` key file (96 bytes), inside a folder named after the game's Title ID.

1. Copy your save pack to `PS4/IMPORT/<GAME NAME>/` on the USB drive. Any folder depth works; archives must be extracted first.
2. Open **USB Saves** → **Bulk Save Management** → **Resign save folder (PS4/IMPORT) to PS4/SAVEDATA**.
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
> The PS4 only reads saves from `PS4/SAVEDATA/<account-id>/<TITLE_ID>/`. To use a save, move its Title ID folder (e.g. `CUSA09171`) directly under `PS4/SAVEDATA/<account-id>/`.

The current user needs an Account ID. If the account is not activated, go to **User Tools** → **Activate PS4 Accounts** first.

# Setup instructions

Download the `.pkg` from the [latest release][app_latest], or from a build in the **[Actions](../../actions)** tab, and install it on your PlayStation 4.
On first run, the application will detect and setup the required user settings.

## Data folders

| PS4 | Folder |
|-----|--------|
| **Save packs to resign** | `/mnt/usbX/PS4/IMPORT/` |
| **USB saves** | your decrypted saves must be placed on `/mnt/usbX/PS4/APOLLO/`. |
| **USB saves** | encrypted saves must be placed on `/mnt/usbX/PS4/SAVEDATA/<account-id>/`. |
| **External saves (HDD)** | `/data/fakeusb/` can be used in place of a USB drive, with the same folders. |
| **HDD saves** | files will be scanned from the hard disk, based on the current `User ID`. |

# Usage

 - Move <kbd>UP</kbd>/<kbd>DOWN</kbd> to select a save-game file, and press ![X button](https://github.com/bucanero/pkgi-ps3/raw/master/data/CROSS.png) to see the available actions.
 - To view the item's details, press ![Triangle](https://github.com/bucanero/pkgi-ps3/raw/master/data/TRIANGLE.png). Press ![O button](https://github.com/bucanero/pkgi-ps3/raw/master/data/CIRCLE.png) to return to the list.
 - To reload the list, press ![Square](https://github.com/bucanero/pkgi-ps3/raw/master/data/SQUARE.png).
 - Press <kbd>L1</kbd>/<kbd>L2</kbd> or <kbd>R1</kbd>/<kbd>R2</kbd> trigger buttons to move pages up or down.

# Credits

* [Bucanero](https://github.com/bucanero): developer of the original [Apollo Save Tool](https://github.com/bucanero/apollo-ps4)

## Acknowledgments

* [Dnawrkshp](https://github.com/Dnawrkshp/): [Artemis PS3](https://github.com/Dnawrkshp/ArtemisPS3)
* [hzh](https://github.com/hzhreal)/[Team-Alua](https://github.com/Team-Alua/cecie.nim): [vsh-utils](https://github.com/hzhreal/PS4-vsh-utils)
* [Berion](https://www.psx-place.com/members/berion.1431/): GUI design
* [flatz](https://github.com/flatz): [SFO tools](https://github.com/bucanero/pfd_sfo_tools/)
* [aldostools](https://aldostools.org/): [Bruteforce Save Data](https://bruteforcesavedata.forumms.net/)
* [jimmikaelkael](https://github.com/jimmikaelkael): ps3mca tool
* [ShendoXT](https://github.com/ShendoXT): [MemcardRex](https://github.com/ShendoXT/memcardrex)
* [Nobody/Wild Light](https://github.com/nobodo): [Background music track](https://github.com/bucanero/apollo-vita/blob/main/data/haiku.s3m)

## Translators

Apollo supports multiple languages for its user interface. Thanks to the following contributors for their help with translations:

- [Akela](https://x.com/Akela_1979) (Russian), Algol (French), [Bucanero](https://github.com/bucanero) (Spanish), [TheheroGAC](https://x.com/TheheroGAC) (Italian), [yyoossk](https://x.com/Cloud0835) (Japanese), [Phoenixx1202](https://github.com/Phoenixx1202) (Portuguese), [Gabor Sari](https://github.com/oldman63) (Hungarian), SpyroMancer (Greek), [SoftwareRat](https://github.com/SoftwareRat) (German), [孙中磊 Sun Zhonglei](https://github.com/1987Lr1987) (Chinese)

# Building

You need to have installed:

- [Open Orbis SDK](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain/)
- [Apollo](https://github.com/bucanero/apollo-lib) library
- [mbedTLS](https://github.com/Mbed-TLS/mbedtls/releases/tag/v2.16.12) library
- [cURL](https://github.com/bucanero/oosdk_libraries/tree/master/curl-7.64.1) library
- [libZip](https://github.com/bucanero/oosdk_libraries/tree/master/libzip-1.9.2) library
- [SDL2](https://github.com/PacBrew/SDL/tree/ps4) library
- [libJbc](https://github.com/bucanero/ps4-libjbc) library
- [libs3m](https://github.com/bucanero/s3mplay) library
- [libSQLite](https://github.com/bucanero/libSQLite-ps4) library
- [Mini-XML](https://github.com/bucanero/mxml) library
- [mini18n](https://github.com/bucanero/mini18n) library
- [dbglogger](https://github.com/bucanero/dbglogger) library

> [!TIP]
> **Developers:** You can find detailed technical documentation about the project in [this Wiki page](https://deepwiki.com/bucanero/apollo-ps4/) (original project).

Run `make` to create a release build. If you want to include the latest save patches in your `.pkg` file, run `make createzip`.

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
