# Plan: 6 multiplayer army slots instead of 3 (Test 23, USER 2026-11-19)

## Why it can't just be a bigger number
- MP armies live in the game's own profile army sets: 3 sets of 0x3C bytes at profile+0x1DC (profile = call 0x4372D0),
  each with a troop list at +0x30, hero char +0x20, etc.; the next profile field follows at +0x290/+0x294.
- Game functions take the set index 0..2: 0x71CFD0 (hero of set), 0x71B050, 0x71D010; current set = [0xAE1D7C] (mod).
- mp_save writes the 3 sets to MultiplayerArmies.sav ("KMP1" + per set: troop count, troops via trooptab, 0x20 name, +0x20 w,
  +0x24/+0x28/+0x2c); mp_load reads it through the game's profile loader 0x71A540 with a memory stream (fs_read).

## Design: two pages of 3
- Page A = armies 1-3, page B = armies 4-6; the profile always holds one page. [0xAE1D7C] stays 0..2 (all existing code
  unchanged); new byte = current page. Shown number = page*3 + slot + 1 ("Army n of 6": t_army1..6).
- File: new magic "KMP2" + page A blob + page B blob (each = today's KMP1 body with a length); a KMP1 file loads as page A,
  page B empty. mp_save writes the loaded page into its blob and keeps the other blob as is.
- Left/Right (as_left/as_right, as_mod div 3 -> div 6): when the new army is on the other page, save the current page,
  load the other page (mp_load of that blob), then set the slot. Instant (small file).
- Side portraits (dh_set: neighbours slot+-1): neighbours can be on the unloaded page -> keep a 6-entry hero-char cache
  (filled from both blobs at load/save) for the portraits.
- Create / Delete / Barracks / Battle Order / Hero Battle / co-op paths all work on the loaded page via [0xAE1D7C]: unchanged.
- Network: only the chosen army's troop records are sent at load (net.c armies exchange): unaffected.
- Check: Custom-mode save (cs_fname path) shares mp_save/mp_load -> keep it single-page (KMP1) or give it the same paging.
