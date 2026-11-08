import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__))) + '/'   # repository root
import struct
S=ROOT
def load(p):
    x=open(p,'rb').read()
    pe=struct.unpack_from('<I',x,0x3c)[0]; opt=pe+24
    ns=struct.unpack_from('<H',x,pe+6)[0]; so=opt+struct.unpack_from('<H',x,pe+20)[0]
    base=struct.unpack_from('<I',x,opt+28)[0]; size=struct.unpack_from('<I',x,opt+56)[0]; hdr=struct.unpack_from('<I',x,opt+60)[0]
    img=bytearray(size); img[:hdr]=x[:hdr]
    for i in range(ns):
        n,vs,va,rs,ro=struct.unpack_from('<8sIIII',x,so+i*40)
        img[va:va+min(rs,max(vs,rs))]=x[ro:ro+rs]
    return x,img,base,size,opt
_,ei,eb,es,_= load(_os_.environ['KUF_HEROES_EXE'])
x,di,db,ds,opt=load(S+'cpatch/dinput8.dll')
open('exe.img','wb').write(ei); open('dll.img','wb').write(di)
irva=struct.unpack_from('<I',x,opt+96+8)[0]
imps=[]
i=irva
while True:
    olt,ts,fc,name,ft=struct.unpack_from('<IIIII',di,i)
    if not name: break
    dn=di[name:di.index(b'\0',name)].decode()
    k=0
    while True:
        e=struct.unpack_from('<I',di,(olt or ft)+k*4)[0]
        if not e: break
        fn=('#%d'%(e&0xffff)) if e&0x80000000 else di[e+2:di.index(b'\0',e+2)].decode()
        imps.append((db+ft+k*4,dn,fn)); k+=1
    i+=20
print(hex(eb),hex(es),hex(db),hex(ds),len(imps))
with open('imps.h','w') as f:
    f.write('static const struct {unsigned iat; const char *dll,*fn;} IMPS[]={\n')
    for a,d,n in imps: f.write('{0x%x,"%s","%s"},\n'%(a,d,n))
    f.write('};\n#define NIMP %d\n#define EXE_SIZE 0x%x\n#define DLL_SIZE 0x%x\n'%(len(imps),es,ds))
print(sorted(set(n for _,_,n in imps)))
