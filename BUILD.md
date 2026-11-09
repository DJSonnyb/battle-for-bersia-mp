# Building Battle for Bersia MP

The build is reproducible: following these steps gives **byte-identical** files to the v1.4 release.

| Output | SHA-256 |
|---|---|
| `dist/Heroes_MP_v1.4.exe` | `51df96a69ae1fcb4ee3854cbee341da1c2b5bc1e5d807fcfac4a0df588114888` |
| `Data/Interface/Interface_Misc.txt` (installed) | `f9ca955d87282cb491acc6be80958bc697a939f617fa029fd2bb5d2b7070fab2` |
| `dist/BattleForBersiaMP_v1.4_Install.bat` | `2344c9aa81abbe9ff7a52f10cf9aa0172c96ab7bc17b445e6739b77cca09e2c4` |

## 1. What you need

* **Your own copy of Kingdom Under Fire: Heroes (Steam).** The build reads two things from it:
  * the original `Heroes.exe`, SHA-256 `8b2702c1aac408f1ef687af18e6a9ff7d133ad0368c9c16ed48ac8f7c345aa1c` (file size 7,041,568 bytes). Use Steam > Verify integrity of game files if yours differs.
  * the stage files in `Data/Mission/`. The co-op army levels are read from them.
  * `Data/Interface/Interface_Misc.txt`, the game's menu layout, SHA-256 `e10bb4cff0be2201a4b69d4c39447416220edc7fa314a33bfa610e5febe0ea91`. `ui/Interface_Misc.diff` adds the multiplayer menus to it.
* **Linux with these tools.** The release was built on Ubuntu 24.04 with the versions below. Other versions build a working mod, but the bytes may differ.

| Tool | Ubuntu 24.04 package | Version used |
|---|---|---|
| Python 3 + Pillow | `python3 python3-pil` | Python 3.12/3.13, Pillow 10–12 |
| clang, lld, llvm-objcopy, llvm-readelf | `clang-18 lld-18 llvm-18` (with `clang`, `ld.lld`, `llvm-objcopy`, `llvm-readelf` on PATH) | 18.1.3 |
| nasm | `nasm` | 2.16.01 |
| ImageMagick 6 (`convert`, for the DXT5 icon data) | `imagemagick` | 6.9.12-98 |
| `nm`, `sha256sum`, `patch` | `binutils coreutils patch` | |

Install everything on Ubuntu 24.04 (Windows users: WSL with Ubuntu 24.04 works the same way):

```
sudo apt install python3 python3-pil clang lld llvm nasm imagemagick binutils patch
```

Or use the included `Dockerfile`:

```
docker build -t bfb-build .
docker run --rm -v "$PWD":/src -v "/path/to/KUF Heroes":/game:ro -w /src \
    -e KUF_GAME=/game -e KUF_HEROES_EXE=/game/Heroes.exe bfb-build ./build.sh
```

## 2. Build

```
export KUF_GAME="/path/to/KUF Heroes"              # the install folder (contains Data/Mission)
export KUF_HEROES_EXE="$KUF_GAME/Heroes.exe"        # the original Steam exe
./build.sh
```

`build.sh` runs these steps:

1. `scripts/make_kuf.py`:
   * generates `net/jobs.inc` (custom jobs and icon data, `scripts/jobs_gen.py`) and `net/coops.inc` (co-op mission list);
   * compiles `net/net.c`, which includes `net/eq.c`, with clang into `net/net.bin`, `net/netdata.bin` and `net/ktext.bin`;
   * writes the hook assembly `newcode/kuf.asm` and assembles it with nasm.
2. `scripts/relink.py` puts the built code into `out/patch_heroes_multi_lobby.py` and fixes the jump targets.
3. `out/patch_heroes_multi_lobby.py <original Heroes.exe> dist/Heroes_MP_v1.4.exe` applies every patch. Each patch is checked against the original bytes first. The script then adds the sections.
4. `scripts/exe_patcher.py` writes `dist/BattleForBersiaMP_v1.4_Install.bat`, the release installer. It applies `ui/Interface_Misc.diff` to your `Interface_Misc.txt` and stores both targets as copy/insert deltas.

`./build.sh E` builds the separate Encablossa test build (`Heroes_MP_v1.4E.exe`), which is not part of the Nexus release.

`KUF_BUILD_STAMP` is set by `build.sh` to the release's build id, which goes into the multiplayer version fingerprint.
Unset it (`KUF_BUILD_STAMP= ./build.sh`) for a development build. That build uses the compile time, so players cannot
accidentally mix it with the release.

## 3. Verify

```
sha256sum dist/*
python3 tools/inspect_installer.py dist/BattleForBersiaMP_v1.4_Install.bat
```

Compare the hashes with the table at the top. `inspect_installer.py` prints the source and target hashes. It also counts the
copy runs taken from the player's own exe and the inserted bytes (the mod's code).

## 4. The Crusaders installer (optional)

`BattleForBersiaMP_Crusaders_Install.bat` is generated from a Kingdom Under Fire: The Crusaders install. The steps:

1. `crusconv/convert.py` converts the Crusaders mission files into the Heroes stage layout. `crusconv/music.py` renames the music bank.
2. `scripts/crus_installer.py` (`KUF_CRUS_DATA` = the extracted Crusaders data) compares every converted file with its Crusaders original. It stores either "copy file X" or a copy/insert delta, with SHA-256 checks.
3. `scripts/crus_bat.py` writes the .bat.

No Crusaders file is stored in this repository or in the installer.

## Testing the installer scripts without Windows

The installer scripts run under PowerShell 7 on Linux (`pwsh`) as well. Extract the text between `#PSSTART` and `#PAYLOAD`,
set the environment variable `HMP_BAT` to the .bat path, and run it with `pwsh -File`. Note that paths use Windows `\`
separators.
