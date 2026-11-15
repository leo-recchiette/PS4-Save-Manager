# Changelog

All notable changes to Lucente Save Tool will be documented in this file. This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

The history of the original project is in the [Apollo Save Tool changelog](https://github.com/bucanero/apollo-ps4/blob/main/CHANGELOG.md).

## [Unreleased]()

### Added

* Bulk resign of every encrypted save in `PS4/IMPORT/`, copied to `PS4/SAVEDATA/<account-id>/` with the original folder structure.
* Copy all encrypted USB saves to the HDD (resigned for the current user).
* Cheats on encrypted USB saves, applied in place with **Apply Changes & Resign**.

### Changed

* New main menu: dark theme with four tiles (USB Saves, Bulk Resign, Copy to HDD, Tools) and Settings/About buttons in the header. The bulk commands start from the main menu.

### Removed

* Everything that is not bulk save copy/resign or offline account activation: per-save actions other than cheats, HDD saves, decrypted saves, trophies, online database, FTP upload and web server, PS1/PS2/Dreamcast memory cards, App.db tools, archive extraction.
* Background music.
* Settings for save sorting, online database, FTP server and save-patch updates (patches now update with the pkg).
