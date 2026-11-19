**REQUIRED TESTS FOR TEST 24** (2 players, both on Test 24, both send KUF_netlog.txt after EVERY game, even if it seemed fine)

1. A co-op mission with enemy story heroes or officers, e.g. a Rupert, Leinhart or Gerald mission - 10 minutes. Are the story heroes and officers shown as themselves (not plain soldiers)? Do enemy heroes / officers use their spells? Does it stay in sync?
2. Urukubarr 5 co-op - do the allied Scorpions use Shock Wave on the walls? Does it stay in sync?
3. A co-op mission with dark elves or other spell-using computer troops (e.g. a Cirith or Morene mission) - 10 minutes.
4. Ellen 2 co-op again with the officer's fire spell - check it still stays in sync.
5. Gerald co-op: use Ellen's officer move (it used to crash) - does it crash?

For each game, note: mission, who hosted, whether the "out of sync" message appeared, and anything that looks wrong (missing or odd-looking units).

**Test build 24 - for testers only. Not the official release (the official version is v1.4).**

Test 23 fixed the Ellen 2 desync: spell hits now go through the host. With that fixed, Test 24 brings back what earlier tests had to take out because of desyncs:
- Co-op: story heroes and officers are shown as themselves again, and officers are back (co-op only).
- Co-op: computer-controlled troops have their skills again, as in single player.
- Co-op: spell heals and spell effects (burning, freeze...) are synced through the host too.
- Versus and Invasion: back to exactly how they played before Test 23 (no officers there).

Please update from Test 23 - do not play Test 23.

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
  against enemies of level 6-14). With two players enemies get +15% levels.
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
- Network battles: spell hits (e.g. the officer Fire Ball assist) no longer
  put the two PCs out of sync. A spell's hit is applied when its visual
  effect reaches the target, and that happened up to a second apart on the
  two PCs. Now the host's PC decides each spell hit and both PCs apply it on
  the same turn. This was the cause of the Ellen 2 co-op desyncs.
- Co-op levels (original campaigns): both players' heroes and troops are now
  the enemies' average level + 2 (Ellen 2: level 5 instead of 1). Player 2
  matches the host.
- Co-op with two players (all missions, Crusaders too): every enemy troop is
  at least 2 levels above the players' hero (Ellen 2: level 7).
- Lobby: your own colour icon is now drawn as bright as the others.
- Co-op: story heroes and officers (e.g. Rupert, Morene, the King) are shown
  as themselves again instead of plain soldiers, and officers are back. Their
  look only - Versus and Invasion are unchanged.
- Co-op: when a mission upgrades a troop (e.g. Infantry -> Knight), a story
  hero or officer leading it now stays its leader.
- Co-op: computer-controlled troops (enemies and allies) have their skills
  again, as in single player (e.g. Scorpions' Shock Wave, dark elf spells).
- Spell hits, heals and spell effects such as burning are synced through the
  host in co-op only; Versus and Invasion work exactly as before.
- After a desync, the netlog now also lists every hit in the last 64 turns
  (who hit whom, how hard, and with what), and records which attack caused
  each status effect (Tests 22c / 22d).
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
  chasing the respawning dark elves and go for the walls (and Shock Wave
  them again now that computer troops have their skills).
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


Known issues still being looked at
----------------------------------
- Invasion desyncs reported with test build 12 (v1.4 is fine). This build
  runs Invasion and versus battles like v1.4 for draw distance, shadow range
  and Shock Wave - please retest Invasion.
- A crash in Gerald co-op when Ellen's officer move is used.
- Ellen 2 co-op: player 2's troops are not moved into place for the
  Gerald / Ellen cut-scene.
- Flickering light on Paladins.
- Holy Explosion and Elemental Boost show no glow on the troops, and burning,
  shock, freeze, poison and curse show no effect on units (the effects work,
  only the visuals are missing - a PC port bug, to be restored from the Xbox
  version).
```
