"""Write release/BattleForBersiaMP_Crusaders_Install.bat = cmd stub + PowerShell installer + base64 payload (crus_installer.py)."""
import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__))) + '/'   # repository root

import base64
K = ROOT
pk = open(K + 'dist/crus_payload.bin', 'rb').read()
b64 = base64.b64encode(pk).decode()
lines = [b64[i:i + 100] for i in range(0, len(b64), 100)]
BAT = r'''@echo off
rem ============================================================================
rem  Battle for Bersia MP - Crusaders co-op missions installer
rem  Put this file in your KUF Heroes folder (next to Heroes.exe) and run it.
rem  It needs Kingdom Under Fire: The Crusaders installed on this PC: every
rem  Crusaders mission, voice and music file is taken from YOUR Crusaders
rem  install - none are included in this file.
rem  Run "BattleForBersiaMP_Crusaders_Install.bat uninstall" to remove the files again.
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
$heroes = Split-Path -Parent $env:HMP_BAT
$listFile = Join-Path $heroes 'BattleForBersiaMP_Crusaders_files.txt'
Write-Host ''
Write-Host '  Battle for Bersia MP - Crusaders co-op missions installer' -ForegroundColor Cyan
Write-Host ''
if (-not (Test-Path (Join-Path $heroes 'Data\Mission'))) {
    Write-Host '  This file must be in your KUF Heroes folder (the one with Heroes.exe and the Data folder).' -ForegroundColor Red
    return
}
if ($env:HMP_MODE -eq 'uninstall') {
    if (-not (Test-Path $listFile)) { Write-Host '  Nothing to remove (BattleForBersiaMP_Crusaders_files.txt not found).'; return }
    $n = 0
    foreach ($r in Get-Content $listFile) { $p = Join-Path $heroes $r; if ($r -and (Test-Path $p)) { Remove-Item $p -Force; $n++ } }
    Remove-Item $listFile -Force
    Write-Host "  Removed $n Crusaders files." -ForegroundColor Green
    return
}
# ---- find the Crusaders install ----
function Test-Crus($p) { return ($p -and (Test-Path (Join-Path $p 'Data\Mission')) -and (Test-Path (Join-Path $p 'Data\Sound'))) }
$crus = $null
$libs = New-Object System.Collections.Generic.List[string]
foreach ($k in 'HKCU:\Software\Valve\Steam', 'HKLM:\SOFTWARE\WOW6432Node\Valve\Steam', 'HKLM:\SOFTWARE\Valve\Steam') {
    try { $v = Get-ItemProperty $k -ErrorAction Stop; foreach ($n in 'SteamPath', 'InstallPath') { if ($v.$n) { $libs.Add(($v.$n -replace '/', '\')) } } } catch {}
}
$libs.Add("${env:ProgramFiles(x86)}\Steam"); $libs.Add("$env:ProgramFiles\Steam")
foreach ($s in @($libs)) {
    $vdf = Join-Path $s 'steamapps\libraryfolders.vdf'
    if (Test-Path $vdf) { foreach ($m in [regex]::Matches((Get-Content $vdf -Raw), '"path"\s+"([^"]+)"')) { $libs.Add(($m.Groups[1].Value -replace '\\\\', '\')) } }
}
foreach ($l in ($libs | Select-Object -Unique)) {
    $c = Join-Path $l 'steamapps\common'
    if (Test-Path $c) { foreach ($d in Get-ChildItem $c -Directory -ErrorAction SilentlyContinue) { if ($d.Name -match 'Crusaders' -and (Test-Crus $d.FullName)) { $crus = $d.FullName; break } } }
    if ($crus) { break }
}
if ($crus) {
    Write-Host "  Found Kingdom Under Fire: The Crusaders at:"; Write-Host "    $crus"
    $a = Read-Host '  Use this folder? (Y/n)'
    if ($a -match '^[nN]') { $crus = $null }
}
while (-not $crus) {
    $p = Read-Host '  Paste the path of your Crusaders folder (the one with the Data folder), or press Enter to cancel'
    if (-not $p) { Write-Host '  Cancelled.'; return }
    $p = $p.Trim('"', ' ')
    if (Test-Crus $p) { $crus = $p } else { Write-Host '  That folder has no Data\Mission and Data\Sound folders - try again.' -ForegroundColor Yellow }
}
# ---- payload ----
$s = [IO.File]::ReadAllText($env:HMP_BAT)
$b64 = ($s.Substring($s.IndexOf('#PAY' + 'LOAD') + 9) -replace '[^A-Za-z0-9+/=]', '')
$gz = New-Object IO.Compression.GZipStream((New-Object IO.MemoryStream(, [Convert]::FromBase64String($b64))), [IO.Compression.CompressionMode]::Decompress)
$ms = New-Object IO.MemoryStream; $gz.CopyTo($ms); $pay = $ms.ToArray()
$jl = [BitConverter]::ToInt32($pay, 0)
$man = [Text.Encoding]::UTF8.GetString($pay, 4, $jl) | ConvertFrom-Json
$dbase = 4 + $jl
# ---- index the Crusaders Data folder ----
Write-Host '  Checking your Crusaders files...'
$idx = @{}
foreach ($f in Get-ChildItem (Join-Path $crus 'Data') -Recurse -File -ErrorAction SilentlyContinue) {
    $k = $f.Name.ToLower(); if (-not $idx.ContainsKey($k)) { $idx[$k] = New-Object System.Collections.Generic.List[object] }; $idx[$k].Add($f)
}
$hashes = @{}
function Get-Sha($p) { if (-not $hashes.ContainsKey($p)) { $hashes[$p] = (Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash.ToLower() }; return $hashes[$p] }
$srcOf = @{}; $missing = New-Object System.Collections.Generic.List[string]
foreach ($e in $man) {
    $hit = $null
    if ($idx.ContainsKey($e.src.ToLower())) {
        foreach ($f in $idx[$e.src.ToLower()]) { if ($f.Length -eq $e.size -and (Get-Sha $f.FullName) -eq $e.sha) { $hit = $f.FullName; break } }
    }
    if ($hit) { $srcOf[$e.out] = $hit } else { $missing.Add($e.src) }
}
if ($missing.Count) {
    Write-Host ''
    Write-Host ("  {0} of the {1} Crusaders files needed were not found or are a different version:" -f $missing.Count, $man.Count) -ForegroundColor Red
    $missing | Select-Object -Unique -First 15 | ForEach-Object { Write-Host "    $_" }
    Write-Host '  This installer needs the Steam version of The Crusaders with English voices. Nothing was changed.' -ForegroundColor Red
    return
}
# ---- build the files ----
Write-Host ("  Installing {0} files into {1}" -f $man.Count, $heroes)
$done = New-Object System.Collections.Generic.List[string]; $n = 0
foreach ($e in $man) {
    $dst = Join-Path $heroes $e.out
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dst) | Out-Null
    if ((Test-Path $dst) -and ((Get-FileHash -Algorithm SHA256 -LiteralPath $dst).Hash.ToLower() -eq $e.osha)) { $done.Add($e.out); continue }
    if ($e.op -eq 'copy') { Copy-Item -LiteralPath $srcOf[$e.out] -Destination $dst -Force }
    else {
        $src = [IO.File]::ReadAllBytes($srcOf[$e.out]); $o = New-Object IO.MemoryStream
        $p = $dbase + $e.off; $end = $p + $e.len
        while ($p -lt $end) {
            if ($pay[$p] -eq 67) { $a = [BitConverter]::ToInt32($pay, $p + 1); $c = [BitConverter]::ToInt32($pay, $p + 5); $o.Write($src, $a, $c); $p += 9 }
            else { $c = [BitConverter]::ToInt32($pay, $p + 1); $o.Write($pay, $p + 5, $c); $p += 5 + $c }
        }
        [IO.File]::WriteAllBytes($dst, $o.ToArray())
    }
    if ((Get-FileHash -Algorithm SHA256 -LiteralPath $dst).Hash.ToLower() -ne $e.osha) { Write-Host "  Failed to build $($e.out)" -ForegroundColor Red; return }
    $done.Add($e.out); $n++
    if ($n % 50 -eq 0) { Write-Host "    $n ..." }
}
Set-Content -LiteralPath $listFile -Value $done
Write-Host ''
Write-Host "  Done - $($done.Count) Crusaders files are installed." -ForegroundColor Green
Write-Host '  Gerald, Lucretia, Regnier and Kendal missions are now in the Co-op mission list, with Crusaders music.'
Write-Host '  Every player in a Crusaders co-op game needs to run this installer.'
'''
out = BAT.replace('\n', '\r\n') + '#PAYLOAD\r\n' + '\r\n'.join(lines) + '\r\n'
open(K + 'dist/BattleForBersiaMP_Crusaders_Install.bat', 'w', newline='').write(out)
print(len(out))
