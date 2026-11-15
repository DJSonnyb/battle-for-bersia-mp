```
KINGDOM UNDER FIRE: HEROES - BATTLE FOR BERSIA MP v1.4
=================================================

Online multiplayer for Kingdom Under Fire: Heroes (Steam PC).

WORK IN PROGRESS: this mod is still being developed and tested. Expect the
occasional bug, desync or crash. Please report them (see "Reporting problems").

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
2. Copy BattleForBersiaMP_v1.4_Install.bat into that folder (next to Heroes.exe)
   and double-click it. Close the game first.
   It checks that your Heroes.exe is the Steam version, keeps it as
   Heroes_original.exe and turns it into Battle for Bersia MP v1.4.
   It also installs the multiplayer menus (Data\Interface\Interface_Misc.txt);
   your old copy of that file is kept as Interface_Misc_original.txt.
3. Start the game from Steam as usual. The title screen shows
   "Battle for Bersia MP v1.4" when the mod is active.

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
2. Copy BattleForBersiaMP_Crusaders_Install.bat into your KUF Heroes folder (next to
   Heroes.exe).
3. Double-click it. It finds The Crusaders through Steam (or asks you for
   the folder), checks every file it needs, and then installs about 400
   files into Heroes' Data folder. No Heroes file is overwritten.
4. The Gerald, Lucretia, Regnier and Kendal missions now appear in the
   Co-op mission list, with the Crusaders music.

Every player in a Crusaders co-op game needs to run the installer.
If it says files are missing or a different version, nothing is changed.

To remove them again, open a command prompt in the Heroes folder and run:
    BattleForBersiaMP_Crusaders_Install.bat uninstall


Playing
-------
Title screen > Multiplayer:
- Create an army (or use the game's Custom armies), pick a hero, set it up
  in the Barracks.
- Host a game (Public or Friends only) or join one from the game list.
- In the lobby: choose team, colour, army; chat; the host picks the game
  type, map and settings and starts when everyone is ready.

Game types: Versus (with damage scale 100/75/50/25/1%), Hero Battle,
Invasion (Ruin, Ice Storm, Plains) and Co-op.


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
Run "BattleForBersiaMP_v1.4_Install.bat uninstall" from a command prompt in the
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
