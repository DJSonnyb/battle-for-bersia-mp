# Bersia Map Forge

Browser map editor for Battle for Bersia MP. `bersia-map-forge.html` is the built page (published as a private artifact).

- `core.js` - .map / .stg / minimap DDS / zip code (no DOM). `node rt.js` rebuilds every map and stage file in `$KUF_GAME/Data/Mission` and checks they come out byte-identical.
- `waves.js` - Invasion waves panel (actions 183/184/185 in the stage triggers) and the change-only byte diff used for stage files in projects; spliced into `app.js`.
- `app.js`, `page.html`, `propcat.min.json` - editor UI; `python3 assemble.py` builds `bersia-map-forge.html` (and `test.html` for `node e2e.js`, a Playwright run on 9_IceStorm; `node e2e2.js` tests wave editing, undo and the project round trip).
- No game files ship: the player opens their own map; project files (`.mapforge.json`) store only the changes against the original.
