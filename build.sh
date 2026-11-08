#!/bin/bash
# Build Battle for Bersia MP from source.  See BUILD.md.
#   KUF_GAME=/path/to/KUF Heroes  KUF_HEROES_EXE=/path/to/original/Heroes.exe  ./build.sh [E]
set -e
cd "$(dirname "$0")"
: "${KUF_GAME:?set KUF_GAME to your KUF Heroes install folder}"
: "${KUF_HEROES_EXE:?set KUF_HEROES_EXE to the original Steam Heroes.exe}"
VER=1.4
# build id of the released v1.4 / v1.4E (multiplayer fingerprint). Unset it for a dev build (uses the compile time).
export KUF_BUILD_STAMP="${KUF_BUILD_STAMP-Nov  8 202611:27:05}"
mkdir -p dist
echo "original Heroes.exe: $(sha256sum "$KUF_HEROES_EXE" | cut -d' ' -f1)"
echo "expected           : 8b2702c1aac408f1ef687af18e6a9ff7d133ad0368c9c16ed48ac8f7c345aa1c"
if [ "$1" = "E" ]; then export KUF_ENCA=1; OUT=dist/Heroes_MP_v${VER}E.exe; else unset KUF_ENCA; OUT=dist/Heroes_MP_v${VER}.exe; fi
python3 scripts/make_kuf.py          # hooks (newcode/kuf.asm -> nasm) + C code (net/ -> clang/lld)
python3 scripts/relink.py            # puts the built code into the patcher
python3 out/patch_heroes_multi_lobby.py "$KUF_HEROES_EXE" "$OUT"
echo "built $OUT  sha256 $(sha256sum "$OUT" | cut -d' ' -f1)"
if [ "$1" != "E" ]; then
  python3 scripts/exe_patcher.py $VER "$OUT"    # -> dist/BattleForBersiaMP_v1.4_Install.bat
fi
