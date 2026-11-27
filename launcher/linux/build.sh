#!/bin/sh
# Builds the single-file Linux launcher: core + embedded art + GUI.
set -e
cd "$(dirname "$0")"
python3 - <<'P'
import base64, io
from PIL import Image
def png(im):
    b = io.BytesIO(); im.save(b, 'PNG', optimize=True); return base64.encodebytes(b.getvalue()).decode()
bg = Image.open('../art/bg.png').convert('RGB').quantize(colors=96, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
art = 'BG_PNG = """%s"""\nLOGO_PNG = """%s"""\nICON_PNG = """%s"""\n' % (png(bg), png(Image.open('../art/logo.png')), png(Image.open('../art/icon_preview.png').resize((64, 64), Image.LANCZOS)))
core = open('bfb_launcher_core.py').read()
gui = open('bfb_launcher_gui.py').read()
open('../out/BattleForBersiaLauncher-linux.py', 'w').write(core + '\n\n# ------------------------------------------------------------------ artwork (made by make_art.py, no game files)\n' + art + '\n\n' + gui)
P
chmod +x ../out/BattleForBersiaLauncher-linux.py
python3 -m py_compile ../out/BattleForBersiaLauncher-linux.py
echo built ../out/BattleForBersiaLauncher-linux.py
