"""Decode and list the embedded data of the two installer .bat files, so reviewers can see exactly what they contain.

usage:  python inspect_installer.py BattleForBersiaMP_v1.4_Install.bat
        python inspect_installer.py BattleForBersiaMP_Crusaders_Install.bat

The text between '#PSSTART' and '#PAYLOAD' in each .bat is the complete, plain PowerShell installer.
Everything after '#PAYLOAD' is base64 of a gzip stream:
    u32 length of a JSON header, the JSON header, then a list of operations:
        'C' u32 offset u32 count  -> copy <count> bytes from the player's own source file at <offset>
        'L' u32 count <bytes>     -> insert these <count> bytes (the mod's own code / changed bytes)
The installer rebuilds each target file from those operations and checks the SHA-256 of the
source and of the result against the header before writing anything.
"""
import sys, re, base64, gzip, struct, json

s = open(sys.argv[1], 'r', newline='').read()
b64 = re.sub(r'[^A-Za-z0-9+/=]', '', s[s.index('#PAYLOAD') + 8:])
pay = gzip.decompress(base64.b64decode(b64))
jl = struct.unpack_from('<I', pay, 0)[0]
head = json.loads(pay[4:4 + jl])
ops = pay[4 + jl:]

def walk(buf):
    p = 0; c = l = lb = 0
    while p < len(buf):
        if buf[p:p + 1] == b'C': c += 1; p += 9
        else: n = struct.unpack_from('<I', buf, p + 1)[0]; l += 1; lb += n; p += 5 + n
    return c, l, lb

if isinstance(head, dict):          # main installer: Heroes.exe + Interface_Misc.txt
    print('Battle for Bersia MP installer, version', head['ver'])
    for f in head['files']:
        c, l, lb = walk(ops[f['off']:f['off'] + f['len']])
        print('  %s' % f['path'])
        print('    source (player\'s own file) sha256 %s size %d' % (f['osha'], f['osize']))
        print('    result                      sha256 %s size %d' % (f['nsha'], f['nsize']))
        print('    %d copy runs from the player\'s file, %d inserted blocks (%d bytes of mod content)' % (c, l, lb))
else:                               # Crusaders installer: list of files
    print('Crusaders installer: %d files' % len(head))
    for e in head:
        if e['op'] == 'copy':
            print('  copy   %-50s <- %s (sha256 %s...)' % (e['out'], e['src'], e['sha'][:16]))
        else:
            c, l, lb = walk(ops[e['off']:e['off'] + e['len']])
            print('  delta  %-50s <- %s: %d copy runs, %d inserts (%d bytes)' % (e['out'], e['src'], c, l, lb))
