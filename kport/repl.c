/* minimal stand-ins for the MSVC CRT helpers Keaton's save routines call (cdecl, same contracts) */
typedef unsigned int size_t;
void *k_memset(void *d, int c, size_t n){ unsigned char *p=d; while(n--) *p++=(unsigned char)c; return d; }
int k_strcpy_s(char *d, size_t sz, const char *s){ if(!d||!sz) return 22; if(!s){ d[0]=0; return 22; }
  for(size_t i=0;i<sz;i++){ d[i]=s[i]; if(!s[i]) return 0; } d[0]=0; return 34; }
int k_strcat_s(char *d, size_t sz, const char *s){ if(!d||!sz) return 22; if(!s){ d[0]=0; return 22; }
  size_t i=0; while(i<sz&&d[i]) i++; if(i==sz){ d[0]=0; return 22; }
  for(size_t j=0;i<sz;i++,j++){ d[i]=s[j]; if(!s[j]) return 0; } d[0]=0; return 34; }
int k_stricmp(const char *a, const char *b){ for(;;){ int x=(unsigned char)*a++, y=(unsigned char)*b++;
  if(x>='A'&&x<='Z') x+=32; if(y>='A'&&y<='Z') y+=32; if(x!=y||!x) return x-y; } }
char *k_strrchr(const char *s, int c){ const char *r=0; c=(char)c; for(;;s++){ if(*s==c) r=s; if(!*s) return (char*)r; } }
