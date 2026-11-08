"""Copy the Crusaders battle music into the Heroes folder under its own names.
  S_BGM.xsb -> Data\\Sound\\C_BGM.xsb  (internal names: S_BGM -> S_CBGM, W_BGM -> W_CBGM, W_BGMTitle -> W_CBGMTitl)
  W_BGM.xwb -> Data\\Sound\\W_CBGM.xwb (internal name W_BGM -> W_CBGM)
The sound bank's header CRC (CRC-16/X-25 over bytes 0x12..end, stored at 0x08) is recomputed.
usage: music.py CRUS_SOUND_DIR OUT_DIR"""
import sys, os, struct

def crc_x25(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8): crc = (crc >> 1) ^ 0x8408 if crc & 1 else crc >> 1
    return crc ^ 0xFFFF

def rename(buf, old, new, size=64, count=None):
    n = 0; i = 0
    while True:
        i = buf.find(old + b'\0', i)
        if i < 0: break
        if n == count: break
        assert len(new) < size
        buf[i:i + size] = new + b'\0' * (size - len(new)); n += 1; i += size
    return n

src, out = sys.argv[1:3]
d = os.path.join(out, 'Data', 'Sound'); os.makedirs(d, exist_ok=True)
xsb = bytearray(open(os.path.join(src, 'S_BGM.xsb'), 'rb').read())
a = rename(xsb, b'S_BGM', b'S_CBGM'); b = rename(xsb, b'W_BGMTitle', b'W_CBGMTitl'); c = rename(xsb, b'W_BGM', b'W_CBGM')
assert (a, b, c) == (1, 1, 1), (a, b, c)
struct.pack_into('<H', xsb, 8, crc_x25(xsb[0x12:]))
open(os.path.join(d, 'C_BGM.xsb'), 'wb').write(xsb)
xwb = bytearray(open(os.path.join(src, 'W_BGM.xwb'), 'rb').read())
assert rename(xwb, b'W_BGM', b'W_CBGM', count=1) == 1
open(os.path.join(d, 'W_CBGM.xwb'), 'wb').write(xwb)
print('wrote Data\\Sound\\C_BGM.xsb and Data\\Sound\\W_CBGM.xwb')
