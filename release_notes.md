**REQUIRED TESTS FOR TEST 47** (everyone in a game must be on Test 47; send KUF_netlog.txt from EVERY player after EVERY game, even if it seemed fine)

New in Test 47:
0. Urukubarr 2 co-op: the crash for player 2 (when the mission turns the minimap on) is fixed. Please play it again.

From Test 46:
1. Co-op desyncs between PCs with different processors (Intel / AMD): the game's vector maths now gives the same result on every PC. Please replay Urukubarr 1, Leinhart 1, Walter 5 and Ellen 9 in co-op. Does "OUT OF SYNC" still appear? Send BOTH netlogs either way.
2. Ellen 9 co-op: player 2 and the computer's allies are now level 30 like the host (player 2 was level 24). Better balanced?
3. Co-op: the heroes' own troops now get level upgrades too (e.g. Ellen leads Knights in Ellen 9, not basic Infantry).
4. Co-op enemies: heroes, officers and troop leaders now have +70% HP (was +50%), and enemy officers / officer-type leaders hit 20% harder. Too weak, right, or too strong?
5. If enemy troops seem to do far too much damage, say which mission and which troops, and send both netlogs (the log shows every troop's real level).

From Test 45:
6. "OUT OF SYNC" only shows for a real desync (two checks in a row differ).
7. File-mismatch warning also shows in the battle message box.

Still to check from earlier tests:
8. Scorpion voice lines; Black Wyverns vs archers / Dirigibles; soldiers' sounds at 40 m; AI target fix; Dark Elf Taming.
9. Effects: Heal and Chilling Attack sprites, status effects, Elemental Boost aura, Flame Wraith / Ice Maiden glow.
10. Invasion Randomiser; co-op cut-scenes; custom map auto-download; Roguelike cards and Reinforcements.
11. Normal Invasion, Versus and co-op: still exactly as before?

Known, not fixed yet: Ellen's weapon disappears after she is knocked down.

For each game, note: mission or mode, who hosted, whether "OUT OF SYNC" appeared, and anything that looks, sounds or feels wrong.

**Test build 47 - for testers only. Not the official release (the official version is v1.4).**

- NEW: Urukubarr 2 co-op crash (player 2) fixed.
- From Test 46: fix for co-op desyncs between PCs with different processors.
- NEW: Ellen 9 co-op player 2 and allies at the host's level (30).
- NEW: heroes' own troops get level upgrades in co-op.
- NEW: co-op enemy leaders +70% HP; enemy officers hit 20% harder.
- Includes everything from Test 46 and earlier.

Please update from Test 46 - everyone in a game should be on the same test build.

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
  against enemies of level 6-14). With two players enemies get +9% levels (at least +3).
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
- After leaving an online battle early (host left, disconnected or quit),
  single-player battles in the same session no longer keep online-only
  rules such as the Versus damage scale or leaders' item stats being
  ignored.
- Computer troops: fixed an original game bug where AI target filters
  numbered above 127 were ignored or misread. Flying troops, archers using
  Fire Arrow and catapults could pick troops of their own side as targets,
  and some campaign AI orders never worked (thanks Keaton).


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
- Co-op with two players (all missions, Crusaders too): enemies get +9%
  levels (at least +3), and every enemy troop is at least 1 level above the
  players' hero.
- Co-op: computer allies (e.g. the Hironeiden army in Ellen 2) are raised to
  the players' hero level. Before, they could stay at level 1.
- Ogre troops were invisible in multiplayer battles: their coloured ogre
  looks were never loaded there. They are now loaded for multiplayer too; if
  that ever fails, Ogres are drawn as Urukubarr-style ogres instead of
  disappearing.
- Invasion randomiser: enemy troops can now also become the new custom troops
  (dwarves, elves, dark elf guards, liches, ogres), with their own models.
  Before, an enemy Lich showed as a troop of Geralds, could not be killed and
  could crash the game.
- The new team colours showed the colour before them in battle (black came
  out brown, white black, turquoise white, and so on). Fixed.
- The new team colours (and brown) now keep the original artwork's detail.
  Only the team-coloured cloth is recoloured, with its light and shade kept;
  trims, metal, leather and gold stay as they are, like the game's own colours.
- Sound: mortar, cannon, mine and trap explosions, troop victory cheers, the
  archers' bow draw and some Storm Rider and Regnier sounds now play their own
  sounds. Since the original PC version they played a quiet sword swing or
  nothing (one wrong number in the game). Single player gets this too.
- Co-op: every officer now has its troop's level - enemy officers, computer
  allies' officers and both players' assist officers. Every officer in the
  original missions is level 1, so the level raises only reached the troop
  leaders and officers died in a few hits.
- Co-op difficulty raised again now that officers are levelled (they get
  their spells, e.g. Meteor): with two players enemies get at least +3 levels (was +1), and every enemy troop is at least 3 levels above the
  players' hero (was 1). Enemy officers follow their troop's level.
- Elvish speech is shown as Elvish words (e.g. "Dal fijas waka quwehdedlo!")
  instead of symbols like "&$* 5.,$". The PC version lost the Elvish rune
  letters from its font. Single player gets this too.
- Graphics: DirectX 9 is the default again (a few crashes with DirectX 12).
  DirectX 12 stays in the Options menu; everyone starts on DirectX 9 once,
  even if they had picked DirectX 12 before.
- Ogres: the coloured ogre models are now loaded in multiplayer, so Ogre
  troops show their mixed ogre colours instead of all being Urukubarr's model.
  The game only loaded one ogre model; the other three were never loaded. If a
  model still fails to load, that ogre falls back to Urukubarr's model, never
  invisible.
- Graphics: spell effects (Bless & Heal, Heal, Holy Explosion, Elemental
  Boost and other glows and particles) should no longer look blocky or
  pixelated. The game asked for a texture filter that many graphics cards
  don't offer for enlarged effect pictures, so they fell back to square
  pixels; enlarged pictures now use smooth (linear) filtering. Single player
  gets this too. (Test 37: the in-game Anisotropy option no longer turns the
  effect pictures back to the blocky filter.)
- Co-op difficulty lowered a little (players found Test 34 too hard): with
  two players enemies now get +9% levels (was +15%), still at least +3.
- NEW: Invasion - Roguelike (first version, pick it in Create Game like the
  other Invasion variants):
  - Every army starts the battle at level 1 (your Barracks armies are not
    changed) and gains 2 levels at the start of every wave.
  - Before every wave you choose 1 of 3 upgrades for your whole army: press
    1, 2 or 3 during the countdown (15 seconds). No choice = a random one of
    the three. Upgrades stack. The cards come from: Attack +10%, Defence +8%,
    Elemental resist +15%, HP +12%, Charge/blast resist +12%, Glass cannon
    (+25% attack, but 12% more damage taken) and +3 levels.
  - Every wave the enemy gets a random trait that also stacks: Ironclad,
    Berserk, Warded, Hardened or Brutal. The countdown shows the next one.
  - Resistances never go past 75%.
  - Keyboard only for now (1, 2, 3); controller support comes later.
  - Fixed (Test 36 feedback): the cards were drawn on top of each other and
    key presses were missed. They are now one readable list under the
    countdown, and 1 / 2 / 3 (or numpad 1 / 2 / 3) are caught at any moment.
- NEW: Invasion - Roguelike Hero (USER, Iddy's idea): like Roguelike, but you
  start with only your hero. Your 3 other troops wait off the field as random
  kinds of troop; a "Reinforcements" card brings one in at the next wave
  start (on every odd wave one of the 3 cards is Reinforcements while any
  troop still waits).
- Co-op cut-scenes (Keaton's fix): a cut-scene's camera no longer drops the
  character it was told to film (e.g. Gerald in Ellen 2's second scene) on
  both players, and player 2 no longer gets the host's hero instead of the
  scene's actor. Only the view changes.
- NEW: Custom maps in the lobby (first part). Maps saved by Bersia Map Forge
  into Data\Mission\Live show up in the map list as "Custom: <name>".
  - Every player needs the same map files: the lobby checks them, and Start
    says "A player lacks this map" until everyone has them.
  - Auto-download: a player who joins without the host's custom map (or with
    a different version of it) gets it from the host automatically - the
    chat says "Downloaded custom map <name>" and Start works a moment later.
    Only custom maps are sent, never the built-in ones.
  - The same check now runs for the normal multiplayer maps too, so an edited
    copy of a built-in map can't cause an "out of sync" (Start says "Map files
    differ").
  - The netlog notes whether the map has a navigation mesh (.nav) - the only
    thing that stops troops walking through walls and cliffs.
- Status effects now show on units (Keaton's findings): burning, shock,
  freeze, poison and curse, plus the Elemental Boost and Holy Explosion
  auras. The PC version never drew them (only the unit's colour changed);
  they now work the way they do on the Xbox. Looks only.
- Effects fixed to a unit now show: the Flame Wraith's fire, the Ice
  Maiden's icy glow, hand glows and other effects attached to units. They
  stayed behind instead of following the unit.
- Bless & Heal, Heal and Chilling Attack (Keaton's findings): their glowing
  sprites no longer come out as blocky squares. The PC version drew them the
  Xbox way, which mixed in leftovers from whatever was drawn before them.
- Invasion Randomiser now uses every troop: besides the normal Human and
  Dark Legion troops and the custom troops, enemy troops can become Bomber
  Wings, Mortars, Ballistas, Catapults, Battaloons, Scorpions, Swamp
  Mammoths, Dirigibles, Wyverns, Bone Dragons, Exploding Ghouls, every kind
  of Encablossa and the four elementals. Boss and support waves are
  randomised too. Walls and heroes stay as they are.
- Co-op (all missions, Crusaders too): enemy heroes, officers and troop
  leaders have +70% HP, so they no longer go down in a few hits. Enemy
  officers and officer-type troop leaders also hit 20% harder. Levels are
  unchanged. Computer allies are not affected.
- Network battles: the maths mode of the processor is now set the same on
  every PC each turn. A different renderer (Direct3D 12) or an overlay /
  capture tool could change it on one PC only, and the battle then went out
  of sync. The netlog shows when it had to be reset.
- Each test build now has its own build id, so players on different test
  builds get the "different builds" warning in chat at the start of a
  battle (before, all v1.5 test builds looked the same).
- Network battles: when the players' games go out of sync, "OUT OF SYNC"
  now appears in the battle's message box (where "Fire at Will: ON" shows),
  once per battle, and again in the lobby chat afterwards. Before, a desync
  was only recorded in KUF_netlog.txt.
- Sound: soldiers' action sounds (bows firing, mortar men firing, sword
  swings and so on) were cut off 15 m from the camera, so with the normal
  battle camera many were silent and others only played when the camera was
  close or at a low angle. They now carry to 40 m (they still get quieter
  with distance).
- Co-op: the lobby now warns when the players' camera files (Data\Camera,
  e.g. the user.cam from Keaton's patch) differ. Different camera files
  change the campaign's cut-scenes and put the game out of sync.
- Co-op: when a game goes out of sync, KUF_netlog.txt now records which part
  of a troop went different first (position, height, HP or what the soldiers
  were doing), to track down the Urukubarr 1 desync.
- Co-op: fixed a desync between PCs with different processors (seen in
  Urukubarr 1 about a minute in). The game's vector maths (from Direct3D's
  helper library) picks different code on different processors, so Intel and
  AMD PCs could get a slightly different result and a troop slowly drifted
  apart. The game now uses its own version that gives the same result on
  every PC.
- Crash in Urukubarr 2 co-op (player 2): the mission script turns the
  minimap on, and the game crashed when the minimap was not there. It is
  now skipped in that case.
- Co-op: in missions where the host's army has a minimum level (Ellen 9:
  level 30), player 2 and the computer's allied troops now get that level
  too. Before, player 2 was at 24 next to a level 30 host, and died very
  quickly to the level 27 wall archers. The enemies' levels are unchanged.
- Co-op: the heroes' own troops now get the same level-based upgrades as the
  host's other troops (e.g. Ellen's infantry become Knights in Ellen 9).
- Co-op: the OUT OF SYNC message no longer shows for a one-off difference
  that matches again at the next check. It now only shows when two checks
  in a row differ (about one second apart). One-off differences are still
  recorded in KUF_netlog.txt.
- Sound: Scorpion troops (orc army) answered orders with the wrong voice
  line - Stop played their victory line. The game's own scorpion voice file
  is missing two lines, so every later line was off by two; they now play
  the right ones.
- Dark Elf troops can now learn Taming in the skill window (it was only
  open to Orc troops).
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
- Renderer / AA: DirectX 12, or DirectX 9 with anti-aliasing Off, 2x, 4x, 8x
                  (default DirectX 12). Anti-aliasing is only offered with
                  DirectX 9 (with DirectX 12 it could leave the screen
                  blank on some PCs). Restart the
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
