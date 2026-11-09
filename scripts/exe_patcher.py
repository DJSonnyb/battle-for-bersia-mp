"""Build dist/BattleForBersiaMP_v<ver>_Install.bat: turns the player's own Steam files into Battle for Bersia MP.
usage: exe_patcher.py <ver> <patched exe>
Targets (each checked by SHA-256, original kept as *_original):
  Heroes.exe                          <- original Steam Heroes.exe ($KUF_HEROES_EXE) + mod code
  Data\\Interface\\Interface_Misc.txt  <- the game's own file ($KUF_GAME) + ui/Interface_Misc.diff (the multiplayer menus)"""
import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__))) + '/'   # repository root
import sys, struct, gzip, base64, hashlib, json, subprocess, tempfile, shutil
ORIG = _os_.environ.get('KUF_HEROES_EXE') or exit('set KUF_HEROES_EXE to the original Steam Heroes.exe')
GAME = _os_.environ.get('KUF_GAME') or exit('set KUF_GAME to your KUF Heroes install folder')
ver, new = sys.argv[1], sys.argv[2]
sha = lambda b: hashlib.sha256(b).hexdigest()

def delta(s, t):
    """copy runs from s (8-byte gram index, prefers the expected position) + literal inserts; self-checked"""
    G = 8; idx = {}
    for i in range(0, len(s) - G + 1):
        l = idx.setdefault(s[i:i + G], [])
        if len(l) < 8: l.append(i)
    out = bytearray(); lit = bytearray(); i = 0; exp = 0
    def flush():
        nonlocal lit
        if lit: out.extend(b'L' + struct.pack('<I', len(lit)) + lit); lit = bytearray()
    while i < len(t):
        best = None; bl = 0
        cands = ([exp] if s[exp:exp + G] == t[i:i + G] and len(t[i:i + G]) == G else []) + idx.get(t[i:i + G], [])
        for c in cands:
            n = 0
            while i + n < len(t) and c + n < len(s) and t[i + n] == s[c + n]: n += 1
            if n > bl: bl, best = n, c
            if c == exp and n >= 4096: break
        if bl >= 16: flush(); out.extend(b'C' + struct.pack('<II', best, bl)); i += bl; exp = best + bl
        else: lit.append(t[i]); i += 1; exp += 1
    flush()
    r = bytearray(); p = 0
    while p < len(out):
        if out[p:p + 1] == b'C': a, n = struct.unpack_from('<II', out, p + 1); r += s[a:a + n]; p += 9
        else: n = struct.unpack_from('<I', out, p + 1)[0]; r += out[p + 5:p + 5 + n]; p += 5 + n
    assert bytes(r) == t
    return bytes(out)

# Interface_Misc.txt: the game's own file + our diff
ui_src = None
for n in _os_.listdir(_os_.path.join(GAME, 'Data', 'Interface')):
    if n.lower() == 'interface_misc.txt': ui_src = _os_.path.join(GAME, 'Data', 'Interface', n)
if not ui_src: exit('Data/Interface/Interface_Misc.txt not found in KUF_GAME')
tmp = tempfile.mkdtemp(); ui_new = _os_.path.join(tmp, 'Interface_Misc.txt'); shutil.copy(ui_src, ui_new)
subprocess.run(['patch', '-s', '--no-backup-if-mismatch', ui_new, ROOT + 'ui/Interface_Misc.diff'], check=True)

targets = [('Heroes.exe', 'Heroes_original.exe', open(ORIG, 'rb').read(), open(new, 'rb').read()),
           ('Data\\Interface\\Interface_Misc.txt', 'Data\\Interface\\Interface_Misc_original.txt', open(ui_src, 'rb').read(), open(ui_new, 'rb').read())]
shutil.rmtree(tmp)
meta = []; blob = bytearray()
for path, bak, a, b in targets:
    d = delta(a, b) if path.endswith('.txt') else None
    if d is None:   # exe: same-offset runs (the mod only appends sections and patches bytes in place)
        ops = bytearray(); i = 0; n = len(b)
        while i < n:
            if i < len(a) and a[i] == b[i]:
                j = i
                while j < n and j < len(a) and a[j] == b[j]: j += 1
                ops += b'C' + struct.pack('<II', i, j - i); i = j
            else:
                j = i
                while j < n and not (j + 16 <= len(a) and a[j:j + 16] == b[j:j + 16]): j += 1
                ops += b'L' + struct.pack('<I', j - i) + b[i:j]; i = j
        d = bytes(ops)
    meta.append(dict(path=path, bak=bak, osha=sha(a), osize=len(a), nsha=sha(b), nsize=len(b), off=len(blob), len=len(d)))
    blob += d
mj = json.dumps(dict(ver=ver, files=meta)).encode()
pk = gzip.compress(struct.pack('<I', len(mj)) + mj + bytes(blob), 9, mtime=0)
b64 = base64.b64encode(pk).decode(); lines = [b64[k:k + 100] for k in range(0, len(b64), 100)]
BAT = r'''@echo off
rem ============================================================================
rem  Battle for Bersia MP v@VER@ - installer
rem  Put this file in your KUF Heroes folder (next to Heroes.exe) and run it.
rem  It turns YOUR OWN Steam Heroes.exe and Data\Interface\Interface_Misc.txt
rem  into Battle for Bersia MP v@VER@ (the originals are kept as *_original).
rem  No game files are included in this file. Source: github.com/DJSonnyb/battle-for-bersia-mp
rem  Run "BattleForBersiaMP_v@VER@_Install.bat uninstall" to put the originals back.
rem ============================================================================
setlocal
set "HMP_BAT=%~f0"
set "HMP_MODE=%~1"
powershell -NoProfile -ExecutionPolicy Bypass -Command "$s=[IO.File]::ReadAllText($env:HMP_BAT); $i=$s.IndexOf('#PS'+'START'); $j=$s.IndexOf('#PAY'+'LOAD'); iex $s.Substring($i, $j-$i)"
echo.
pause
exit /b
#PSSTART
$ErrorActionPreference = 'Stop'
$dir = Split-Path -Parent $env:HMP_BAT
$s = [IO.File]::ReadAllText($env:HMP_BAT)
$b64 = ($s.Substring($s.IndexOf('#PAY' + 'LOAD') + 9) -replace '[^A-Za-z0-9+/=]', '')
$gz = New-Object IO.Compression.GZipStream((New-Object IO.MemoryStream(, [Convert]::FromBase64String($b64))), [IO.Compression.CompressionMode]::Decompress)
$ms = New-Object IO.MemoryStream; $gz.CopyTo($ms); $pay = $ms.ToArray()
$ml = [BitConverter]::ToInt32($pay, 0)
$m = [Text.Encoding]::UTF8.GetString($pay, 4, $ml) | ConvertFrom-Json
function Sha($p) { if (Test-Path -LiteralPath $p) { return (Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash.ToLower() } else { return '' } }
Write-Host ''
Write-Host "  Battle for Bersia MP v$($m.ver) installer" -ForegroundColor Cyan
Write-Host ''
if (-not (Test-Path (Join-Path $dir 'Data\Mission'))) { Write-Host '  Put this file in your KUF Heroes folder (the one with Heroes.exe and the Data folder) and run it again.' -ForegroundColor Red; return }
if (Get-Process -Name 'Heroes' -ErrorAction SilentlyContinue) { Write-Host '  Please close the game first.' -ForegroundColor Red; return }
# ---- check every file first, change nothing until all are fine ----
$plan = @()
foreach ($f in $m.files) {
    $dst = Join-Path $dir $f.path; $bak = Join-Path $dir $f.bak
    $hd = Sha $dst; $hb = Sha $bak
    if ($env:HMP_MODE -eq 'uninstall') {
        if ($hb -eq $f.osha) { $plan += ,@($f, 'restore', $bak, $dst) }
        elseif ($hd -ne $f.osha) { Write-Host "  No original backup of $($f.path) found - use Steam > Verify integrity of game files instead." -ForegroundColor Yellow }
        continue
    }
    if ($hd -eq $f.nsha) { $plan += ,@($f, 'ok', $dst, $dst); continue }
    if ($hd -eq $f.osha) { $plan += ,@($f, 'build', $dst, $dst); continue }
    if ($hb -eq $f.osha) { $plan += ,@($f, 'build', $bak, $dst); continue }
    Write-Host "  $($f.path) is not the Steam version this mod is made for (or is already modified)," -ForegroundColor Red
    Write-Host '  and no original backup was found. In Steam: right-click KUF Heroes > Properties > Installed Files >' -ForegroundColor Red
    Write-Host '  Verify integrity of game files, then run this installer again. Nothing was changed.' -ForegroundColor Red
    return
}
if ($env:HMP_MODE -eq 'uninstall') {
    foreach ($p in $plan) { Copy-Item -LiteralPath $p[2] -Destination $p[3] -Force; Write-Host "  Restored $($p[0].path)" -ForegroundColor Green }
    return
}
# ---- build ----
$outs = @()
foreach ($p in $plan) {
    $f = $p[0]
    if ($p[1] -eq 'ok') { continue }
    $a = [IO.File]::ReadAllBytes($p[2]); $o = New-Object IO.MemoryStream
    $q = 4 + $ml + $f.off; $end = $q + $f.len
    while ($q -lt $end) {
        if ($pay[$q] -eq 67) { $o.Write($a, [BitConverter]::ToInt32($pay, $q + 1), [BitConverter]::ToInt32($pay, $q + 5)); $q += 9 }
        else { $c = [BitConverter]::ToInt32($pay, $q + 1); $o.Write($pay, $q + 5, $c); $q += 5 + $c }
    }
    $out = $o.ToArray()
    $h = [BitConverter]::ToString([Security.Cryptography.SHA256]::Create().ComputeHash($out)).Replace('-', '').ToLower()
    if ($h -ne $f.nsha) { Write-Host "  Building $($f.path) failed - nothing was changed." -ForegroundColor Red; return }
    $outs += ,@($f, $out, $p[2])
}
foreach ($x in $outs) {
    $f = $x[0]; $dst = Join-Path $dir $f.path; $bak = Join-Path $dir $f.bak
    if ((Sha $bak) -ne $f.osha) { Copy-Item -LiteralPath $x[2] -Destination $bak -Force; Write-Host "  Backed up the original as $($f.bak)" }
    [IO.File]::WriteAllBytes($dst, $x[1])
}
Write-Host ''
Write-Host "  Done - Battle for Bersia MP v$($m.ver) is installed. Start KUF Heroes from Steam as usual." -ForegroundColor Green
Write-Host '  (If Steam verifies the game files it restores the originals - just run this installer again.)'
'''.replace('@VER@', ver)
out = BAT.replace('\n', '\r\n') + '#PAYLOAD\r\n' + '\r\n'.join(lines) + '\r\n'
_os_.makedirs(ROOT + 'dist', exist_ok=True)
fn = ROOT + 'dist/BattleForBersiaMP_v%s_Install.bat' % ver
open(fn, 'w', newline='').write(out)
print(fn, len(out), 'targets', [(m['path'], m['len']) for m in meta])
