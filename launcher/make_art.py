#!/usr/bin/env python3
"""Launcher artwork, generated (no game files used): background, logo, icon.
Output: launcher/art/bg.png (960x600), logo.png, icon.ico"""
import os, random, math
from PIL import Image, ImageDraw, ImageFilter, ImageFont, ImageChops

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, 'art')
os.makedirs(OUT, exist_ok=True)
FONT = '/usr/share/fonts/truetype/google-fonts/Lora-Variable.ttf'
W, H = 960, 600
random.seed(7)


def font(size, weight='Bold'):
    f = ImageFont.truetype(FONT, size)
    try:
        f.set_variation_by_name(weight)
    except Exception:
        pass
    return f


def noise(w, h, cell, seed):
    rnd = random.Random(seed)
    small = Image.new('L', (w // cell + 2, h // cell + 2))
    small.putdata([rnd.randint(0, 255) for _ in range(small.width * small.height)])
    return small.resize((w + cell * 2, h + cell * 2), Image.BICUBIC).crop((cell, cell, cell + w, cell + h))


def background():
    # warm charcoal with an ember glow and a fine woven texture (no map / contour look)
    img = Image.new('RGB', (W, H))
    ip = img.load()
    n1 = noise(W, H, 120, 11).load()
    n2 = noise(W, H, 9, 12).load()
    for y in range(H):
        for x in range(W):
            # ember glow from the lower left, cooler at the top right
            gx, gy = x / W, y / H
            glow = max(0.0, 1.0 - ((gx - 0.08) ** 2 * 1.6 + (gy - 1.05) ** 2 * 1.3) ** 0.5)
            v = n1[x, y] / 255.0 * 0.6 + n2[x, y] / 255.0 * 0.4
            r = 20 + 22 * v + 70 * glow ** 2
            g = 14 + 14 * v + 26 * glow ** 2
            b = 12 + 10 * v + 10 * glow ** 2
            ip[x, y] = (int(min(255, r)), int(min(255, g)), int(min(255, b)))
    d = ImageDraw.Draw(img, 'RGBA')
    # faint diagonal weave
    for k in range(-H, W, 14):
        d.line([(k, 0), (k + H, H)], fill=(255, 210, 160, 6))
    for k in range(0, W + H, 14):
        d.line([(k, 0), (k - H, H)], fill=(0, 0, 0, 10))
    # drifting embers
    rnd = random.Random(5)
    for _ in range(140):
        x = rnd.random() * W * 0.7; y = H * (0.35 + rnd.random() * 0.65)
        r = rnd.choice((1, 1, 1, 2))
        a = rnd.randint(40, 140)
        d.ellipse([x - r, y - r, x + r, y + r], fill=(255, 140 + rnd.randint(0, 60), 60, a))
    img = img.filter(ImageFilter.GaussianBlur(0.4))
    # vignette
    vig = Image.new('L', (W, H), 0)
    vd = ImageDraw.Draw(vig)
    for i in range(60):
        k = i / 60
        vd.ellipse([-W * 0.25 + k * W * 0.3, -H * 0.35 + k * H * 0.3, W * 1.25 - k * W * 0.3, H * 1.35 - k * H * 0.3], fill=int(255 * k))
    vig = vig.filter(ImageFilter.GaussianBlur(40))
    img = Image.composite(img, Image.new('RGB', (W, H), (8, 5, 4)), vig)
    # top bar: deep crimson band with a bronze rule
    bar = Image.new('RGBA', (W, 96), (36, 10, 10, 190))
    img.paste(bar, (0, 0), bar)
    d = ImageDraw.Draw(img, 'RGBA')
    d.line([(0, 95), (W, 95)], fill=(176, 128, 70, 200), width=2)
    img.save(os.path.join(OUT, 'bg.png'), optimize=True)


def metal(size, top=(250, 222, 160), bottom=(168, 92, 40)):
    g = Image.new('RGB', (1, size[1]))
    for y in range(size[1]):
        k = y / max(1, size[1] - 1)
        g.putpixel((0, y), tuple(int(top[i] + (bottom[i] - top[i]) * k) for i in range(3)))
    return g.resize(size)


def logo():
    w, h = 300, 92
    img = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    f1, f2, f3 = font(44), font(17), font(15, 'Regular')
    mask = Image.new('L', (w, h), 0)
    md = ImageDraw.Draw(mask)
    # "BATTLE FOR" small, "BERSIA" big, "MULTIPLAYER" spaced
    def ctext(d, y, s, f, spacing=0):
        tw = sum(d.textlength(c, font=f) for c in s) + spacing * (len(s) - 1)
        x = (w - tw) / 2
        for c in s:
            d.text((x, y), c, font=f, fill=255)
            x += d.textlength(c, font=f) + spacing
    ctext(md, 0, 'BATTLE FOR', f2, 5)
    ctext(md, 18, 'BERSIA', f1, 6)
    ctext(md, 70, 'MULTIPLAYER', f3, 7)
    shadow = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    shadow.putalpha(mask.filter(ImageFilter.GaussianBlur(3)).point(lambda v: v * 0.8))
    img = Image.alpha_composite(img, shadow)
    face = metal((w, h)).convert('RGBA')
    face.putalpha(mask)
    img = Image.alpha_composite(img, face)
    img.save(os.path.join(OUT, 'logo.png'))


def icon():
    S = 256
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    shield = [(28, 20), (228, 20), (228, 120), (200, 190), (128, 240), (56, 190), (28, 120)]
    d.polygon(shield, fill=(200, 150, 84))
    inner = [(44, 36), (212, 36), (212, 118), (188, 180), (128, 222), (68, 180), (44, 118)]
    d.polygon(inner, fill=(92, 18, 20))
    f = font(150)
    tw = d.textlength('B', font=f)
    m = Image.new('L', (S, S), 0)
    ImageDraw.Draw(m).text(((S - tw) / 2, 34), 'B', font=f, fill=255)
    face = metal((S, S)).convert('RGBA')
    face.putalpha(m)
    img = Image.alpha_composite(img, face)
    img.save(os.path.join(OUT, 'icon.ico'), sizes=[(256, 256), (48, 48), (32, 32), (16, 16)])
    img.save(os.path.join(OUT, 'icon_preview.png'))


if __name__ == '__main__':
    background(); logo(); icon()
    print('art written to', OUT)
