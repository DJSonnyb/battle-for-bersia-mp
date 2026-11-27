#!/usr/bin/env python3
# Battle for Bersia MP - Linux launcher (core: Steam detection, GitHub releases, installer, settings).
# Python 3.8+, standard library only. The GUI (tkinter) is in the main file; this part also works on its own.
import base64, configparser, gzip, hashlib, json, os, re, shutil, struct, subprocess, sys, tempfile, threading, urllib.request

LAUNCHER_VERSION = "1.0.2-linux"
REPO = "DJSonnyb/battle-for-bersia-mp"
API = "https://api.github.com/repos/%s/releases?per_page=30" % REPO
PAGE = "https://github.com/%s/releases" % REPO
UA = "BattleForBersiaLauncher-Linux/" + LAUNCHER_VERSION

CFG_DIR = os.path.join(os.environ.get("XDG_CONFIG_HOME") or os.path.expanduser("~/.config"), "battle-for-bersia-launcher")
CFG_FILE = os.path.join(CFG_DIR, "launcher.ini")


# ------------------------------------------------------------------ launcher settings
class Config:
    def __init__(self):
        self.cp = configparser.ConfigParser()
        self.cp.optionxform = str
        try:
            self.cp.read(CFG_FILE, encoding="utf-8")
        except Exception:
            pass

    def get(self, sec, key, default=""):
        try:
            return self.cp.get(sec, key)
        except Exception:
            return default

    def geti(self, sec, key, default=0):
        try:
            return int(self.get(sec, key, str(default)))
        except ValueError:
            return default

    def set(self, sec, key, val):
        if not self.cp.has_section(sec):
            self.cp.add_section(sec)
        if val is None:
            self.cp.remove_option(sec, key)
        else:
            self.cp.set(sec, key, str(val))
        try:
            os.makedirs(CFG_DIR, exist_ok=True)
            with open(CFG_FILE, "w", encoding="utf-8") as f:
                self.cp.write(f)
        except OSError:
            pass


# ------------------------------------------------------------------ paths (the game uses Windows names; Linux is case-sensitive)
def ci_path(base, rel, must_exist=False):
    """Join a Windows-style relative path onto base, matching each part case-insensitively when it exists."""
    parts = [p for p in re.split(r"[\\/]", rel) if p]

    def find(cur, ps):   # an existing path matching ps case-insensitively, or None
        if not ps:
            return cur
        try:
            names = os.listdir(cur)
        except OSError:
            return None
        cands = ([ps[0]] if ps[0] in names else []) + sorted(e for e in names if e.lower() == ps[0].lower() and e != ps[0])
        for c in cands:
            r = find(os.path.join(cur, c), ps[1:])
            if r is not None:
                return r
        return None

    r = find(base, parts)
    if r is not None:
        return r
    if must_exist:
        return None
    for k in range(len(parts) - 1, -1, -1):   # deepest existing folder, then the rest as written
        d = find(base, parts[:k])
        if d is not None and os.path.isdir(d):
            return os.path.join(d, *parts[k:])
    return os.path.join(base, *parts)


def game_ok(d):
    return bool(d) and ci_path(d, "Heroes.exe", True) is not None and ci_path(d, "Data/Mission", True) is not None


# ------------------------------------------------------------------ Steam
def _vdf_values(text, key):
    return [m.group(1).replace("\\\\", "\\") for m in re.finditer(r'"%s"\s+"([^"]*)"' % re.escape(key), text, re.I)]


def steam_roots():
    h = os.path.expanduser("~")
    cands = [h + "/.steam/steam", h + "/.steam/root", h + "/.local/share/Steam",
             h + "/.var/app/com.valvesoftware.Steam/.local/share/Steam", h + "/.var/app/com.valvesoftware.Steam/data/Steam",
             h + "/snap/steam/common/.local/share/Steam"]
    out, seen = [], set()
    for c in cands:
        if os.path.isdir(os.path.join(c, "steamapps")):
            r = os.path.realpath(c)
            if r not in seen:
                seen.add(r)
                out.append(c)
    return out


def steam_libraries():
    libs, seen = [], set()
    for root in steam_roots():
        for p in [root] + _read_libfolders(root):
            r = os.path.realpath(p)
            if r not in seen and os.path.isdir(os.path.join(p, "steamapps")):
                seen.add(r)
                libs.append(p)
    return libs


def _read_libfolders(root):
    for f in ("steamapps/libraryfolders.vdf", "config/libraryfolders.vdf"):
        try:
            with open(os.path.join(root, f), encoding="utf-8", errors="replace") as fh:
                return _vdf_values(fh.read(), "path")
        except OSError:
            continue
    return []


def _manifests(lib):
    d = os.path.join(lib, "steamapps")
    try:
        names = os.listdir(d)
    except OSError:
        return
    for n in names:
        if n.startswith("appmanifest_") and n.endswith(".acf"):
            try:
                with open(os.path.join(d, n), encoding="utf-8", errors="replace") as fh:
                    t = fh.read()
            except OSError:
                continue
            ids, dirs, names_ = _vdf_values(t, "appid"), _vdf_values(t, "installdir"), _vdf_values(t, "name")
            if ids and dirs:
                yield ids[0], dirs[0], (names_[0] if names_ else "")


def find_game():
    """(game folder, appid, library) of Kingdom Under Fire: Heroes, or (None, None, None)."""
    for lib in steam_libraries():
        for appid, inst, name in _manifests(lib):
            d = os.path.join(lib, "steamapps", "common", inst)
            if ("heroes" in (inst + name).lower() or "kingdom under fire" in (inst + name).lower()) and game_ok(d):
                return d, appid, lib
    return None, None, None


def appid_for(gamedir):
    """Steam app id and library of a game folder <lib>/steamapps/common/<installdir>."""
    common = os.path.dirname(os.path.abspath(gamedir))
    lib = os.path.dirname(os.path.dirname(common))
    inst = os.path.basename(os.path.abspath(gamedir))
    for appid, d, _ in _manifests(lib):
        if d.lower() == inst.lower():
            return appid, lib
    return None, lib


def proton_docs(gamedir):
    """Documents folder inside the game's Proton prefix (None if the game was never started)."""
    appid, lib = appid_for(gamedir)
    if not appid:
        return None
    for base in [lib] + steam_roots():
        pfx = os.path.join(base, "steamapps", "compatdata", appid, "pfx", "drive_c", "users", "steamuser")
        if os.path.isdir(pfx):
            for dn in ("Documents", "My Documents"):
                p = os.path.join(pfx, dn)
                if os.path.isdir(p):
                    return p
            return os.path.join(pfx, "Documents")
    return None


def launch_game(gamedir):
    appid, _ = appid_for(gamedir)
    if not appid:
        return False, "Steam app id not found - start KUF Heroes from Steam."
    url = "steam://rungameid/%s" % appid
    tries = []
    if shutil.which("steam"):
        tries.append(["steam", url])
    if shutil.which("flatpak") and os.path.isdir(os.path.expanduser("~/.var/app/com.valvesoftware.Steam")):
        tries.append(["flatpak", "run", "com.valvesoftware.Steam", url])
    if shutil.which("xdg-open"):
        tries.append(["xdg-open", url])
    for cmd in tries:
        try:
            subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, start_new_session=True)
            return True, "Starting KUF Heroes through Steam..."
        except OSError:
            continue
    return False, "Could not start Steam - start KUF Heroes from Steam."


def game_running():
    try:
        for pid in os.listdir("/proc"):
            if not pid.isdigit():
                continue
            try:
                with open("/proc/%s/cmdline" % pid, "rb") as f:
                    c = f.read().lower()
            except OSError:
                continue
            for a in c.split(b"\0"):
                if a.replace(b"\\", b"/").rsplit(b"/", 1)[-1] == b"heroes.exe":
                    return True
    except OSError:
        pass
    return False


# ------------------------------------------------------------------ the game's KUFH.ini (Windows INI, kept as it is)
class WinIni:
    def __init__(self, path):
        self.path = path
        self.lines, self.nl = [], "\r\n"
        try:
            with open(path, "rb") as f:
                raw = f.read()
            txt = raw.decode("utf-8", errors="replace")
            if "\r\n" not in txt and "\n" in txt:
                self.nl = "\n"
            self.lines = txt.splitlines()
        except OSError:
            pass

    def _find(self, sec, key):
        cur, sec_at, last = None, -1, -1
        for i, ln in enumerate(self.lines):
            s = ln.strip()
            if s.startswith("[") and "]" in s:
                cur = s[1:s.index("]")].strip().lower()
                if cur == sec.lower():
                    sec_at = i
                    last = i
                continue
            if cur == sec.lower():
                if s:
                    last = i
                if "=" in s and s.split("=", 1)[0].strip().lower() == key.lower():
                    return sec_at, i, last
        return sec_at, -1, last

    def get(self, sec, key, default):
        _, i, _ = self._find(sec, key)
        if i < 0:
            return default
        try:
            return int(self.lines[i].split("=", 1)[1].strip().split()[0])
        except (ValueError, IndexError):
            return default

    def set(self, sec, key, val):
        sec_at, i, last = self._find(sec, key)
        line = "%s=%s" % (key, val)
        if i >= 0:
            self.lines[i] = line
        elif sec_at >= 0:
            self.lines.insert(last + 1, line)
        else:
            if self.lines and self.lines[-1].strip():
                self.lines.append("")
            self.lines += ["[%s]" % sec, line]

    def save(self):
        os.makedirs(os.path.dirname(self.path), exist_ok=True)
        with open(self.path, "wb") as f:
            f.write((self.nl.join(self.lines) + self.nl).encode("utf-8"))


GX = [  # (label, section, key, values, default, describe)
    ("Texture Filter", "BattleForBersia", "Anisotropy", [0, 2, 4, 8, 16], 16, lambda v: "Off" if v < 2 else "%dx" % v),
    ("Shadow Range", "BattleForBersia", "ShadowRange", [1, 2, 3, 4], 2, lambda v: "Original" if v <= 1 else "%dx" % v),
    ("Shadow Detail", "General", "ShadowMapSize", [512, 1024, 2048, 4096], 512, lambda v: "%d%s" % (v, " (original)" if v == 512 else "")),
    ("Draw Distance", "BattleForBersia", "DrawDistance", list(range(100, 301, 25)), 150, lambda v: "%d%%%s" % (v, " (original)" if v == 100 else "")),
    ("Anti-Aliasing", "BattleForBersia", "AntiAliasing", [0, 2, 4, 8], 0, lambda v: "Off" if v == 0 else "%dx" % v),
]


# ------------------------------------------------------------------ GitHub
def http_get(url, dest=None, progress=None, timeout=30):
    req = urllib.request.Request(url, headers={"User-Agent": UA, "Accept": "application/vnd.github+json" if "api.github" in url else "*/*"})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        total = int(r.headers.get("Content-Length") or 0)
        if dest is None:
            return r.read()
        got = 0
        while True:
            b = r.read(65536)
            if not b:
                break
            dest.write(b)
            got += len(b)
            if progress and total:
                progress(got / total)
        return got


def tidy_body(md):
    out = []
    for ln in (md or "").replace("\r", "").split("\n"):
        ln = re.sub(r"^#+\s?", "", ln)
        ln = re.sub(r"^[-*] ", "• ", ln)
        ln = ln.replace("**", "").replace("`", "").replace("__", "")
        out.append(ln)
    return "\n".join(out).strip()


def pick_release(releases, test_builds, encablossa):
    for r in releases:
        if r.get("draft") or (r.get("prerelease") and not test_builds):
            continue
        bat = bat_e = None
        for a in r.get("assets") or []:
            n, u = a.get("name") or "", a.get("browser_download_url") or ""
            if "Crusaders" in n:
                continue
            if n.endswith("E_Install.bat"):
                bat_e = bat_e or u
            elif n.endswith("_Install.bat"):
                bat = bat or u
        use = bat_e if (encablossa and bat_e) else bat
        if use and r.get("tag_name"):
            return {"tag": r["tag_name"], "name": r.get("name") or r["tag_name"], "date": (r.get("published_at") or "")[:10],
                    "url": use, "page": r.get("html_url") or PAGE, "body": tidy_body(r.get("body") or ""), "pre": bool(r.get("prerelease"))}
    return None


def check_releases(test_builds, encablossa):
    data = json.loads(http_get(API).decode("utf-8"))
    return pick_release(data, test_builds, encablossa)


# ------------------------------------------------------------------ installer (same steps as the .bat's PowerShell part)
def sha(p):
    if not p or not os.path.isfile(p):
        return ""
    h = hashlib.sha256()
    with open(p, "rb") as f:
        for b in iter(lambda: f.read(1 << 20), b""):
            h.update(b)
    return h.hexdigest()


def read_payload(bat_path):
    with open(bat_path, "rb") as f:
        s = f.read().decode("latin-1")
    k = s.index("#PAY" + "LOAD")
    b64 = re.sub(r"[^A-Za-z0-9+/=]", "", s[k + 9:])
    pay = gzip.decompress(base64.b64decode(b64))
    ml = struct.unpack_from("<i", pay, 0)[0]
    m = json.loads(pay[4:4 + ml].decode("utf-8"))
    return m, pay, ml


def run_installer(bat_path, gamedir, uninstall=False, log=print):
    """Returns True when done. Messages go to log(text)."""
    m, pay, ml = read_payload(bat_path)
    log("Battle for Bersia MP v%s installer (Linux)" % m.get("ver", "?"))
    if ci_path(gamedir, "Data/Mission", True) is None:
        log("ERROR: the game folder has no Data/Mission folder - choose the folder with Heroes.exe.")
        return False
    if game_running():
        log("ERROR: Please close the game first.")
        return False
    plan = []
    for f in m["files"]:
        dst, bak = ci_path(gamedir, f["path"]), ci_path(gamedir, f["bak"])
        hd, hb = sha(dst), sha(bak)
        anyf = bool(f.get("any"))
        if uninstall:
            if hb == f["osha"] or (anyf and hb != ""):
                plan.append((f, "restore", bak, dst))
            elif hd != f["osha"]:
                log("No original backup of %s found - use Steam > Verify integrity of game files instead." % f["path"])
            continue
        if hd == f["nsha"]:
            plan.append((f, "ok", dst, dst))
        elif hd == f["osha"]:
            plan.append((f, "build", dst, dst))
        elif hb == f["osha"]:
            plan.append((f, "build", bak, dst))
        elif anyf:
            plan.append((f, "build", dst, dst))
        else:
            log("ERROR: %s is not the Steam version this mod is made for (or is already modified)," % f["path"])
            log("and no original backup was found. In Steam: right-click KUF Heroes > Properties > Installed Files >")
            log("Verify integrity of game files, then try again. Nothing was changed.")
            return False
    if uninstall:
        done = 0
        for f, _, src, dst in plan:
            shutil.copyfile(src, dst)
            log("Restored %s" % f["path"])
            done += 1
        fo = m.get("font2")
        if fo:
            fb = ci_path(gamedir, fo["bak"], True)
            if fb:
                shutil.copyfile(fb, ci_path(gamedir, fo["path"]))
                log("Restored %s" % fo["path"])
        return done > 0
    outs = []
    for f, act, src, dst in plan:
        if act == "ok":
            continue
        a = None
        if not f.get("any"):
            with open(src, "rb") as fh:
                a = fh.read()
        o = bytearray()
        q = 4 + ml + f["off"]
        end = q + f["len"]
        while q < end:
            if pay[q] == 67:  # 'C': copy from the original
                so, n = struct.unpack_from("<ii", pay, q + 1)
                o += a[so:so + n]
                q += 9
            else:  # literal bytes
                n = struct.unpack_from("<i", pay, q + 1)[0]
                o += pay[q + 5:q + 5 + n]
                q += 5 + n
        if hashlib.sha256(o).hexdigest() != f["nsha"]:
            log("ERROR: Building %s failed - nothing was changed." % f["path"])
            return False
        outs.append((f, bytes(o), src))
    for f, data, src in outs:
        dst, bak = ci_path(gamedir, f["path"]), ci_path(gamedir, f["bak"])
        hb = sha(bak)
        anyf = bool(f.get("any"))
        if (anyf and hb == "" and os.path.exists(src)) or (not anyf and hb != f["osha"]):
            shutil.copyfile(src, bak)
            log("Backed up the original as %s" % f["bak"])
        tmp = dst + ".bfbtmp"
        with open(tmp, "wb") as fh:
            fh.write(data)
        os.replace(tmp, dst)
    fo = m.get("font2")
    if fo:
        fp, fb = ci_path(gamedir, fo["path"]), ci_path(gamedir, fo["bak"])
        try:
            d = None
            if os.path.isfile(fp):
                with open(fp, "rb") as fh:
                    d = bytearray(fh.read())
            if d and len(d) == fo["size"] and d[84:88] == b"DXT5" and struct.unpack_from("<i", d, 12)[0] == 512 and struct.unpack_from("<i", d, 16)[0] == 1024:
                if not os.path.exists(fb):
                    shutil.copyfile(fp, fb)
                for off, b64 in fo["blocks"]:
                    x = base64.b64decode(b64)
                    d[int(off):int(off) + len(x)] = x
                with open(fp, "wb") as fh:
                    fh.write(d)
                log("Added the purple and brown team colour icons")
            else:
                log("Data/Interface/Font2.dds is missing or not the usual size - the purple and brown colour icons will be blank.")
        except Exception:
            log("Could not add the purple and brown colour icons (Font2.dds) - everything else is installed.")
    log("Done - Battle for Bersia MP v%s is installed." % m.get("ver", "?"))
    return True


def installed_version(gamedir):
    p = ci_path(gamedir, "Heroes.exe", True) if gamedir else None
    if not p:
        return None
    with open(p, "rb") as f:
        b = f.read()
    i = b.find(b"Battle for Bersia MP v")
    if i < 0:
        return ""
    j = i + 21
    k = j
    while k < len(b) and k - j < 20 and 32 < b[k] < 127 and b[k] not in b"N)":
        k += 1
    return b[j:k].decode("ascii", "replace")


def exe_stamp(gamedir):
    p = ci_path(gamedir, "Heroes.exe", True) if gamedir else None
    if not p:
        return ""
    st = os.stat(p)
    return "%d-%d" % (st.st_size, int(st.st_mtime))
