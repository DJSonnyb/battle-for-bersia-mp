```
KINGDOM UNDER FIRE: HEROES - BATTLE FOR BERSIA MP v1.5
=================================================

Online multiplayer for Kingdom Under Fire: Heroes (Steam PC).

Please report any bug, desync or crash (see "Reporting problems").

What's new in v1.5
------------------
NEW FACTION: ENCABLOSSA
- Play as the Encablossa with their own hero, E.Regnier - a darker Regnier with blue fire
- Four Encablossa troop types: Brood, Spitter, Infested and Flyer
- Each troop type has its own skills to learn (Ranged for Spitters, Frontal for Infested, Riding for Flyers, magic for the ground troops)
- Encablossa portraits and barracks pictures

CO-OP CAMPAIGN
- New co-op missions: Rupert 6, Morene 2 and Cirith 2
- Army levels now follow the mission's enemies, so you're no longer far stronger than the enemy
- With two players the enemies get stronger to match
- Enemy heroes, officers and troop leaders are much tougher and hit harder
- Every officer now has its troop's level, so officers no longer die in a few hits and use their spells (e.g. Meteor)
- Troops upgrade with their level (Archer to Longbow, Infantry to Knight and so on)
- Computer-controlled troops have their skills again, as in single player
- Story heroes and officers lead their troops and look like themselves again
- Computer allies keep up with your level and keep their campaign colours
- Player 2's camera follows cut-scenes
- Player 2's troops come along when the mission moves the host's army
- Missions that ended on the world map, the next mission or a movie no longer freeze at the end
- Many desync causes fixed, including Ellen 2 and Urukubarr 1
- Crashes fixed in Ellen 1, Ellen 3 and Urukubarr 2
- Keaton's Urukubarr 1 camera fix comes with the installer
- Host troops get their full job skills again (Archers keep Melee and Ranged, etc.)
- Player 2's skills are scaled to the mission without losing levels
- Troops look like their troop type (Knights in full plate, heroes keep their own troops)
- If player 2 brings no support troop, they get a copy of the host's support troop
- Damage taken is spread across the troop like in single player

INVASION
- Invasion - Randomiser: random enemy troops every game, from every troop in the game (Wyverns, Bone Dragons, Mammoths, Dirigibles, Encablossa, elementals, custom troops and more)
- Invasion - Insanity: every enemy troop level 99, fully upgraded
- Invasion - Random Insanity: both at once
- Invasion - Roguelike: start at level 1, gain levels every wave, and pick 1 of 3 upgrade cards before each wave while the enemy gains a new trait
- Invasion - Roguelike Hero: start with just your hero and call in reinforcements with cards

LOBBY AND ARMIES
- 6 multiplayer army slots instead of 3
- 12 team colours (new: black, white, turquoise, orange, pink and grey)
- Team-mates can pick the same colour in Versus
- Custom maps: maps made with Bersia Map Forge show up in the lobby, and players who don't have the map download it from the host automatically
- The lobby checks that everyone has the same map and camera files
- "OUT OF SYNC" now shows in the battle if a game desyncs
- Versus and Invasion pick a new battle track when one finishes instead of looping
- Battle Order: choose which troops go into battle on the campaign's briefing
  screen - hero + 3 normal troops + 1 support troop (yellow slot)
- Launcher 1.0.3: new MAP EDITOR tab - downloads and opens Bersia Map Forge

GRAPHICS
- New graphics options: sharper textures, wider shadows, sharper shadows, longer draw distance and anti-aliasing
- Optional DirectX 12 (DirectX 9 stays the default)
- Status effects show on units again (burning, shock, freeze, poison, curse, Elemental Boost and Holy Explosion auras) - the PC version never drew them
- Effects attached to units now follow them (Flame Wraith fire, Ice Maiden glow and so on)
- Spell effects like Bless & Heal no longer look blocky or pixelated
- Ogre troops show their proper models and colours in multiplayer (they could be invisible before)

SOUND AND TEXT
- Explosions, cheers, bow draws and some Storm Rider and Regnier sounds play their real sounds again (on PC they were a quiet sword swing or silent since release)
- Soldiers' action sounds carry much further, so battles sound fuller
- Scorpion troops answer orders with the right voice lines
- Elvish speech shows Elvish words instead of random symbols

TROOPS AND BALANCE
- Dwarf, Elf Warrior, Dark Elf Guard, Lich and Ogre troops get their own equipment in the barracks
- Dark Elf troops can learn Taming
- Archers hit a little softer in multiplayer battles
- Fixed an original game bug where flying troops, Fire Arrow archers and catapults could target their own side, and some campaign AI orders never worked

OTHER FIXES
- Fixed the sky going black or spinning in long battles
- Single-player battles no longer keep online rules after leaving an online battle early
- Removed the "Xbox Live Options" entry from the in-battle Options menu (it crashed the game)

Known issues
------------
- Encablossa troops make no movement or attack sounds on the Versus maps
  (they do in Invasion and co-op).
- Ellen 2 co-op: player 2's troops are not moved into place for the
  Gerald / Ellen cut-scene.


Requirements
------------
- Kingdom Under Fire: Heroes, Steam version.
- Everyone in a game must use the SAME version of Battle for Bersia MP.
- Optional: Kingdom Under Fire: The Crusaders (Steam) for the Crusaders
  co-op missions. You must own and have The Crusaders installed.


Installing
----------
1. Open your KUF Heroes folder. In Steam: right-click KUF Heroes >
   Manage > Browse local files. It is usually:
     C:\Program Files (x86)\Steam\steamapps\common\KUF Heroes\
2. Copy BattleForBersiaMP_v1.5_Install.bat into that folder (next to Heroes.exe)
   and double-click it. Close the game first.
   It checks that your Heroes.exe is the Steam version, keeps it as
   Heroes_original.exe and turns it into Battle for Bersia MP v1.5.
   It also installs the multiplayer menus (Data\Interface\Interface_Misc.txt);
   your old copy of that file is kept as Interface_Misc_original.txt,
   plus the Encablossa barracks/portrait pictures and Keaton's camera fix.
3. Start the game from Steam as usual. The title screen shows
   "Battle for Bersia MP v1.5" when the mod is active.

Updating to a newer Battle for Bersia MP version: just run the newer installer - it
builds from your Heroes_original.exe backup.

KeatonKuF's Community Patch fixes are already built in. If you installed
the Community Patch before, remove its dinput8.dll (and Patches.ini) from
the Heroes folder so the fixes are not applied twice. Its loose files
(fonts, voice files) can stay.

Note: if you use Steam's "Verify integrity of game files", Steam puts the
original Heroes.exe back - just run the installer again.


Installing the Crusaders co-op missions (optional)
--------------------------------------------------
These missions are rebuilt from YOUR OWN copy of The Crusaders - no
Crusaders files are included with this mod.

1. Install Kingdom Under Fire: The Crusaders from Steam (English voices).
2. Copy BattleForBersiaMP_Crusaders_Installer.bat into your KUF Heroes folder (next to
   Heroes.exe).
3. Double-click it. It finds The Crusaders through Steam (or asks you for
   the folder), checks every file it needs, and then installs about 400
   files into Heroes' Data folder. No Heroes file is overwritten.
4. The Gerald, Lucretia, Regnier and Kendal missions now appear in the
   Co-op mission list, with the Crusaders music.

Every player in a Crusaders co-op game needs to run the installer.
If it says files are missing or a different version, nothing is changed.

To remove them again, open a command prompt in the Heroes folder and run:
    BattleForBersiaMP_Crusaders_Installer.bat uninstall


Playing
-------
Title screen > Multiplayer:
- Create an army (or use the game's Custom armies), pick a hero, set it up
  in the Barracks.
- Host a game (Public or Friends only) or join one from the game list.
- In the lobby: choose team, colour, army; chat; the host picks the game
  type, map and settings and starts when everyone is ready.

Game types: Versus (with damage scale 100/75/50/25/1%), Hero Battle,
Invasion (Ruin, Ice Storm, Plains, plus Randomiser, Insanity, Random
Insanity, Roguelike and Roguelike Hero) and Co-op.

Map editor: open Bersia_Map_Forge.html in your browser (or use the
MAP EDITOR tab in the launcher). Custom maps show up in the lobby and are
sent to players who don't have them.


Reporting problems
------------------
Please send:
- KUF_netlog.txt from the Heroes folder (written every session - send it
  before starting the game again).
- After a crash: the newest heroes_..._crash_....mdmp file from the Heroes
  folder.
- A short note: game type, map or mission, how many players, what happened.
If the game freezes without crashing: Task Manager > right-click Heroes >
"Create memory dump file" while it is frozen, and send that file too.


Uninstalling
------------
Run "BattleForBersiaMP_v1.5_Install.bat uninstall" from a command prompt in the
Heroes folder (restores Heroes.exe and Interface_Misc.txt), or use
Steam > Verify integrity of game files.
Remove the Crusaders missions with the uninstall command above.


Credits
-------
Battle for Bersia MP: DJ Sonny B
Community Patch fixes: KeatonKuF
Testing: the Battle for Bersia MP community
Kingdom Under Fire: Heroes and The Crusaders (c) Blueside / Phantagram.
This is a fan-made, non-commercial mod.
```
