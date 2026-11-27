# ------------------------------------------------------------------ window: a local page on 127.0.0.1 opened in the browser
# (no extra packages needed; the page talks to this script only, with a random key, and the script ends when the page is closed)
import http.server, secrets, socketserver, time, webbrowser


class Launcher:
    def __init__(self, open_browser=True):
        self.cfg = Config()
        self.lock = threading.Lock()
        self.state, self.progress, self.status, self.rel, self.log_text = "idle", 0.0, "", None, ""
        self.test = self.cfg.geti("Launcher", "TestBuilds", 0)
        self.enc = self.cfg.geti("Launcher", "Encablossa", 0)
        self.close_on_play = self.cfg.geti("Launcher", "CloseOnPlay", 1)
        g = self.cfg.get("Launcher", "GameFolder", "")
        self.gamedir = g if game_ok(g) else None
        if not self.gamedir:
            self.gamedir = find_game()[0]
            if self.gamedir:
                self.cfg.set("Launcher", "GameFolder", self.gamedir)
        self.last_ping = time.time()
        self.quit_at = None
        self.detect()
        self.check()

    # ---------------- state
    def detect(self):
        self.installed, self.note = "", ""
        if not self.gamedir:
            self.note = "Game folder not set"
            return
        v = installed_version(self.gamedir)
        if v is None:
            self.note = "Heroes.exe not found"
        elif not v:
            self.note = "Not installed"
        else:
            tag, st = self.cfg.get("Installed", "Tag", ""), self.cfg.get("Installed", "Exe", "")
            if tag and st == exe_stamp(self.gamedir):
                self.installed = tag
            else:
                self.installed, self.note = v, "installed outside the launcher"

    def need(self):
        return bool(self.gamedir and self.rel and not (self.installed and not self.note and self.installed == self.rel["tag"]))

    def gx_path(self):
        d = proton_docs(self.gamedir) if self.gamedir else None
        return os.path.join(d, "KUF2 Heroes", "KUFH.ini") if d else None

    def snapshot(self):
        with self.lock:
            gp = self.gx_path()
            gx = None
            if gp:
                ini = WinIni(gp)
                gx = [{"label": lab, "value": desc(ini.get(sec, key, d))} for lab, sec, key, vals, d, desc in GX]
            return {"gamedir": self.gamedir, "installed": self.installed, "note": self.note, "rel": self.rel, "state": self.state,
                    "progress": self.progress, "status": self.status, "test": self.test, "enc": self.enc, "close": self.close_on_play,
                    "log": self.log_text, "gx": gx, "gxpath": gp, "need": self.need(), "version": LAUNCHER_VERSION,
                    "quitting": self.quit_at is not None, "can_pick": bool(shutil.which("zenity") or shutil.which("kdialog"))}

    # ---------------- jobs
    def check(self):
        with self.lock:
            if self.state != "idle":
                return
            self.state, self.status = "checking", "Checking for updates..."
        test, enc = self.test, self.enc

        def job():
            try:
                r = check_releases(test, enc)
                st = "" if r else "No release found."
            except Exception as e:
                r, st = None, "Could not reach GitHub (%s)." % (getattr(e, "reason", None) or e.__class__.__name__)
            with self.lock:
                self.rel, self.status, self.state = r, st, "idle"
        threading.Thread(target=job, daemon=True).start()

    def install(self, uninstall=False):
        with self.lock:
            if self.state != "idle" or not self.gamedir:
                return
            rel, gd = self.rel, self.gamedir
            if not uninstall and not rel:
                self.status = "No installer to download - check your internet connection."
                return
            self.state, self.progress = ("installing" if uninstall else "downloading"), 0.0
            self.status = "Removing the mod..." if uninstall else "Downloading..."

        def setp(p):
            self.progress = p

        def job():
            lines, ok = [], False
            bat = ci_path(gd, "BattleForBersiaMP_Install.bat")
            try:
                if not uninstall:
                    with open(bat + ".download", "wb") as f:
                        http_get(rel["url"], f, setp, timeout=60)
                    os.replace(bat + ".download", bat)
                elif not os.path.isfile(bat):
                    with self.lock:
                        self.state, self.status = "idle", "Installer not found - use Repair first, or Steam > Verify integrity of game files."
                    return
                with self.lock:
                    self.state, self.status = "installing", ("Removing the mod..." if uninstall else "Installing...")
                ok = run_installer(bat, gd, uninstall, lambda t: lines.append(t.replace("\\", "/")))
            except Exception as e:
                lines.append("ERROR: %s" % e)
            out = "\n".join(lines)
            with self.lock:
                self.log_text = out
                if uninstall:
                    self.cfg.set("Installed", "Tag", None)
                    self.cfg.set("Installed", "Exe", None)
                    self.status = "Mod removed - the original game files are back." if ok else "Nothing removed - see the LOGS tab."
                elif ok:
                    self.cfg.set("Installed", "Tag", rel["tag"])
                    self.cfg.set("Installed", "Exe", exe_stamp(gd))
                    self.status = "Installed - ready to play."
                elif "close the game" in out:
                    self.status = "Please close the game first, then try again."
                else:
                    self.status = "Install did not finish - see the LOGS tab for the details."
                self.detect()
                self.state = "idle"
        threading.Thread(target=job, daemon=True).start()

    def set_folder(self, p):
        p = os.path.expanduser((p or "").strip())
        if not game_ok(p):
            return "That folder does not contain Heroes.exe and the Data folder."
        with self.lock:
            self.gamedir = p
            self.cfg.set("Launcher", "GameFolder", p)
            self.detect()
        return ""

    def pick_folder(self):
        start = self.gamedir or os.path.expanduser("~")
        for cmd in (["zenity", "--file-selection", "--directory", "--title=Choose your KUF Heroes folder (the one with Heroes.exe)", "--filename=" + start + "/"],
                    ["kdialog", "--getexistingdirectory", start, "--title", "Choose your KUF Heroes folder (the one with Heroes.exe)"]):
            if shutil.which(cmd[0]):
                try:
                    r = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
                except Exception:
                    continue
                p = r.stdout.strip()
                return self.set_folder(p) if p else ""
        return "No folder chooser found - type the folder path instead."

    def action(self, a, arg):
        msg = ""
        if a == "ping":
            self.last_ping = time.time()
        elif a == "check":
            self.check()
        elif a == "main":
            if self.gamedir and self.state == "idle":
                if self.need():
                    self.install()
                else:
                    ok, m = launch_game(self.gamedir)
                    with self.lock:
                        self.status = m
                        if ok and self.close_on_play:
                            self.quit_at = time.time() + 2
        elif a == "repair":
            self.install()
        elif a == "uninstall":
            self.install(uninstall=True)
        elif a == "toggle":
            with self.lock:
                if arg == "test":
                    self.test = 0 if self.test else 1
                    self.cfg.set("Launcher", "TestBuilds", self.test)
                elif arg == "enc":
                    self.enc = 0 if self.enc else 1
                    self.cfg.set("Launcher", "Encablossa", self.enc)
                else:
                    self.close_on_play = 0 if self.close_on_play else 1
                    self.cfg.set("Launcher", "CloseOnPlay", self.close_on_play)
            if arg in ("test", "enc"):
                self.rel = None
                self.check()
        elif a == "gx":
            i, step = int(arg[0]), int(arg[1])
            p = self.gx_path()
            if p and 0 <= i < len(GX):
                with self.lock:
                    ini = WinIni(p)
                    lab, sec, key, vals, d, _ = GX[i]
                    v = ini.get(sec, key, d)
                    k = min(range(len(vals)), key=lambda n: abs(vals[n] - v))
                    ini.set(sec, key, vals[max(0, min(len(vals) - 1, k + step))])
                    try:
                        ini.save()
                    except OSError as e:
                        msg = "Could not save KUFH.ini (%s)" % e
        elif a == "folder":
            msg = self.set_folder(arg)
        elif a == "pick":
            msg = self.pick_folder()
        elif a == "openfolder":
            if self.gamedir and shutil.which("xdg-open"):
                subprocess.Popen(["xdg-open", self.gamedir], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        elif a == "quit":
            self.quit_at = time.time()
        return msg


PAGE_HTML = r"""<!doctype html><html><head><meta charset="utf-8"><title>Battle for Bersia MP Launcher</title>
<link rel="icon" href="data:image/png;base64,@ICON@">
<style>
:root{--gold:#e8b668;--pale:#e9dccb;--dim:#a8927c;--red:#7d1d17;--redhi:#9b2a21;--line:#b0804a;--panel:rgba(26,15,12,.92)}
*{box-sizing:border-box}html,body{margin:0;height:100%;background:#120a08;color:var(--pale);font:14px "DejaVu Sans","Noto Sans",Cantarell,Ubuntu,sans-serif}
#app{width:960px;height:600px;margin:0 auto;position:relative;background:#120a08 url(data:image/png;base64,@BG@) center/cover;overflow:hidden}
header{height:95px;border-bottom:1px solid var(--line);background:rgba(28,12,10,.55);position:relative}
.tab{position:absolute;top:36px;font:23px "DejaVu Serif","Liberation Serif",serif;color:var(--dim);cursor:pointer;user-select:none}
.tab.on{color:#fff}.tab.on:after{content:"";position:absolute;left:0;top:44px;width:80px;height:3px;background:#c0392b}
#logo{position:absolute;right:20px;top:8px}
.serif{font-family:"DejaVu Serif","Liberation Serif",serif}
.btn{background:var(--red);border:1px solid var(--gold);color:#fff;cursor:pointer;font:15px "DejaVu Serif",serif;display:flex;align-items:center;justify-content:center;user-select:none}
.btn:hover{background:var(--redhi)}.btn.off{background:#3a2520;border-color:#6a5040;color:var(--dim);cursor:default}
.page{position:absolute;left:0;right:0;top:96px;bottom:0;display:none}.page.on{display:block}
#news{position:absolute;left:470px;top:34px;width:450px;height:380px;border:1px solid var(--line);background:var(--panel);border-radius:6px;overflow:hidden}
#newshead{height:44px;background:#5a1611;display:flex;align-items:center;justify-content:space-between;padding:0 16px}
#newsbody{height:282px;overflow:auto;padding:10px 16px;white-space:pre-wrap;overflow-wrap:anywhere;font-size:13px;line-height:1.45}
.chk{display:flex;align-items:center;gap:10px;cursor:pointer;margin:0 0 12px;user-select:none}.box{width:16px;height:16px;border:1px solid var(--gold)}.box.on{background:var(--gold)}
input[type=text]{background:#1a0f0c;color:var(--pale);border:1px solid var(--line);padding:6px 8px;font:13px monospace;width:560px}
#menu{position:absolute;display:none;background:#1a0f0c;border:1px solid var(--line);z-index:5;min-width:250px}
#menu div{padding:8px 14px;cursor:pointer}#menu div:hover{background:var(--red);color:#fff}#menu div.off{color:#6a5a4c;cursor:default;background:none}
pre{margin:0}
</style></head><body><div id="app">
<header><div class="tab on" data-t="home" style="left:40px">HOME</div><div class="tab" data-t="settings" style="left:190px">SETTINGS</div><div class="tab" data-t="logs" style="left:370px">LOGS</div>
<img id="logo" src="data:image/png;base64,@LOGO@"></header>
<div class="page on" id="p-home">
 <div class="serif" style="position:absolute;left:40px;top:40px;font-size:28px;color:#fff">Battle for Bersia MP</div>
 <div id="l1" style="position:absolute;left:40px;top:96px"></div><div id="l2" style="position:absolute;left:40px;top:122px"></div>
 <div id="st" style="position:absolute;left:40px;top:156px;width:410px;color:var(--gold)"></div>
 <div id="main" class="btn" style="position:absolute;left:40px;top:344px;width:300px;height:64px;font-size:23px;font-weight:bold"></div>
 <div id="more" class="btn" style="position:absolute;left:344px;top:344px;width:40px;height:64px;font-size:22px">&#8942;</div>
 <div id="bar" style="position:absolute;left:40px;top:416px;width:344px;height:8px;border:1px solid var(--line);background:#1e100c;display:none"><div id="fill" style="height:100%;background:var(--gold);width:0"></div></div>
 <div id="foot" style="position:absolute;left:40px;top:470px;font-size:11px;color:var(--dim)"></div>
 <div id="news"><div id="newshead"><span id="ntitle" class="serif" style="color:var(--gold);font-weight:bold;font-size:16px"></span><span id="ndate" style="font-size:11px;color:var(--dim)"></span></div>
  <div id="newsbody"></div><div id="ghbtn" class="btn" style="margin:8px 16px;height:32px;font-size:13px;font-family:sans-serif">Read more on GitHub</div></div>
 <div id="menu"></div>
</div>
<div class="page" id="p-settings" style="padding:28px 40px">
 <div class="serif" style="font-size:19px;color:#fff;margin-bottom:8px">Game folder</div>
 <div style="display:flex;gap:8px;align-items:center"><input type="text" id="gd" spellcheck="false"><div class="btn" id="gdset" style="width:70px;height:30px;font-size:13px">Use</div><div class="btn" id="gdpick" style="width:100px;height:30px;font-size:13px">Browse...</div></div>
 <div id="gdmsg" style="color:#e68c78;font-size:12px;height:18px;margin:4px 0 10px"></div>
 <div class="chk" data-k="test"><div class="box"></div>Receive test builds (pre-releases, for testers)</div>
 <div class="chk" data-k="enc"><div class="box"></div>Encablossa edition</div>
 <div class="chk" data-k="close"><div class="box"></div>Close the launcher when the game starts</div>
 <div class="serif" style="font-size:19px;color:#fff;margin:16px 0 10px">Graphics</div>
 <div style="display:flex;gap:40px"><div id="gx"></div><div id="gxnote" style="font-size:11px;color:var(--dim);width:400px;line-height:1.5;white-space:pre-wrap;overflow-wrap:anywhere"></div></div>
</div>
<div class="page" id="p-logs" style="padding:20px 40px">
 <div class="serif" style="font-size:19px;color:#fff;margin-bottom:10px">Last install</div>
 <pre id="log" style="background:var(--panel);border:1px solid var(--line);height:380px;overflow:auto;padding:12px;font-size:12px;white-space:pre-wrap"></pre>
</div>
</div>
<script>
const KEY="@KEY@";let S=null,tab="home",gdEdit=false;
const $=i=>document.getElementById(i);
function api(a,arg){return fetch("/api",{method:"POST",headers:{"X-Key":KEY,"Content-Type":"application/json"},body:JSON.stringify({a:a,arg:arg===undefined?null:arg})}).then(r=>r.json()).then(j=>{if(j.msg){$("gdmsg").textContent=j.msg;if(tab!="settings")alert(j.msg)}poll()}).catch(()=>{})}
document.querySelectorAll(".tab").forEach(t=>t.onclick=()=>{tab=t.dataset.t;document.querySelectorAll(".tab").forEach(x=>x.classList.toggle("on",x==t));document.querySelectorAll(".page").forEach(p=>p.classList.toggle("on",p.id=="p-"+tab));render()});
document.querySelectorAll(".chk").forEach(c=>c.onclick=()=>api("toggle",c.dataset.k));
$("main").onclick=()=>{if(!S)return;if(!S.gamedir){tab="settings";document.querySelector('.tab[data-t=settings]').click();return}if(!$("main").classList.contains("off"))api("main")};
$("ghbtn").onclick=()=>window.open(S&&S.rel?S.rel.page:"https://github.com/DJSonnyb/battle-for-bersia-mp/releases","_blank");
$("gd").oninput=()=>gdEdit=true;$("gdset").onclick=()=>{gdEdit=false;$("gdmsg").textContent="";api("folder",$("gd").value)};$("gdpick").onclick=()=>{$("gdmsg").textContent="";api("pick")};
$("more").onclick=e=>{if(!S||S.state!="idle")return;const m=$("menu");const it=[["Check for updates","check",1],["Repair (reinstall this version)","repair",!!S.rel],["Uninstall the mod","uninstall",!!S.installed],["Open the game folder","openfolder",!!S.gamedir]];
 m.innerHTML="";it.forEach(([l,a,on])=>{const d=document.createElement("div");d.textContent=l;if(!on)d.className="off";else d.onclick=()=>{m.style.display="none";if(a=="uninstall"&&!confirm("Remove Battle for Bersia MP and put the original game files back?"))return;api(a)};m.appendChild(d)});
 m.style.left="344px";m.style.top="208px";m.style.display="block";e.stopPropagation()};
document.body.onclick=()=>$("menu").style.display="none";
function render(){if(!S)return;const s=S,busy=s.state!="idle";
 $("l1").textContent="Installed: "+(s.installed?s.installed+(s.note?"  ("+s.note+")":""):(s.note||"-"));
 $("l2").textContent="Latest: "+(s.rel?s.rel.name:(s.state=="checking"?"checking...":"-"))+(s.test?"   (test builds on)":"");
 $("st").textContent=s.quitting?"The launcher has closed - you can close this tab.":(s.status||(s.need?"New version available: "+s.rel.name:""));
 const lbl=!s.gamedir?"FIND GAME FOLDER":s.state=="checking"?"CHECKING...":s.state=="downloading"?"DOWNLOADING...":s.state=="installing"?"INSTALLING...":s.need?(s.installed?"UPDATE":"INSTALL"):"LAUNCH GAME";
 $("main").textContent=lbl;$("main").classList.toggle("off",busy||s.quitting);$("more").classList.toggle("off",busy||s.quitting);
 $("bar").style.display=busy&&s.state!="checking"?"block":"none";$("fill").style.width=(s.state=="installing"?100:Math.round(s.progress*100))+"%";$("fill").style.background=s.state=="installing"?"#96503c":"var(--gold)";
 $("foot").textContent="Mod: "+(s.installed||"not installed")+"  |  Launcher: "+s.version+"  |  "+(s.gamedir?"Game folder found":"Game folder not set");
 $("ntitle").textContent=s.rel?s.rel.name:"News";$("ndate").textContent=s.rel?s.rel.date:"";
 $("newsbody").textContent=s.rel?s.rel.body:(s.state=="checking"?"Checking GitHub for the latest release...":"No news.");
 if(!gdEdit)$("gd").value=s.gamedir||"";$("gd").placeholder="Not set - the folder with Heroes.exe";$("gdpick").style.display=s.can_pick?"flex":"none";
 document.querySelectorAll(".chk").forEach(c=>c.firstElementChild.classList.toggle("on",!!s[c.dataset.k]));
 const g=$("gx");if(!s.gx){g.innerHTML='<div style="color:#e68c78;width:520px">Start KUF Heroes once from Steam (so Proton makes its Documents folder), then come back here.</div>';$("gxnote").textContent=""}
 else{g.innerHTML="";s.gx.forEach((o,i)=>{const r=document.createElement("div");r.style.cssText="display:flex;align-items:center;margin-bottom:10px";
  r.innerHTML='<div style="width:150px">'+o.label+'</div><div class="btn" style="width:30px;height:26px">&lt;</div><div style="width:160px;text-align:center;color:var(--gold)">'+o.value+'</div><div class="btn" style="width:30px;height:26px">&gt;</div>';
  r.children[1].onclick=()=>api("gx",[i,-1]);r.children[3].onclick=()=>api("gx",[i,1]);g.appendChild(r)});
  $("gxnote").textContent="Saved straight away to KUFH.ini in the game's Proton folder and used from the next battle. Anti-Aliasing needs a game restart.\n\n"+s.gxpath}
 $("log").textContent=s.log||"Nothing installed in this session yet.";}
function poll(){fetch("/api/state",{headers:{"X-Key":KEY}}).then(r=>r.json()).then(j=>{S=j;render()}).catch(()=>{})}
setInterval(poll,500);setInterval(()=>api("ping"),3000);poll();
</script></body></html>"""


class _Handler(http.server.BaseHTTPRequestHandler):
    app = None
    key = ""
    page = b""

    def log_message(self, *a):
        pass

    def _send(self, code, body, ctype="application/json"):
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path == "/?k=" + self.key:
            self._send(200, self.page, "text/html; charset=utf-8")
        elif self.path == "/api/state" and self.headers.get("X-Key") == self.key:
            self.app.last_ping = time.time()
            self._send(200, json.dumps(self.app.snapshot()).encode())
        else:
            self._send(404, b"{}")

    def do_POST(self):
        if self.path != "/api" or self.headers.get("X-Key") != self.key:
            return self._send(403, b"{}")
        try:
            n = int(self.headers.get("Content-Length") or 0)
            j = json.loads(self.rfile.read(n) or b"{}")
            msg = self.app.action(j.get("a"), j.get("arg"))
        except Exception as e:
            msg = "Error: %s" % e
        self._send(200, json.dumps({"msg": msg}).encode())


class _Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True


def open_window(url):
    for b in ("google-chrome", "google-chrome-stable", "chromium", "chromium-browser", "brave-browser", "microsoft-edge", "vivaldi"):
        if shutil.which(b):
            try:
                subprocess.Popen([b, "--app=" + url, "--window-size=980,660"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, start_new_session=True)
                return
            except OSError:
                pass
    if shutil.which("xdg-open"):
        try:
            subprocess.Popen(["xdg-open", url], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, start_new_session=True)
            return
        except OSError:
            pass
    webbrowser.open(url)


def serve(open_browser=True, port=0):
    app = Launcher()
    key = secrets.token_hex(16)
    _Handler.app, _Handler.key = app, key
    _Handler.page = (PAGE_HTML.replace("@KEY@", key).replace("@BG@", BG_PNG.replace("\n", "")).replace("@LOGO@", LOGO_PNG.replace("\n", ""))
                     .replace("@ICON@", ICON_PNG.replace("\n", ""))).encode("utf-8")
    srv = _Server(("127.0.0.1", port), _Handler)
    url = "http://127.0.0.1:%d/?k=%s" % (srv.server_address[1], key)
    print("Battle for Bersia MP launcher %s" % LAUNCHER_VERSION)
    print("Opening the launcher in your browser. If nothing opens, go to:\n  " + url)
    print("(Close the launcher page to stop it, or press Ctrl+C here.)")
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    if open_browser:
        open_window(url)
    app.last_ping = time.time() + 30   # time for the browser to open
    try:
        while True:
            time.sleep(1)
            if app.quit_at and time.time() > app.quit_at:
                break
            if time.time() - app.last_ping > 20 and app.state == "idle":
                break   # page closed
    except KeyboardInterrupt:
        pass
    srv.shutdown()
    return 0


def cli(argv):
    import argparse
    ap = argparse.ArgumentParser(description="Battle for Bersia MP launcher (Linux). With no command it opens the launcher window.")
    ap.add_argument("command", nargs="?", default="window", choices=["window", "status", "install", "uninstall", "launch"])
    ap.add_argument("--folder", help="KUF Heroes folder (the one with Heroes.exe)")
    ap.add_argument("--test", action="store_true", help="take test builds (pre-releases)")
    ap.add_argument("--encablossa", action="store_true")
    ap.add_argument("--bat", help="use this installer .bat instead of downloading")
    ap.add_argument("--no-browser", action="store_true", help="window: only print the address")
    ap.add_argument("--port", type=int, default=0)
    a = ap.parse_args(argv)
    if a.command == "window":
        return serve(not a.no_browser, a.port)
    cfg = Config()
    gd = a.folder or cfg.get("Launcher", "GameFolder", "") or find_game()[0]
    if not game_ok(gd):
        print("KUF Heroes folder not found - pass --folder /path/to/the/folder/with/Heroes.exe")
        return 1
    print("Game folder:", gd)
    print("Installed:  ", installed_version(gd) or "not installed")
    if a.command == "status":
        r = check_releases(a.test or cfg.geti("Launcher", "TestBuilds", 0), a.encablossa)
        print("Latest:     ", r["name"] if r else "-")
        d = proton_docs(gd)
        print("KUFH.ini:   ", os.path.join(d, "KUF2 Heroes", "KUFH.ini") if d else "not found (start the game once)")
        return 0
    if a.command == "launch":
        print(launch_game(gd)[1])
        return 0
    bat = a.bat
    if not bat and a.command == "install":
        r = check_releases(a.test, a.encablossa)
        if not r:
            print("No release found.")
            return 1
        print("Downloading", r["name"], "...")
        bat = ci_path(gd, "BattleForBersiaMP_Install.bat")
        with open(bat + ".download", "wb") as f:
            http_get(r["url"], f, timeout=60)
        os.replace(bat + ".download", bat)
    elif not bat:
        bat = ci_path(gd, "BattleForBersiaMP_Install.bat")
    ok = run_installer(bat, gd, a.command == "uninstall", lambda t: print(" ", t.replace("\\", "/")))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(cli(sys.argv[1:]))
