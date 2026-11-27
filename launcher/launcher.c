/* Battle for Bersia MP - Launcher
   Freestanding Win32 (x64), no C runtime. Built with clang + lld-link (see build_launcher.sh).
   - checks GitHub Releases of DJSonnyb/battle-for-bersia-mp for the newest installer (_Install.bat)
     (stable releases, or pre-releases too with "Receive test builds")
   - downloads it into the KUF Heroes folder and runs it (the installer patches the player's OWN game files)
   - starts the game through Steam, edits the Graphics settings in Documents\KUF2 Heroes\KUFH.ini,
     opens the netlog / game folder, updates itself (asset BattleForBersiaLauncher-X.Y.Z.exe) */

#define LAUNCHER_VER L"1.0.3"
#define LV_MAJ 1
#define LV_MIN 0
#define LV_PAT 3
#define REPO_API L"/repos/DJSonnyb/battle-for-bersia-mp/releases?per_page=30"
#define REPO_PAGE L"https://github.com/DJSonnyb/battle-for-bersia-mp/releases"

typedef unsigned short WCHAR;
typedef unsigned int UINT, DWORD;
typedef int BOOL, LONG, INT;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long long ULONG_PTR, SIZE_T, WPARAM, UINT64;
typedef long long LONG_PTR, LPARAM, LRESULT, INT64;
typedef void *HANDLE, *HWND, *HDC, *HINSTANCE, *HICON, *HCURSOR, *HBRUSH, *HFONT, *HBITMAP, *HGDIOBJ, *HMENU, *HGLOBAL, *HRSRC, *HINTERNET, *HPEN;
typedef const WCHAR *LPCWSTR;
typedef WCHAR *LPWSTR;
#define NULL ((void *)0)
#define WINAPI __stdcall
#define IMP __declspec(dllimport)

typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { LONG x, y; } POINT;
typedef struct { HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt; DWORD lPrivate; } MSG;
typedef LRESULT (WINAPI *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef struct { UINT cbSize, style; WNDPROC lpfnWndProc; int cbClsExtra, cbWndExtra; HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground; LPCWSTR lpszMenuName, lpszClassName; HICON hIconSm; } WNDCLASSEXW;
typedef struct { HDC hdc; BOOL fErase; RECT rcPaint; BOOL fRestore, fIncUpdate; BYTE rgbReserved[32]; } PAINTSTRUCT;
typedef struct { DWORD cb; LPWSTR lpReserved, lpDesktop, lpTitle; DWORD dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags; WORD wShowWindow, cbReserved2; BYTE *lpReserved2; HANDLE hStdInput, hStdOutput, hStdError; } STARTUPINFOW;
typedef struct { HANDLE hProcess, hThread; DWORD dwProcessId, dwThreadId; } PROCESS_INFORMATION;
typedef struct { DWORD nLength; void *lpSecurityDescriptor; BOOL bInheritHandle; } SECURITY_ATTRIBUTES;
typedef struct { DWORD lo, hi; } FILETIME;
typedef struct { DWORD attr; FILETIME c, a, w; DWORD sizeHigh, sizeLow; } WIN32_FILE_ATTRIBUTE_DATA;
typedef struct { DWORD attr; FILETIME c, a, w; DWORD sizeHigh, sizeLow, r0, r1; WCHAR name[260]; WCHAR alt[14]; } WIN32_FIND_DATAW;
typedef struct { HWND hwndOwner; void *pidlRoot; LPWSTR pszDisplayName; LPCWSTR lpszTitle; UINT ulFlags; void *lpfn; LPARAM lParam; int iImage; } BROWSEINFOW;
typedef struct { DWORD cbSize, dwFlags; HWND hwndTrack; DWORD dwHoverTime; } TRACKMOUSEEVENT;
typedef struct { DWORD dwStructSize; LPWSTR lpszScheme; DWORD dwSchemeLength; int nScheme; LPWSTR lpszHostName; DWORD dwHostNameLength; WORD nPort; LPWSTR lpszUserName; DWORD dwUserNameLength; LPWSTR lpszPassword; DWORD dwPasswordLength; LPWSTR lpszUrlPath; DWORD dwUrlPathLength; LPWSTR lpszExtraInfo; DWORD dwExtraInfoLength; } URL_COMPONENTS;
typedef struct { UINT GdiplusVersion; void *DebugEventCallback; BOOL SuppressBackgroundThread, SuppressExternalCodecs; } GdiplusStartupInput;

_Static_assert(sizeof(WNDCLASSEXW) == 80, "WNDCLASSEXW");
_Static_assert(sizeof(MSG) == 48, "MSG");
_Static_assert(sizeof(PAINTSTRUCT) == 72, "PAINTSTRUCT");
_Static_assert(sizeof(STARTUPINFOW) == 104, "STARTUPINFOW");
_Static_assert(sizeof(PROCESS_INFORMATION) == 24, "PROCESS_INFORMATION");
_Static_assert(sizeof(URL_COMPONENTS) == 104, "URL_COMPONENTS");
_Static_assert(sizeof(BROWSEINFOW) == 64, "BROWSEINFOW");
_Static_assert(sizeof(WIN32_FIND_DATAW) == 592, "WIN32_FIND_DATAW");
_Static_assert(sizeof(WIN32_FILE_ATTRIBUTE_DATA) == 36, "WIN32_FILE_ATTRIBUTE_DATA");
_Static_assert(sizeof(TRACKMOUSEEVENT) == 24, "TRACKMOUSEEVENT");
_Static_assert(sizeof(SECURITY_ATTRIBUTES) == 24, "SECURITY_ATTRIBUTES");
/* kernel32 */
IMP DWORD WINAPI GetModuleFileNameW(HINSTANCE, LPWSTR, DWORD);
IMP HINSTANCE WINAPI GetModuleHandleW(LPCWSTR);
IMP void WINAPI ExitProcess(UINT);
IMP HANDLE WINAPI CreateFileW(LPCWSTR, DWORD, DWORD, SECURITY_ATTRIBUTES *, DWORD, DWORD, HANDLE);
IMP BOOL WINAPI ReadFile(HANDLE, void *, DWORD, DWORD *, void *);
IMP BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
IMP BOOL WINAPI CloseHandle(HANDLE);
IMP DWORD WINAPI GetFileSize(HANDLE, DWORD *);
IMP BOOL WINAPI DeleteFileW(LPCWSTR);
IMP BOOL WINAPI MoveFileExW(LPCWSTR, LPCWSTR, DWORD);
IMP BOOL WINAPI CreateProcessW(LPCWSTR, LPWSTR, SECURITY_ATTRIBUTES *, SECURITY_ATTRIBUTES *, BOOL, DWORD, void *, LPCWSTR, STARTUPINFOW *, PROCESS_INFORMATION *);
IMP DWORD WINAPI WaitForSingleObject(HANDLE, DWORD);
IMP BOOL WINAPI GetExitCodeProcess(HANDLE, DWORD *);
IMP HANDLE WINAPI CreateThread(void *, SIZE_T, DWORD (WINAPI *)(void *), void *, DWORD, DWORD *);
IMP DWORD WINAPI GetPrivateProfileStringW(LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, DWORD, LPCWSTR);
IMP BOOL WINAPI WritePrivateProfileStringW(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR);
IMP UINT WINAPI GetPrivateProfileIntW(LPCWSTR, LPCWSTR, INT, LPCWSTR);
IMP HRSRC WINAPI FindResourceW(HINSTANCE, LPCWSTR, LPCWSTR);
IMP HGLOBAL WINAPI LoadResource(HINSTANCE, HRSRC);
IMP void *WINAPI LockResource(HGLOBAL);
IMP DWORD WINAPI SizeofResource(HINSTANCE, HRSRC);
IMP HGLOBAL WINAPI GlobalAlloc(UINT, SIZE_T);
IMP void *WINAPI GlobalLock(HGLOBAL);
IMP BOOL WINAPI GlobalUnlock(HGLOBAL);
IMP void *WINAPI HeapAlloc(HANDLE, DWORD, SIZE_T);
IMP void *WINAPI HeapReAlloc(HANDLE, DWORD, void *, SIZE_T);
IMP BOOL WINAPI HeapFree(HANDLE, DWORD, void *);
IMP HANDLE WINAPI GetProcessHeap(void);
IMP BOOL WINAPI CreatePipe(HANDLE *, HANDLE *, SECURITY_ATTRIBUTES *, DWORD);
IMP BOOL WINAPI SetHandleInformation(HANDLE, DWORD, DWORD);
IMP DWORD WINAPI GetFileAttributesW(LPCWSTR);
IMP BOOL WINAPI GetFileAttributesExW(LPCWSTR, int, void *);
IMP int WINAPI MultiByteToWideChar(UINT, DWORD, const char *, int, LPWSTR, int);
IMP int WINAPI WideCharToMultiByte(UINT, DWORD, LPCWSTR, int, char *, int, const char *, BOOL *);
IMP HANDLE WINAPI FindFirstFileW(LPCWSTR, WIN32_FIND_DATAW *);
IMP BOOL WINAPI FindNextFileW(HANDLE, WIN32_FIND_DATAW *);
IMP BOOL WINAPI FindClose(HANDLE);
IMP DWORD WINAPI GetEnvironmentVariableW(LPCWSTR, LPWSTR, DWORD);
IMP void WINAPI Sleep(DWORD);
/* user32 */
IMP WORD WINAPI RegisterClassExW(const WNDCLASSEXW *);
IMP HWND WINAPI CreateWindowExW(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, void *);
IMP BOOL WINAPI ShowWindow(HWND, int);
IMP BOOL WINAPI UpdateWindow(HWND);
IMP BOOL WINAPI GetMessageW(MSG *, HWND, UINT, UINT);
IMP BOOL WINAPI TranslateMessage(const MSG *);
IMP LRESULT WINAPI DispatchMessageW(const MSG *);
IMP LRESULT WINAPI DefWindowProcW(HWND, UINT, WPARAM, LPARAM);
IMP void WINAPI PostQuitMessage(int);
IMP HDC WINAPI BeginPaint(HWND, PAINTSTRUCT *);
IMP BOOL WINAPI EndPaint(HWND, const PAINTSTRUCT *);
IMP BOOL WINAPI InvalidateRect(HWND, const RECT *, BOOL);
IMP HCURSOR WINAPI LoadCursorW(HINSTANCE, LPCWSTR);
IMP HCURSOR WINAPI SetCursor(HCURSOR);
IMP BOOL WINAPI ReleaseCapture(void);
IMP LRESULT WINAPI SendMessageW(HWND, UINT, WPARAM, LPARAM);
IMP BOOL WINAPI PostMessageW(HWND, UINT, WPARAM, LPARAM);
IMP int WINAPI MessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT);
IMP HICON WINAPI LoadIconW(HINSTANCE, LPCWSTR);
IMP BOOL WINAPI DestroyWindow(HWND);
IMP BOOL WINAPI TrackMouseEvent(TRACKMOUSEEVENT *);
IMP HDC WINAPI GetDC(HWND);
IMP int WINAPI ReleaseDC(HWND, HDC);
IMP BOOL WINAPI ScreenToClient(HWND, POINT *);
IMP BOOL WINAPI ClientToScreen(HWND, POINT *);
IMP int WINAPI FillRect(HDC, const RECT *, HBRUSH);
IMP int WINAPI DrawTextW(HDC, LPCWSTR, int, RECT *, UINT);
IMP HMENU WINAPI CreatePopupMenu(void);
IMP BOOL WINAPI AppendMenuW(HMENU, UINT, ULONG_PTR, LPCWSTR);
IMP BOOL WINAPI TrackPopupMenu(HMENU, UINT, int, int, int, HWND, const RECT *);
IMP BOOL WINAPI DestroyMenu(HMENU);
IMP int WINAPI GetSystemMetrics(int);
IMP BOOL WINAPI SetForegroundWindow(HWND);
IMP ULONG_PTR WINAPI SetTimer(HWND, ULONG_PTR, UINT, void *);
/* gdi32 */
IMP HDC WINAPI CreateCompatibleDC(HDC);
IMP HBITMAP WINAPI CreateCompatibleBitmap(HDC, int, int);
IMP HGDIOBJ WINAPI SelectObject(HDC, HGDIOBJ);
IMP BOOL WINAPI BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
IMP BOOL WINAPI DeleteDC(HDC);
IMP BOOL WINAPI DeleteObject(HGDIOBJ);
IMP HFONT WINAPI CreateFontW(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCWSTR);
IMP int WINAPI SetBkMode(HDC, int);
IMP DWORD WINAPI SetTextColor(HDC, DWORD);
IMP HBRUSH WINAPI CreateSolidBrush(DWORD);
IMP int WINAPI GetDeviceCaps(HDC, int);
/* shell32 / ole32 */
IMP HINSTANCE WINAPI ShellExecuteW(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, int);
IMP void *WINAPI SHBrowseForFolderW(BROWSEINFOW *);
IMP BOOL WINAPI SHGetPathFromIDListW(void *, LPWSTR);
IMP LONG WINAPI SHGetFolderPathW(HWND, int, HANDLE, DWORD, LPWSTR);
IMP LONG WINAPI CreateStreamOnHGlobal(HGLOBAL, BOOL, void **);
IMP LONG WINAPI CoInitializeEx(void *, DWORD);
IMP void WINAPI CoTaskMemFree(void *);
/* winhttp */
IMP HINTERNET WINAPI WinHttpOpen(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
IMP HINTERNET WINAPI WinHttpConnect(HINTERNET, LPCWSTR, WORD, DWORD);
IMP HINTERNET WINAPI WinHttpOpenRequest(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR *, DWORD);
IMP BOOL WINAPI WinHttpSendRequest(HINTERNET, LPCWSTR, DWORD, void *, DWORD, DWORD, ULONG_PTR);
IMP BOOL WINAPI WinHttpReceiveResponse(HINTERNET, void *);
IMP BOOL WINAPI WinHttpQueryHeaders(HINTERNET, DWORD, LPCWSTR, void *, DWORD *, DWORD *);
IMP BOOL WINAPI WinHttpQueryDataAvailable(HINTERNET, DWORD *);
IMP BOOL WINAPI WinHttpReadData(HINTERNET, void *, DWORD, DWORD *);
IMP BOOL WINAPI WinHttpCloseHandle(HINTERNET);
IMP BOOL WINAPI WinHttpCrackUrl(LPCWSTR, DWORD, DWORD, URL_COMPONENTS *);
/* gdiplus (flat API) */
IMP int WINAPI GdiplusStartup(ULONG_PTR *, const GdiplusStartupInput *, void *);
IMP int WINAPI GdipCreateBitmapFromStream(void *, void **);
IMP int WINAPI GdipCreateFromHDC(HDC, void **);
IMP int WINAPI GdipDeleteGraphics(void *);
IMP int WINAPI GdipDrawImageRectI(void *, void *, int, int, int, int);
IMP int WINAPI GdipSetSmoothingMode(void *, int);
IMP int WINAPI GdipSetInterpolationMode(void *, int);
IMP int WINAPI GdipCreateSolidFill(DWORD, void **);
IMP int WINAPI GdipDeleteBrush(void *);
IMP int WINAPI GdipCreatePen1(DWORD, float, int, void **);
IMP int WINAPI GdipDeletePen(void *);
IMP int WINAPI GdipCreatePath(int, void **);
IMP int WINAPI GdipDeletePath(void *);
IMP int WINAPI GdipAddPathArcI(void *, int, int, int, int, float, float);
IMP int WINAPI GdipClosePathFigure(void *);
IMP int WINAPI GdipFillPath(void *, void *, void *);
IMP int WINAPI GdipDrawPath(void *, void *, void *);
IMP int WINAPI GdipFillRectangleI(void *, void *, int, int, int, int);
IMP int WINAPI GdipDrawLineI(void *, void *, int, int, int, int);
IMP int WINAPI GdipGetImageWidth(void *, UINT *);
IMP int WINAPI GdipGetImageHeight(void *, UINT *);

int _fltused = 1;
void *memset(void *d, int c, SIZE_T n) { BYTE *p = d; while (n--) *p++ = (BYTE)c; return d; }
void *memcpy(void *d, const void *s, SIZE_T n) { BYTE *p = d; const BYTE *q = s; while (n--) *p++ = *q++; return d; }

/* ---------------------------------------------------------------- small helpers */
static HANDLE heap;
static void *mem(SIZE_T n) { return HeapAlloc(heap, 8 /* zero */, n ? n : 1); }
static void *remem(void *p, SIZE_T n) { return p ? HeapReAlloc(heap, 8, p, n) : mem(n); }
static void mfree(void *p) { if (p) HeapFree(heap, 0, p); }
static int wlen(LPCWSTR s) { int n = 0; while (s && s[n]) n++; return n; }
static void wcpy(LPWSTR d, LPCWSTR s, int cap) { int i = 0; if (s) for (; s[i] && i < cap - 1; i++) d[i] = s[i]; d[i] = 0; }
static void wcat(LPWSTR d, LPCWSTR s, int cap) { int n = wlen(d); wcpy(d + n, s, cap - n); }
static int weq(LPCWSTR a, LPCWSTR b) { int i = 0; for (; a[i] && b[i]; i++) if (a[i] != b[i]) return 0; return a[i] == b[i]; }
static WCHAR lw(WCHAR c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }
static int wieq(LPCWSTR a, LPCWSTR b) { int i = 0; for (; a[i] && b[i]; i++) if (lw(a[i]) != lw(b[i])) return 0; return a[i] == b[i]; }
static int wends(LPCWSTR s, LPCWSTR e) { int a = wlen(s), b = wlen(e); return a >= b && wieq(s + a - b, e); }
static int wcontains(LPCWSTR s, LPCWSTR t) { int n = wlen(s), m = wlen(t); for (int i = 0; i + m <= n; i++) { int k = 0; while (k < m && lw(s[i + k]) == lw(t[k])) k++; if (k == m) return 1; } return 0; }
static void witoa(int v, LPWSTR o) { WCHAR t[16]; int n = 0, neg = v < 0; unsigned u = neg ? -v : v; do { t[n++] = '0' + u % 10; u /= 10; } while (u); int i = 0; if (neg) o[i++] = '-'; while (n) o[i++] = t[--n]; o[i] = 0; }
static int watoi(LPCWSTR s) { int v = 0; while (*s >= '0' && *s <= '9') v = v * 10 + (*s++ - '0'); return v; }
static LPWSTR u8w(const char *s, int n) { int k = MultiByteToWideChar(65001, 0, s, n, NULL, 0); LPWSTR w = mem((k + 1) * 2); MultiByteToWideChar(65001, 0, s, n, w, k); w[k] = 0; return w; }

/* ---------------------------------------------------------------- state */
enum { ST_IDLE, ST_CHECKING, ST_DOWNLOADING, ST_INSTALLING, ST_SELFUPDATE };
enum { TAB_HOME, TAB_SETTINGS, TAB_LOGS, TAB_FORGE };
static HWND hwnd;
static HINSTANCE hinst;
static float dpi = 1.0f;
#define S(x) ((int)((x) * dpi + 0.5f))
static int tab;
static volatile int state, progress;          /* progress 0..1000, -1 = unknown */
static WCHAR selfpath[520], selfdir[520], gamedir[520], inipath[560], kufini[560];
static WCHAR installed[64], installed_note[160];
static WCHAR status[256];
static WCHAR rel_tag[64], rel_name[128], rel_date[16], rel_url[1024], rel_page[512];
static LPWSTR rel_body;                        /* news text */
static WCHAR launcher_url[1024], launcher_newver[32];
static WCHAR crus_url[1024];                   /* Crusaders missions installer (any visible release) */
static WCHAR forge_url[1024], forge_msg[200];  /* 1.0.3 (USER): Bersia Map Forge - newest release that has it, test builds included */
static volatile int forge_busy;
static LPWSTR install_log;                     /* last installer output */
static int news_scroll, news_h;
static int opt_test, opt_close = 1, opt_enc;
static int gx_an = 16, gx_sr = 2, gx_sm = 512, gx_dd = 150, gx_aa = 0;
static int hover = -1, pressed = -1;
static void *img_bg, *img_logo;
static HFONT f_tab, f_btn, f_head, f_text, f_small, f_big;

/* hot regions rebuilt at each paint */
typedef struct { RECT r; int id; } HOT;
static HOT hots[64];
static int nhots;
static void hot(int x, int y, int w, int h, int id) { if (nhots < 64) { hots[nhots].r.left = x; hots[nhots].r.top = y; hots[nhots].r.right = x + w; hots[nhots].r.bottom = y + h; hots[nhots].id = id; nhots++; } }
enum { H_MIN = 1, H_CLOSE, H_TAB0, H_TAB1, H_TAB2, H_TAB3, H_FORGE, H_FORGEDIR, H_MAIN, H_MORE, H_CHANGELOG, H_SELFUPD, H_NEWS,
       H_TEST, H_ENC, H_CLOSEOPT, H_FOLDER, H_GX = 30, H_LOGNET = 50, H_LOGDIR, H_LOGDUMP, H_LOGCOPY };
#define WM_APP_UPDATE (0x8000 + 1)

/* ---------------------------------------------------------------- files / ini */
static int exists(LPCWSTR p) { return GetFileAttributesW(p) != 0xFFFFFFFF; }
static void path_join(LPWSTR out, LPCWSTR dir, LPCWSTR name) { wcpy(out, dir, 520); int n = wlen(out); if (n && out[n - 1] != '\\') wcat(out, L"\\", 520); wcat(out, name, 520); }
static int game_ok(LPCWSTR dir) { WCHAR p[560]; path_join(p, dir, L"Heroes.exe"); if (!exists(p)) return 0; path_join(p, dir, L"Data\\Mission"); return exists(p); }
static void ini_get(LPCWSTR sec, LPCWSTR key, LPWSTR out, int cap) { GetPrivateProfileStringW(sec, key, L"", out, cap, inipath); }
static void ini_set(LPCWSTR sec, LPCWSTR key, LPCWSTR v) { WritePrivateProfileStringW(sec, key, v, inipath); }
static void ini_seti(LPCWSTR sec, LPCWSTR key, int v) { WCHAR t[16]; witoa(v, t); ini_set(sec, key, t); }

static BYTE *read_all(LPCWSTR p, DWORD *len)
{
    HANDLE h = CreateFileW(p, 0x80000000, 3, NULL, 3, 0x80, NULL);
    if (h == (HANDLE)-1) return NULL;
    DWORD n = GetFileSize(h, NULL), got = 0;
    BYTE *b = mem(n + 2);
    if (n != 0xFFFFFFFF) ReadFile(h, b, n, &got, NULL);
    CloseHandle(h);
    if (len) *len = got;
    return b;
}
static void exe_stamp(LPWSTR out)   /* size + write time of Heroes.exe, to notice Steam restoring it */
{
    WCHAR p[560]; WIN32_FILE_ATTRIBUTE_DATA a; path_join(p, gamedir, L"Heroes.exe");
    out[0] = 0;
    if (!GetFileAttributesExW(p, 0, &a)) return;
    WCHAR t[16]; witoa((int)a.sizeLow, t); wcat(out, t, 64); wcat(out, L"-", 64); witoa((int)(a.w.lo & 0x7fffffff), t); wcat(out, t, 64);
}
static void detect_installed(void)
{
    WCHAR p[560], stamp[64], st2[64];
    installed[0] = 0; installed_note[0] = 0;
    if (!gamedir[0]) { wcpy(installed_note, L"Game folder not set", 160); return; }
    path_join(p, gamedir, L"Heroes.exe");
    DWORD n = 0; BYTE *b = read_all(p, &n);
    if (!b) { wcpy(installed_note, L"Heroes.exe not found", 160); return; }
    const char *key = "Battle for Bersia MP v"; int kl = 22, found = -1;
    for (DWORD i = 0; i + kl + 8 < n && found < 0; i++) { int k = 0; while (k < kl && b[i + k] == (BYTE)key[k]) k++; if (k == kl) found = (int)i; }
    WCHAR ver[32] = {0};
    if (found >= 0) { int j = 0; for (int i = found + kl - 1; j < 20 && b[i] > 32 && b[i] < 127 && b[i] != 'N'; i++) ver[j++] = b[i]; ver[j] = 0; }
    mfree(b);
    if (!ver[0]) { wcpy(installed_note, L"Not installed", 160); return; }
    ini_get(L"Installed", L"Tag", installed, 64);
    ini_get(L"Installed", L"Exe", stamp, 64);
    exe_stamp(st2);
    if (!installed[0] || !weq(stamp, st2)) { wcpy(installed, ver, 64); wcpy(installed_note, L"installed outside the launcher", 160); }
}

static void kuf_ini_path(void)
{
    WCHAR docs[520]; docs[0] = 0;
    SHGetFolderPathW(NULL, 5 /* CSIDL_PERSONAL */, NULL, 0, docs);
    path_join(kufini, docs, L"KUF2 Heroes\\KUFH.ini");
}
static void gx_read(void)
{
    gx_an = GetPrivateProfileIntW(L"BattleForBersia", L"Anisotropy", 16, kufini);
    gx_sr = GetPrivateProfileIntW(L"BattleForBersia", L"ShadowRange", 2, kufini);
    gx_aa = GetPrivateProfileIntW(L"BattleForBersia", L"AntiAliasing", 0, kufini);
    gx_dd = GetPrivateProfileIntW(L"BattleForBersia", L"DrawDistance", 150, kufini);
    gx_sm = GetPrivateProfileIntW(L"General", L"ShadowMapSize", 512, kufini);
}
static void gx_write(void)
{
    WCHAR t[16];
    witoa(gx_an, t); WritePrivateProfileStringW(L"BattleForBersia", L"Anisotropy", t, kufini);
    witoa(gx_sr, t); WritePrivateProfileStringW(L"BattleForBersia", L"ShadowRange", t, kufini);
    witoa(gx_aa, t); WritePrivateProfileStringW(L"BattleForBersia", L"AntiAliasing", t, kufini);
    witoa(gx_dd, t); WritePrivateProfileStringW(L"BattleForBersia", L"DrawDistance", t, kufini);
    witoa(gx_sm, t); WritePrivateProfileStringW(L"General", L"ShadowMapSize", t, kufini);
}

/* ---------------------------------------------------------------- HTTP */
static HINTERNET hs;
static BYTE *http_get(LPCWSTR url, DWORD *outlen, HANDLE file)   /* file != NULL: stream to file, report progress */
{
    URL_COMPONENTS uc; WCHAR host[256], path[2048];
    memset(&uc, 0, sizeof uc); uc.dwStructSize = sizeof uc;
    uc.lpszHostName = host; uc.dwHostNameLength = 256; uc.lpszUrlPath = path; uc.dwUrlPathLength = 2048;
    if (!WinHttpCrackUrl(url, 0, 0, &uc)) return NULL;
    if (!hs) hs = WinHttpOpen(L"BattleForBersiaLauncher/" LAUNCHER_VER, 4, NULL, NULL, 0);
    if (!hs) hs = WinHttpOpen(L"BattleForBersiaLauncher/" LAUNCHER_VER, 0, NULL, NULL, 0);
    if (!hs) return NULL;
    HINTERNET hc = WinHttpConnect(hs, host, uc.nPort, 0), hr = NULL;
    BYTE *buf = NULL; DWORD len = 0, cap = 0, ok = 0;
    if (hc) hr = WinHttpOpenRequest(hc, L"GET", path, NULL, NULL, NULL, uc.nScheme == 2 ? 0x800000 : 0);
    if (hr && WinHttpSendRequest(hr, L"Accept: application/vnd.github+json, application/octet-stream\r\n", (DWORD)-1, NULL, 0, 0, 0) && WinHttpReceiveResponse(hr, NULL)) {
        DWORD code = 0, sz = 4, total = 0;
        WinHttpQueryHeaders(hr, 19 | 0x20000000, NULL, &code, &sz, NULL);
        sz = 4; if (!WinHttpQueryHeaders(hr, 5 | 0x20000000, NULL, &total, &sz, NULL)) total = 0;
        if (code == 200) {
            ok = 1;
            DWORD acc = 0;
            for (;;) {
                DWORD avail = 0, got = 0;
                if (!WinHttpQueryDataAvailable(hr, &avail)) { ok = 0; break; }
                if (!avail) break;
                if (len + avail + 1 > cap) { cap = (len + avail + 1) * 2; buf = remem(buf, cap); }
                if (!WinHttpReadData(hr, buf + len, avail, &got)) { ok = 0; break; }
                if (file) { DWORD w = 0; WriteFile(file, buf + len, got, &w, NULL); if (w != got) { ok = 0; break; } }
                else len += got;
                if (file && total) { acc += got; progress = (int)((UINT64)acc * 1000 / total); if (progress > 1000) progress = 1000; PostMessageW(hwnd, WM_APP_UPDATE, 0, 0); }
            }
        }
    }
    if (hr) WinHttpCloseHandle(hr);
    if (hc) WinHttpCloseHandle(hc);
    if (!ok) { mfree(buf); return NULL; }
    if (!buf) buf = mem(1);
    buf[len] = 0;
    if (outlen) *outlen = len;
    return buf;
}

/* ---------------------------------------------------------------- tiny JSON reader (GitHub releases list) */
static const char *js;
static void jws(void) { while (*js == ' ' || *js == '\n' || *js == '\r' || *js == '\t') js++; }
static char *jstr(void)   /* at '"': returns heap UTF-8 string, unescaped */
{
    if (*js != '"') return NULL;
    js++;
    int cap = 64, n = 0; char *o = mem(cap);
    while (*js && *js != '"') {
        unsigned c = (BYTE)*js++;
        if (c == '\\') {
            char e = *js++;
            if (e == 'n') c = '\n'; else if (e == 'r') c = '\r'; else if (e == 't') c = '\t'; else if (e == 'b' || e == 'f') c = ' ';
            else if (e == 'u') {
                unsigned v = 0; for (int i = 0; i < 4 && *js; i++) { char h = *js++; v = v * 16 + (h <= '9' ? h - '0' : (h | 32) - 'a' + 10); }
                if (v >= 0xD800 && v <= 0xDFFF) v = '?';
                char t[4]; int k = 0;
                if (v < 0x80) t[k++] = (char)v; else if (v < 0x800) { t[k++] = (char)(0xC0 | v >> 6); t[k++] = (char)(0x80 | (v & 63)); }
                else { t[k++] = (char)(0xE0 | v >> 12); t[k++] = (char)(0x80 | ((v >> 6) & 63)); t[k++] = (char)(0x80 | (v & 63)); }
                for (int i = 0; i < k; i++) { if (n + 2 > cap) { cap *= 2; o = remem(o, cap); } o[n++] = t[i]; }
                continue;
            } else c = (BYTE)e;
        }
        if (n + 2 > cap) { cap *= 2; o = remem(o, cap); }
        o[n++] = (char)c;
    }
    if (*js == '"') js++;
    o[n] = 0;
    return o;
}
static void jskip(void)
{
    jws();
    if (*js == '"') { mfree(jstr()); return; }
    if (*js == '{' || *js == '[') {
        int d = 0;
        do {
            if (*js == '"') { mfree(jstr()); continue; }
            if (*js == '{' || *js == '[') d++;
            else if (*js == '}' || *js == ']') d--;
            js++;
        } while (*js && d > 0);
        return;
    }
    while (*js && *js != ',' && *js != '}' && *js != ']') js++;
}
static int seq(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
static int s_ends(const char *s, const char *e) { int a = 0, b = 0; while (s[a]) a++; while (e[b]) b++; if (a < b) return 0; for (int i = 0; i < b; i++) if ((s[a - b + i] | 32) != (e[i] | 32)) return 0; return 1; }
static int s_has(const char *s, const char *t) { for (; *s; s++) { int k = 0; while (t[k] && (s[k] | 32) == (t[k] | 32)) k++; if (!t[k]) return 1; } return 0; }
static int ver3(const char *s, int *a, int *b, int *c)   /* "...-1.2.3.exe" */
{
    const char *p = s; while (*p && !(*p >= '0' && *p <= '9')) p++;
    *a = *b = *c = 0; if (!*p) return 0;
    while (*p >= '0' && *p <= '9') *a = *a * 10 + *p++ - '0'; if (*p++ != '.') return 1;
    while (*p >= '0' && *p <= '9') *b = *b * 10 + *p++ - '0'; if (*p++ != '.') return 1;
    while (*p >= '0' && *p <= '9') *c = *c * 10 + *p++ - '0';
    return 1;
}
static void set_w(LPWSTR dst, int cap, const char *s) { LPWSTR w = u8w(s, -1); wcpy(dst, w, cap); mfree(w); }

static LPWSTR tidy_body(const char *md)   /* light markdown clean-up for the news panel */
{
    int n = 0; while (md[n]) n++;
    char *o = mem(n * 3 + 8); int k = 0, bol = 1;
    for (int i = 0; i < n; i++) {
        char c = md[i];
        if (c == '\r') continue;
        if (bol) {
            while (md[i] == '#') i++;
            if (md[i] == ' ' && i > 0 && md[i - 1] == '#') i++;
            if ((md[i] == '-' || md[i] == '*') && md[i + 1] == ' ') { o[k++] = (char)0xE2; o[k++] = (char)0x80; o[k++] = (char)0xA2; i++; }
            c = md[i];
            if (!c) break;
        }
        bol = (c == '\n');
        if (c == '*' || c == '`' || (c == '_' && (md[i + 1] == '_' || (i && md[i - 1] == '_')))) continue;
        o[k++] = c;
    }
    o[k] = 0;
    LPWSTR w = u8w(o, -1); mfree(o);
    return w;
}

static int parse_releases(const char *json)
{
    int found = 0;
    js = json; jws();
    if (*js != '[') return 0;
    js++;
    while (*js) {
        jws();
        if (*js == ']') break;
        if (*js == ',') { js++; continue; }
        if (*js != '{') { jskip(); continue; }
        js++;
        char *tag = 0, *name = 0, *body = 0, *date = 0, *page = 0, *bat = 0, *bat_e = 0, *lurl = 0, *lname = 0, *crus = 0, *forge = 0;
        int pre = 0, draft = 0;
        while (*js) {
            jws();
            if (*js == '}') { js++; break; }
            if (*js == ',') { js++; continue; }
            char *key = jstr(); jws(); if (*js == ':') js++; jws();
            if (!key) break;
            if (seq(key, "tag_name")) tag = jstr();
            else if (seq(key, "name") && *js == '"') name = jstr();
            else if (seq(key, "body") && *js == '"') body = jstr();
            else if (seq(key, "published_at") && *js == '"') date = jstr();
            else if (seq(key, "html_url") && *js == '"') page = jstr();
            else if (seq(key, "prerelease")) { pre = *js == 't'; jskip(); }
            else if (seq(key, "draft")) { draft = *js == 't'; jskip(); }
            else if (seq(key, "assets") && *js == '[') {
                js++;
                while (*js) {
                    jws();
                    if (*js == ']') { js++; break; }
                    if (*js == ',') { js++; continue; }
                    if (*js != '{') { jskip(); continue; }
                    js++;
                    char *an = 0, *au = 0;
                    while (*js) {
                        jws();
                        if (*js == '}') { js++; break; }
                        if (*js == ',') { js++; continue; }
                        char *k2 = jstr(); jws(); if (*js == ':') js++; jws();
                        if (!k2) break;
                        if (seq(k2, "name") && *js == '"') { mfree(an); an = jstr(); }
                        else if (seq(k2, "browser_download_url") && *js == '"') { mfree(au); au = jstr(); }
                        else jskip();
                        mfree(k2);
                    }
                    if (an && au) {
                        if (s_has(an, "Crusaders") && s_ends(an, ".bat")) { if (!crus) { crus = au; au = 0; } }
                        else if (s_has(an, "Map_Forge") && s_ends(an, ".html")) { if (!forge) { forge = au; au = 0; } }
                        else if (s_ends(an, "E_Install.bat")) { if (!bat_e) { bat_e = au; au = 0; } }
                        else if (s_ends(an, "_Install.bat")) { if (!bat) { bat = au; au = 0; } }
                        else if (s_has(an, "BattleForBersiaLauncher") && s_ends(an, ".exe")) { if (!lurl) { lurl = au; lname = an; an = 0; au = 0; } }
                    }
                    mfree(an); mfree(au);
                }
            } else jskip();
            mfree(key);
        }
        if (!draft && forge && !forge_url[0]) set_w(forge_url, 1024, forge);   /* the map editor: newest release that has it, for everyone */
        if (!draft && (!pre || opt_test)) {
            if (lurl && !launcher_url[0]) {
                int a, b, c;
                if (ver3(lname, &a, &b, &c) && (a > LV_MAJ || (a == LV_MAJ && (b > LV_MIN || (b == LV_MIN && c > LV_PAT))))) {
                    set_w(launcher_url, 1024, lurl);
                    WCHAR t[16]; launcher_newver[0] = 0;
                    witoa(a, t); wcat(launcher_newver, t, 32); wcat(launcher_newver, L".", 32); witoa(b, t); wcat(launcher_newver, t, 32); wcat(launcher_newver, L".", 32); witoa(c, t); wcat(launcher_newver, t, 32);
                }
            }
            if (crus && !crus_url[0]) set_w(crus_url, 1024, crus);
            char *use = opt_enc && bat_e ? bat_e : bat;
            if (!found && use && tag) {
                found = 1;
                set_w(rel_tag, 64, tag);
                set_w(rel_name, 128, name && name[0] ? name : tag);
                rel_date[0] = 0; if (date) { char d[11]; for (int i = 0; i < 10 && date[i]; i++) d[i] = date[i], d[i + 1] = 0; set_w(rel_date, 16, d); }
                set_w(rel_url, 1024, use);
                rel_page[0] = 0; if (page) set_w(rel_page, 512, page);
                mfree(rel_body); rel_body = tidy_body(body ? body : "");
            }
        }
        mfree(tag); mfree(name); mfree(body); mfree(date); mfree(page); mfree(bat); mfree(bat_e); mfree(lurl); mfree(lname); mfree(crus); mfree(forge);
    }
    return found;
}

/* ---------------------------------------------------------------- background work */
static void set_status(LPCWSTR s) { wcpy(status, s, 256); PostMessageW(hwnd, WM_APP_UPDATE, 0, 0); }
static DWORD WINAPI check_thread(void *arg)
{
    (void)arg;
    state = ST_CHECKING; set_status(L"Checking for updates...");
    launcher_url[0] = 0; crus_url[0] = 0; forge_url[0] = 0;
    DWORD n = 0;
    BYTE *j = http_get(L"https://api.github.com" REPO_API, &n, NULL);
    if (!j) { state = ST_IDLE; set_status(L"Could not reach GitHub - you can still play."); return 0; }
    rel_tag[0] = 0;
    int ok = parse_releases((const char *)j);
    mfree(j);
    state = ST_IDLE;
    if (!ok) set_status(opt_test ? L"No release found yet." : L"No release found yet (testers: tick \"Receive test builds\").");
    else if (installed[0] && !installed_note[0] && weq(installed, rel_tag)) set_status(L"Up to date.");
    else { WCHAR t[200]; wcpy(t, L"New version available: ", 200); wcat(t, rel_name, 200); set_status(t); }
    return 0;
}

static LPWSTR run_capture(LPWSTR cmd, LPCWSTR dir, DWORD *exitcode, LPCWSTR input)
{
    SECURITY_ATTRIBUTES sa = { sizeof sa, NULL, 1 };
    HANDLE rd = NULL, wr = NULL;
    if (!CreatePipe(&rd, &wr, &sa, 0)) return NULL;
    SetHandleInformation(rd, 1, 0);
    HANDLE nul = (HANDLE)-1, inw = NULL;
    if (input) {   /* answers for the installer's questions, e.g. the Crusaders folder */
        if (CreatePipe(&nul, &inw, &sa, 0)) {
            SetHandleInformation(inw, 1, 0);
            int k = WideCharToMultiByte(1, 0, input, -1, NULL, 0, NULL, NULL);
            char *a = mem(k + 1); WideCharToMultiByte(1, 0, input, -1, a, k, NULL, NULL);
            DWORD w = 0; WriteFile(inw, a, (DWORD)(k - 1), &w, NULL); mfree(a);
            CloseHandle(inw);
        }
    } else nul = CreateFileW(L"NUL", 0x80000000, 3, &sa, 3, 0, NULL);
    STARTUPINFOW si; PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof si); memset(&pi, 0, sizeof pi);
    si.cb = sizeof si; si.dwFlags = 0x100; si.hStdInput = nul; si.hStdOutput = wr; si.hStdError = wr;
    BOOL ok = CreateProcessW(NULL, cmd, NULL, NULL, 1, 0x08000000, NULL, dir, &si, &pi);
    CloseHandle(wr);
    if (nul != (HANDLE)-1) CloseHandle(nul);
    if (!ok) { CloseHandle(rd); return NULL; }
    char *out = NULL; DWORD len = 0, cap = 0, got;
    for (;;) {
        if (len + 4097 > cap) { cap = (len + 4097) * 2; out = remem(out, cap); }
        if (!ReadFile(rd, out + len, 4096, &got, NULL) || !got) break;
        len += got;
    }
    CloseHandle(rd);
    WaitForSingleObject(pi.hProcess, 0xFFFFFFFF);
    if (exitcode) GetExitCodeProcess(pi.hProcess, exitcode);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    if (!out) return NULL;
    out[len] = 0;
    int k = MultiByteToWideChar(1 /* OEM */, 0, out, (int)len, NULL, 0);
    LPWSTR w = mem((k + 1) * 2); MultiByteToWideChar(1, 0, out, (int)len, w, k); w[k] = 0;
    mfree(out);
    return w;
}

static int install_mode;   /* 0 install/update/repair, 1 uninstall */
static int crus_installed(void) { WCHAR p[560]; path_join(p, gamedir, L"BattleForBersiaMP_Crusaders_files.txt"); return gamedir[0] && exists(p); }
static int crus_mode;              /* 0 install (find Crusaders automatically), 1 remove, 2 install with crus_dir typed in */
static WCHAR crus_dir[520];
#define WM_APP_CRUSDIR (0x8000 + 2)
static DWORD WINAPI crus_thread(void *arg)
{
    (void)arg;
    WCHAR bat[560], tmp[560], cmd[1400], in[600];
    path_join(bat, gamedir, L"BattleForBersiaMP_Crusaders_Install.bat");
    if (crus_mode != 1) {
        if (!crus_url[0]) { state = ST_IDLE; set_status(L"No Crusaders installer found in the releases."); return 0; }
        path_join(tmp, gamedir, L"BattleForBersiaMP_Crusaders_Install.download");
        state = ST_DOWNLOADING; progress = 0; set_status(L"Downloading the Crusaders missions installer...");
        HANDLE f = CreateFileW(tmp, 0x40000000, 0, NULL, 2, 0x80, NULL);
        if (f == (HANDLE)-1) { state = ST_IDLE; set_status(L"Cannot write in the game folder."); return 0; }
        BYTE *r = http_get(crus_url, NULL, f);
        CloseHandle(f);
        if (!r) { DeleteFileW(tmp); state = ST_IDLE; set_status(L"Download failed - try again."); return 0; }
        mfree(r);
        if (!MoveFileExW(tmp, bat, 1)) { state = ST_IDLE; set_status(L"Could not save the installer in the game folder."); return 0; }
    } else if (!exists(bat)) { state = ST_IDLE; set_status(L"Crusaders installer not found in the game folder."); return 0; }
    state = ST_INSTALLING; progress = -1;
    set_status(crus_mode == 1 ? L"Removing the Crusaders missions..." : L"Building the Crusaders missions from your Crusaders install (this can take a minute)...");
    wcpy(cmd, L"cmd.exe /d /c \"\"", 1400); wcat(cmd, bat, 1400); wcat(cmd, crus_mode == 1 ? L"\" uninstall\"" : L"\"\"", 1400);
    LPCWSTR input = NULL;
    if (crus_mode == 2) { wcpy(in, L"n\r\n", 600); wcat(in, crus_dir, 600); wcat(in, L"\r\n\r\n", 600); input = in; }
    DWORD code = 0;
    LPWSTR out = run_capture(cmd, gamedir, &code, input);
    mfree(install_log); install_log = out;
    state = ST_IDLE;
    if (crus_mode == 1) set_status(out && wcontains(out, L"Removed") ? L"Crusaders missions removed." : L"Nothing removed - see the LOGS tab.");
    else if (out && wcontains(out, L"Crusaders files are installed.")) set_status(L"Crusaders missions installed - Gerald, Lucretia, Regnier and Kendal are in the Co-op list.");
    else if (out && wcontains(out, L"Cancelled.") && crus_mode == 0) { set_status(L"Could not find The Crusaders automatically."); PostMessageW(hwnd, WM_APP_CRUSDIR, 0, 0); }
    else set_status(L"Crusaders missions not installed - see the LOGS tab (it needs the Steam version of The Crusaders, English voices).");
    return 0;
}
static void start_crus(int mode)
{
    if (state != ST_IDLE || !gamedir[0]) return;
    crus_mode = mode; state = ST_DOWNLOADING;
    CreateThread(NULL, 0, crus_thread, NULL, 0, NULL);
}
static DWORD WINAPI install_thread(void *arg)
{
    (void)arg;
    WCHAR bat[560], tmp[560], cmd[1400];
    path_join(bat, gamedir, L"BattleForBersiaMP_Install.bat");
    if (install_mode == 0) {
        if (!rel_url[0]) { state = ST_IDLE; set_status(L"No installer to download - check your internet connection."); return 0; }
        path_join(tmp, gamedir, L"BattleForBersiaMP_Install.download");
        state = ST_DOWNLOADING; progress = 0; set_status(L"Downloading...");
        HANDLE f = CreateFileW(tmp, 0x40000000, 0, NULL, 2, 0x80, NULL);
        if (f == (HANDLE)-1) { state = ST_IDLE; set_status(L"Cannot write in the game folder (try running the launcher as administrator)."); return 0; }
        BYTE *r = http_get(rel_url, NULL, f);
        CloseHandle(f);
        if (!r) { DeleteFileW(tmp); state = ST_IDLE; set_status(L"Download failed - try again."); return 0; }
        mfree(r);
        if (!MoveFileExW(tmp, bat, 1)) { state = ST_IDLE; set_status(L"Could not save the installer in the game folder."); return 0; }
    } else if (!exists(bat)) { state = ST_IDLE; set_status(L"Installer not found - use Repair first, or Steam > Verify integrity of game files."); return 0; }
    state = ST_INSTALLING; progress = -1; set_status(install_mode ? L"Removing the mod..." : L"Installing...");
    wcpy(cmd, L"cmd.exe /d /c \"\"", 1400); wcat(cmd, bat, 1400); wcat(cmd, install_mode ? L"\" uninstall\"" : L"\"\"", 1400);
    DWORD code = 0;
    LPWSTR out = run_capture(cmd, gamedir, &code, NULL);
    mfree(install_log); install_log = out;
    state = ST_IDLE;
    if (install_mode == 0) {
        if (out && wcontains(out, L"is installed.")) {
            WCHAR st[64]; exe_stamp(st);
            ini_set(L"Installed", L"Tag", rel_tag); ini_set(L"Installed", L"Exe", st);
            detect_installed();
            set_status(L"Installed - ready to play.");
        } else if (out && wcontains(out, L"close the game")) set_status(L"Please close the game first, then try again.");
        else set_status(L"Install did not finish - see the LOGS tab for the details.");
    } else {
        ini_set(L"Installed", L"Tag", NULL); ini_set(L"Installed", L"Exe", NULL);
        detect_installed();
        set_status(out && wcontains(out, L"Restored") ? L"Mod removed - the original game files are back." : L"Nothing removed - see the LOGS tab.");
    }
    return 0;
}

/* 1.0.3 (USER: a MAP EDITOR tab after LOGS that opens the map editor, downloaded with the launcher into the KUF Heroes folder):
   Bersia_Map_Forge.html is fetched from the newest release that has it (again whenever that release changes) and opened in the
   default web browser. It is one self-contained page - no install, works offline. */
static void forge_path(LPWSTR out) { path_join(out, gamedir[0] ? gamedir : selfdir, L"Bersia_Map_Forge.html"); }
static void forge_set(LPCWSTR m) { wcpy(forge_msg, m, 200); PostMessageW(hwnd, WM_APP_UPDATE, 0, 0); }
static DWORD WINAPI forge_thread(void *arg)
{
    WCHAR dst[560], tmp[560], have[1024];
    int open = arg != NULL;
    forge_path(dst);
    ini_get(L"MapForge", L"Url", have, 1024);
    if (forge_url[0] && (!exists(dst) || !weq(have, forge_url))) {
        path_join(tmp, gamedir[0] ? gamedir : selfdir, L"Bersia_Map_Forge.download");
        forge_set(L"Downloading the latest map editor...");
        HANDLE f = CreateFileW(tmp, 0x40000000, 0, NULL, 2, 0x80, NULL);
        BYTE *r = f != (HANDLE)-1 ? http_get(forge_url, NULL, f) : NULL;
        if (f != (HANDLE)-1) CloseHandle(f);
        progress = 0;
        if (r && MoveFileExW(tmp, dst, 1)) { mfree(r); ini_set(L"MapForge", L"Url", forge_url); }
        else { mfree(r); DeleteFileW(tmp); forge_set(exists(dst) ? L"Could not download the newest map editor - opening the one you have." : L"Could not download the map editor - check your connection."); }
    }
    if (exists(dst)) {
        if (open) { ShellExecuteW(NULL, L"open", dst, NULL, NULL, 1); forge_set(L"Opened in your web browser."); }
        else forge_set(L"Ready - the newest map editor is in your game folder.");
    } else if (!forge_url[0]) forge_set(L"Map editor not found online yet - check your connection.");
    forge_busy = 0;
    return 0;
}
static void start_forge(int open) { if (!forge_busy) { forge_busy = 1; CreateThread(NULL, 0, forge_thread, open ? (void *)1 : NULL, 0, NULL); } }
static DWORD WINAPI selfupdate_thread(void *arg)
{
    (void)arg;
    WCHAR nw[560], old[560];
    wcpy(nw, selfpath, 560); wcat(nw, L".new", 560);
    wcpy(old, selfpath, 560); wcat(old, L".old", 560);
    state = ST_SELFUPDATE; progress = 0; set_status(L"Updating the launcher...");
    HANDLE f = CreateFileW(nw, 0x40000000, 0, NULL, 2, 0x80, NULL);
    BYTE *r = f != (HANDLE)-1 ? http_get(launcher_url, NULL, f) : NULL;
    if (f != (HANDLE)-1) CloseHandle(f);
    if (!r) { DeleteFileW(nw); state = ST_IDLE; set_status(L"Launcher update failed."); return 0; }
    mfree(r);
    DeleteFileW(old);
    if (!MoveFileExW(selfpath, old, 1) || !MoveFileExW(nw, selfpath, 1)) { state = ST_IDLE; set_status(L"Launcher update failed (file in use)."); return 0; }
    ShellExecuteW(NULL, L"open", selfpath, NULL, selfdir, 1);
    PostMessageW(hwnd, 0x10 /* WM_CLOSE */, 0, 0);
    return 0;
}

/* ---------------------------------------------------------------- game start through Steam */
static int steam_appid(void)
{
    /* game folder = <lib>\steamapps\common\<installdir>: find the appmanifest naming that installdir */
    WCHAR dir[520], name[260], pat[560];
    wcpy(dir, gamedir, 520);
    int n = wlen(dir); while (n && dir[n - 1] == '\\') dir[--n] = 0;
    int s = n; while (s && dir[s - 1] != '\\') s--;
    wcpy(name, dir + s, 260);
    if (s < 1) return 0;
    dir[s - 1] = 0;                                  /* ...\steamapps\common */
    n = wlen(dir); while (n && dir[n - 1] != '\\') n--;
    if (!n) return 0;
    dir[n - 1] = 0;                                  /* ...\steamapps */
    path_join(pat, dir, L"appmanifest_*.acf");
    WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
    if (h == (HANDLE)-1) return 0;
    int id = 0;
    do {
        WCHAR p[560]; path_join(p, dir, fd.name);
        DWORD len = 0; BYTE *b = read_all(p, &len);
        if (!b) continue;
        LPWSTR w = u8w((const char *)b, (int)len); mfree(b);
        /* "installdir"		"Kingdom Under Fire Heroes" */
        int L = wlen(w);
        for (int i = 0; i < L && !id; i++) {
            if (w[i] == '"' && i + 12 < L) {
                LPCWSTR k = L"\"installdir\""; int m = 0; while (k[m] && lw(w[i + m]) == lw(k[m])) m++;
                if (!k[m]) {
                    int j = i + m; while (j < L && w[j] != '"') j++; j++;
                    int e = j; while (e < L && w[e] != '"') e++;
                    WCHAR v[260]; int q = 0; for (int t = j; t < e && q < 259; t++) v[q++] = w[t]; v[q] = 0;
                    if (wieq(v, name)) {
                        LPCWSTR a = L"\"appid\"";
                        for (int t = 0; t < L; t++) {
                            int r = 0; while (a[r] && lw(w[t + r]) == lw(a[r])) r++;
                            if (!a[r]) { int u = t + r; while (u < L && !(w[u] >= '0' && w[u] <= '9')) u++; id = watoi(w + u); break; }
                        }
                    }
                }
            }
        }
        mfree(w);
    } while (!id && FindNextFileW(h, &fd));
    FindClose(h);
    return id;
}
static void play(void)
{
    int id = steam_appid();
    if (id > 0) {
        WCHAR url[64] = L"steam://rungameid/", t[16]; witoa(id, t); wcat(url, t, 64);
        ShellExecuteW(NULL, L"open", url, NULL, NULL, 1);
    } else {
        WCHAR exe[560]; path_join(exe, gamedir, L"Heroes.exe");
        ShellExecuteW(NULL, L"open", exe, NULL, gamedir, 1);
    }
    if (opt_close) PostMessageW(hwnd, 0x10, 0, 0);
    else ShowWindow(hwnd, 6);
}

/* ---------------------------------------------------------------- drawing */
static void *res_image(int id)
{
    HRSRC r = FindResourceW(hinst, (LPCWSTR)(ULONG_PTR)id, (LPCWSTR)(ULONG_PTR)10);
    if (!r) return NULL;
    DWORD n = SizeofResource(hinst, r); void *p = LockResource(LoadResource(hinst, r));
    HGLOBAL g = GlobalAlloc(2, n); void *d = GlobalLock(g); memcpy(d, p, n); GlobalUnlock(g);
    void *stream = NULL, *img = NULL;
    if (CreateStreamOnHGlobal(g, 1, &stream) == 0) { GdipCreateBitmapFromStream(stream, &img); }
    return img;   /* stream kept alive with the image */
}
#define RGB(r, g, b) ((DWORD)((r) | ((g) << 8) | ((b) << 16)))
#define ARGB(a, r, g, b) ((DWORD)(((DWORD)(a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
static void *gfx;
static void rrect(int x, int y, int w, int h, int rad, DWORD fill, DWORD line)
{
    void *p = NULL; GdipCreatePath(0, &p);
    int d = rad * 2;
    if (rad < 1) { GdipAddPathArcI(p, x, y, 1, 1, 180, 90); GdipAddPathArcI(p, x + w - 1, y, 1, 1, 270, 90); GdipAddPathArcI(p, x + w - 1, y + h - 1, 1, 1, 0, 90); GdipAddPathArcI(p, x, y + h - 1, 1, 1, 90, 90); }
    else { GdipAddPathArcI(p, x, y, d, d, 180, 90); GdipAddPathArcI(p, x + w - d, y, d, d, 270, 90); GdipAddPathArcI(p, x + w - d, y + h - d, d, d, 0, 90); GdipAddPathArcI(p, x, y + h - d, d, d, 90, 90); }
    GdipClosePathFigure(p);
    if (fill) { void *b = NULL; GdipCreateSolidFill(fill, &b); GdipFillPath(gfx, b, p); GdipDeleteBrush(b); }
    if (line) { void *pen = NULL; GdipCreatePen1(line, (float)S(1), 2, &pen); GdipDrawPath(gfx, pen, p); GdipDeletePen(pen); }
    GdipDeletePath(p);
}
static void line(int x1, int y1, int x2, int y2, DWORD c, float w) { void *pen = NULL; GdipCreatePen1(c, w, 2, &pen); GdipDrawLineI(gfx, pen, x1, y1, x2, y2); GdipDeletePen(pen); }

/* text is drawn after the GDI+ pass: queue it */
typedef struct { RECT r; HFONT f; DWORD col; UINT fl; LPCWSTR s; WCHAR own[200]; } TXT;
static TXT txts[96];
static int ntxt;
static void text(int x, int y, int w, int h, HFONT f, DWORD col, UINT fl, LPCWSTR s)
{
    if (ntxt >= 96) return;
    TXT *t = &txts[ntxt++];
    t->r.left = x; t->r.top = y; t->r.right = x + w; t->r.bottom = y + h; t->f = f; t->col = col; t->fl = fl | 0x800;
    if (wlen(s) < 199) { wcpy(t->own, s, 200); t->s = t->own; } else t->s = s;
}
#define GOLD RGB(232, 182, 104)
#define PALE RGB(236, 224, 206)
#define DIM RGB(170, 146, 124)
#define WHITE RGB(250, 244, 234)

static void button(int id, int x, int y, int w, int h, LPCWSTR label, HFONT f, int primary, int enabled)
{
    int hv = hover == id && enabled, pr = pressed == id && enabled;
    DWORD fill = primary ? (enabled ? (pr ? ARGB(255, 104, 22, 22) : hv ? ARGB(255, 164, 40, 34) : ARGB(255, 136, 30, 28)) : ARGB(255, 70, 40, 36))
                         : (pr ? ARGB(210, 40, 18, 14) : hv ? ARGB(225, 72, 34, 26) : ARGB(195, 30, 16, 12));
    rrect(x, y, w, h, S(6), fill, primary ? ARGB(255, 214, 160, 90) : ARGB(210, 176, 128, 70));
    text(x, y, w, h, f, enabled ? WHITE : DIM, 1 | 4 | 0x20, label);
    if (enabled) hot(x, y, w, h, id);
}
static void toggle(int id, int x, int y, LPCWSTR label, int on)
{
    rrect(x, y + S(3), S(18), S(18), S(4), on ? ARGB(255, 136, 30, 28) : ARGB(170, 30, 16, 12), hover == id ? ARGB(255, 232, 182, 104) : ARGB(255, 176, 128, 70));
    if (on) { line(x + S(4), y + S(12), x + S(8), y + S(17), ARGB(255, 240, 240, 240), (float)S(2)); line(x + S(8), y + S(17), x + S(15), y + S(7), ARGB(255, 240, 240, 240), (float)S(2)); }
    text(x + S(28), y, S(420), S(24), f_text, hover == id ? WHITE : PALE, 4 | 0x20, label);
    hot(x, y, S(440), S(24), id);
}

static LPCWSTR gx_name(int i)
{
    static WCHAR b[32];
    switch (i) {
    case 0: return gx_an < 2 ? L"Off" : gx_an == 2 ? L"2x" : gx_an == 4 ? L"4x" : gx_an == 8 ? L"8x" : L"16x";
    case 1: return gx_sr <= 1 ? L"Original" : gx_sr == 2 ? L"2x" : gx_sr == 3 ? L"3x" : L"4x";
    case 2: witoa(gx_sm, b); if (gx_sm == 512) wcat(b, L" (original)", 32); return b;
    case 3: witoa(gx_dd, b); wcat(b, L"%", 32); if (gx_dd == 100) wcat(b, L" (original)", 32); return b;
    default: return gx_aa == 0 ? L"Off" : gx_aa == 2 ? L"2x" : gx_aa == 4 ? L"4x" : L"8x";
    }
}
static void gx_step(int i, int dir)
{
    static const int an[] = {0, 2, 4, 8, 16}, sr[] = {1, 2, 3, 4}, sm[] = {512, 1024, 2048, 4096}, aa[] = {0, 2, 4, 8};
    const int *t; int n, *v;
    switch (i) {
    case 0: t = an; n = 5; v = &gx_an; break;
    case 1: t = sr; n = 4; v = &gx_sr; break;
    case 2: t = sm; n = 4; v = &gx_sm; break;
    case 3: gx_dd += dir * 25; if (gx_dd < 100) gx_dd = 100; if (gx_dd > 300) gx_dd = 300; gx_write(); return;
    default: t = aa; n = 4; v = &gx_aa; break;
    }
    int k = 0; for (int j = 0; j < n; j++) if (t[j] <= *v) k = j;
    k += dir; if (k < 0) k = 0; if (k >= n) k = n - 1;
    *v = t[k];
    gx_write();
}

static void paint(HDC dc)
{
    int W = S(960), Hh = S(600);
    nhots = 0; ntxt = 0;
    GdipCreateFromHDC(dc, &gfx);
    GdipSetSmoothingMode(gfx, 4);
    GdipSetInterpolationMode(gfx, 7);
    if (img_bg) GdipDrawImageRectI(gfx, img_bg, 0, 0, W, Hh);
    else { void *b; GdipCreateSolidFill(ARGB(255, 24, 14, 12), &b); GdipFillRectangleI(gfx, b, 0, 0, W, Hh); GdipDeleteBrush(b); }
    rrect(0, 0, W - 1, Hh - 1, 0, 0, ARGB(255, 120, 80, 44));
    if (img_logo) GdipDrawImageRectI(gfx, img_logo, S(640), S(2), S(300), S(92));

    /* tabs */
    static LPCWSTR tn[4] = { L"HOME", L"SETTINGS", L"LOGS", L"MAP EDITOR" };
    int tx = S(40);
    for (int i = 0; i < 4; i++) {
        int tw = i == 1 ? S(170) : i == 3 ? S(200) : S(110);
        int on = tab == i, hv = hover == H_TAB0 + i;
        text(tx, S(30), tw, S(40), f_tab, on ? WHITE : hv ? PALE : DIM, 4 | 0x20, tn[i]);
        if (on) { line(tx, S(80), tx + tw - S(30), S(80), ARGB(255, 196, 46, 38), (float)S(3)); line(tx, S(84), tx + tw - S(30), S(84), ARGB(160, 232, 182, 104), 1.0f); }
        hot(tx, S(24), tw - S(20), S(56), H_TAB0 + i);
        tx += tw + S(10);
    }
    
    /* window buttons */
    text(S(890), S(2), S(30), S(24), f_text, hover == H_MIN ? WHITE : DIM, 1 | 4 | 0x20, L"\x2013");
    text(S(924), S(2), S(30), S(24), f_text, hover == H_CLOSE ? WHITE : DIM, 1 | 4 | 0x20, L"\x2715");
    hot(S(888), 0, S(34), S(26), H_MIN); hot(S(922), 0, S(36), S(26), H_CLOSE);

    if (tab == TAB_HOME) {
        /* news panel */
        int nx = S(470), ny = S(130), nw = S(450), nh = S(380);
        rrect(nx, ny, nw, nh, S(8), ARGB(180, 16, 9, 8), ARGB(220, 176, 128, 70));
        rrect(nx, ny, nw, S(44), S(8), ARGB(210, 92, 20, 20), 0);
        text(nx + S(16), ny, nw - S(32), S(44), f_head, GOLD, 4 | 0x20, rel_name[0] ? rel_name : L"News");
        if (rel_date[0]) text(nx + S(16), ny, nw - S(32), S(44), f_small, DIM, 2 | 4 | 0x20, rel_date);
        hot(nx, ny + S(44), nw, nh - S(44), H_NEWS);
        if (rel_page[0]) { button(H_CHANGELOG, nx + S(16), ny + nh - S(44), nw - S(32), S(32), L"Read more on GitHub", f_small, 0, 1); }
        /* left: status + main button */
        text(S(40), S(140), S(400), S(36), f_big, WHITE, 0x20, L"Battle for Bersia MP");
        WCHAR line1[200]; wcpy(line1, L"Installed: ", 200);
        if (installed[0]) { wcat(line1, installed, 200); if (installed_note[0]) { wcat(line1, L"  (", 200); wcat(line1, installed_note, 200); wcat(line1, L")", 200); } }
        else wcat(line1, installed_note[0] ? installed_note : L"-", 200);
        text(S(40), S(190), S(410), S(24), f_text, PALE, 0x20 | 0x8000, line1);
        WCHAR line2[200]; wcpy(line2, L"Latest: ", 200); wcat(line2, rel_tag[0] ? rel_name : (state == ST_CHECKING ? L"checking..." : L"-"), 200);
        if (opt_test) wcat(line2, L"   (test builds on)", 200);
        text(S(40), S(216), S(410), S(24), f_text, PALE, 0x20 | 0x8000, line2);
        text(S(40), S(402), S(410), S(24), f_small, DIM, 0x20 | 0x8000, !gamedir[0] ? L"" : crus_installed() ? L"Crusaders co-op missions: installed" :
             crus_url[0] ? L"Crusaders co-op missions: not installed (\x22EE menu > Install Crusaders missions)" : L"Crusaders co-op missions: not installed");
        text(S(40), S(250), S(410), S(80), f_text, GOLD, 0x10, status);

        if (launcher_url[0]) {
            WCHAR t[96]; wcpy(t, L"Launcher ", 96); wcat(t, launcher_newver, 96); wcat(t, L" available - click to update", 96);
            button(H_SELFUPD, S(40), S(340), S(410), S(34), t, f_small, 0, state == ST_IDLE);
        }
        int busy = state != ST_IDLE;
        int need = gamedir[0] && rel_tag[0] && !(installed[0] && !installed_note[0] && weq(installed, rel_tag));
        LPCWSTR lbl = !gamedir[0] ? L"FIND GAME FOLDER" : state == ST_CHECKING ? L"CHECKING..." : state == ST_DOWNLOADING || state == ST_SELFUPDATE ? L"DOWNLOADING..." : state == ST_INSTALLING ? L"INSTALLING..." :
                      need ? (installed[0] ? L"UPDATE" : L"INSTALL") : L"LAUNCH GAME";
        button(H_MAIN, S(40), S(440), S(300), S(64), lbl, f_btn, 1, !busy || state == ST_CHECKING);
        button(H_MORE, S(344), S(440), S(40), S(64), L"\x22EE", f_btn, 1, !busy);
        if (busy && state != ST_CHECKING) {
            rrect(S(40), S(512), S(344), S(8), S(3), ARGB(200, 30, 16, 12), ARGB(170, 176, 128, 70));
            int p = progress < 0 ? 1000 : progress;
            if (p > 0) rrect(S(41), S(513), (S(342) * p) / 1000, S(6), S(2), progress < 0 ? ARGB(255, 150, 90, 60) : ARGB(255, 232, 182, 104), 0);
        }
        WCHAR foot[300]; wcpy(foot, L"Mod: ", 300); wcat(foot, installed[0] ? installed : L"not installed", 300); wcat(foot, L"  |  Launcher: " LAUNCHER_VER L"  |  ", 300);
        wcat(foot, gamedir[0] ? L"Game folder found" : L"Game folder not set", 300);
        text(S(40), S(560), S(600), S(24), f_small, DIM, 0x20 | 4, foot);
    } else if (tab == TAB_SETTINGS) {
        text(S(40), S(118), S(400), S(30), f_head, GOLD, 0x20 | 4, L"Updates");
        toggle(H_TEST, S(40), S(154), L"Receive test builds (for testers)", opt_test);
        toggle(H_CLOSEOPT, S(40), S(184), L"Close the launcher when the game starts", opt_close);
        text(S(40), S(258), S(400), S(30), f_head, GOLD, 0x20 | 4, L"Game folder");
        text(S(40), S(290), S(560), S(24), f_small, gamedir[0] ? PALE : RGB(230, 140, 120), 0x20 | 4 | 0x8000, gamedir[0] ? gamedir : L"Not set - choose the folder with Heroes.exe");
        button(H_FOLDER, S(40), S(320), S(170), S(32), L"Change...", f_small, 0, state == ST_IDLE);

        int gx = S(500), gy = S(118);
        text(gx, gy, S(420), S(30), f_head, GOLD, 0x20 | 4, L"Graphics");
        static LPCWSTR gl[5] = { L"Texture Filter", L"Shadow Range", L"Shadow Detail", L"Draw Distance", L"Anti-Aliasing" };
        for (int i = 0; i < 5; i++) {
            int y = gy + S(40) + i * S(44);
            rrect(gx, y, S(420), S(36), S(6), ARGB(160, 16, 9, 8), ARGB(140, 176, 128, 70));
            text(gx + S(14), y, S(170), S(36), f_text, PALE, 4 | 0x20, gl[i]);
            int lh = hover == H_GX + i * 2, rh = hover == H_GX + i * 2 + 1;
            text(gx + S(196), y, S(36), S(36), f_btn, lh ? GOLD : DIM, 1 | 4 | 0x20, L"\x2039");
            text(gx + S(232), y, S(140), S(36), f_text, WHITE, 1 | 4 | 0x20, gx_name(i));
            text(gx + S(372), y, S(36), S(36), f_btn, rh ? GOLD : DIM, 1 | 4 | 0x20, L"\x203A");
            hot(gx + S(196), y, S(36), S(36), H_GX + i * 2); hot(gx + S(372), y, S(36), S(36), H_GX + i * 2 + 1);
        }
        text(gx, gy + S(266), S(420), S(110), f_small, DIM, 0x10,
             L"Saved straight away to Documents\\KUF2 Heroes\\KUFH.ini and used from the next battle. "
             L"In multiplayer, Draw Distance and Shadow Range stay at the original values for every player. "
             L"Anti-Aliasing needs a game restart; set it back to Off if water or effects look wrong.");
    } else if (tab == TAB_FORGE) {
        text(S(40), S(130), S(600), S(36), f_big, WHITE, 0x20, L"Bersia Map Forge");
        text(S(40), S(178), S(600), S(200), f_text, PALE, 0x10,
             L"Make your own battle maps for Battle for Bersia MP - terrain, walls, buildings, start points and Invasion waves.\n\n"
             L"It opens in your web browser (no install, works offline). Save your map into Data\\Mission\\Live and it shows up in the "
             L"multiplayer lobby as \x22Custom: <name>\x22 - players who don't have it get it from the host automatically.\n\n"
             L"Press Help inside the editor for a step-by-step guide.");
        button(H_FORGE, S(40), S(392), S(300), S(64), forge_busy ? L"PLEASE WAIT..." : L"OPEN MAP EDITOR", f_btn, 1, !forge_busy);
        button(H_FORGEDIR, S(350), S(404), S(190), S(40), L"Open maps folder", f_small, 0, gamedir[0] != 0);
        text(S(40), S(470), S(600), S(28), f_text, GOLD, 0x20, forge_msg);
        WCHAR fp[600]; wcpy(fp, L"Saved as: ", 600); { WCHAR p[560]; forge_path(p); wcat(fp, p, 600); }
        text(S(40), S(560), S(880), S(24), f_small, DIM, 0x20 | 4 | 0x8000, fp);
    } else {
        text(S(40), S(118), S(600), S(30), f_head, GOLD, 0x20 | 4, L"Logs and reports");
        text(S(40), S(150), S(880), S(44), f_small, PALE, 0x10, L"When something goes wrong in multiplayer, send KUF_netlog.txt from EVERY player in that game, plus any crash dump (.mdmp).");
        button(H_LOGNET, S(40), S(200), S(210), S(36), L"Open KUF_netlog.txt", f_small, 0, gamedir[0] != 0);
        button(H_LOGDIR, S(260), S(200), S(210), S(36), L"Open game folder", f_small, 0, gamedir[0] != 0);
        button(H_LOGDUMP, S(480), S(200), S(210), S(36), L"Open crash dumps", f_small, 0, 1);
        text(S(40), S(256), S(880), S(28), f_head, GOLD, 0x20 | 4, L"Last install");
        rrect(S(40), S(290), S(880), S(260), S(8), ARGB(180, 16, 9, 8), ARGB(170, 176, 128, 70));
        text(S(56), S(300), S(848), S(240), f_small, PALE, 0x10 | 0x4000 /* DT_EDITCONTROL */, install_log ? install_log : L"(nothing yet)");
    }
    GdipDeleteGraphics(gfx); gfx = NULL;

    /* GDI text pass */
    SetBkMode(dc, 1);
    for (int i = 0; i < ntxt; i++) {
        SelectObject(dc, txts[i].f);
        SetTextColor(dc, txts[i].col);
        DrawTextW(dc, txts[i].s, -1, &txts[i].r, txts[i].fl);
    }
    if (tab == TAB_HOME) {   /* news body with scrolling, clipped by the panel */
        RECT clip = { S(486), S(184), S(904), S(456) };
        LPCWSTR body = rel_body && rel_body[0] ? rel_body : (state == ST_CHECKING ? L"Loading..." : L"No news yet.");
        SelectObject(dc, f_text);
        RECT m = { 0, 0, clip.right - clip.left, 0 };
        DrawTextW(dc, body, -1, &m, 0x10 | 0x400 | 0x800);
        news_h = m.bottom - m.top;
        int maxs = news_h - (clip.bottom - clip.top); if (maxs < 0) maxs = 0;
        if (news_scroll > maxs) news_scroll = maxs;
        HDC md = CreateCompatibleDC(dc);
        HBITMAP bm = CreateCompatibleBitmap(dc, clip.right - clip.left, clip.bottom - clip.top);
        HGDIOBJ ob = SelectObject(md, bm);
        BitBlt(md, 0, 0, clip.right - clip.left, clip.bottom - clip.top, dc, clip.left, clip.top, 0xCC0020);
        SelectObject(md, f_text); SetBkMode(md, 1); SetTextColor(md, PALE);
        RECT r = { 0, -news_scroll, clip.right - clip.left, news_h - news_scroll };
        DrawTextW(md, body, -1, &r, 0x10 | 0x800);
        BitBlt(dc, clip.left, clip.top, clip.right - clip.left, clip.bottom - clip.top, md, 0, 0, 0xCC0020);
        SelectObject(md, ob); DeleteObject(bm); DeleteDC(md);
        if (maxs > 0) {   /* scroll hint */
            int th = (clip.bottom - clip.top) * (clip.bottom - clip.top) / news_h, ty = clip.top + (clip.bottom - clip.top - th) * news_scroll / maxs;
            RECT sb = { S(908), ty, S(911), ty + th }; HBRUSH b = CreateSolidBrush(RGB(176, 128, 70)); FillRect(dc, &sb, b); DeleteObject(b);
        }
    }
}

/* ---------------------------------------------------------------- actions */
static void start_check(void) { if (state == ST_IDLE) { state = ST_CHECKING; CreateThread(NULL, 0, check_thread, NULL, 0, NULL); } }
static void start_install(int mode)
{
    if (state != ST_IDLE || !gamedir[0]) return;
    install_mode = mode;
    state = ST_DOWNLOADING;
    CreateThread(NULL, 0, install_thread, NULL, 0, NULL);
}
static void choose_folder(void)
{
    BROWSEINFOW bi; WCHAR disp[520];
    memset(&bi, 0, sizeof bi);
    bi.hwndOwner = hwnd; bi.pszDisplayName = disp; bi.lpszTitle = L"Choose your Kingdom Under Fire: Heroes folder (the one with Heroes.exe)"; bi.ulFlags = 1 | 0x40;
    void *pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return;
    WCHAR p[520];
    if (SHGetPathFromIDListW(pidl, p)) {
        if (game_ok(p)) { wcpy(gamedir, p, 520); ini_set(L"Launcher", L"GameFolder", gamedir); detect_installed(); }
        else MessageBoxW(hwnd, L"That folder does not contain Heroes.exe and the Data folder.", L"Battle for Bersia MP", 0x30);
    }
    CoTaskMemFree(pidl);
}
static void open_path(LPCWSTR p) { ShellExecuteW(NULL, L"open", p, NULL, NULL, 1); }
static void open_dumps(void)
{
    WCHAR p[560], lad[520];
    lad[0] = 0; GetEnvironmentVariableW(L"LOCALAPPDATA", lad, 520);
    path_join(p, lad, L"CrashDumps");
    if (exists(p)) { open_path(p); return; }
    open_path(gamedir);
}
static void more_menu(void)
{
    HMENU m = CreatePopupMenu();
    AppendMenuW(m, installed[0] && gamedir[0] ? 0 : 1, 6, L"Launch game without updating");
    AppendMenuW(m, 0, 1, L"Check for updates");
    AppendMenuW(m, rel_url[0] ? 0 : 1, 2, L"Repair (reinstall this version)");
    AppendMenuW(m, installed[0] ? 0 : 1, 3, L"Uninstall the mod");
    AppendMenuW(m, 0x800, 0, NULL);
    AppendMenuW(m, crus_url[0] ? 0 : 1, 7, crus_installed() ? L"Reinstall Crusaders missions" : L"Install Crusaders missions (needs The Crusaders)");
    AppendMenuW(m, crus_installed() ? 0 : 1, 8, L"Remove Crusaders missions");
    AppendMenuW(m, 0x800, 0, NULL);
    AppendMenuW(m, gamedir[0] ? 0 : 1, 4, L"Open game folder");
    AppendMenuW(m, 0, 5, L"Release page");
    POINT pt = { S(344), S(506) }; ClientToScreen(hwnd, &pt);
    SetForegroundWindow(hwnd);
    int c = TrackPopupMenu(m, 0x100 | 0x80 /* RETURNCMD | NONOTIFY */, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(m);
    if (c == 1) start_check();
    else if (c == 2) start_install(0);
    else if (c == 3 && MessageBoxW(hwnd, L"Remove Battle for Bersia MP and put the original game files back?", L"Uninstall", 0x24) == 6) start_install(1);
    else if (c == 4) open_path(gamedir);
    else if (c == 5) open_path(REPO_PAGE);
    else if (c == 6) play();
    else if (c == 7) start_crus(0);
    else if (c == 8 && MessageBoxW(hwnd, L"Remove the Crusaders co-op missions?", L"Crusaders missions", 0x24) == 6) start_crus(1);
}
static void click(int id)
{
    if (id == H_MIN) ShowWindow(hwnd, 6);
    else if (id == H_CLOSE) PostMessageW(hwnd, 0x10, 0, 0);
    else if (id >= H_TAB0 && id <= H_TAB3) { tab = id - H_TAB0; if (tab == TAB_FORGE && !forge_msg[0]) start_forge(0); }
    else if (id == H_FORGE) start_forge(1);
    else if (id == H_FORGEDIR) { WCHAR p[560]; path_join(p, gamedir, L"Data\\Mission\\Live"); open_path(exists(p) ? p : gamedir); }
    else if (id == H_MAIN) {
        if (!gamedir[0]) { choose_folder(); start_check(); }
        else if (state == ST_IDLE) {
            int need = rel_tag[0] && !(installed[0] && !installed_note[0] && weq(installed, rel_tag));
            if (need) start_install(0); else if (installed[0]) play();
            else set_status(L"Nothing installed yet and no release found - check your connection.");
        }
    }
    else if (id == H_MORE) more_menu();
    else if (id == H_CHANGELOG) open_path(rel_page[0] ? rel_page : REPO_PAGE);
    else if (id == H_SELFUPD && state == ST_IDLE) CreateThread(NULL, 0, selfupdate_thread, NULL, 0, NULL);
    else if (id == H_TEST) { opt_test = !opt_test; ini_seti(L"Launcher", L"TestBuilds", opt_test); start_check(); }
    else if (id == H_ENC) { opt_enc = !opt_enc; ini_seti(L"Launcher", L"Encablossa", opt_enc); start_check(); }
    else if (id == H_CLOSEOPT) { opt_close = !opt_close; ini_seti(L"Launcher", L"CloseOnPlay", opt_close); }
    else if (id == H_FOLDER) choose_folder();
    else if (id >= H_GX && id < H_GX + 10) gx_step((id - H_GX) / 2, (id - H_GX) % 2 ? 1 : -1);
    else if (id == H_LOGNET) { WCHAR p[560]; path_join(p, gamedir, L"KUF_netlog.txt"); if (exists(p)) open_path(p); else MessageBoxW(hwnd, L"No KUF_netlog.txt yet - it is written during multiplayer games.", L"Logs", 0x40); }
    else if (id == H_LOGDIR) open_path(gamedir);
    else if (id == H_LOGDUMP) open_dumps();
    InvalidateRect(hwnd, NULL, 0);
}
static int hit(int x, int y) { for (int i = nhots - 1; i >= 0; i--) { RECT *r = &hots[i].r; if (x >= r->left && x < r->right && y >= r->top && y < r->bottom) return hots[i].id; } return -1; }

static LRESULT WINAPI wndproc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m) {
    case 0x14: return 1;   /* WM_ERASEBKGND */
    case 0xF: {            /* WM_PAINT: double-buffered */
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        HDC md = CreateCompatibleDC(dc); HBITMAP bm = CreateCompatibleBitmap(dc, S(960), S(600));
        HGDIOBJ ob = SelectObject(md, bm);
        paint(md);
        BitBlt(dc, 0, 0, S(960), S(600), md, 0, 0, 0xCC0020);
        SelectObject(md, ob); DeleteObject(bm); DeleteDC(md);
        EndPaint(h, &ps);
        return 0;
    }
    case 0x200: {          /* WM_MOUSEMOVE */
        int x = (short)(l & 0xFFFF), y = (short)((l >> 16) & 0xFFFF), id = hit(x, y);
        if (id != hover) { hover = id; InvalidateRect(h, NULL, 0); TRACKMOUSEEVENT t = { sizeof t, 2, h, 0 }; TrackMouseEvent(&t); }
        return 0;
    }
    case 0x2A3: if (hover != -1) { hover = -1; InvalidateRect(h, NULL, 0); } return 0;   /* WM_MOUSELEAVE */
    case 0x20:             /* WM_SETCURSOR */
        if ((l & 0xFFFF) == 1) { SetCursor(LoadCursorW(NULL, (LPCWSTR)(ULONG_PTR)(hover >= 0 && hover != H_NEWS ? 32649 : 32512))); return 1; }
        break;
    case 0x201: {          /* WM_LBUTTONDOWN */
        int x = (short)(l & 0xFFFF), y = (short)((l >> 16) & 0xFFFF), id = hit(x, y);
        if (id >= 0 && id != H_NEWS) { pressed = id; InvalidateRect(h, NULL, 0); return 0; }
        if (y < S(96)) { ReleaseCapture(); SendMessageW(h, 0xA1, 2, 0); }   /* drag the window */
        return 0;
    }
    case 0x202: {          /* WM_LBUTTONUP */
        int x = (short)(l & 0xFFFF), y = (short)((l >> 16) & 0xFFFF), id = hit(x, y), p = pressed;
        pressed = -1;
        if (p >= 0 && id == p) click(id); else InvalidateRect(h, NULL, 0);
        return 0;
    }
    case 0x20A:            /* WM_MOUSEWHEEL */
        if (tab == TAB_HOME) { news_scroll -= (short)((w >> 16) & 0xFFFF) / 120 * S(40); if (news_scroll < 0) news_scroll = 0; InvalidateRect(h, NULL, 0); }
        return 0;
    case WM_APP_UPDATE: InvalidateRect(h, NULL, 0); return 0;
    case WM_APP_CRUSDIR: {   /* Crusaders not found by the installer: let the player pick its folder */
        BROWSEINFOW bi; WCHAR disp[520], p[520];
        memset(&bi, 0, sizeof bi);
        bi.hwndOwner = h; bi.pszDisplayName = disp; bi.lpszTitle = L"Choose your Kingdom Under Fire: The Crusaders folder (the one with the Data folder)"; bi.ulFlags = 1 | 0x40;
        void *pidl = SHBrowseForFolderW(&bi);
        if (pidl) { if (SHGetPathFromIDListW(pidl, p)) { wcpy(crus_dir, p, 520); start_crus(2); } CoTaskMemFree(pidl); }
        InvalidateRect(h, NULL, 0);
        return 0;
    }
    case 0x10: DestroyWindow(h); return 0;
    case 2: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

/* ---------------------------------------------------------------- start-up */
static HFONT font(int px, int weight, LPCWSTR face) { return CreateFontW(-S(px), 0, 0, 0, weight, 0, 0, 0, 1, 0, 0, 5 /* CLEARTYPE */, 0, face); }

void start(void)
{
    heap = GetProcessHeap();
    hinst = GetModuleHandleW(NULL);
    CoInitializeEx(NULL, 2);
    GetModuleFileNameW(NULL, selfpath, 520);
    wcpy(selfdir, selfpath, 520);
    { int n = wlen(selfdir); while (n && selfdir[n - 1] != '\\') n--; selfdir[n] = 0; }
    { WCHAR old[560]; wcpy(old, selfpath, 560); wcat(old, L".old", 560); DeleteFileW(old); }
    path_join(inipath, selfdir, L"BattleForBersiaLauncher.ini");
    opt_test = GetPrivateProfileIntW(L"Launcher", L"TestBuilds", 0, inipath);
    opt_enc = GetPrivateProfileIntW(L"Launcher", L"Encablossa", 0, inipath);
    opt_close = GetPrivateProfileIntW(L"Launcher", L"CloseOnPlay", 1, inipath);
    /* game folder: next to the launcher, else the saved one */
    if (game_ok(selfdir)) wcpy(gamedir, selfdir, 520);
    else { WCHAR g[520]; ini_get(L"Launcher", L"GameFolder", g, 520); if (g[0] && game_ok(g)) wcpy(gamedir, g, 520); }
    { int n = wlen(gamedir); while (n > 3 && gamedir[n - 1] == '\\') gamedir[--n] = 0; }
    detect_installed();
    kuf_ini_path(); gx_read();

    HDC sdc = GetDC(NULL); dpi = GetDeviceCaps(sdc, 88) / 96.0f; ReleaseDC(NULL, sdc);
    if (dpi < 1.0f) dpi = 1.0f;
    ULONG_PTR tok; GdiplusStartupInput gi = { 1, NULL, 0, 0 }; GdiplusStartup(&tok, &gi, NULL);
    img_bg = res_image(101); img_logo = res_image(102);
    f_tab = font(26, 400, L"Georgia"); f_btn = font(22, 700, L"Georgia"); f_head = font(18, 700, L"Georgia"); f_big = font(28, 400, L"Georgia");
    f_text = font(15, 400, L"Segoe UI"); f_small = font(13, 400, L"Segoe UI");

    WNDCLASSEXW wc; memset(&wc, 0, sizeof wc);
    wc.cbSize = sizeof wc; wc.style = 0x20000 /* CS_DROPSHADOW */; wc.lpfnWndProc = wndproc; wc.hInstance = hinst;
    wc.hIcon = LoadIconW(hinst, (LPCWSTR)(ULONG_PTR)1); wc.hIconSm = wc.hIcon; wc.hCursor = LoadCursorW(NULL, (LPCWSTR)(ULONG_PTR)32512);
    wc.lpszClassName = L"BattleForBersiaLauncher";
    RegisterClassExW(&wc);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    hwnd = CreateWindowExW(0, wc.lpszClassName, L"Battle for Bersia MP", 0x80000000 | 0x00080000 | 0x00020000 /* POPUP|SYSMENU|MINIMIZEBOX */,
                           (sw - S(960)) / 2, (sh - S(600)) / 2, S(960), S(600), NULL, NULL, hinst, NULL);
    ShowWindow(hwnd, 1); UpdateWindow(hwnd);
    if (!gamedir[0]) set_status(L"Put the launcher in your KUF Heroes folder, or press the button to find it.");
    start_check();
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    ExitProcess(0);
}
