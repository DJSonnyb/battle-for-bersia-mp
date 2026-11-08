"""Convert a Kingdom Under Fire: The Crusaders mission to a Heroes co-op mission.

usage: convert.py CRUS_DATA_ROOT HEROES_LIST OUT_DIR MISSION [MISSION...]
  CRUS_DATA_ROOT: folder holding the unpacked Crusaders folders (Mission/Mission, AI/AI, Set/Set,
                  Camera/Camera, ENG/ENG (text), ENGvoice/ENG (voice banks))
  HEROES_LIST:    `dir /s /b` listing of the Heroes install (relative paths), to avoid name clashes
  OUT_DIR:        output tree laid out like the Heroes install (Data\\...)
"""
import struct, sys, os, shutil, json

TEXT_BASE = 10000          # Crusaders UserTextTable N -> Heroes UserTextTable 10000+N
QUICK_SAVE = 159           # ActQuickSave: would write a campaign save - dropped in co-op
DROP_ACTIONS = {QUICK_SAVE}

class R:
    def __init__(s, b): s.b = b; s.p = 0
    def u(s, f):
        v = struct.unpack_from(f, s.b, s.p); s.p += struct.calcsize(f); return v[0] if len(v) == 1 else v
    def raw(s, n): v = s.b[s.p:s.p + n]; s.p += n; return v

def cstr(b): return b.split(b'\0')[0].decode('latin1')
def pstr(s, n=64):
    b = s.encode('latin1'); assert len(b) < n, s; return b + b'\0' * (n - len(b))

def read_value(r):
    st = r.p; t = r.u('<I')
    if t == 2: r.raw(r.u('<I'))
    else: r.u('<I')
    return r.b[st:r.p]

def convert_stg(data, rename, utt_new, log):
    r = R(data)
    ver, my = r.u('<Ii')
    ptype = r.u('<8i'); pgroup = r.u('<8i')
    rest = bytearray(r.raw(7 * 64 + 4 + 64 + 4 + 4 + 8 + 4 + 12 + 4))
    names = {}
    for i, k in enumerate(('map', 'minimap', 'defcam', 'usercam', 'set', 'sky', 'ai')):
        v = cstr(rest[i * 64:(i + 1) * 64]); nv = rename(k, v)
        names[k] = (v, nv)
        if nv != v: rest[i * 64:(i + 1) * 64] = pstr(nv)
    dtt, utt = struct.unpack_from('<hh', rest, 448)
    struct.pack_into('<hh', rest, 448, dtt, utt_new(utt))
    out = bytearray(struct.pack('<Ii', ver, my))
    out += struct.pack('<8b', *ptype) + struct.pack('<8b', *pgroup) + bytes(8)   # + TeamColour (Heroes)
    out += rest
    n = r.u('<I'); out += struct.pack('<I', n)
    for i in range(n):
        t = r.raw(544)
        out += t[:40] + bytes(4) + t[40:]          # + InternalHeroes (unused)
    st = r.p
    nr = r.u('<I'); r.raw(nr * 84)
    nv = r.u('<I')
    for i in range(nv):
        r.raw(64); r.u('<i'); t = r.u('<i')
        if t == 2: r.raw(r.u('<I'))
        else: r.u('<i')
    out += data[st:r.p]                           # rects + variables unchanged
    ng = r.u('<I'); out += struct.pack('<I', ng)
    dropped = 0
    for g in range(ng):
        gid, nt = r.u('<II'); out += struct.pack('<II', gid, nt)
        for t in range(nt):
            out += r.raw(64) + struct.pack('<I', r.u('<I'))
            for kind in 'CA':
                cnt = r.u('<I'); ops = []
                for o in range(cnt):
                    op = r.u('<I'); na = r.u('<I'); args = [read_value(r) for _ in range(na)]
                    if kind == 'A' and op in DROP_ACTIONS: dropped += 1; continue
                    ops.append(struct.pack('<II', op, na) + b''.join(args))
                out += struct.pack('<I', len(ops)) + b''.join(ops)
    out += data[r.p:]                             # trailing bytes as-is
    log.append(f'  header: ptype {ptype} -> int8, group {pgroup} -> int8, + team colours')
    log.append(f'  troops: {n} (+4 bytes each), triggers dropped: {dropped} QuickSave')
    log.append(f'  text table {utt} -> {utt_new(utt)}')
    return bytes(out), names, (dtt, utt)

def main():
    croot, hlist, outd = sys.argv[1:4]; missions = sys.argv[4:]
    heroes = set()
    for l in open(hlist, encoding='latin1'):
        l = l.strip().replace('/', '\\')
        if l: heroes.add(l.lower())
    def hexists(rel): return rel.lower() in heroes
    folders = {'ai': ('AI/AI', 'Data\\AI'), 'set': ('Set/Set', 'Data\\Set'),
               'defcam': ('Camera/Camera', 'Data\\Camera'), 'usercam': ('Camera/Camera', 'Data\\Camera')}
    def find(d, name):
        p = os.path.join(croot, d)
        for f in os.listdir(p):
            if f.lower() == name.lower(): return os.path.join(p, f)
    report = []
    for m in missions:
        log = [m]; copies = []
        src = find('Mission/Mission', m + '.stg'); data = open(src, 'rb').read()
        def rename(k, v):
            if not v: return v
            if k == 'map':
                nv = m + '.map'
                copies.append((find('Mission/Mission', v), 'Data\\Mission\\' + nv))
                nav = find('Mission/Mission', os.path.splitext(v)[0] + '.nav') or find('Mission/Mission', m + '.nav')
                if nav: copies.append((nav, 'Data\\Mission\\' + m + '.nav'))
                return nv
            if k == 'minimap':
                nv = m + '.bmp'; dds = find('Mission/Mission', os.path.splitext(v)[0] + '.dds')
                if dds: copies.append((dds, 'Data\\Mission\\' + m + '.dds'))
                else: log.append(f'  ! minimap {v} not found')
                return nv
            if k == 'sky': return v                      # sky models live in the .dat archives
            cd, hd = folders[k]; f = find(cd, v)
            if f is None: log.append(f'  ! {k} {v} not in the Crusaders upload'); return v
            nv = ('C_' + v) if hexists(hd + '\\' + v) else v
            copies.append((f, hd + '\\' + nv))
            if k in ('defcam', 'usercam'):          # the game also loads <name>_.cam and <name>@.cam
                b, e = os.path.splitext(v); nb = os.path.splitext(nv)[0]
                for suf in ('_', '@'):
                    fv = find(cd, b + suf + e)
                    if fv: copies.append((fv, hd + '\\' + nb + suf + e))
                    elif k == 'defcam': log.append(f'  ! {b + suf + e} not found')
            return nv
        out, names, (dtt, utt) = convert_stg(data, rename, lambda u: TEXT_BASE + u, log)
        copies.append((None, 'Data\\Mission\\' + m + '.stg'))
        t = find('ENG/ENG', 'UserTextTable%04d_ENG.txt' % utt)
        if t: copies.append((t, 'Data\\Text\\ENG\\UserTextTable%d_ENG.txt' % (TEXT_BASE + utt)))
        else: log.append(f'  ! text table {utt} missing')
        for ext in ('.xwb', '.xsb'):
            v = find('ENGvoice/ENG', m + ext)
            if v: copies.append((v, 'Data\\Voice\\ENG\\' + m + ext))
            else: log.append(f'  ! voice {m}{ext} missing')
        for k, (a, b) in names.items(): log.append(f'  {k}: {a} -> {b}')
        for s, d in copies:
            if hexists(d): log.append(f'  ! CLASH would overwrite Heroes file {d}')
            p = os.path.join(outd, *d.split('\\')); os.makedirs(os.path.dirname(p), exist_ok=True)
            if s is None: open(p, 'wb').write(out)
            else: shutil.copyfile(s, p)
            log.append(f'  wrote {d}')
        report += log
    print('\n'.join(report))

if __name__ == '__main__': main()
