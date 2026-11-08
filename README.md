# Kingdom Under Fire: Heroes – Battle for Bersia MP

Online multiplayer for the Steam PC version of **Kingdom Under Fire: Heroes**. The mod restores the game's Xbox Live
modes over Steam and adds Versus, Hero Battle, Invasion and Co-op campaign play for up to 6 players, plus new units,
fixes and balance options.

This repository holds the complete source of the mod and of its installers. **It contains no game files.**
To build it you need your own copy of the game (see [BUILD.md](BUILD.md)).

* Release, install instructions and feature list: Nexus Mods page.
* How to build and verify a release: [BUILD.md](BUILD.md).

## How the mod works

The mod is a set of binary patches plus three new code sections added to the 32-bit `Heroes.exe`:

| Section | Contents | Built from |
|---|---|---|
| `.kuf` (0xAE1000) | x86 hooks that redirect game functions into the new code (`newcode/kuf.asm`). From 0xAF1000 it holds the networking, lockstep multiplayer, game modes, co-op, custom jobs and fixes, written in C. | `scripts/make_kuf.py` (nasm), `net/net.c` + `net/eq.c` (clang, freestanding i686, no C runtime, linked at fixed addresses by `net/link.ld`) |
| `.kfx` (0xB01000) | KeatonKuF's Community Patch fixes | `out/keaton_fixes.py` (made by `kport/`) |
| `.kjb` (0xB13000) | Data: job icons and the C code's data, plus overflow code | `scripts/jobs_gen.py`, `net/` |

`out/patch_heroes_multi_lobby.py` applies every patch. Each patch first checks that the original bytes are still there
and refuses to run on any other exe. The script then appends the new sections. `out/keaton_fixes.py` holds
KeatonKuF's Community Patch fixes, which `kport/` moved from her DLL into the exe; they are used with her permission.

At run time the mod uses only the game's own imports and the `steam_api.dll` the game already ships with.
Multiplayer goes through Steam lobbies and Steam peer-to-peer networking. Nothing is downloaded.

Files the patched game writes:
* `KUF_netlog.txt` in the game folder: a multiplayer log for bug reports.
* `MultiplayerArmies.sav` and `CustomArmies.sav`: the multiplayer armies.
* `%TEMP%\kufmp_*`: copies of game data tables with the mod's extra rows.
* `Data\Mission\Live\9_Plains.*`: the flat Invasion map, generated from the game's own Ice Storm files.

## Installers

The release ships two Windows batch files instead of a modified exe. The complete PowerShell installer is plain text in
each .bat, between the lines `#PSSTART` and `#PAYLOAD`. After `#PAYLOAD` comes base64 gzip data made only of
"copy bytes N..M from your own file" and "insert these bytes" instructions. **`tools/inspect_installer.py` decodes it.**

* `BattleForBersiaMP_v1.4_Install.bat` (made by `scripts/exe_patcher.py`):
  * Checks that `Heroes.exe` is the Steam original (SHA-256 `8b2702c1…aa1c`).
  * Keeps the original as `Heroes_original.exe`.
  * Rebuilds `Heroes.exe` as Battle for Bersia MP and checks the result's SHA-256.
  * `uninstall` restores the original.
* `BattleForBersiaMP_Crusaders_Install.bat` (made by `scripts/crus_installer.py` and `scripts/crus_bat.py`). This one is optional:
  * Builds the Crusaders co-op missions from the player's own *Kingdom Under Fire: The Crusaders* install.
  * Copies 362 files and converts 54 from deltas, checking every file's SHA-256.

Both installers write only inside the Heroes folder they are run from and only *read* the registry (to find Steam).

## Layout

| Path | |
|---|---|
| `net/` | C source and linker script |
| `scripts/` | build scripts and installer generators |
| `out/` | exe patcher and Keaton's fixes table |
| `jobs/` | custom job and ability icons and the scripts that draw them |
| `kport/` | the tool that moved Keaton's DLL fixes into the exe (needs her DLL, not included) |
| `crusconv/` | converter for the Crusaders mission and music files |
| `tools/` | `inspect_installer.py` |

## Credits

Battle for Bersia MP – DJ Sonny B. Community Patch fixes – KeatonKuF (used with permission).
Kingdom Under Fire: Heroes and The Crusaders belong to their respective owners. This is a fan-made, non-commercial mod.
