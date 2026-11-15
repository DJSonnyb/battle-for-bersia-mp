@echo off
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
#PAYLOAD
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
ZgIA
