typedef unsigned u32; typedef unsigned short u16;
#include "imps.h"
#define SC __attribute__((stdcall))
static int sys6(int n,int a,int b,int c,int d,int e,int f){int r;__asm__ volatile("push %%ebp; mov %7,%%ebp; int $0x80; pop %%ebp":"=a"(r):"a"(n),"b"(a),"c"(b),"d"(c),"S"(d),"D"(e),"m"(f):"memory");return r;}
static int sys(int n,int a,int b,int c){return sys6(n,a,b,c,0,0,0);}
static int slen(const char*s){int n=0;while(s[n])n++;return n;}
static void pr(const char*s){sys(4,2,(int)s,slen(s));}
static void prx(u32 v){char b[11];b[0]='0';b[1]='x';for(int i=0;i<8;i++){int d=(v>>(28-4*i))&15;b[2+i]=d<10?'0'+d:'a'+d-10;}b[10]=0;pr(b);}
static void die(const char*s){pr(s);pr("\n");sys(1,3,0,0);}
static void *mapfix(u32 a,u32 n){int r=sys6(192,a,n,7,0x32,-1,0);if((u32)r!=a)die("mmap fail");return (void*)r;}
static void loadf(const char*p,void*dst,u32 n){int fd=sys(5,(int)p,0,0);if(fd<0)die(p);u32 got=0;while(got<n){int r=sys(3,fd,(int)dst+got,n-got);if(r<=0)break;got+=r;}sys(6,fd,0,0);}
static void savef(const char*p,const void*src,u32 n){int fd=sys(5,(int)p,0x241,0644);u32 w=0;while(w<n){int r=sys(4,fd,(int)src+w,n-w);if(r<=0)break;w+=r;}sys(6,fd,0,0);}
/* heap */
static u32 hp=0x30000000; static u32 va=0x34000000;
static u32 vab[64],vas[64]; static int nva;
static void*halloc(u32 n){u32 p=(hp+15)&~15u;*(u32*)p=n;p+=16;hp=p+n;char*c=(char*)p;for(u32 i=0;i<n;i++)c[i]=0;return (void*)p;}
SC void* HeapAlloc_(u32 h,u32 f,u32 n){return halloc(n);}
SC int HeapFree_(u32 h,u32 f,void*p){return 1;}
SC u32 HeapSize_(u32 h,u32 f,void*p){return ((u32*)p)[-4];}
SC void* HeapReAlloc_(u32 h,u32 f,void*p,u32 n){char*q=halloc(n);u32 o=((u32*)p)[-4];for(u32 i=0;i<o&&i<n;i++)q[i]=((char*)p)[i];return q;}
SC u32 GetProcessHeap_(void){return 1;}
SC void* VirtualAlloc_(u32 a,u32 n,u32 t,u32 pr_){u32 p=va;n=(n+0xfff)&~0xfffu;va+=n+0x1000;vab[nva]=p;vas[nva++]=n;pr("VirtualAlloc ");prx(n);pr(" -> ");prx(p);pr("\n");return (void*)p;}
SC int VirtualFree_(u32 a,u32 n,u32 t){return 1;}
SC int VirtualProtect_(u32 a,u32 n,u32 np,u32*old){if(old)*old=0x20;return 1;}
SC int FlushInstructionCache_(u32 h,u32 a,u32 n){return 1;}
SC u32 GetCurrentProcess_(void){return 0xffffffff;}
SC u32 GetModuleHandleW_(const u16*n){if(!n)return 0x400000;pr("GetModuleHandleW(name)\n");return 0;}
SC int GetModuleHandleExW_(u32 fl,const void*n,u32*out){*out=(fl&4)?0x10000000:0x400000;return 1;}
static const u16 EXEPATH[]={'C',':','\\','G','\\','H','e','r','o','e','s','.','e','x','e',0};
SC u32 GetModuleFileNameW_(u32 h,u16*b,u32 n){u32 i;for(i=0;EXEPATH[i]&&i+1<n;i++)b[i]=EXEPATH[i];b[i]=0;return i;}
SC u32 GetSystemDirectoryW_(u16*b,u32 n){const char*s="C:\\W";u32 i;for(i=0;s[i];i++)b[i]=s[i];b[i]=0;return i;}
#ifndef ONLY
#define ONLY ""
#endif
SC u32 GetPrivateProfileIntW_(const u16*s,const u16*k,u32 d,const u16*f){const char*o=ONLY;if(!o[0])return 1;int i=0;for(;o[i];i++)if(k[i]!=(u16)o[i])return 0;return k[i]==0;}
SC u32 Inv1(u32 a){return 0xffffffff;}
SC u32 CreateFile_(u32 a,u32 b,u32 c,u32 d,u32 e,u32 f,u32 g){return 0xffffffff;}
SC u32 FindFirst2(u32 a,u32 b){return 0xffffffff;}
SC u32 FindFirstEx(u32 a,u32 b,u32 c,u32 d,u32 e,u32 f){return 0xffffffff;}
static u32 lasterr; SC u32 GetLastError_(void){return lasterr;} SC void SetLastError_(u32 e){lasterr=e;}
SC u32 Id1(u32 a){return a;}
SC void Nop1(u32 a){} SC int Ok1(u32 a){return 1;} SC int Ok2(u32 a,u32 b){return 1;} SC int Ok3(u32 a,u32 b,u32 c){return 1;}
SC u32 Zero0(void){return 0;} SC u32 One0(void){return 1;}
SC int IsPF(u32 f){return f==10||f==6||f==23;}
SC int QPC(u32*p){p[0]=1;p[1]=0;return 1;}
SC void GSTFT(u32*p){p[0]=1;p[1]=0;}
typedef SC int (*initfn)(void*,void*,void**);
SC int InitOnce_(void*o,initfn f,void*p,void**c){return f(o,p,c);}
static u32 tls[64]; static int ntls=1;
SC u32 TlsAlloc_(void){return ntls++;} SC u32 TlsGet_(u32 i){return tls[i];} SC int TlsSet_(u32 i,u32 v){tls[i]=v;return 1;}
SC u32 FlsAlloc_(u32 cb){return ntls++;}
SC u32 GetStdHandle_(u32 a){return 0xffffffff;}
SC u32 GetFileType_(u32 a){return 0;}
SC u32 GetACP_(void){return 1252;}
SC u32 Zero1(u32 a){return 0;} SC u32 Zero2(u32 a,u32 b){return 0;}
static const u16 CMDW[]={'h',0}; SC const u16* GetCmdW(void){return CMDW;} SC const char* GetCmdA(void){return "h";}
static u16 envw[4]; SC u16* GetEnvW(void){return envw;}
SC void GetStartupInfoW_(char*p){for(int i=0;i<68;i++)p[i]=0;*(u32*)p=68;}
SC int GetCPInfo_(u32 cp,char*p){for(int i=0;i<20;i++)p[i]=0;*(u32*)p=1;p[4]='?';return 1;}
SC int MBtoWC(u32 cp,u32 fl,const char*s,int n,u16*d,int dn){if(n<0)n=slen(s)+1;if(!dn)return n;for(int i=0;i<n&&i<dn;i++)d[i]=(unsigned char)s[i];return n;}
SC int WCtoMB(u32 cp,u32 fl,const u16*s,int n,char*d,int dn,u32 a,u32 b){if(n<0){n=0;while(s[n])n++;n++;}if(!dn)return n;for(int i=0;i<n&&i<dn;i++)d[i]=(char)s[i];return n;}
SC u32 LoadLib(const void*n){return 0;} SC u32 LoadLibEx(const void*n,u32 a,u32 b){return 0;} SC u32 GetProc(u32 h,const char*n){pr("GetProcAddress ");if((u32)n>0xffff)pr(n);pr("\n");return 0;}
SC int GetStringTypeW_(u32 a,const u16*s,int n,u16*o){if(n<0){n=0;while(s[n])n++;}for(int i=0;i<n;i++)o[i]=0x100;return 1;}
SC int LCMap(u32 a,u32 b,const u16*s,int n,u16*d,int dn){if(n<0){n=0;while(s[n])n++;n++;}if(!dn)return n;for(int i=0;i<n&&i<dn;i++)d[i]=s[i];return n;}
SC int IsValidCP(u32 a){return 1;}
static int unk_idx;
static void unk(void){pr("UNIMPLEMENTED import: ");pr(IMPS[unk_idx].fn);pr("\n");sys(1,4,0,0);}
#define TH(i) static void __attribute__((naked)) th##i(void){__asm__ volatile("movl $" #i ",%0; jmp *%1"::"m"(unk_idx),"r"(unk));}
#include "thunks.h"
static const struct {const char*n;void*f;} IMPL[]={
{"HeapAlloc",HeapAlloc_},{"HeapFree",HeapFree_},{"HeapSize",HeapSize_},{"HeapReAlloc",HeapReAlloc_},{"GetProcessHeap",GetProcessHeap_},
{"VirtualAlloc",VirtualAlloc_},{"VirtualFree",VirtualFree_},{"VirtualProtect",VirtualProtect_},{"FlushInstructionCache",FlushInstructionCache_},
{"GetCurrentProcess",GetCurrentProcess_},{"GetModuleHandleW",GetModuleHandleW_},{"GetModuleHandleExW",GetModuleHandleExW_},
{"GetModuleFileNameW",GetModuleFileNameW_},{"GetSystemDirectoryW",GetSystemDirectoryW_},{"GetPrivateProfileIntW",GetPrivateProfileIntW_},
{"CreateFileA",CreateFile_},{"CreateFileW",CreateFile_},{"GetFileAttributesA",Inv1},{"GetFileAttributesW",Inv1},
{"FindFirstFileA",FindFirst2},{"FindFirstFileExW",FindFirstEx},{"GetLastError",GetLastError_},{"SetLastError",SetLastError_},
{"EncodePointer",Id1},{"DecodePointer",Id1},{"InitializeCriticalSectionAndSpinCount",Ok2},{"InitializeCriticalSectionEx",Ok3},
{"EnterCriticalSection",Nop1},{"LeaveCriticalSection",Nop1},{"DeleteCriticalSection",Nop1},{"DisableThreadLibraryCalls",Ok1},
{"IsProcessorFeaturePresent",IsPF},{"QueryPerformanceCounter",QPC},{"GetSystemTimeAsFileTime",GSTFT},{"GetCurrentProcessId",One0},
{"GetCurrentThreadId",One0},{"InitOnceExecuteOnce",InitOnce_},{"TlsAlloc",TlsAlloc_},{"TlsGetValue",TlsGet_},{"TlsSetValue",TlsSet_},{"TlsFree",Ok1},
{"FlsAlloc",FlsAlloc_},{"FlsGetValue",TlsGet_},{"FlsSetValue",TlsSet_},{"FlsFree",Ok1},{"InitializeSListHead",Nop1},{"IsDebuggerPresent",Zero0},
{"GetStdHandle",GetStdHandle_},{"GetFileType",GetFileType_},{"GetACP",GetACP_},{"GetOEMCP",GetACP_},{"GetCommandLineW",GetCmdW},{"GetCommandLineA",GetCmdA},
{"GetEnvironmentStringsW",GetEnvW},{"FreeEnvironmentStringsW",Ok1},{"GetStartupInfoW",GetStartupInfoW_},{"GetCPInfo",GetCPInfo_},
{"MultiByteToWideChar",MBtoWC},{"WideCharToMultiByte",WCtoMB},{"LoadLibraryW",LoadLib},{"LoadLibraryExW",LoadLibEx},{"GetProcAddress",GetProc},
{"GetStringTypeW",GetStringTypeW_},{"LCMapStringW",LCMap},{"IsValidCodePage",IsValidCP},{"SetUnhandledExceptionFilter",Zero1},{"CloseHandle",Ok1},{"FindClose",Ok1},
{0,0}};
static int seq(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
struct ud{u32 e,base,lim;u32 flags;};
typedef SC int (*entry_t)(u32,u32,u32);
void __attribute__((used)) cmain(void){
    mapfix(0x400000,EXE_SIZE); loadf("exe.img",(void*)0x400000,EXE_SIZE);
    mapfix(0x10000000,DLL_SIZE); loadf("dll.img",(void*)0x10000000,DLL_SIZE);
    mapfix(0x30000000,0x8000000);
    mapfix(0x3c000000,0x10000); u32 teb=0x3c000000; ((u32*)teb)[0]=0xffffffff; ((u32*)teb)[6]=teb; ((u32*)teb)[12]=teb+0x1000; ((u32*)teb)[11]=teb+0x2000;
    struct ud d={0xffffffff,teb,0xfffff,0x51}; /* seg_32bit|limit_in_pages|useable */
    if(sys(243,(int)&d,0,0)) die("set_thread_area");
    u32 sel=(d.e<<3)|3; __asm__ volatile("movw %w0,%%fs"::"r"(sel));
    for(int i=0;i<NIMP;i++){void*f=THK[i];for(int j=0;IMPL[j].n;j++)if(seq(IMPL[j].n,IMPS[i].fn)){f=IMPL[j].f;break;}*(void**)IMPS[i].iat=f;}
    pr("calling entry\n");
    int r=((entry_t)ENTRY)(0x10000000,1,0);
    pr("entry returned ");prx(r);pr("\n"); ((void(*)(void))0x10001370)(); pr("installer returned\n");
    savef("exe.after",(void*)0x400000,EXE_SIZE); savef("dll.after",(void*)0x10000000,DLL_SIZE);
    for(int i=0;i<nva;i++){char n[]="va0.bin";n[2]='0'+i;savef(n,(void*)vab[i],vas[i]);}
    sys(1,0,0,0);
}
void __attribute__((naked)) _start(void){__asm__ volatile("and $-16,%esp; call cmain");}
