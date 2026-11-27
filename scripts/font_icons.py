"""Purple and brown lobby colour icons (28x28), drawn here, written as DXT5 blocks for Font2.dds cells
(PlayerColorWhite) 256,484 and (PlayerColorBrown) 286,484 -> block-aligned 7x7 blocks each."""
import struct
from PIL import Image
def icon(base):
    im = Image.new('RGBA', (28, 28), (0, 0, 0, 0)); px = im.load()
    def mix(c, k): return tuple(max(0, min(255, int(v * k))) for v in c)
    light, dark, inner = (174, 162, 132), (148, 134, 99), (50, 46, 36)
    for y in range(28):
        for x in range(28):
            e = min(x, y, 27 - x, 27 - y)
            if e == 0: px[x, y] = (0, 0, 0, 0x24); continue
            if e == 1: px[x, y] = (0, 0, 0, 0x55 if (y in (1, 26) or x in (1, 26)) and e == 1 else 255); continue
            if e == 2: px[x, y] = (0, 0, 0, 255); continue
            if e in (3, 4):
                px[x, y] = (*(light if (x + y) < 27 and (x <= 4 or y <= 4) and not (x >= 23 or y >= 23) else dark), 255); continue
            if e == 5: px[x, y] = (*(inner if y <= 5 or x <= 5 else (0, 0, 0)), 255) if (y <= 5 or y >= 22 or x <= 5 or x >= 22) else (0, 0, 0, 255); continue
            px[x, y] = (0, 0, 0, 255)
    # coloured square 8..20 with bevel
    for y in range(8, 20):          # 12x12, on whole 4x4 blocks so DXT keeps the edges clean
        for x in range(8, 20):
            k = 1.0
            if y == 8: k = 1.25
            elif x == 8: k = 0.55
            elif y >= 18: k = 0.62
            elif x == 19: k = 0.85
            c = mix(base, k)
            if y == 8 and x > 8: c = tuple(min(255, v + 40) for v in c)
            px[x, y] = (*c, 255)
    return im
def dxt5_block(pxs):
    al = [p[3] for p in pxs]; a0, a1 = max(al), min(al)
    if a0 == a1: ab = bytes([a0, a1]) + b'\0' * 6
    else:
        pal = [a0, a1] + [((7 - i) * a0 + i * a1) // 7 for i in range(1, 7)]
        bits = 0
        for k, a in enumerate(al): j = min(range(8), key=lambda i: abs(pal[i] - a)); bits |= j << (3 * k)
        ab = bytes([a0, a1]) + bits.to_bytes(6, 'little')
    def to565(c): return ((c[0] >> 3) << 11) | ((c[1] >> 2) << 5) | (c[2] >> 3)
    def fr565(v): return (((v >> 11) & 31) * 255 // 31, ((v >> 5) & 63) * 255 // 63, (v & 31) * 255 // 31)
    lum = lambda c: c[0] * 3 + c[1] * 6 + c[2]
    cs = [p[:3] for p in pxs]; hi = max(cs, key=lum); lo = min(cs, key=lum)
    c0, c1 = to565(hi), to565(lo)
    if c0 < c1: c0, c1 = c1, c0
    if c0 == c1: return ab + struct.pack('<HHI', c0, c1, 0)
    e0, e1 = fr565(c0), fr565(c1); pal = [e0, e1, tuple((2 * a + b) / 3 for a, b in zip(e0, e1)), tuple((a + 2 * b) / 3 for a, b in zip(e0, e1))]
    bits = 0
    for k, c in enumerate(cs): j = min(range(4), key=lambda i: sum((c[t] - pal[i][t]) ** 2 for t in range(3))); bits |= j << (2 * k)
    return ab + struct.pack('<HHI', c0, c1, bits)
def blocks(im, x0, y0, W=1024):
    out = []
    for by in range(7):
        for bx in range(7):
            pxs = [im.getpixel((bx * 4 + (k & 3), by * 4 + (k >> 2))) for k in range(16)]
            off = 128 + (((y0 // 4) + by) * (W // 4) + (x0 // 4) + bx) * 16
            out.append((off, dxt5_block(pxs)))
    return out
PURPLE, BROWN = (168, 52, 230), (150, 86, 44)
if __name__ == '__main__':
    import sys
    p, b = icon(PURPLE), icon(BROWN)
    ref = Image.open('loose_user.dds').convert('RGBA')
    row = Image.new('RGBA', (28 * 6 + 50, 28), (60, 60, 60, 255))
    for i, c in enumerate([(376,454),(406,454),(436,454),(466,454)]): row.alpha_composite(ref.crop((c[0], c[1], c[0]+28, c[1]+28)), (i * 30, 0))
    row.alpha_composite(p, (4 * 30 + 10, 0)); row.alpha_composite(b, (5 * 30 + 10, 0))
    row.resize((row.width * 5, 140), Image.NEAREST).save('icons_preview.png')
    # apply to copies of both font sheets, decode and preview
    for name in ('loose_user', 'keaton'):
        d = bytearray(open(name + '.dds', 'rb').read())
        for off, blk in blocks(p, 256, 484) + blocks(b, 286, 484): d[off:off + 16] = blk
        open(name + '_patched.dds', 'wb').write(d)
    t = Image.open('loose_user_patched.dds').convert('RGBA').crop((250, 448, 500, 516)).resize((1000, 272), Image.NEAREST)
    bg = Image.new('RGBA', t.size, (60, 60, 60, 255)); bg.alpha_composite(t); bg.save('patched_preview.png')
