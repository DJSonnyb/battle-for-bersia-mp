#!/bin/sh
# Builds launcher/out/BattleForBersiaLauncher.exe (x64, no C runtime). Needs clang, lld-link, llvm-dlltool, llvm-rc, python3 + Pillow.
set -e
cd "$(dirname "$0")"
mkdir -p out
python3 make_art.py
for d in def/*.def; do llvm-dlltool -m i386:x86-64 -d "$d" -l "out/$(basename "$d" .def).lib"; done
llvm-rc /FO out/launcher.res launcher.rc
clang --target=x86_64-pc-windows-msvc -O2 -ffreestanding -fno-builtin -fno-stack-protector -mno-stack-arg-probe \
      -Wall -Wno-unused-function -c launcher.c -o out/launcher.obj
lld-link /nologo /subsystem:windows /entry:start /nodefaultlib /out:out/BattleForBersiaLauncher.exe \
      out/launcher.obj out/launcher.res out/kernel32.lib out/user32.lib out/gdi32.lib out/shell32.lib out/ole32.lib out/winhttp.lib out/gdiplus.lib
echo "built out/BattleForBersiaLauncher.exe"
