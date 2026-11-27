# Co-op levels - Test 23 (USER 2026-11-19)

- Players' hero = enemy average level in the mission + 2. Player 2 matches the host.
  - Ellen 2: enemies 3 -> both heroes 5 (stage file has Ellen at level 1 = "campaign level", never filled in network games).
- Crusaders missions (Gerald, Lucretia, Regnier, Kendal): player levels stay as they are (troops avg + 2, hero avg + 4).
- Two-player co-op, ALL missions (originals and Crusaders): every enemy troop is raised to at least the players' hero level + 2.
  - Ellen 2: enemies at least 7. Gerald was "a bit strong" - this raise covers it.
  - The existing +15% HP/damage for a second player stays.
- Enemy levels in the original campaign stages are often level 1 in the file (set at run time, e.g. 3 in Ellen 2):
  find where the game sets them, so the average and the raise use the real levels.

# Next build after Test 23 (USER 2026-11-19)
- Spell hit sync (fxh_hit, host-authoritative): limit to co-op only (ls_coop). Versus / Invasion have no officers and hero spells
  (Curatio, Meteor...) never desynced there - leave them exactly as in Test 22.
- Release notes: drop the "Versus or Invasion with spells" required test.
