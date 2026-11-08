import struct
import os
D=open(os.environ.get('KUF_HEROES_EXE') or exit('set KUF_HEROES_EXE to the original Steam Heroes.exe'),'rb').read()
pe=struct.unpack_from('<I',D,0x3c)[0]; ns=struct.unpack_from('<H',D,pe+6)[0]; so=struct.unpack_from('<H',D,pe+20)[0]
S=[struct.unpack_from('<8sIIII',D,pe+24+so+i*40) for i in range(ns)]
def off(va):
  r=va-0x400000
  for n,vs,vaa,rs,ro in S:
    if vaa<=r<vaa+max(vs,rs): return r-vaa+ro
def u32(va): return struct.unpack_from('<I',D,off(va))[0]
