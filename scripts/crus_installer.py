"""Build HeroesMP_Crusaders_Install.bat: rebuilds the converted Crusaders co-op files (crusout3, v92 set) and the Crusaders
music bank (crusmus) from the PLAYER'S OWN Crusaders install. The .bat carries only copy/insert instructions (no Crusaders
file is shipped): plain copies are 'take file X', converted files are deltas (copy runs from the player's file + the few
bytes our converter inserted). Every source and result is checked by SHA-256."""
import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__))) + '/'   # repository root

import os, hashlib, struct, gzip, base64, json, sys
K = ROOT
CRUS = (_os_.environ.get('KUF_CRUS_DATA') or exit('set KUF_CRUS_DATA (see BUILD.md)')) + '/'
OUTS = [K + 'crusout3/', K + 'crusmus/']
# folder of each crus/ upload sub-tree inside a Crusaders install (last folder name is what the installer matches)
CDIR = {'Mission/Mission': 'Mission', 'AI/AI': 'AI', 'Set/Set': 'Set', 'Camera/Camera': 'Camera', 'ENG/ENG': 'ENG',
        'ENGvoice/ENG': 'ENG', 'Sound/Sound': 'Sound'}
def sha(b): return hashlib.sha256(b).hexdigest()
src = {}       # sha -> (crus rel path, folder hint, name)
byname = {}
for top, hint in CDIR.items():
    d = CRUS + top
    for f in os.listdir(d):
        p = os.path.join(d, f)
        if not os.path.isfile(p): continue
        b = open(p, 'rb').read(); h = sha(b)
        src.setdefault(h, (top + '/' + f, hint, f, len(b)))
        byname.setdefault((top, f.lower()), (p, hint, f, len(b), h))

def delta(s, t):
    """ops: copy runs from s, literals otherwise. 8-byte gram index, prefers continuing at the expected source offset."""
    G = 8; idx = {}
    for i in range(0, len(s) - G + 1):
        g = s[i:i + G]
        l = idx.get(g)
        if l is None: idx[g] = [i]
        elif len(l) < 8: l.append(i)
    ops = []; lit = bytearray(); i = 0; exp = 0
    def flush():
        nonlocal lit
        if lit: ops.append(('L', bytes(lit))); lit = bytearray()
    while i < len(t):
        best = None; bl = 0
        cands = idx.get(t[i:i + G], [])
        if exp + G <= len(s) and s[exp:exp + G] == t[i:i + G]: cands = [exp] + cands
        for c in cands:
            n = 0
            while i + n < len(t) and c + n < len(s) and t[i + n] == s[c + n]: n += 1
            if n > bl: bl, best = n, c
            if c == exp and n >= 64: break
        if bl >= 16:
            flush(); ops.append(('C', best, bl)); i += bl; exp = best + bl
        else:
            lit.append(t[i]); i += 1; exp += 1
    flush()
    out = bytearray()
    for o in ops:
        if o[0] == 'C': out += b'C' + struct.pack('<II', o[1], o[2])
        else: out += b'L' + struct.pack('<I', len(o[1])) + o[1]
    # self-check
    r = bytearray(); p = 0
    while p < len(out):
        if out[p:p+1] == b'C': a, n = struct.unpack_from('<II', out, p + 1); r += s[a:a + n]; p += 9
        else: n = struct.unpack_from('<I', out, p + 1)[0]; r += out[p + 5:p + 5 + n]; p += 5 + n
    assert bytes(r) == t
    return bytes(out)

DERIVED = {'C_BGM.xsb': ('Sound/Sound', 's_bgm.xsb'), 'W_CBGM.xwb': ('Sound/Sound', 'w_bgm.xwb')}
man = []; blob = bytearray(); lit_total = 0
for root in OUTS:
    for dp, dn, fn in os.walk(root):
        for f in sorted(fn):
            if f.lower().endswith('.txt') and f.startswith('README'): continue
            p = os.path.join(dp, f); rel = os.path.relpath(p, root).replace('/', '\\')
            b = open(p, 'rb').read(); h = sha(b)
            if h in src:
                s = src[h]; man.append(dict(out=rel, src=s[2], dir=s[1], size=s[3], sha=h, op='copy', osha=h)); continue
            if f.lower().endswith('.stg'): key = ('Mission/Mission', f.lower())
            elif f in DERIVED: key = DERIVED[f]
            else: sys.exit('no source for ' + rel)
            sp, hint, sname, ssize, sh = byname[key]
            d = delta(open(sp, 'rb').read(), b)
            lit_total += sum(1 for _ in [0])
            man.append(dict(out=rel, src=sname, dir=hint, size=ssize, sha=sh, op='delta', off=len(blob), len=len(d), osha=h, osize=len(b)))
            blob += d
js = json.dumps(man, separators=(',', ':')).encode()
pk = gzip.compress(struct.pack('<I', len(js)) + js + bytes(blob), 9, mtime=0)
print('files', len(man), 'deltas', sum(m['op'] == 'delta' for m in man), 'delta bytes', len(blob), 'packed', len(pk))
open(K + 'dist/crus_payload.bin', 'wb').write(pk)
