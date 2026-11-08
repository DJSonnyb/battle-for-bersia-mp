"""Move Keaton's runtime fix routines (from her dinput8.dll, with permission) into a new exe section.
Uses the DLL's own base relocations to fix every absolute address; rel32 branches are re-aimed."""
import re,struct,sys
B=0x10000000
DLLF=open('../cpatch/dinput8.dll','rb').read()
SNAP=open('dll.all','rb').read()          # DLL image after its setup ran (runtime globals filled in)
EXE=open('exe.img','rb').read()
def secs(x):
    pe=struct.unpack_from('<I',x,0x3c)[0]; ns=struct.unpack_from('<H',x,pe+6)[0]; so=pe+24+struct.unpack_from('<H',x,pe+20)[0]
    return pe,[struct.unpack_from('<8sIIIIIIHHI',x,so+i*40) for i in range(ns)]
pe,DS=secs(DLLF)
sec={s[0].rstrip(b'\0').decode():(B+s[2],max(s[1],s[3])) for s in DS}
TEXT=sec['.text']; RDATA=sec['.rdata']; DATA=sec['.data']
# base relocations
opt=pe+24; rrva,rsz=struct.unpack_from('<II',DLLF,opt+96+5*8)
def rva2off(r):
    for s in DS:
        if s[2]<=r<s[2]+s[3]: return r-s[2]+s[4]
RELOC=set(); o=rva2off(rrva); end=o+rsz
while o<end:
    pg,bs=struct.unpack_from('<II',DLLF,o)
    for i in range((bs-8)//2):
        e=struct.unpack_from('<H',DLLF,o+8+2*i)[0]
        if e>>12==3: RELOC.add(B+pg+(e&0xfff))
    o+=bs
imps={int(a,16):n for a,d,n in re.findall(r'\{0x([0-9a-f]+),"([^"]+)","([^"]+)"\}',open('imps.h').read())}
def exe_iat():
    ep,ES=secs(EXE)
    irva=struct.unpack_from('<I',EXE,ep+24+96+8)[0]; m={}; i=irva
    while True:
        olt,ts,fc,name,ft=struct.unpack_from('<IIIII',EXE,i)
        if not name: break
        k=0
        while True:
            e=struct.unpack_from('<I',EXE,(olt or ft)+k*4)[0]
            if not e: break
            if not e&0x80000000: m[EXE[e+2:EXE.index(b'\0',e+2)].decode()]=0x400000+ft+k*4
            k+=1
        i+=20
    return m
EIAT=exe_iat()
# disassembly index
ins={}; order=[]
for l in open('../cpatch/dll.dis'):
    m=re.match(r'^([0-9a-f]{8}):\t((?:[0-9a-f]{2} )+)\s*\t?(.*)$',l.rstrip('\n'))
    if m: a=int(m.group(1),16); ins[a]=(bytes.fromhex(m.group(2)),m.group(3)); order.append(a)
idx={a:i for i,a in enumerate(order)}
def func_range(s):
    i=idx[s]; far=s
    while True:
        a=order[i]; b,t=ins[a]
        if t.startswith('j'):
            for tg in re.findall(r'0x(100[0-9a-f]{5})',t):
                tg=int(tg,16)
                if tg>far and tg-s<0x2000: far=tg
        if b==b'\xcc' and a>far: return s,a
        i+=1
STUB_RET={0x100138bd}                      # __security_check_cookie -> plain ret
REPL={0x10015fa0:'k_memset',0x10015b80:'k_strrchr',0x10017bb0:'k_strcpy_s',0x10017c40:'k_strcat_s',0x10017db0:'k_stricmp'}
import subprocess
def compile_repl(at):
    subprocess.run(['clang','-target','i686-unknown-elf','-O2','-ffreestanding','-fno-builtin','-fno-pic','-fno-stack-protector','-mno-sse',
                    '-fno-asynchronous-unwind-tables','-c','repl.c','-o','repl.o'],check=True)
    subprocess.run(['ld.lld','-Ttext=0x%x'%at,'-e','0','repl.o','-o','repl.elf'],check=True)
    subprocess.run(['llvm-objcopy','-O','binary','-j','.text','repl.elf','repl.bin'],check=True)
    sy={l.split()[2]:int(l.split()[0],16) for l in subprocess.run(['llvm-nm','repl.elf'],capture_output=True,text=True).stdout.splitlines() if len(l.split())==3}
    b=open('repl.bin','rb').read(); assert len(b)<=0x200,len(b)
    return b,sy
def build(roots,KFX_VA):
    funcs={}; todo=list(roots); ext=set()
    while todo:
        f=todo.pop()
        if f in funcs or f in STUB_RET or f in REPL: continue
        s,e=func_range(f); funcs[f]=e
        for a in order[idx[s]:idx[e]]:
            b,t=ins[a]
            m=re.match(r'(call|j\w+)\s+0x(100[0-9a-f]{5})$',t)
            if m:
                tg=int(m.group(2),16)
                if not (s<=tg<e): todo.append(tg)
    rb,sy=compile_repl(KFX_VA)
    code=bytearray(rb+b'\xcc'*(0x200-len(rb))); new={f:sy[n] for f,n in REPL.items()}
    def al(n):
        while len(code)%n: code.append(0xcc)
    for f in sorted(funcs):
        al(16); new[f]=KFX_VA+len(code); code+=SNAP[f-B:funcs[f]-B]
    al(16); ret_stub=KFX_VA+len(code); code+=b'\xc3'
    for f in STUB_RET: new[f]=ret_stub
    al(16); trap=KFX_VA+len(code); code+=b'\xcc'
    # lazy import resolvers for imports the exe lacks
    lazy={}; strs=bytearray()
    need=set()
    for f,e in funcs.items():
        for r in RELOC:
            if f<=r<e:
                v=struct.unpack_from('<I',SNAP,r-B)[0]
                if v in imps and imps[v] not in EIAT: need.add(imps[v])
    al(16); slots_at=len(code); code+=b'\0'*(4*len(need))
    k32=None
    for i,n in enumerate(sorted(need)):
        lazy[n]=(KFX_VA+slots_at+4*i,None)
    strbase=len(code); code+=b'kernel32.dll\0'
    for n in sorted(need):
        al(1); lazy[n]=(lazy[n][0],KFX_VA+len(code)); code+=n.encode()+b'\0'
    thunks={}
    for n in sorted(need):
        al(16); slot,name=lazy[n]; t=KFX_VA+len(code); thunks[n]=t
        c=bytearray(b'\x60')                                            # pushad
        c+=b'\x68'+struct.pack('<I',KFX_VA+strbase)+b'\xff\x15'+struct.pack('<I',EIAT['GetModuleHandleA'])
        c+=b'\x68'+struct.pack('<I',name)+b'\x50'+b'\xff\x15'+struct.pack('<I',EIAT['GetProcAddress'])
        c+=b'\xa3'+struct.pack('<I',slot)+b'\x61'+b'\xff\x25'+struct.pack('<I',slot)  # mov [slot],eax; popad; jmp [slot]
        code+=c
    for n in need: struct.pack_into('<I',code,lazy[n][0]-KFX_VA,thunks[n])
    al(0x1000); rd_base=KFX_VA+len(code); code+=SNAP[RDATA[0]-B:RDATA[0]-B+RDATA[1]]
    al(0x1000); da_base=KFX_VA+len(code); code+=SNAP[DATA[0]-B:DATA[0]-B+DATA[1]]
    def m_addr(v,ctx):
        if v in new: return new[v]
        for f,e in funcs.items():
            if f<=v<e: return new[f]+(v-f)
        if v in imps:
            n=imps[v]
            return EIAT[n] if n in EIAT else lazy[n][0]
        if RDATA[0]<=v<RDATA[0]+RDATA[1]: return rd_base+v-RDATA[0]
        if DATA[0]<=v<DATA[0]+DATA[1]: return da_base+v-DATA[0]
        if TEXT[0]<=v<TEXT[0]+TEXT[1]: return trap
        return None
    def put(off,v,ctx):
        r=m_addr(v,ctx)
        if r is None: raise SystemExit('unmapped %x at %s'%(v,ctx))
        struct.pack_into('<I',code,off,r)
    # relocate code
    for f,e in funcs.items():
        for r in RELOC:
            if f<=r<e: put(new[f]-KFX_VA+(r-f),struct.unpack_from('<I',SNAP,r-B)[0],'code %x'%r)
        for a in order[idx[f]:idx[e]]:
            b,t=ins[a]
            if b[:1] in (b'\xe8',b'\xe9') and len(b)==5 or (b[:1]==b'\x0f' and len(b)==6 and 0x80<=b[1]<=0x8f):
                p=a+len(b)-4; tg=(a+len(b)+struct.unpack_from('<i',b,len(b)-4)[0])&0xffffffff
                if f<=tg<e: continue
                nt=m_addr(tg,'branch %x'%a)
                np_=new[f]+(p-f)
                struct.pack_into('<i',code,np_-KFX_VA,nt-(np_+4))
    # relocate data copies (relocs + runtime-written pointers into DLL)
    for base,(sa,sz) in ((rd_base,RDATA),(da_base,DATA)):
        for p in range(sa,sa+sz-3):
            v=struct.unpack_from('<I',SNAP,p-B)[0]
            if p in RELOC or (sa==DATA[0] and p%4==0 and B<=v<B+0x3d000):
                r=m_addr(v,'data %x'%p)
                if r is not None: struct.pack_into('<I',code,base-KFX_VA+(p-sa),r)
    return bytes(code),new,lazy
