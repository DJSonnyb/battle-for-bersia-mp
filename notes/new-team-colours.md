# Plan: 6 new team colours (after Test 21/22)

Agreed with the user 2026-11-18: add Black, White (slightly grey), Turquoise, Orange, Pink, Grey -> 12 colours.
Not before Test 21/22 results (keep desync testing clean).

## Findings (Heroes.exe, Steam)
- Per-slot colour tables in G (0xA75198): +0x718[slot] = texture colour, +0x738[slot] = second copy.
  Setters 0x6C31A0 / 0x6C31C0; getters 0x6AFD60 (+0x718), 0x6A4830 (+0x738).
- +0x738 is read only by 0x69BA5A (`cmp eax,6` special case) and copied to the save at 0x6AA65B -> keep +0x738 at an old colour (0-5).
- +0x718 users: battle-save copy 0x6AA640, texture prep 0x6B948A.. (0x6B9260), and 0x6AB8F0(key, slot) via 0x4E10E0 <- 0x8673E0 (unit view).
- Team texture table: vector at G+0x70C of entries {short group key; short tex[6]} (14 bytes).
  Fill 0x6AB800(key, tex, colour) (entry init loop `cmp ...,6`), lookup 0x6AB8F0(key, slot) -> tex[colour of slot].
  Only 6 colour slots -> replace both with our own table (e.g. 16 colours).
- Textures are requested per used colour as `<teamInfo group>_<colour+1>.tga` (packs hold .dds), loaded by 0x87D4C0 from the
  request list built by 0x6BAE90 (0x6BA88C loop) -> new colours need `<group>_7..12.dds`.
- 13 teamInfo groups; textures 128x128 / 256x256 DXT1 (data1.dat, data1_1500.dat holds _1.._4 of 11 groups).

## Build plan
1. Installer (C# via Add-Type): read the player's data1*.dat, for each group take two colour variants, mask = pixels that differ,
   recolour masked pixels (hue for colours, brightness for black/white/grey), write loose Data\Units\<group>_<n>.dds.
   Check in game that loose textures in Data\Units are found (likely: loose Font2.dds overrides the pack).
2. Hook 0x6AB800 / 0x6AB8F0 -> 16-colour table. Lobby NCOL 6 -> 12, col_game[] extended, +0x738 clamp to 0-5.
3. Lobby icons for the 6 new colours in empty named Font2 cells (TeamIndex1-6 etc.), drawn like scripts/font_icons.py.
