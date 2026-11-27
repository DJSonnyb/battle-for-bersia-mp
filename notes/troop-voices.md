# Troop voice lines (USER 2026-11-21)
- Reporter banks: Data\SysVoice\<lang>\W_<R>_Report.xwb (xWMA 44.1 kHz mono; HIR 219 waves), cue lists S_<R>_<TROOP>.xsb
  (XACT3 SDBK v43). Reporters: HIR, ECL (human), HEX, VEL (dark). Commands: W_HU_Cmd / W_DU_Cmd (shared per side).
- Cues per troop: attacked, critical, defeated, melee, victory (+ _c second take), special1-3; air/support use attack/move/retreat.
- USER: HK_attacked = "Knights taking casualties". HE = Spearmen. HF = Heavy Infantry (the dwarves borrow it today). HS = Sappers, HT = Catapults, HM = Mortars. Also confirmed: HA Archers, HB Ballista, HC Cavalry, HP Paladin, BA Battaloon, BW Bomber Wing, ST Storm Rider, SC Scout.
- Extractor: scratchpad xsb.py + xwb -> RIFF XWMA (fmt 0x161 + dpds + data) -> ffmpeg.
- Dwarf recording list (USER 2026-11-21, Iddy to record): select x2, move x2, attack x2, stop, retreat, formation open/normal/closed
  (cues frm_max / frm_norm / frm_min), attacked x2, critical x2, melee x2, defeated x2, victory x2 = 21 lines.
  Spell lines stay the shared human ones (S_HIR_COMMON use_* / warning_*).
