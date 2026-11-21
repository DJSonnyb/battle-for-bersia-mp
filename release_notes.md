**REQUIRED TESTS FOR TEST 27** (everyone in a game must be on Test 27; send KUF_netlog.txt from every player after EVERY game, even if it seemed fine)

1. Ellen 2 co-op, start to finish: after the cut-scene, is Gerald there leading his troop? Do the cut-scenes show the right characters?
2. Gerald 5 co-op: does Regnier appear as himself (not as an orc)?
3. Any other co-op mission with story characters - do they all appear and lead their troops?
4. Versus, Invasion and Hero Battle: one quick game each - still no officers or story leaders there, and nothing else changed?
5. DirectX 12 (now the default): play a big battle, then switch Multiplayer Options > Renderer / AA to DirectX 9, restart the game and play the same battle. Which felt smoother? The netlog records the frame rate and which renderer was used. Report anything that looks wrong (water, effects, shadows, anti-aliasing).

For each game, note: mission or mode, who hosted, whether the "out of sync" message appeared, and anything that looks wrong.

**Test build 27 - for testers only. Not the official release (the official version is v1.4).**

- Co-op: story heroes and officers now lead their troops (e.g. Gerald in Ellen 2), so cut-scenes find the right units. Found and fixed by Keaton - thank you!
- Graphics: DirectX 12 (Windows' D3D9On12) is now on by default and should be faster on most PCs. Multiplayer Options > Renderer / AA switches back to DirectX 9 (restart the game after changing it). If your PC can't use it, the game uses DirectX 9 automatically.
- Everything from Test 26 (Xbox Live Options removed, co-op levels, co-op colours).

Please update from Test 26 - Test 26 and Test 27 players cannot join each other.

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
- Invasion variants (Create Game, Game Type): Invasion - Randomiser (the
  enemy's troops are random kinds every game, custom troops included; wave
  levels as normal), Invasion - Insanity (every enemy troop level 99, fully
  upgraded, with all its job's skills) and Invasion - Random Insanity (both).
- Multiplayer armies: 6 army slots instead of 3 (armies 4-6 are kept in
  MultiplayerArmies2.sav next to the first file).
- Lobby: six new team colours - black, white, turquoise, orange, pink and
  grey (12 in all). They are made in the game from its own team textures,
  so no extra files are needed.
- Multiplayer armies: Battle Order now opens the campaign's briefing troop
  list over the army screen. Your hero is fixed at the top; pick the 4 troops
  that go into battle (max 1 support). Press B and choose Save or Don't
  Save. The order is used in every mode.
- Co-op levels (original campaigns): both players' heroes and troops are now
  the typical (median) enemy level + 1. One very strong story enemy (e.g.
  Regnier in Gerald 5) no longer lifts the players' levels. Player 2 matches
  the host.
- Co-op with two players (all missions, Crusaders too): enemies get +15%
  levels (at least +1), and every enemy troop is at least 1 level above the
  players' hero.
- Co-op: computer allies (e.g. the Hironeiden army in Ellen 2) are raised to
  the players' hero level. Before, they could stay at level 1.
- Co-op: computer armies keep their campaign colours. Before, an army whose
  colour matched a player's lobby colour was given another colour, so the
  Hironeiden army changed colour depending on what player 2 picked.
- Options menu during a battle: the "Xbox Live Options" entry is gone. It
  crashed the game.
- Co-op: story heroes and officers now lead their troops (e.g. Gerald in
  Ellen 2), so cut-scenes find the right units. The network battle load was
  removing every mission leader except the players' heroes; co-op now keeps
  them. Versus, Invasion and Hero Battle are unchanged. Found and fixed by
  Keaton - thank you!
- KUF_netlog.txt now records each story hero in co-op (model, HP, state and
  position) to track down Gerald missing in Ellen 2.
- Co-op (Test 24 bug): Gerald invisible in Ellen 2 and Rupert's cut-scene
  shot going under the ground. Only officers now use the campaign look;
  story heroes leading a troop look as they did in Test 23.
- Lobby colours: Yellow no longer plays as red, and Brown is now its own
  brown (made in the game, like the new colours). Red uses the game's red.
- Lobby: your own colour icon is now really drawn as bright as the others
  (the colour button was being drawn over it).
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
- Graphics: DirectX 12, on by default. The game now runs through Windows'
  own D3D9On12 layer (Windows 10 / 11), which should be faster on most PCs.
  Multiplayer Options > "Renderer / AA" switches back to DirectX 9 (restart
  the game after changing it). If DirectX 12 isn't available on your PC the
  game simply uses DirectX 9 as before. Drawing only - players
  can use different settings. KUF_netlog.txt records which one was used.


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
- Renderer / AA: DirectX 9 or DirectX 12, with anti-aliasing Off, 2x, 4x, 8x
                  (default DirectX 12, Off). Restart the
                  game after changing it. If water or effects look wrong,
                  set it back to DirectX 9, Off.

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
