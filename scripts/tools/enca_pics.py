#!/usr/bin/env python3
"""Body / Portrait pictures (128x256, picture in the top 128x160) for the Encablossa troops: the E build shows them as
looks 2 (Spitter), 3 (Flyer) and 4 (Infested) of type 39 (make_kuf.py enca_pic), rendered from the
game's own models with smfx_render.py. Reads the player's/developer's data1.dat; nothing from the game is stored here.

  enca_pics.py DATA1.DAT OUTDIR [--preview PNG]
"""
import argparse, copy, os, sys
import numpy as np
from PIL import Image
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import smfx_render as R

# name, model, skeleton, clip, frame (None = middle), yaw, pitch, portrait crop (x0, y0, x1, y1 as fractions of the figure
# box), exposure (the Encablossa textures are very dark; the game lights them brighter than this renderer's key light)
UNITS = [
    ('UV_ANCIENTHEART_2',  'C_ENCABLOSA_MONSTER.SMFX',   'c_encablosa_monster.saf',   12, None, 160, 10, (0.30, 0.00, 0.95, 0.42), 1.7),
    ('UV_ANCIENTHEART_3',  'C_ENCABLOSA_MONSTER02.SMFX', 'c_encablosa_monster02.saf',  0, None, 225, 10, (0.30, 0.10, 1.00, 0.85), 1.8),
    ('UV_ANCIENTHEART_4',  'C_ENCA_INFEST.SMFX',         'c_enca_infest.saf',          0, None, 200, 10, (0.20, 0.00, 0.80, 0.40), 1.15),
]


def render_unit(pk, model, saf, clip, frame, yaw, pitch, W, H):
    objs = R.parse_smfx(R.pack_read(pk, model))
    sf = R.parse_saf(R.pack_read(pk, saf))
    R.apply_pose(objs, R.pose_skin(sf, clip, sf['clips'][clip] // 2 if frame is None else frame))
    return R.render(objs, pk, W, H, yaw, pitch, ss=2)


def fit(im, bw, bh):
    im = im.crop(im.getbbox()); s = min(bw / im.width, bh / im.height)
    return im.resize((max(1, round(im.width * s)), max(1, round(im.height * s))), Image.LANCZOS)


def sheet(fig, box, y=0, cw=124):
    out = Image.new('RGBA', (128, 256), (0, 0, 0, 0))
    p = fit(fig, *box); out.alpha_composite(p, ((cw - p.width) // 2, y)); return out


def save_dds(im, fn):
    """Uncompressed 32-bit DDS (A8R8G8B8): the game reads pictures through D3DX, which takes any DDS format; Pillow's DXT
    encoder leaves coloured speckles on these dark pictures."""
    import struct
    w, h = im.size; px = np.asarray(im.convert('RGBA'))[..., [2, 1, 0, 3]].tobytes()   # BGRA in memory = A8R8G8B8
    hdr = struct.pack('<4sIIIIIII44x', b'DDS ', 124, 0x1 | 0x2 | 0x4 | 0x1000 | 0x8, h, w, w * 4, 0, 1)
    pf = struct.pack('<IIIIIIII', 32, 0x41, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000)
    caps = struct.pack('<IIIII', 0x1000, 0, 0, 0, 0)
    open(fn, 'wb').write(hdr + pf + caps + px)


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('pack'); ap.add_argument('out'); ap.add_argument('--preview')
    a = ap.parse_args(); os.makedirs(a.out, exist_ok=True); pk = R.pack_index(a.pack); prev = []
    for name, model, saf, clip, frame, yaw, pitch, pc, ex in UNITS:
        fig = render_unit(pk, model, saf, clip, frame, yaw, pitch, 768, 960)
        fig = fig.crop(fig.getbbox())
        a4 = np.asarray(fig).astype(np.float32); a4[..., :3] = np.clip(a4[..., :3] * ex, 0, 255)
        fig = Image.fromarray(a4.astype(np.uint8), 'RGBA')
        body = sheet(fig, (60, 78), 0, 62)   # USER 2026-11-26: half size in the barracks ('a tad too big ... half their size'); eq.c enca_uv body rect 64 x 80, feet at 31, 73
        x0, y0, x1, y1 = pc; w, h = fig.size
        head = fig.crop((int(x0 * w), int(y0 * h), int(x1 * w), int(y1 * h)))
        # USER 2026-11-26 (lobby portrait "doesn't quite fit"): the portrait now uses the heroes' rectangle (100 x 84 @ 50,42,
        # eq.c enca_uv) - head inside 96 x 82, centred across the 100 px, standing on the bottom edge
        p_ = fit(head, 96, 82); prt = Image.new('RGBA', (128, 256), (0, 0, 0, 0)); prt.alpha_composite(p_, ((100 - p_.width) // 2, 84 - p_.height))
        for im, kind in ((body, 'Bdy'), (prt, 'Prt')):
            sub = os.path.join(a.out, 'Data', 'interface', 'Body' if kind == 'Bdy' else 'Portrait'); os.makedirs(sub, exist_ok=True)
            fn = os.path.join(sub, '%s_%s.dds' % (name, kind))
            save_dds(im, fn)
            prev.append(Image.open(fn).convert('RGBA'))
            print(fn)
    if a.preview:
        bg = Image.new('RGBA', (128 * len(prev), 170), (36, 30, 24, 255))
        for i, im in enumerate(prev): bg.alpha_composite(im.crop((0, 0, 128, 170)), (128 * i, 0))
        bg.resize((bg.width * 2, bg.height * 2), Image.NEAREST).save(a.preview)


if __name__ == '__main__':
    main()
