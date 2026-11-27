# Custom maps, .nav generation and Keaton's findings (saved 2026-11-19)

## Agreed build plan
- Test 22: co-op fixes, music rotation, custom-job equipment, lobby colours (team mates share any colour; purple/brown icons).
- Test 23: custom maps slot + lobby auto-download of map files between players + file check before battle;
  six new team colours (notes/new-team-colours.md); .nav generation in the editor; Bersia_Map_Forge.html on the GitHub test page.
- Testers use the standalone tools/map-editor/Bersia_Map_Forge.html (the artifact page can't save for people outside the account).

## What really blocks troops (Keaton, confirmed against the code)
- The .map cell grid (chunk 2) never blocks movement. 0x40 is read only by the troop-AI attack check (FUN_00574bb0) and
  Set Fire targeting (FUN_0047bb00); 0x38 obstacles give collision/bounce (trees, props); 0x07 slow levels + slope give drag
  (speed factor clamp 0.2..1.2, never zero). Keaton: "initial resistance, then you can walk through".
- Only the .nav mesh stops troops (they can't leave it, slide along open edges), plus the 3000-unit map edge and the troop's
  allowed rectangle. Most Live maps have no .nav -> troops can walk through walls/cliffs there.
- Docs say the PC game never loads .nav from Data\Mission\Live (only 4_Colonock, 4_ToukenCliff, 6_DryGlade, 6_Nymphbarren ship one)
  -> the mod must point the loader (FUN_005985c0, called from FUN_00595010) at the Live folder.
- ACT_BLOCK_AREA (63) / ACT_OPEN_AREA (64): used by no shipped stage file (scanned all 209); BLOCK_AREA fills flipped rows. Skip.

## Castle walls (Keaton)
- Wall cells = 0x60 (0x20 special/large collision + 0x40), a 0x40 surround strip; both cleared when the wall breaks.
- Base .nav covers the wall area with small dense triangles; K2CastleWall cuts nav cells within 1200 units at battle start and
  restores them when broken -> placed castle walls (401, 425, 430, 434 + broken 402/426/431) become real breakable walls.
- Wall HP: ACT_SET_WALL_HP (144) by prop instance id (used 30x: 0080, 2000, 2040, 3060).
- Map chunks per Keaton: 00 height/tiles, 01 props, 02 collision (0x80 forest), 03 exposure/canopy, 04 water/river, 05 fuel.
  (Our code check: arrow/wyvern forest penalty uses the 0x80 bit, x0.8 shooter / x0.5 target; chunk 3 is loaded but no reader
  found. Editor keeps 3 and 5 in step with forest anyway.)

## Editor work for Test 23
1. .nav generator: walkable cells minus Block paint and solid props; conforming triangulation (shared edges must match corner to
   corner or the game won't link cells, FUN_00799c80 / FUN_0059d2d0); dense near castle walls; target a few hundred triangles.
2. Placing a castle wall stamps 0x60 footprint + 0x40 strip, rotated, footprint from the player's PropAttrMap.DAT.
3. "Wall HP" box per placed wall -> ACT_SET_WALL_HP in the stage triggers.
4. Warn when heights change under an existing river (chunk 4/14 kept as-is; no river making yet).
5. Ask Keaton to check the first generated .nav with her parser.

## Keaton's 255-troop engine edit (not released yet)
- Makes 255 troops on a map run without lag; she is still fixing volume (sound) problems. User will ask her to share when ready.
- Before adding: lockstep safety (any change to troop updates must be identical on both PCs; render/sound-only changes are safe),
  her list of changed addresses/bytes to check against our hooks, and fit with our sound code (music rotation, snd fixes).
- Ties in with: 5-player slowdown, Insanity/bigger waves, editor limits (stage troop warning at 230, max troops at once 230).
- After co-op is stable.
