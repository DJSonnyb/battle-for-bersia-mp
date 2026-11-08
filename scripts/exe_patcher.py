"""Build release/BattleForBersiaMP_v<ver>_Install.bat: turns the player's own Steam Heroes.exe into Battle for Bersia MP.
usage: exe_patcher.py <ver> <patched exe>    (original = the Steam Heroes.exe, SHA-256 checked)"""
import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__))) + '/'   # repository root

import sys, struct, gzip, base64, hashlib, json
K = ROOT
ORIG = _os_.environ.get('KUF_HEROES_EXE') or exit('set KUF_HEROES_EXE to the original Steam Heroes.exe')
ver, new = sys.argv[1], sys.argv[2]
a = open(ORIG, 'rb').read(); b = open(new, 'rb').read()
ops = bytearray(); i = 0; n = len(b)
while i < n:
    if i < len(a) and a[i] == b[i]:
        j = i
        while j < n and j < len(a) and a[j] == b[j]: j += 1
        ops += b'C' + struct.pack('<II', i, j - i); i = j
    else:
        j = i
        while j < n and not (j < len(a) and a[j:j + 16] == b[j:j + 16] and len(a[j:j+16]) == 16): j += 1
        if j >= len(a): j = n if j >= n else j
        ops += b'L' + struct.pack('<I', j - i) + b[i:j]; i = j
# self-check
r = bytearray(); p = 0
while p < len(ops):
    if ops[p:p+1] == b'C': o, c = struct.unpack_from('<II', ops, p + 1); r += a[o:o + c]; p += 9
    else: c = struct.unpack_from('<I', ops, p + 1)[0]; r += ops[p + 5:p + 5 + c]; p += 5 + c
assert bytes(r) == b
meta = json.dumps(dict(ver=ver, osha=hashlib.sha256(a).hexdigest(), osize=len(a), nsha=hashlib.sha256(b).hexdigest(), nsize=len(b))).encode()
pk = gzip.compress(struct.pack('<I', len(meta)) + meta + bytes(ops), 9, mtime=0)
b64 = base64.b64encode(pk).decode(); lines = [b64[k:k + 100] for k in range(0, len(b64), 100)]
BAT = r'''@echo off
rem ============================================================================
rem  Battle for Bersia MP v@VER@ - installer
rem  Put this file in your KUF Heroes folder (next to Heroes.exe) and run it.
rem  It turns YOUR OWN Steam Heroes.exe into Battle for Bersia MP v@VER@ (the original is
rem  kept as Heroes_original.exe). No game files are included in this file.
rem  Run "BattleForBersiaMP_v@VER@_Install.bat uninstall" to put the original back.
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
$exe = Join-Path $dir 'Heroes.exe'
$bak = Join-Path $dir 'Heroes_original.exe'
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
$he = Sha $exe; $hb = Sha $bak
if ($env:HMP_MODE -eq 'uninstall') {
    if ($hb -ne $m.osha) { Write-Host '  No original backup found (Heroes_original.exe). Use Steam > Verify integrity of game files instead.' -ForegroundColor Yellow; return }
    Copy-Item -LiteralPath $bak -Destination $exe -Force
    Write-Host '  The original Heroes.exe is back. (Heroes_original.exe was kept.)' -ForegroundColor Green
    return
}
if ($he -eq $m.nsha) { Write-Host "  Battle for Bersia MP v$($m.ver) is already installed." -ForegroundColor Green; return }
if ($he -eq $m.osha) { $src = $exe; if ($hb -ne $m.osha) { Copy-Item -LiteralPath $exe -Destination $bak -Force; Write-Host '  Backed up the original as Heroes_original.exe' } }
elseif ($hb -eq $m.osha) { $src = $bak; Write-Host '  Updating from your Heroes_original.exe backup' }
else {
    Write-Host '  Your Heroes.exe is not the Steam version this mod is made for (or is already modified),' -ForegroundColor Red
    Write-Host '  and no original backup was found. In Steam: right-click KUF Heroes > Properties > Installed Files >' -ForegroundColor Red
    Write-Host '  Verify integrity of game files, then run this installer again. Nothing was changed.' -ForegroundColor Red
    return
}
$a = [IO.File]::ReadAllBytes($src); $o = New-Object IO.MemoryStream
$p = 4 + $ml
while ($p -lt $pay.Length) {
    if ($pay[$p] -eq 67) { $o.Write($a, [BitConverter]::ToInt32($pay, $p + 1), [BitConverter]::ToInt32($pay, $p + 5)); $p += 9 }
    else { $c = [BitConverter]::ToInt32($pay, $p + 1); $o.Write($pay, $p + 5, $c); $p += 5 + $c }
}
$out = $o.ToArray()
$sha = [BitConverter]::ToString([Security.Cryptography.SHA256]::Create().ComputeHash($out)).Replace('-', '').ToLower()
if ($sha -ne $m.nsha) { Write-Host '  Building the new Heroes.exe failed - nothing was changed.' -ForegroundColor Red; return }
[IO.File]::WriteAllBytes($exe, $out)
Write-Host ''
Write-Host "  Done - Battle for Bersia MP v$($m.ver) is installed. Start KUF Heroes from Steam as usual." -ForegroundColor Green
Write-Host '  (If Steam verifies the game files it restores the original - just run this installer again.)'
'''.replace('@VER@', ver)
out = BAT.replace('\n', '\r\n') + '#PAYLOAD\r\n' + '\r\n'.join(lines) + '\r\n'
fn = K + 'dist/BattleForBersiaMP_v%s_Install.bat' % ver
open(fn, 'w', newline='').write(out)
print(fn, len(out), 'ops', len(ops))
