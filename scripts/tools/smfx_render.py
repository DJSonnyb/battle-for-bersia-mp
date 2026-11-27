#!/usr/bin/env python3
"""Render a KUF Heroes unit model (.SMFX + its .dds textures) from the game's own data packs to a cut-out PNG.

Used to make barracks / portrait pictures for units the game has none for (Encablossa, E.Regnier). It reads the
player's / developer's own data1.dat - nothing from the game is stored in the repository.

  smfx_render.py PACK MODEL OUT.png [--yaw DEG] [--pitch DEG] [--size W H] [--tint dark] [--skip NAME,NAME]

Formats: docs formats/strident-mesh.md (pack 3.x, mesh 4.x, material 6.1, skin 7.1). Models are drawn in their
stored bind pose with a fixed key light, orthographic, 3x supersampled, transparent background.
"""
import argparse, io, math, struct, sys
import numpy as np
from PIL import Image


def pack_index(path):
    f = open(path, 'rb'); f.seek(-4, 2); d = struct.unpack('<I', f.read(4))[0]
    f.seek(d + 0x24); nf = struct.unpack('<I', f.read(4))[0]; f.seek(d + 0x40)
    idx = {}
    for _ in range(nf):
        n = struct.unpack('<I', f.read(4))[0]; f.read(260)
        for _ in range(n):
            e = f.read(40); nm = e[:32].split(b'\0')[0].decode('latin1').lower()
            off, sz = struct.unpack('<II', e[32:40])
            if sz: idx.setdefault(nm, (off, sz))
    return f, idx


def pack_read(pk, name):
    f, idx = pk
    e = idx.get(name.lower())
    if not e: return None
    f.seek(e[0]); return f.read(e[1])


def cstr(b): return b.split(b'\0')[0].decode('latin1')


def parse_smfx(d):
    ver = struct.unpack('<i', d[0x20:0x24])[0]; cnt = d[0x24]; p = 0x30
    msize = 0x850 if ver >= 15 else 0x84C
    objs = []
    for _ in range(cnt):
        flags, nuv, prim = struct.unpack('<IBB', d[p:p + 6]); p += 12
        parent = cstr(d[p:p + 255]); p += 255; name = cstr(d[p:p + 255]); p += 255
        V, I, hasmat = struct.unpack('<III', d[p:p + 12]); p += 12
        tex = None
        if hasmat:
            m = d[p:p + msize]; p += msize
            tex = cstr(m[0x150:0x150 + 255]) or None
        else:
            nuv = 1
        stride = 24 + 8 * max(1, nuv)
        vb = np.frombuffer(d[p:p + V * stride], dtype='<f4').reshape(V, stride // 4); p += V * stride
        if prim == 0:
            ib = np.frombuffer(d[p:p + 6 * I], dtype='<u2').reshape(-1, 3); p += 6 * I
            tris = ib.astype(np.int64)
        else:
            s = np.frombuffer(d[p:p + 2 * I], dtype='<u2').astype(np.int64); p += 2 * I
            t = []
            for k in range(len(s) - 2):
                a, b, c = s[k], s[k + 1], s[k + 2]
                if a == b or b == c or a == c: continue
                t.append((a, b, c) if k % 2 == 0 else (b, a, c))
            tris = np.array(t, dtype=np.int64).reshape(-1, 3)
        bones = None; vb_bone = None
        if flags:
            B = struct.unpack('<I', d[p:p + 4])[0]; p += 4
            bones = [cstr(d[p + 255 * k:p + 255 * k + 255]) for k in range(B)]; p += 255 * B
            W = struct.unpack('<I', d[p:p + 4])[0]; p += 4
            vb_bone = np.zeros(V, np.int64)
            for k in range(W):   # engine rule: one bone per vertex, the last listed record wins
                vi, bi, _w = struct.unpack('<IIf', d[p + 12 * k:p + 12 * k + 12]); vb_bone[vi] = bi
            p += 12 * W
        objs.append(dict(name=name, parent=parent, tex=tex, pos=vb[:, 0:3].copy(), nrm=vb[:, 3:6].copy(),
                         uv=vb[:, 6:8].copy(), tris=tris, marker=name.startswith('booking'),
                         bones=bones, vbone=vb_bone))
    return objs


def parse_saf(d):
    """Skeleton + clips (docs formats/strident-mesh.md section 8)."""
    A, NB = struct.unpack('<II', d[0x24:0x2C]); p = 0x34
    clips = list(struct.unpack('<%dI' % A, d[p:p + 4 * A])); p += 4 * A
    bones = []
    for _ in range(NB):
        name = cstr(d[p:p + 255]); p += 255; parent = cstr(d[p:p + 255]); p += 255
        inv = np.array(struct.unpack('<16f', d[p:p + 64])).reshape(4, 4); p += 64 + 64
        aff = struct.unpack('<14f', d[p:p + 56]); p += 56
        tr = []
        for _c in range(A):
            p += 4; P = struct.unpack('<I', d[p:p + 4])[0]; p += 4
            pk = [struct.unpack('<I3f', d[p + 16 * k:p + 16 * k + 16]) for k in range(P)]; p += 16 * P
            R = struct.unpack('<I', d[p:p + 4])[0]; p += 4
            rk = [struct.unpack('<I4f', d[p + 20 * k:p + 20 * k + 20]) for k in range(R)]; p += 20 * R
            tr.append((pk, rk))
        bones.append(dict(name=name, parent=parent, inv=inv, t=aff[0:3], q=aff[3:7], tracks=tr))
    return dict(clips=clips, bones=bones)


def _qmat(q):   # D3DXMatrixRotationQuaternion (row vectors)
    x, y, z, w = q
    return np.array([[1 - 2 * (y * y + z * z), 2 * (x * y + z * w), 2 * (x * z - y * w)],
                     [2 * (x * y - z * w), 1 - 2 * (x * x + z * z), 2 * (y * z + x * w)],
                     [2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (x * x + y * y)]])


def _slerp(a, b, t):
    a = np.array(a); b = np.array(b); c = float(a @ b)
    if c < 0: b = -b; c = -c
    if c > 0.9995: r = a + (b - a) * t; return r / np.linalg.norm(r)
    th = math.acos(c); return (math.sin((1 - t) * th) * a + math.sin(t * th) * b) / math.sin(th)


def _sample(keys, f, lerp):
    if not keys: return None
    if len(keys) == 1 or f <= keys[0][0]: return keys[0][1:]
    for k in range(1, len(keys)):
        if keys[k][0] >= f:
            a, b = keys[k - 1], keys[k]; t = (f - a[0]) / max(1, b[0] - a[0])
            return lerp(a[1:], b[1:], t)
    return keys[-1][1:]


def pose_skin(saf, clip, frame):
    """name -> 4x4 skin matrix (inverse bind x world), engine rules incl. in-place root."""
    bones = saf['bones']; names = {b['name']: i for i, b in enumerate(bones)}
    world = {}
    def W(i):
        if i in world: return world[i]
        b = bones[i]
        if clip is None: t, q = b['t'], b['q']
        else:
            pk, rk = b['tracks'][clip]
            t = _sample(pk, frame, lambda a, c, u: tuple(np.array(a) + (np.array(c) - np.array(a)) * u))
            q = _sample(rk, frame, _slerp)
            if t is None: t = b['t']
            if q is None: q = b['q']
            if i == 0 and pk: t = (pk[0][1], t[1], pk[0][3])
        L = np.eye(4); L[:3, :3] = _qmat(q); L[3, :3] = t
        par = names.get(b['parent']) if i else None
        world[i] = L @ W(par) if par is not None and par != i else L
        return world[i]
    return {b['name']: b['inv'] @ W(i) for i, b in enumerate(bones)}


def apply_pose(objs, skin):
    for o in objs:
        if o['bones']:
            Ms = [skin.get(n, np.eye(4)) for n in o['bones']]
            P = np.c_[o['pos'], np.ones(len(o['pos']))]; N = o['nrm']
            o['pos'] = np.array([P[k] @ Ms[o['vbone'][k]] for k in range(len(P))])[:, :3]
            o['nrm'] = np.array([N[k] @ Ms[o['vbone'][k]][:3, :3] for k in range(len(N))])
        elif o['parent'] in skin:
            M = skin[o['parent']]
            o['pos'] = (np.c_[o['pos'], np.ones(len(o['pos']))] @ M)[:, :3]; o['nrm'] = o['nrm'] @ M[:3, :3]


def load_tex(pk, name):
    if not name: return None
    base = name.rsplit('.', 1)[0]
    for n in (base + '.dds', name):
        b = pack_read(pk, n)
        if b:
            try: return np.asarray(Image.open(io.BytesIO(b)).convert('RGBA')).astype(np.float32) / 255.0
            except Exception: pass
    return None


def render(objs, pk, W, H, yaw, pitch, ss=3, skip=(), tint=None):
    W2, H2 = W * ss, H * ss
    cy, sy = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
    cp, sp = math.cos(math.radians(pitch)), math.sin(math.radians(pitch))
    R = np.array([[cy, 0, -sy], [0, 1, 0], [sy, 0, cy]]) @ np.eye(3)
    P = np.array([[1, 0, 0], [0, cp, -sp], [0, sp, cp]])
    M = P @ R
    draw = [o for o in objs if not o['marker'] and len(o['tris']) and o['name'] not in skip]
    allp = np.concatenate([o['pos'] @ M.T for o in draw])
    mn, mx = allp.min(0), allp.max(0)
    scale = min((W2 * 0.96) / (mx[0] - mn[0]), (H2 * 0.96) / (mx[1] - mn[1]))
    ox = W2 / 2 - (mn[0] + mx[0]) / 2 * scale; oy = H2 / 2 + (mn[1] + mx[1]) / 2 * scale
    col = np.zeros((H2, W2, 4), np.float32); zb = np.full((H2, W2), np.inf, np.float32)
    L = np.array([-0.45, 0.75, -0.5]); L /= np.linalg.norm(L)
    for o in draw:
        tex = load_tex(pk, o['tex'])
        p = o['pos'] @ M.T; n = o['nrm'] @ M.T
        sx = ox + p[:, 0] * scale; syy = oy - p[:, 1] * scale; z = p[:, 2]
        shade = 0.45 + 0.75 * np.clip(n @ (M @ L), 0, 1)
        for a, b, c in o['tris']:
            xs = sx[[a, b, c]]; ys = syy[[a, b, c]]
            x0, x1 = int(max(0, math.floor(xs.min()))), int(min(W2 - 1, math.ceil(xs.max())))
            y0, y1 = int(max(0, math.floor(ys.min()))), int(min(H2 - 1, math.ceil(ys.max())))
            if x0 > x1 or y0 > y1: continue
            den = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2])
            if abs(den) < 1e-9: continue
            gx, gy = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
            w0 = ((ys[1] - ys[2]) * (gx - xs[2]) + (xs[2] - xs[1]) * (gy - ys[2])) / den
            w1 = ((ys[2] - ys[0]) * (gx - xs[2]) + (xs[0] - xs[2]) * (gy - ys[2])) / den
            w2 = 1 - w0 - w1
            ins = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
            if not ins.any(): continue
            zz = w0 * z[a] + w1 * z[b] + w2 * z[c]
            sub = zb[y0:y1 + 1, x0:x1 + 1]
            vis = ins & (zz < sub)
            if not vis.any(): continue
            sh = w0 * shade[a] + w1 * shade[b] + w2 * shade[c]
            if tex is not None:
                u = w0 * o['uv'][a, 0] + w1 * o['uv'][b, 0] + w2 * o['uv'][c, 0]
                v = w0 * o['uv'][a, 1] + w1 * o['uv'][b, 1] + w2 * o['uv'][c, 1]
                th, tw = tex.shape[:2]
                tx = (np.mod(u, 1.0) * tw).astype(int).clip(0, tw - 1); ty = (np.mod(v, 1.0) * th).astype(int).clip(0, th - 1)
                c4 = tex[ty, tx]
            else:
                c4 = np.ones(gx.shape + (4,), np.float32) * 0.6
            vis &= c4[..., 3] > 0.35
            rgb = c4[..., :3] * sh[..., None]
            sub[vis] = zz[vis]
            dst = col[y0:y1 + 1, x0:x1 + 1]
            dst[vis, :3] = rgb[vis]; dst[vis, 3] = 1.0
    col = np.clip(col, 0, 1)
    if tint == 'dark':   # E.Regnier: strong reds kept, everything else grey and darker (same rule as net/eq.c rn_dark)
        r, g, b = col[..., 0], col[..., 1], col[..., 2]
        red = (r > 0.35) & (r > g * 1.6) & (r > b * 1.6)
        grey = (0.3 * r + 0.59 * g + 0.11 * b) * 0.62 + 0.02
        for ch in range(3): col[..., ch] = np.where(red, col[..., ch] * 0.85, grey)
    im = Image.fromarray((col * 255).astype(np.uint8), 'RGBA').resize((W, H), Image.LANCZOS)
    return im


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('pack'); ap.add_argument('model'); ap.add_argument('out')
    ap.add_argument('--yaw', type=float, default=200); ap.add_argument('--pitch', type=float, default=10)
    ap.add_argument('--size', type=int, nargs=2, default=(256, 384))
    ap.add_argument('--skip', default=''); ap.add_argument('--tint', default=None)
    ap.add_argument('--list', action='store_true')
    ap.add_argument('--saf'); ap.add_argument('--clip', type=int); ap.add_argument('--frame', type=float, default=0)
    ap.add_argument('--clips', action='store_true')
    a = ap.parse_args()
    pk = pack_index(a.pack)
    d = pack_read(pk, a.model)
    if not d: sys.exit('model not found: ' + a.model)
    objs = parse_smfx(d)
    if a.saf:
        saf = parse_saf(pack_read(pk, a.saf))
        if a.clips: print(len(saf['bones']), 'bones; clip lengths', saf['clips']); return
        apply_pose(objs, pose_skin(saf, a.clip, a.frame))
    if a.list:
        for o in objs: print(o['name'], o['parent'], o['tex'], len(o['pos']), len(o['tris']))
        return
    im = render(objs, pk, a.size[0], a.size[1], a.yaw, a.pitch, skip=set(filter(None, a.skip.split(','))), tint=a.tint)
    im.save(a.out)


if __name__ == '__main__':
    main()
