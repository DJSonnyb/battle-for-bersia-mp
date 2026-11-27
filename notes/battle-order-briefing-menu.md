# Battle Order via the campaign briefing troop menu (USER idea 2026-11-20) - investigation

## Findings
- The campaign briefing troop list is menu 10 MENU_BATTLE, class K2MenuBattle (ctor 0x707860, vtable 0x9A0658,
  size 0x3D0, build 0x707FF0, fill 0x709070, choice list 0x709970, start check 0x7084F0).
- It is a plain form (layout in interface_Worldmap_Live.txt). The 3D map table is a separate object
  (KufBriefingView 0x82D930, made by the briefing state 0x69D790) -> the menu does not need the map at all.
- The army screen runs in the custom set-up state (0x3E-0x40), menu manager mode 7, which loads
  interface_Misc.txt AND interface_Worldmap_Live.txt -> form 10 is available there (Barracks/form 14 already opens
  the same way). So an overlay on the army screen (USER suggestion) is the natural fit.
- Created by the generic manager switch at 0x6F00D3-0x6F0145: K2MenuBattle(mgr+8, mgr, roster, slots, arg):
  - roster = [0xA75198+0x59C] -> 0x6AFD50 (campaign world-map troop list, vector of troop pointers, count 0x6B1D30);
    the menu reads roster troop +0x44 type, +0x4C role (3 = normal troop), +0x5C enabled.
  - slots = 0x426310 -> 0x43B870 -> 0x6B6C70 (briefing script's BRIEFING_TROOP vector, 8-byte records:
    +0 battle troop id, +2 WORLDMAPID preset (-1 free; >=0 = fixed, @(Briefingfix)), TYPE, SUPPORTTROOP flag).
  - the ctor clears +2 on free slots when 0x6A4080 (0xA75198 -> 0x5ABDB0) is true.
- Choice rule (0x709970): a slot only takes a troop whose support class (0x5839F0) EQUALS the slot's flag ->
  fixed ground/support slots. Versus rule is hero + 4 with max 1 support -> needs a small patch ("any" slot,
  then the existing max-1-support check on confirm).

## Plan if built
1. Battle Order button -> open menu 10 on top of the army screen, with our own slot list (hero preset + 4 slots)
   and a roster built from the current army (adapter records if the army troop layout differs from +0x44/+0x4C/+0x5C).
2. Patch 0x709970 for "any" slots; keep max 1 support.
3. Confirm (start check 0x7084F0 / result) -> reorder the army set (hero, chosen 4 in slot order, rest), mp_save,
   close the overlay; replace the "Start the mission?" text with a battle-order wording.
4. Old swap editor stays as fallback until the new one is proven.
Menu-only: no battle logic, no desync risk.
