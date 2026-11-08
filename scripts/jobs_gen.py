"""v93: custom jobs 56-60 - single source for net/jobs.inc (C) and the asm tables in make_kuf.py.
Ids 56.. are appended to the job tables at load time (sox_fopen in eq.c builds the SOX files with the extra rows)."""
import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__))) + '/'   # repository root

import subprocess, struct, os
from PIL import Image
S = ROOT
# TroopInfo base-row columns (codex tables.md): 2 speed, 10 melee_attack, 13 unit_spacing, 14 dmg melee %, 15 range, 16 frontal,
# 17 explosion, 18 fire, 19 ice, 20 lightning, 21 holy, 22 curse, 23 earth, 25 soldier_hp, 27 cols, 28 rows, 29 hp/level
NEWJOBS = [
  # id, base, race start (tree), col,row, reqs [(skill,level)], soldier char, token, icon, name, desc, TroopInfo overrides
  dict(id=56, base=4,  race=0x00, col=1, row=2, req=[(0,10),(2,5)], ch=53, tok='J_Dwarf', icon='Dwarf', name='Dwarf',
       desc='Stout dwarven infantry. Slower than heavy infantry but much tougher, they hold the line against anything that reaches them.',
       ti={2:99, 10:12, 14:85, 20:130, 25:470, 27:5, 28:4, 29:23}),
  dict(id=57, base=5,  race=0x00, col=1, row=3, req=[(0,12),(12,5)], ch=45, tok='J_FElf', icon='Female_Elf', name='Elf Warrior',
       desc='Swift elven warriors. Faster than knights but lightly armoured, they wield the magic of the earth.',
       ti={2:130, 14:100, 15:90, 20:150, 23:75, 25:340, 27:5, 28:4, 29:17}),
  dict(id=58, base=21, race=0x12, col=3, row=1, req=[(0,10),(2,5)], ch=49, tok='J_DElfM', icon='Male_Dark_Elf', name='Dark Elf Guard',
       desc='Elite dark elf infantry. Fewer in number than the fighters, but each one is hardier and strikes harder.',
       ti={10:13, 25:400, 27:5, 28:3, 29:20}),
  dict(id=59, base=33, race=0x17, col=4, row=1, req=[(0,10),(13,5)], ch=54, tok='J_Lich', icon='Lich', name='Lich',
       desc='Undead sorcerers who wield curses. Weak in close combat and vulnerable to holy magic such as Curatio.',
       ti={10:8, 14:110, 21:300, 22:0, 25:330, 27:5, 28:3, 29:17}),
  dict(id=60, base=27, race=0x17, col=1, row=0, req=[(0,20)], ch=26, tok='J_Ogre', icon='Ogre', name='Ogre',
       desc='Huge, brutal ogres with great strength and endurance. Slow, and easy prey for archers and ballistae.',
       ti={2:95, 10:30, 13:1200, 14:70, 15:160, 17:150, 25:1200, 27:5, 28:2, 29:60}),
]
# Font2.dds spots (block aligned, transparent in the shipped and community Font2, outside every UV rect)
SPOTS = [(948,176),(960,224),(960,272),(964,376),(980,424)]
# ---- Heroes MP E build (USER 2026-11-07: Encablossa faction, kept out of the main build): KUF_ENCA=1 ----
ENCA = bool(os.environ.get('KUF_ENCA'))
# Encablossa tree: own job tree (race start marker 61 - make_kuf's enca_tr hook gives race 4 = Encablossa this range).
# Bases are the mission/Invasion rows (38 Encablossa, 40 Spitter, 55 Infested, 39 Flyer) - copies, so mission units are unchanged.
# TroopInfo skill columns: 30/31 skill1 id/rate(x10), 32/33 skill2.
ENCAJOBS = [
  dict(id=61, base=38, race=61, col=2, row=0, req=[], ch=39, tok='J_EncaBrood', icon='Enca_Brood', name='Encablossa',
       desc='Spawn of the Encablossa. Ferocious, fast-breeding creatures that tear into the enemy with claw and fang. Every Encablossa troop begins as a brood.',
       ti={27:4, 28:4}),
  dict(id=62, base=40, race=61, col=1, row=1, req=[(0,8)], ch=39, tok='J_EncaSpit', icon='Enca_Spitter', name='Enca Spitter',
       desc='Encablossa creatures that spit corrosive bile at range. Deadly behind a wall of brood, but fragile when caught up close.',
       ti={27:4, 28:4}),
  dict(id=63, base=55, race=61, col=2, row=1, req=[(0,15)], ch=63, tok='J_EncaInf', icon='Enca_Infested', name='Enca Infested',
       desc='Hosts swollen with the Encablossa spawn. Slower than the brood but far tougher, they crush anything in their path.',
       ti={27:4, 28:4}),
  dict(id=64, base=39, race=61, col=2, row=2, req=[(0,25)], ch=55, tok='J_EncaFly', icon='Enca_Flyer', name='Enca Flyer',
       desc='A winged horror that bombards troops below with caustic spawn. Very tough, but helpless against archers and ballistae.',
       ti={27:1, 28:1}),
]
ENCASPOTS = [(208,0),(208,44),(204,88),(768,444)]   # free in the shipped and community Font2, outside every UV / font rect
if ENCA: NEWJOBS = NEWJOBS + ENCAJOBS; SPOTS = SPOTS + ENCASPOTS
def dxt5(icon, sz=44):
    im = Image.open(S+'jobs/icons/%s.png'%icon).convert('RGBA')
    c = Image.new('RGBA', (sz,sz), (0,0,0,0)); c.paste(im, (0,0)); p = S+'jobs/build/%s%d.png'%(icon,sz); c.save(p)
    d = S+'jobs/build/%s%d.dds'%(icon,sz)
    subprocess.run(['convert', p, '-define', 'dds:compression=dxt5', '-define', 'dds:mipmaps=0', d], check=True)
    nb = (sz//4)**2
    b = open(d,'rb').read(); assert b[84:88]==b'DXT5' and len(b)==128+nb*16, (icon, len(b))
    # v1.2E: packed 10 bytes per 4x4 block (u16 opaque mask + DXT colour block; the icons' alpha is only 0 / 255) - 9 icons fit .kjb
    out = b''
    for k in range(nb):
        blk = b[128+k*16:128+k*16+16]; a0, a1 = blk[0], blk[1]; bits = int.from_bytes(blk[2:8], 'little')
        pal = [a0, a1] + ([((6-i)*a0+(i)*a1)//6 for i in range(1,7)] if a0 > a1 else [((4-i)*a0+i*a1)//4 for i in range(1,5)]+[0,255])
        m = 0
        for t in range(16):
            if pal[(bits >> (3*t)) & 7] >= 128: m |= 1 << t
        out += struct.pack('<H', m) + blk[8:16]
    return out
def gen():
    o = ['/* generated by scripts/jobs_gen.py */', '#define NJOB %d'%len(NEWJOBS), '#define JOB0 56']
    o.append('static const struct njob { int id, base, race, col, row, req[6], ch, ux, uy; const char *tok, *name, *desc; } njobs[NJOB] = {')
    for k,j in enumerate(NEWJOBS):
        r = (j['req']+[(-1,-1)]*3)[:3]; ti = sorted(j['ti'].items())
        o.append(' { %d,%d,%d,%d,%d,{%s},%d,%d,%d,"(%s)","%s","%s" },'%(j['id'],j['base'],j['race'],j['col'],j['row'],
                 ','.join('%d,%d'%x for x in r),j['ch'],SPOTS[k][0],SPOTS[k][1],j['tok'],j['name'],j['desc']))
    o.append('};')
    o.append('static const short njob_ti[NJOB][16][2] = {')
    for j in NEWJOBS:
        ti = sorted(j['ti'].items()); ti += [(-1,0)]*(16-len(ti))
        o.append(' {%s},'%','.join('{%d,%d}'%x for x in ti))
    o.append('};')
    blob = b''.join(dxt5(j['icon']) for j in NEWJOBS)
    o.append('#define ABIL_DXT ((const unsigned char *)0xB13000 + %d)   /* v1.3: Fire at Will ability icon, 36x36 = 9x9 packed blocks */'%len(blob))
    blob += dxt5('FireAtWill', 36); assert len(blob) <= 0x3000, len(blob)
    open(S+'jobs/build/jobdata.bin','wb').write(blob)
    o.append('#define njob_dxt ((const unsigned char (*)[11*11*10])0xB13000)   /* section .kjb (patch script): 121 x {u16 opaque mask, 8-byte colour block} */')
    o.append('#define JUV_INIT { %s }'%', '.join(str(j.get('uv0', (j['base'] if j['base'] < 35 else 2) + 0x5b)) for j in NEWJOBS))
    open(S+'net/jobs.inc','w').write('\n'.join(o)+'\n')
if __name__ == '__main__': gen(); print('jobs.inc written')
