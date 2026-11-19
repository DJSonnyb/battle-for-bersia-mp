**REQUIRED TESTS FOR TEST 22b** (2 players, both on Test 22b, both send KUF_netlog.txt after EVERY game, even if it seemed fine)

1. Ellen 2 co-op - play at least 5 minutes (Test 22 went out of sync about 2-3 minutes in, with either player hosting). Note roughly what was happening around 2-3 minutes in.
2. Player 2 in Ellen 2: do the catapult shots on the hill and the Gerald / Ellen scene show properly?
3. If you have time: another co-op mission (e.g. Ellen 3 or a Gerald mission) for 5-10 minutes, and Invasion on Ruin to wave 3.

For each game, note: mission, who hosted, and whether the "out of sync" message appeared.
The netlog can be bigger after a desync - that is expected.
Known: story heroes and officers (e.g. Rupert) may be shown as plain soldiers in co-op (being worked on).

**Test build 22b - for testers only. Not the official release (the official version is v1.4).**

Test 22b is Test 22 with extra desync recording only - nothing else changed. After a desync the netlog now records every troop (position, HP and what it is doing) for the last 64 turns, to find the Ellen 2 desync.

Please update from Test 22 - do not play Test 22.

```
KINGDOM UNDER FIRE: HEROES - BATTLE FOR BERSIA MP
v1.5 CHANGELOG (in testing - not released yet)
=================================================

Fixes
-----
- Sky going black or swinging around in long or busy battles.
- Multiplayer: when a desync happens the netlog now records the last 64 turns
  in detail. Please send KUF_netlog.txt from BOTH players after a desync.
- Co-op: fixed a major cause of desyncs, where the two players' games could
  disagree about what the mission script should do next.
- Co-op: fixed a crash for player 2 in Ellen 3.
- Co-op: player 2's troops now have the right strength for the mission.
  Before, their HP, damage and SP followed their Barracks level.
- Co-op: player 2's hero is now the same level as the rest of their army.
- Co-op (Gerald, Lucretia, Regnier, Kendal): both players' armies are now
  set from the mission's own enemies - about 2 levels above the typical
  enemy troop, heroes 2 levels higher again. Before, they followed a fixed
  curve and ended up far above the enemies (e.g. Gerald 7: troops level 26
  against enemies of level 6-14). Enemies still get +15% with two players.
- Co-op: troops are upgraded by their level (Archer -> Longbow from level
  10, Infantry -> Heavy Infantry -> Knight from 10 / 15, Cavalry -> Heavy
  Cavalry from 17 (never Storm Riders, which are support troops), Sapper -> PyroTech from 13, Dark
  Fighter -> Dark Knight from 7, Orc Infantry -> Axemen from 5 (Axemen are
  not made Orc Heavy - too slow), Orc Rider -> Heavy Rider from 17). This replaces the old fixed upgrades,
  so low-level missions can have fewer Knights than before.
- Co-op: player 2's camera now follows cut-scenes.
- Co-op: player 2's troops are now brought along when the mission teleports
  the host's army.
- Co-op: missions that end by going to the world map, loading the next
  mission or playing a movie no longer freeze at the end. This affected
  Ellen 1, 6 and 9, Rupert 6, Urukubarr 5, Leinhart 8, Walter 5, and
  Morene 4 and 5.
- Co-op: when the host leaves, player 2 returns to the menu and the game
  no longer closes.
- Create Game: Left and Right on Game Type now go back and forward.


New
---
- Co-op missions added: Rupert 6, Morene 2 and Cirith 2.
- Cirith 2: army level raised so it matches the enemies.
- Co-op cut-scenes: Player 2 now walks / runs along with the host's hero when
  the script moves it, and Player 2's cinematic shots follow the host's hero.
- Co-op: fixed a crash at the start of missions (e.g. Ellen 1 at the first
  cut-scene) that hit both players.
- Versus and Invasion: when a battle track has finished, another battle track
  is picked at random (Heroes' battle music, plus the Crusaders music when
  the Crusaders missions are installed) instead of the same one looping all
  game. Co-op missions keep their own music.
- Co-op: fixed an intro skipping itself on one PC only at the start of a
  mission (this put the two players out of sync from the first second).
- Co-op cut-scenes: player 2's camera no longer jumps back onto the troops
  at the start of some shots (e.g. the catapults and the Gerald / Ellen
  scene in Ellen 2 showed only trees or soldiers for player 2).
- KUF_netlog.txt now records every status effect (burning, shock and so on)
  given to a unit in network battles, to track down an Ellen 2 desync.
- After a desync, KUF_netlog.txt now also records every troop of every army
  (position, HP and what it is doing) for the last 64 turns, so the troop
  that went different on the two PCs can be found (Test 22b).
- Lobby colours: players on the same team can now pick the same colour in
  Versus (e.g. team 1 both blue, team 2 both red). In Invasion and co-op any
  colour can be shared.
- Lobby colours: the last two colours now have icons (purple and brown)
  instead of the words "White" and "Brown". The installer adds them to your
  own Data\Interface\Font2.dds (the original is kept as Font2_original.dds).
  If you install the community patch afterwards, run the installer again.
- Dwarf, Elf Warrior, Dark Elf Guard, Lich and Ogre troops now get their own
  equipment in the barracks instead of the human / orc gear they borrowed
  (e.g. Elf Warriors: elven sword, ring, bow or staff). Looks only, as with
  all multiplayer equipment. Existing troops of these kinds swap to the new
  default gear automatically.
- Multiplayer balance: archers hit about 15% softer in network battles -
  Archer 20 -> 17, Dark Archer and Cavalry Archer -15%, Longbow 26 -> 20
  (was 23). Single player is unchanged.
- Urukubarr 5 co-op: once the walls can be broken, the allied Scorpions stop
  chasing the respawning dark elves and go for the walls.
- Scorpion Shock Wave: fixed waves that crawled or never ended and left the
  Scorpion stuck (single player only in this test build).
- Graphics options, all on by default except anti-aliasing:
    - sharper textures at a distance (anisotropic filtering)
    - shadows covering a wider area
    - longer draw distance, with troop and AI sight unchanged
    - optional anti-aliasing (experimental)
- More detail in KUF_netlog.txt (sky, performance and co-op events) to help
  track down bugs.


Changing the graphics options
-----------------------------
On the title screen choose Multiplayer, then Multiplayer Options. Move to a
row and press Left / Right (or select it) to change it. "Save and Back"
or Back keeps the settings; they are used from the next battle.

- Texture Filter: Off, 2x, 4x, 8x, 16x (default 16x).
- Shadow Range:   Original, 2x, 3x, 4x (default 2x).
- Shadow Detail:  512 (original), 1024, 2048, 4096. Higher is sharper.
- Draw Distance:  100% (original) to 300% (default 150%). Lower it if big
                  battles get slow.

In this test build Draw Distance and Shadow Range are used in single player
only. Multiplayer battles run them as v1.4 did (original), for every player,
while the desyncs are tracked down. Texture Filter, Shadow Detail and
Anti-Aliasing work everywhere.
- Anti-Aliasing:  Off, 2x, 4x, 8x (default Off, experimental). Restart the
                  game after changing it. If water or effects look wrong,
                  set it back to Off.

The settings are kept in Documents\KUF2 Heroes\KUFH.ini.


Taken out again in this test build
----------------------------------
- Skills for computer-controlled troops in network co-op (test builds 11
  and 12). Co-op desynced in every mission with it, so it is removed until
  it can be made identical on both PCs. Scorpions will not Shock Wave the
  walls in Urukubarr 5 co-op for now.


Known issues still being looked at
----------------------------------
- Invasion desyncs reported with test build 12 (v1.4 is fine). This build
  runs Invasion and versus battles like v1.4 for draw distance, shadow range
  and Shock Wave - please retest Invasion.
- Story heroes and officers shown as plain soldiers in network co-op (the
  model fix from tests 13-20 is taken out again - it was a desync suspect).
- A crash in Gerald co-op when Ellen's officer move is used.
- Ellen 2 co-op: a desync about 9 minutes in, and player 2's troops are not
  moved into place for the Gerald / Ellen cut-scene.
- Flickering light on Paladins.
- Holy Explosion and Elemental Boost show no glow on the troops, and burning,
  shock, freeze, poison and curse show no effect on units (the effects work,
  only the visuals are missing - a PC port bug, to be restored from the Xbox
  version).
```
