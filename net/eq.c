/* ---- Multiplayer cosmetic equipment (included from net.c) ----
   The campaign item shop lives on the world map object [0xA62338], which does not exist outside the campaign.
   eq_init builds a minimal stand-in ("fake world") with only what the barracks item screens use:
     +0x78  -> building whose +0x24 is the item shop (0x63 per-item-type lists, ctor 0x4B6FD0)
     +0x1F8 -> WeaponMeshList (variant -> mesh)      +0x1FC -> WeaponNames
     +0x200 -> item inventory (ctor 0x4B8130)
   The world getters 0x5A2CB0/CE0/D10/D40/D60 are hooked to use it when the real world pointer is null.
   eq_fill replaces the shop's random stock for one item type with one free, stat-less level-1 item per look. */

#include "jobs.inc"
typedef void *(*fn_new)(u32);
#define G_NEW ((fn_new)0x8D5E88)
typedef void (*fn_del)(void*, u32);
#define G_DEL ((fn_del)0x8D5EB8)
typedef char *(*fn_root)(void);
#define G_ROOT ((fn_root)0x6A82D0)
typedef char *(TC *fn_lang)(void*);
#define G_LANG ((fn_lang)0x5ABBC0)
typedef void *(TC *fn_load)(void*, const char*);
#define MESH_LOAD ((fn_load)0x4ACB20)
#define NAME_LOAD ((fn_load)0x4ACDF0)
typedef void *(TC *fn_ctor)(void*);
#define INV_CTOR  ((fn_ctor)0x4B8130)
#define SHOP_CTOR ((fn_ctor)0x4B6FD0)
#define ITEM_CTOR ((fn_ctor)0x4B95F0)
typedef void (TC *fn_name)(void*, void*, void*);
#define ITEM_NAME ((fn_name)0x4B79E0)
typedef int (TC *fn_vsize)(void*);
#define V_SIZE ((fn_vsize)0x4BA0F0)
typedef void **(TC *fn_vat)(void*, int);
#define V_AT ((fn_vat)0x4B9670)
typedef void (TC *fn_vclr)(void*);
#define V_CLEAR ((fn_vclr)0x4B9DC0)
typedef void (TC *fn_vpush)(void*, void**);
#define V_PUSH ((fn_vpush)0x4BA0D0)

#define GDATA ((char*)0xA75198)

u32 eq_world[0x240/4];      /* fake world (0x22C used) */
static u32 eq_bld[0x40/4];
void *eq_fw;                /* = eq_world once built, read by the asm getter hooks */
static char eq_path[0x200];

void *eq_init(void)
{
    if (eq_fw) return eq_fw;
    char *root = G_ROOT();
    char *lang = G_LANG(GDATA);
    if (!root) root = "";
    if (!lang) lang = "ENG";
    void *mesh = G_NEW(8);  if (!mesh) return 0;
    void *names = G_NEW(8); if (!names) return 0;
    void *inv = G_NEW(0xC); if (!inv) return 0;
    void *shop = G_NEW(0x7C0); if (!shop) return 0;
    memset(mesh, 0, 8); memset(names, 0, 8);
    SPRINTF(eq_path, "%s%s", root, "data\\units\\WeaponMeshList.txt");
    MESH_LOAD(mesh, eq_path);
    SPRINTF(eq_path, "%sdata\\text\\%s\\WeaponNames_%s.txt", root, lang, lang);
    NAME_LOAD(names, eq_path);
    INV_CTOR(inv);
    SHOP_CTOR(shop);
    memset(eq_world, 0, sizeof eq_world);
    memset(eq_bld, 0, sizeof eq_bld);
    eq_bld[0x24/4] = (u32)shop;
    eq_world[0x78/4] = (u32)eq_bld;
    eq_world[0x1F8/4] = (u32)mesh;
    eq_world[0x1FC/4] = (u32)names;
    eq_world[0x200/4] = (u32)inv;
    eq_fw = eq_world;
    return eq_fw;
}

static void *eq_new_item(void *shop, int type, int variant, int mesh, char *info)
{
    char *it = G_NEW(0xC0);
    if (!it) return 0;
    memset(it, 0, 0xC0);
    ITEM_CTOR(it);                      /* serial, no stat modifiers, level 1 */
    *(i32*)(it + 0x04) = type;
    *(short*)(it + 0x08) = 1;           /* level 1: every unit can wear it */
    *(short*)(it + 0x0A) = 0;
    *(short*)(it + 0x0C) = (short)variant;
    *(short*)(it + 0x0E) = (short)mesh; /* the look */
    *(i32*)(it + 0x10) = 0;             /* free */
    *(i32*)(it + 0x3C) = 1;             /* in shop */
    ITEM_NAME(shop, it, info);
    return it;
}

/* shop: the barracks' item shop; type: item type of the selected equipment slot */
/* v61c: special looks = armour grade 2 (Master's) on one job each: soldiers AND troop leader become another character
   (CharInfo id). Soldier stats stay the job's; only the leader's HP follows the character. Missions use Elf, Dwarf and male
   Dark Elf troops; Ogre and Lich only ever lead (no soldier precedent). */
struct eq_sp { int job, ch; const char *nm; };
static const struct eq_sp eq_spec[] = {
    { 5, 45, "Elf Warrior Armor" },        /* Knight */
    { 4, 53, "Dwarf Armor" },              /* H.Infantry  (1030.stg: Dwarf-led Dwarf troop) */
    { 27, 26, "Ogre Armor" },              /* Orc Heavy Infantry */
    { 21, 49, "Dark Elf Warrior Armor" },  /* Dark Knight (4000/4020.stg: male Dark Elf soldiers) */
    { 20, 54, "Lich Armor" },              /* Dark Fighter */
};
static const struct eq_sp *spec_for(int job)
{
    for (unsigned k = 0; k < sizeof eq_spec / sizeof eq_spec[0]; k++) if (eq_spec[k].job == job) return &eq_spec[k];
    return 0;
}
static int spec_char(int c)
{
    for (unsigned k = 0; k < sizeof eq_spec / sizeof eq_spec[0]; k++) if (eq_spec[k].ch == c) return 1;
    return 0;
}
/* look tiers each job uses in the campaign stage files (bit t = tier t), from all Mission\\*.stg placements */
static const unsigned short eq_tiers[64] = { 0x27, 0x3F7, 0x8D8F, 0x403, 0x135, 0x1D2B, 0x5, 0x1C1F, 0xF, 0x1, 0x1, 0x0, 0x1, 0x1, 0x1, 0x0, 0x1, 0x0, 0x5B, 0x9, 0x6B, 0xFF, 0x2B, 0x1FF, 0x81, 0x105, 0xA7, 0x1B5, 0x11, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
void eq_fill(void *shop, int type, char *troop)
{
    if (!shop || type < 0 || type >= 0x63) return;
    void *tblo = *(void**)(GDATA + 0x5B4);
    if (!tblo || !*(void**)tblo) return;
    char *info = (char*)*(void**)tblo + type * 0x12C;
    void *list = (char*)shop + type * 0x14;
    int i, n = V_SIZE(list);
    for (i = 0; i < n; i++) { void *p = *V_AT(list, i); if (p) G_DEL(p, 0xC0); }
    V_CLEAR(list);
    int nv = *(short*)(info + 0x2A);
    u32 *ml = eq_fw ? (u32*)eq_world[0x1F8/4] : 0;
    int npairs = 0; i32 *pairs = 0;
    if (ml && (u32)type < ml[0] && ml[1]) {
        npairs = ((i32*)ml[1])[type * 2];
        pairs = (i32*)((i32*)ml[1])[type * 2 + 1];
    }
    int made = 0;
    if (pairs && npairs > 0) {
        int lastmesh = -100000;
        for (i = 0; i < npairs && i < 64; i++) {
            int v = i ? pairs[(i - 1) * 2] : 0;      /* first variant that maps to this pair */
            int mesh = pairs[i * 2 + 1];
            if (nv > 0 && v >= nv) break;            /* the campaign never rolls past nv */
            if (mesh == lastmesh) continue;
            lastmesh = mesh;
            void *it = eq_new_item(shop, type, v, mesh, info);
            if (it) { V_PUSH(list, &it); made++; }
        }
    }
    if (!made) {
        /* v59c: armour has one look entry; the campaign varies it by grade (+0x0A: 0 Improved, 1 Enhanced, 2 Master's - the
           names in ItemTypeInfo). Body mesh override via +0x0E did nothing (v59b). Offer each grade. */
        unsigned mask = 1;
        int job = troop ? *(i32*)(troop + 0x48) : -1;
        if ((type >= 15 && type <= 19) || (type >= 32 && type <= 36)) {   /* troop armour: one item per look tier the job has */
            mask = (job >= 0 && job < 64 && eq_tiers[job]) ? eq_tiers[job] : 1;
            if (job == 6) mask |= 2;   /* Paladin look 1 (winged helmets) works but no mission uses it */
            mask &= 7;   /* v59i: grades above 2 hang the item list (Infantry, scrolling) - only grades 0-2 are safe */
            /* v93d: the special looks (Dwarf/Elf/Ogre/Dark Elf/Lich armour) are gone - those are real jobs 56-60 now */
        }
        for (i = 0; i < 16; i++) {
            if (!(mask & (1u << i))) continue;
            void *it = eq_new_item(shop, type, 0, -1, info);
            if (!it) continue;
            *(short*)((char*)it + 0x0A) = (short)i;
            ITEM_NAME(shop, it, info);
            {   /* v61: name the special looks so the list says what they do (name string at +0x40) */
                const char *nm = 0;
                int armour = (type >= 15 && type <= 19) || (type >= 32 && type <= 36);
                if (armour && job == 6 && i == 1) nm = "Winged Paladin Armor";
                else if (armour && job == 6 && i == 2) nm = "Gold Paladin Armor";
                if (nm) SPRINTF((char *)it + 0x40, "%s", nm);
            }
            V_PUSH(list, &it);
        }
    }
}

/* ---- Encablossa army (experimental): the Encablossa creature (CharInfo 39) as hero + Encablossa troops ---- */
#ifdef KUF_ENCA
static int job_base_e(int j) { return (j >= JOB0 && j < JOB0 + NJOB) ? njobs[j - JOB0].base : j; }
#ifdef KUF_ENCA
/* USER 2026-11-24 (E build: Encablossa flyers spawn too low and clip through the ground): a new troop's flying height comes from
   0x583D80(job) - 2000 for job 39 (Flyer), 0 for any job it does not know, so the E build's own Flyer job 64 flew at ground
   level (its model reaches ~490 below its centre). asm fh_job hands it the base job. */
KTEXT int fly_job(int j) { return job_base_e(j); }
#endif
#endif
#ifdef KUF_ENCA
void rn_variant(void);
#endif
static void enca_set_type(char *troop, int t)
{
    char *g = ((char *(TC*)(void*))0x4A2940)((void*)0xA75198);
    *(i32*)(troop + 0x48) = t;
    char *ji = ((char *(TC*)(void*,int))0x7A8470)(g, t);
    if (ji) *(i32*)(troop + 0x44) = *(i32*)(ji + 4);
    *(i32*)(troop + 0x3c) = ((int (TC*)(void*,int))0x4A28C0)(g, t);
    *(i32*)(troop + 0x40) = ((int (TC*)(void*,int))0x4A28E0)(g, t);
#ifdef KUF_ENCA
    /* v1.2E: soldier / leader CharInfo of the mission's own Encablossa placements (5060, Ruin, Ice Storm): job 38/40 -> 39
       C_ENCABLOSA_MONSTER, 55 -> 63 C_ENCABLOSA_INFEST, 39 Flyer -> 55 C_ENCABLOSA_FLYING_MONSTER. The E build's own jobs 61-64
       carry their soldier char in njobs (jobsold). The v58 guess (55/61/62) gave the flyer / column / big models. */
    {
        int b = job_base_e(t), ch = b == 55 ? 63 : b == 39 ? 55 : 39;
        *(i32*)(troop + 0x28) = ch; *(i32*)(troop + 0x2c) = ch; *(i32*)(troop + 0x34) = ch;
    }
#else
    *(i32*)(troop + 0x28) = 55; *(i32*)(troop + 0x2c) = 55;     /* Encablossa leader */
    /* v58: +0x34 = soldier CharInfo id (1 Infantry, 0x12 DE Fighter, 0x19 Orc Infantry from 0x6B2D10) - was human Infantry for every Enca troop.
       Guess from CharInfo order: 55/61/62 Encablossa variants, 63 Enca Infest; jobs 38 Encablossa, 39 Flyer, 40 Spitter, 55 Infested */
    *(i32*)(troop + 0x34) = t == 39 ? 61 : t == 40 ? 62 : t == 55 ? 63 : 55;
#endif
    *(i32*)(troop + 0x54) = ((int (TC*)(void*,void*))0x6AA240)((void*)0xA75198, troop);
    ((void (TC*)(void*,void*))0x6AA2B0)((void*)0xA75198, troop);
}
#ifdef KUF_ENCA
unsigned char enca_on = 1;   /* asm enca_pic: E build only */
#else
unsigned char enca_on = 0;
#endif
#ifdef KUF_ENCA
/* E build (USER: Encablossa troops are blank in the barracks): the barracks draw each soldier from UnitUVID[unit type][look] ->
   a UnitUVInfo picture (rectangle) + Data\\interface\\Body|Portrait\\<type name>_<look>_*.tga (0x81C680); only the Encablossa
   monster (39) look 1 has them (row 107). Every empty look of the Encablossa types uses row 107, and asm enca_pic its files. */
KTEXT __attribute__((noinline)) static void enca_uv(void)
{
    char *tab = *(char **)(0xA75198 + 0x5d4);
    if (!tab || !*(char **)tab) return;
    char *rows = *(char **)tab; int n = *(i32 *)(tab + 4);
    /* USER 2026-11-25 ("barracks preview still empty"): the barracks / troop-info formation draws each soldier's Body picture with
       the body rectangle of its UnitUVInfo row - row 107 (the Ancient Heart) has none (0 x 0), so nothing was drawn. A new row is
       added after the last one: row 107's portrait rectangle + a body rectangle for our 128 x 256 Body pictures (figure inside
       120 x 156 at the top, centred, feet near the bottom). The Ancient Heart (39 look 1) keeps row 107. */
    int er = 107;
    {
        char *ui = *(char **)(0xA75198 + 0x5d8);           /* UnitUVInfo {rows (0x24 each), count} */
        static int made __attribute__((section(".data")));
        if (ui && *(char **)ui && *(i32 *)(ui + 4) > 107) {
            int c = *(i32 *)(ui + 4);
            if (made) er = c - 1;
            else {
                char *o = *(char **)ui, *m = ((char *(*)(unsigned))0x8D664F)((unsigned)(c + 1) * 0x24);
                if (m) {
                    for (int i = 0; i < c * 0x24; i++) m[i] = o[i];
                    i32 *r = (i32 *)(m + c * 0x24), *a = (i32 *)(o + 107 * 0x24);
                    r[0] = c; r[1] = 100; r[2] = 84; r[3] = 50; r[4] = 42;   /* USER 2026-11-26: the heroes' portrait rectangle (lobby icon overflowed with row 107's 100 x 136) */
                    r[5] = 64; r[6] = 80; r[7] = 31; r[8] = 73;   /* USER 2026-11-26: half size (enca_pics.py body 60 x 78) */
                    *(char **)ui = m; *(i32 *)(ui + 4) = c + 1;  /* the old block is left alone */
                    made = 1; er = c;
                }
            }
        }
    }
    static const unsigned char et[5] = { 39, 55, 61, 62, 63 };
    for (int i = 0; i < 5; i++) if (et[i] < n) for (int v = 0; v < 17; v++) {
        i32 *u = (i32 *)(rows + et[i] * 0x48 + 4 + v * 4);
        if (*u < 0 || (*u == 107 && !(et[i] == 39 && v == 1))) *u = er;
    }
    if (34 < n) { i32 *u = (i32 *)(rows + 34 * 0x48 + 4); if (u[1] < 0) u[1] = u[0]; }   /* E.Regnier = Regnier look 1 */
}
#endif
void enca_build(int set)
{
#ifdef KUF_ENCA
    enca_uv();
#endif
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof || set < 0 || set > 2) return;
    char *sp = prof + 0x1dc + set * 0x3c;
    typedef void (TC *fn_add)(void*, void*, int, int, const char*, int, int);
    fn_add add = (fn_add)0x71BFE0;
#ifdef KUF_ENCA
    const int EH = 34;   /* USER 2026-11-23: the Encablossa hero is E.Regnier - Regnier (hero 34), darkened (rn_dark), blue fire */
#else
    const int EH = 39;
#endif
    *(short*)(sp + 0x20) = (short)EH;
#ifndef KUF_START_EXP
#define KUF_START_EXP 1000
#endif
    *(i32*)(sp + 0x28) = KUF_START_EXP; *(i32*)(sp + 0x2c) = KUF_START_EXP;   /* new army EXP (USER screenshot build: KUF_START_EXP) */
    add(prof, sp, EH, 0, EH == 34 ? "E.Regnier" : "Encablossa", 0, 0);
    {   /* v58: the hero record is the hero's own Leader troop - created as plain Infantry; make it an Encablossa troop */
        void *vec = sp + 0x30;
        int n = ((int (TC*)(void*))0x6B1D30)(vec);
#ifdef KUF_ENCA
        if (n >= 1) { char *t = *((char **(TC*)(void*,int))0x554050)(vec, n - 1); if (t) { enca_set_type(t, 61); *(i32*)(t + 0x28) = EH; *(i32*)(t + 0x2c) = EH; *(i32*)(t + 0x30) = 1; rn_variant(); { const char *e = "E.Regnier"; int q = 0; for (; e[q]; q++) t[q] = e[q]; t[q] = 0; } } }
#else
        if (n >= 1) { char *t = *((char **(TC*)(void*,int))0x554050)(vec, n - 1); if (t) { enca_set_type(t, 38); *(i32*)(t + 0x28) = 39; *(i32*)(t + 0x2c) = 39; } }
#endif
    }
#ifdef KUF_ENCA
    static const int types[8] = { 61, 61, 62, 63, 61, 62, 63, 64 };   /* v1.2E: brood x3, spitter x2, infested x2, flyer */
#else
    static const int types[8] = { 38, 38, 40, 39, 40, 39, 55, 38 };
#endif
    char nm[24];
    for (int i = 0; i < 8; i++) {
        int lvl = ((int (*)(int))0x437F90)(6) + 4;
        SPRINTF(nm, "Enca %d", i + 1);
        add(prof, sp, 0x38, lvl, nm, 0, i + 1);
        void *vec = sp + 0x30;
        int n = ((int (TC*)(void*))0x6B1D30)(vec);
        if (n < 1) continue;
        char *t = *((char **(TC*)(void*,int))0x554050)(vec, n - 1);
        if (t) enca_set_type(t, types[i]);
    }
    *(short*)(sp + 0x20) = 39;
#ifdef KUF_ENCA
    {   /* USER 2026-11-25 ("the encablossa army always starts with 0 exp"): the game's own new-army code (0x71A7C0, end) copies the
           army EXP into the hero's troop (+0x4c == 0) as a float at +0x50 - that is what the barracks shows and spends */
        void *vec = sp + 0x30; int n = ((int (TC*)(void*))0x6B1D30)(vec);
        for (int i = 0; i < n; i++) { char *t = *((char **(TC*)(void*,int))0x554050)(vec, i); if (t && !*(i32 *)(t + 0x4c)) *(float *)(t + 0x50) = (float)*(i32 *)(sp + 0x28); }
    }
#endif
}

/* ---- v54 co-op: upgrade basic jobs for missions after a campaign's first (user rule: Infantry->Knight, Archer->Longbow, Sapper->Pyrotech) ---- */
static int up_job(int t)
{
    switch (t) {
    case 2:  return 5;    /* Infantry -> Knight */
    case 0:  return 1;    /* Archer -> Longbow */
    case 10: return 11;   /* Sappers -> Pyrotech */
    case 7:  return 8;    /* Cavalry -> Heavy Cavalry */
    case 20: return 21;   /* Dark Fighter -> Dark Knight */
    case 23: return 27;   /* Orc Infantry -> Orc Heavy Infantry */
    case 24: return 25;   /* Orc Riders -> Orc Heavy Rider */
    default: return t;
    }
}
static void set_job(char *troop, int t)
{
    char *g = ((char *(TC*)(void*))0x4A2940)((void*)0xA75198);
    *(i32*)(troop + 0x48) = t;
    char *ji = ((char *(TC*)(void*,int))0x7A8470)(g, t);
    if (ji) *(i32*)(troop + 0x44) = *(i32*)(ji + 4);
    *(i32*)(troop + 0x3c) = ((int (TC*)(void*,int))0x4A28C0)(g, t);
    *(i32*)(troop + 0x40) = ((int (TC*)(void*,int))0x4A28E0)(g, t);
    *(i32*)(troop + 0x54) = ((int (TC*)(void*,void*))0x6AA240)((void*)0xA75198, troop);
    ((void (TC*)(void*,void*))0x6AA2B0)((void*)0xA75198, troop);
}
void coop_upgrade(int set)
{
    if (set >= 0) return;   /* TEST16 (USER 2026-11-14): upgrades now follow the troop's level (up_lv in coop_level / the stage) */
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof || set < 0 || set > 2) return;
    void *vec = prof + 0x1dc + set * 0x3c + 0x30;
    int n = ((int (TC*)(void*))0x6B1D30)(vec);
    for (int i = 1; i < n; i++) {                 /* record 0 is the hero */
        char *t = *((char **(TC*)(void*,int))0x554050)(vec, i);
        if (!t) continue;
        int j = *(i32*)(t + 0x48), u = up_job(j);
        if (u != j) set_job(t, u);
    }
}

/* ---- v54 co-op army layout: mission scripts call player troops by position (stage placements +0x5a) ----
   Ellen 2 #1 archers, Ellen 8 #3 sappers, Leinhart 1 #1 DE archers #2 orc axemen, Leinhart 6 #4 orc sappers,
   Cirith 1 #1 knights #2 archers #3 orc axemen. Troops 1..8 (record 0 = hero). Job ids = TroopInfo order. */
static const unsigned char lay_human[8] = { 0, 2, 10, 3, 7, 2, 0, 3 };     /* Archer Infantry Sappers Spearmen Cavalry Infantry Archer Spearmen */
static const unsigned char lay_orc[8]   = { 23, 26, 24, 28, 23, 26, 24, 28 }; /* Orc Inf, Axemen, Riders, Sapper */
static const unsigned char lay_dark[8]  = { 18, 26, 20, 28, 22, 20, 18, 23 }; /* D.Archers, Orc Axemen, D.Fighter, Orc Sapper, D.Cavalry ... */
static const unsigned char lay_cirith[8]= { 21, 18, 26, 28, 20, 18, 26, 22 }; /* D.Knight, D.Archers, Orc Axemen, Orc Sapper ... */
static int job_base(int j);
static int job_leader(int t) { t = job_base(t); return (t >= 23 && t <= 30) ? 0x39 : (t >= 18 && t <= 22) ? 0x3A : 0x38; }
void coop_army(int set, int hero)
{
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof || set < 0 || set > 2) return;
    const unsigned char *lay = hero == 26 ? lay_orc : hero == 38 ? lay_cirith : (hero == 44 || hero == 35 || hero == 43 || hero == 34) ? lay_dark : lay_human;   /* v79: + Lucretia 43, Regnier 34 */
    void *vec = prof + 0x1dc + set * 0x3c + 0x30;
    int n = ((int (TC*)(void*))0x6B1D30)(vec);
    for (int i = 1; i < n && i <= 8; i++) {
        char *t = *((char **(TC*)(void*,int))0x554050)(vec, i);
        if (!t) continue;
        int j = lay[i - 1], ld = job_leader(j);
        *(i32*)(t + 0x28) = ld; *(i32*)(t + 0x2c) = ld;
        set_job(t, j);
    }
}

/* ---- v55 user rule: Scout takes 1 skill slot (was 3), so leader troops can add Holy etc. Skill table: [G+0x5ac] -> {begin,end},
   records 0x168 bytes, +0 id (5 = Scouting), +0x164 u16 slots (getter 0x6b6d20), +0x166 u16 max level (0x707450). Matches its own description text. ---- */
void look_sync(void);
void jobs_inject(void);
void skill_fix(void)
{
    look_sync();
    jobs_inject();
    char *o = *(char **)(0xA75198 + 0x5ac);
    if (!o) return;
    char *b = *(char **)o, *e = *(char **)(o + 4);
    if (!b || e <= b) return;
    int n = (int)((e - b) / 0x168);
    for (int i = 0; i < n && i < 64; i++) {
        char *r = b + i * 0x168;
        if (*(i32*)r == 5 && *(short*)(r + 0x164) > 1) *(short*)(r + 0x164) = 1;
    }
    if (n > 5 && *(short*)(b + 5 * 0x168 + 0x164) > 1) *(short*)(b + 5 * 0x168 + 0x164) = 1;   /* getter 0x6b6d20 indexes by id */
}

/* v56: replaces 0x726210 (troop used-slot total). Troop skill entries +0x70+i*6: id, level, cached slot cost. Scout cached as 3 -> 1. */
int troop_slots(char *t)
{
    int n = *(int *)(t + 0x508), sum = 0;
    for (int i = 0; i < n && i < 8; i++) {
        short *e = (short *)(t + 0x70 + i * 6);
        if (e[0] == 5 && e[2] > 1) e[2] = 1;
        if (e[2] == -1) continue;
        sum += e[2];
    }
    return sum;
}

/* v57: Learn list for human Leaders (troop+0x34 == 1, hero unit +0x4c == 0): the game's list plus Holy (skill 11). List = {begin,end} of int ids. */
/* Leader char (troop+0x28) -> human army per MP builder 0x71A7C0 switch: Gerald 32, Ellen 33, Rupert 36, Kendal 37, Walter 68 (Leinhart 44 = dark elf army) */
int human_leader(int c) { return c == 32 || c == 33 || c == 36 || c == 37 || c == 68; }
/* v100q (USER: Leinhart, Morene, Regnier, Cirith, Urukubarr, Lucretia have a free slot but nothing to learn): magic per hero =
   the magic the game's own stages give that hero (8 Fire, 9 Lightning, 10 Ice, 12 Earth, 13 Curse). */
static const signed char *spell_set(int c)
{
    switch (c) {
    case 26: return (const signed char *)"\x08";              /* Urukubarr: Fire */
    case 34: return (const signed char *)"\x08";              /* Regnier: Fire */
    case 35: return (const signed char *)"\x08\x0a\x0d";      /* Morene: Fire, Ice, Curse */
    case 38: return (const signed char *)"\x08\x09\x0c";      /* Cirith: Fire, Lightning, Earth */
    case 43: return (const signed char *)"\x08\x09\x0a";      /* Lucretia: Fire, Lightning, Ice */
    case 44: return (const signed char *)"\x09";              /* Leinhart: Lightning */
    }
    return 0;
}
/* TEST42 (USER: "as far as the d.elf goes give them the ability to learn taming"): troops led by the Dark Elf Leader (char 58,
   LeaderGeneration row 'Dark Elf Leader': Melee, Ranged, Riding, Frontal, magic - no Taming) may also learn Taming (7). */
int leader_learns(int c) { return human_leader(c) || spell_set(c) != 0; }
static int ll_arr[32];
static int *ll_vec[2];
void *learn_list_c(char *troop, int **list)
{
    if (!troop) return list;
    int c = *(int *)(troop + 0x28);
    const signed char *add;
    if (*(int *)(troop + 0x4c) != 0) {
        int j = *(int *)(troop + 0x48);
        if (c == 58) add = (const signed char *)"\x07";                               /* TEST42: Dark Elf troops + Taming */
#ifdef KUF_ENCA
        /* E build (USER 2026-11-25: Encablossa troops could only learn Melee): Brood 61 and Infested 63 + Frontal (Infested) and
           magic; Spitter 62 Ranged + magic; Flyer 64 Riding (like the other flyers' extra skill). Magic = Fire, Lightning, Ice,
           Earth, Curse (no Holy). */
        else if (j == 61) add = (const signed char *)"\x08\x09\x0a\x0c\x0d";
        else if (j == 62) add = (const signed char *)"\x01\x08\x09\x0a\x0c\x0d";
        else if (j == 63) add = (const signed char *)"\x02\x08\x09\x0a\x0c\x0d";
        else if (j == 64) add = (const signed char *)"\x03";
#endif
        else return list;
    }
    else { if (!leader_learns(c)) return list; add = human_leader(c) ? (const signed char *)"\x0b" : spell_set(c); }
    int n = 0;
    /* list may be NULL: the game has no learn list for some Leader chars (Gerald's army) */
    if (list) for (int *p = list[0]; p && p < list[1] && n < 24; p++) ll_arr[n++] = *p;
    for (; add && *add && n < 31; add++) {
        int has = 0;
        for (int i = 0; i < n; i++) if (ll_arr[i] == *add) has = 1;
        if (!has) ll_arr[n++] = *add;
    }
    ll_vec[0] = ll_arr; ll_vec[1] = ll_arr + n;
    return ll_vec;
}

/* ---- v59d: armour look tier. Stage placements give each troop a look tier (placement +0x196 -> troop +0x38): Walter 3's
   Patriarch Paladins use 2, Walter's own use 0; Knights up to 3, orc troops up to 5. Cosmetic armour items carry the
   tier as their grade (+0x0A: 0 Improved, 1 Enhanced, 2 Master's); copy it to the troop that wears one. ---- */
static int is_armour(int t)
{
    return (t >= 15 && t <= 19) || (t >= 32 && t <= 36);   /* troop armour only; leader/hero armour left alone */
}
/* v1.5: soldiers of jobs 56-60 made before their own equipment rows still hold the base job's items (Knight sword on Elf
   Warriors, orc axe on Ogres...). Swap any item the job's row does not allow for the row's default, cosmetic like the rest
   (level 1, no bonuses). Item layout as the reset at 0x4B9F10. Item k of the troop = page k (0x6B6E10: slot0, slot1, armour min). */
KTEXT __attribute__((noinline)) static int njeq_ok(const signed char *e, int k, int ty)
{
    if (k == 0) return ty == e[0];
    if (k == 1) return ty == e[1] || (ty >= 0 && (ty == e[2] || ty == e[3]));
    return e[4] < 0 ? ty < 0 : ty >= e[4] && ty <= e[5];
}
KTEXT __attribute__((noinline)) static void njeq_set(char *it, int ty)
{
    *(i32*)(it + 0x04) = ty;
    *(short*)(it + 0x08) = 1; *(short*)(it + 0x0A) = 0; *(short*)(it + 0x0C) = 0; *(short*)(it + 0x0E) = -1;
    *(i32*)(it + 0x10) = 0;
    *(i32*)(it + 0x14) = -1; *(i32*)(it + 0x18) = -1; *(i32*)(it + 0x1C) = -1; *(i32*)(it + 0x24) = -1;
    *(i32*)(it + 0x20) = 0; *(i32*)(it + 0x28) = 0; *(i32*)(it + 0x2C) = -1; *(i32*)(it + 0x34) = -1;
    *(i32*)(it + 0x30) = 0; *(i32*)(it + 0x38) = 0; *(i32*)(it + 0x3C) = 1;
    it[0x40] = 0;
    if (ty < 0 || !eq_init()) return;
    u32 *ml = (u32*)eq_world[0x1F8/4];
    if (ml && (u32)ty < ml[0] && ml[1] && ((i32*)ml[1])[ty * 2] > 0)
        *(short*)(it + 0x0E) = (short)((i32*)((i32*)ml[1])[ty * 2 + 1])[1];   /* first look of the type */
    void *tblo = *(void**)(GDATA + 0x5B4);
    if (tblo && *(void**)tblo && eq_bld[0x24/4]) ITEM_NAME((void*)eq_bld[0x24/4], it, (char*)*(void**)tblo + ty * 0x12C);
}
KTEXT __attribute__((noinline)) static void njeq_fix(char *t)
{
    int job = *(i32*)(t + 0x48);
    if (job < JOB0 || job >= JOB0 + NJOB || njob_eq[job - JOB0][0] == -2) return;
    const signed char *e = njob_eq[job - JOB0];
    for (int k = 0; k < 3; k++) {
        char *it = t + 0x88 + k * 0xc0;
        int ty = *(i32*)(it + 4);
        if (!njeq_ok(e, k, ty)) njeq_set(it, k == 0 ? e[0] : k == 1 ? e[1] : e[4]);
    }
}
void look_sync(void)
{
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof) return;
    for (int s = 0; s < 3; s++) {
        void *vec = prof + 0x1dc + s * 0x3c + 0x30;
        int n = ((int (TC*)(void*))0x6B1D30)(vec);
        for (int i = 0; i < n && i < 16; i++) {
            char **pp = ((char **(TC*)(void*,int))0x554050)(vec, i);
            char *t = pp ? *pp : 0;
            if (!t) continue;
            njeq_fix(t);
            for (int k = 0; k < 6; k++) {
                char *it = t + (k < 3 ? 0x88 + k * 0xc0 : 0x2c8 + (k - 3) * 0xc0);
                int ty = *(i32*)(it + 4);
                short g = *(short*)(it + 0x0A);
                if (is_armour(ty) && g >= 0 && g <= 2) {
                    int job = *(i32*)(t + 0x48);
                    const struct eq_sp *sp = 0;   /* v93d: special looks removed (USER: real jobs now, no duplicate troops) */
                    if (sp && g == 2) {
                        /* v65: Ogre = CharInfo 26, whose variation 0 is Urukubarr himself; 1-3 are the coloured ogre
                           officers (campaign 2000/2030/3010.stg). Soldiers get 1-3 by troop slot, the leader the next colour. */
                        int ogre = sp->ch == 26, sv = ogre ? 1 + i % 3 : 0;
                        *(i32*)(t + 0x38) = sv; *(i32*)(t + 0x34) = sp->ch;
                        if (*(i32*)(t + 0x4c)) { *(i32*)(t + 0x28) = sp->ch; *(i32*)(t + 0x2c) = sp->ch; *(i32*)(t + 0x30) = ogre ? 1 + (i + 1) % 3 : 0; }
                    } else {
                        /* v93d: a grade the job has no look for (old special look on Knight etc.) -> look 0 */
                        unsigned ok = (job >= 0 && job < 64 ? eq_tiers[job] : 1) | (job == 6 ? 2 : 0) | 1;
                        *(i32*)(t + 0x38) = (ok >> g) & 1 ? g : 0;
                        /* not a special look: undo one (soldier char back to the job's own, table 0xA4A120; normal leader).
                           Jobs 56+ have these characters as their own soldiers - leave them. */
                        if (spec_char(*(i32*)(t + 0x34)) && job >= 0 && job < 36) *(i32*)(t + 0x34) = *(i32*)(0xA4A120 + job * 12);
                        if (*(i32*)(t + 0x4c) && spec_char(*(i32*)(t + 0x28)) && job >= 0 && job < 36) { int l = job_leader(job); *(i32*)(t + 0x28) = l; *(i32*)(t + 0x2c) = l; *(i32*)(t + 0x30) = 0; }
                    }
                }
            }
        }
    }
}

/* v62: Hero Battle heroes at level 50 (user): the hero record (set's first troop) gets Melee 50. Level = sum of skill levels
   (0x6AA240), HP etc. refreshed by 0x6AA2B0. Skill entries +0x70+i*6 {id, level, slots}, count +0x508; Melee = id 0. */
void hb_level(int set)
{
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof || set < 0 || set > 2) return;
    void *vec = prof + 0x1dc + set * 0x3c + 0x30;
    if (((int (TC*)(void*))0x6B1D30)(vec) < 1) return;
    char **pp = ((char **(TC*)(void*,int))0x554050)(vec, 0);
    char *t = pp ? *pp : 0;
    if (!t) return;
    /* v69e: level = sum of skill levels (Keaton: 51 because the hero also has Scout 1) - Melee fills the rest up to 50 */
    int n = *(int *)(t + 0x508), done = 0, other = 0;
    for (int i = 0; i < n && i < 8; i++) { short *e = (short *)(t + 0x70 + i * 6); if (e[0] != 0) other += e[1]; }
    int mel = 50 - other; if (mel < 1) mel = 1;
    for (int i = 0; i < n && i < 8; i++) {
        short *e = (short *)(t + 0x70 + i * 6);
        if (e[0] == 0) { e[1] = (short)mel; done = 1; }
    }
    if (!done) ((void (TC*)(void*, int, int, int))0x725EB0)(t, 0, mel, 1);
    *(i32*)(t + 0x54) = ((int (TC*)(void*,void*))0x6AA240)((void*)0xA75198, t);
    ((void (TC*)(void*,void*))0x6AA2B0)((void*)0xA75198, t);
}

/* v83: co-op army level per mission (coop_lvtab from make_kuf COOP_CURVE). Troop level = sum of its skill levels (0x6AA240,
   capped 99), so scale every skill by target/sum (each >= 1, <= the skill's max level, SkillInfo +0x166), then recompute
   level (+0x54) and stats (0x6AA2B0) as hb_level does. Hero gets +2. The army is the temporary co-op army (mp_save/restore). */
static int skill_max(int id)
{
    char *o = *(char **)(0xA75198 + 0x5ac);
    (void)id; (void)o;
    return 99;                                     /* v91 USER: every skill can reach 99 in Heroes */
}
static void scale_troop(char *t, int target)
{
    int n = *(int *)(t + 0x508), sum = 0, cnt = 0;
    if (n > 8) n = 8;
    for (int i = 0; i < n; i++) { short *e = (short *)(t + 0x70 + i * 6); if (e[0] >= 0 && e[1] > 0) { sum += e[1]; cnt++; } }
    if (cnt == 0 || sum <= 0) return;
    if (target < cnt) target = cnt;
    int tot = 0;
    for (int i = 0; i < n; i++) {
        short *e = (short *)(t + 0x70 + i * 6);
        if (e[0] < 0 || e[1] <= 0) continue;
        int v = e[1] * target / sum, mx = skill_max(e[0]);
        if (v < 1) v = 1; if (v > mx) v = mx;
        e[1] = (short)v; tot += v;
    }
    for (int pass = 0; pass < 200 && tot != target; pass++) {      /* fix rounding: spread the difference one level at a time */
        int moved = 0;
        for (int i = 0; i < n && tot != target; i++) {
            short *e = (short *)(t + 0x70 + i * 6);
            if (e[0] < 0 || e[1] <= 0) continue;
            if (tot < target && e[1] < skill_max(e[0])) { e[1]++; tot++; moved = 1; }
            else if (tot > target && e[1] > 1) { e[1]--; tot--; moved = 1; }
        }
        if (!moved) break;
    }
    *(i32*)(t + 0x54) = ((int (TC*)(void*,void*))0x6AA240)((void*)0xA75198, t);
    ((void (TC*)(void*,void*))0x6AA2B0)((void*)0xA75198, t);
}
int coop_tgt_lv(int idx);
static int up_lv(int j, int lv);
static int coop_looks(u32 map, u32 cnt);
void coop_level(int set, int idx)
{
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof || set < 0 || set > 2 || idx < 0 || idx >= COOP_N) return;
    int lv = coop_tgt_lv(idx);     /* TEST16: Crusaders missions - from the mission's enemy levels */
    void *vec = prof + 0x1dc + set * 0x3c + 0x30;
    int n = ((int (TC*)(void*))0x6B1D30)(vec);
    for (int i = 0; i < n && i <= 8; i++) {
        char **pp = ((char **(TC*)(void*,int))0x554050)(vec, i);
        char *t = pp ? *pp : 0;
        if (t && i > 0) { int j = *(i32 *)(t + 0x48), u = up_lv(j, lv); if (u != j) set_job(t, u); }   /* TEST16 */
        if (t) scale_troop(t, i == 0 ? lv + 2 : lv);
    }
}

/* ---- v93: custom jobs 56-60 (Dwarf, Elf Warrior, Dark Elf Guard, Lich, Ogre) - definitions in jobs.inc (scripts/jobs_gen.py).
   The job tables are SOX files with one row per job id. sox_fopen (hooked at the two fopen calls of the SOX openers 0x7AE071 /
   0x7AE143) reads TroopInfo, JobInfo, AbilityByJob, WorldMap_TroopInfo and the TroopInfo/WorldMap_TroopInfo language files,
   appends rows 56.. (copies of each new job's base job with its own stats, tree place, requirements, name, description and
   icon token) into %TEMP%\kufmp_<name> and opens that instead - whatever SOX files are installed (community patch) are kept.
   Exe tables with fixed 56 entries are moved or mapped in kuf.asm: soldier table 0xA4A120 -> jobsold, voice table 0xA4F790 ->
   voicetab, race 0x583500 / troop type 0x583F60 / icon UVID 0x6E19F0 map a new id to its base, tree 0x6E17AF adds the new ids.
   Icons: blitted into Font2.dds when it is read (tex_fix, 0x87D6FA), UV ids 600.. (uv_setup) and inline tokens (sfont_reg). */
typedef void *(*fn_fopen)(const char *, const char *);
#define C_FOPEN ((fn_fopen)0x920B8E)
typedef u32 (*fn_fread)(void *, u32, u32, void *);
#define C_FREAD ((fn_fread)0x91E4DB)
typedef int (*fn_fclose)(void *);
#define C_FCLOSE ((fn_fclose)0x91D8BA)
typedef void *(__attribute__((stdcall)) *fn_gmh2)(const char *);
typedef void *(__attribute__((stdcall)) *fn_gpa2)(void *, const char *);
typedef u32 (__attribute__((stdcall)) *fn_gtp)(u32, char *);
typedef int (__attribute__((stdcall)) *fn_wf)(void *, const void *, u32, u32 *, void *);
typedef void *(__attribute__((stdcall)) *fn_cf)(const char *, u32, u32, void *, u32, u32, void *);
typedef int (__attribute__((stdcall)) *fn_ch)(void *);
int juv[NJOB] = JUV_INIT;   /* icon UV id per new job (base icon until placed) */
static unsigned char jplaced[NJOB];
static int jobs_rows;                                   /* set when TroopInfo got the new rows */
static int lc(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }
static int ieq(const char *a, const char *b) { while (*a && lc(*a) == lc(*b)) { a++; b++; } return !*a && !*b; }
static const char *base_name(const char *p) { const char *b = p; for (; *p; p++) if (*p == '\\' || *p == '/') b = p + 1; return b; }
static int sox_kind(const char *path)
{
    const char *b = base_name(path);
    if (lc(b[0]) == 'l' && lc(b[1]) == 'i' && lc(b[2]) == 'v' && lc(b[3]) == 'e' && b[4] == '_') b += 5;
    if (ieq(b, "troopinfo.sox")) return 'T';
    if (ieq(b, "jobinfo.sox")) return 'J';
    if (ieq(b, "abilitybyjob.sox")) return 'A';
    if (ieq(b, "worldmap_troopinfo.sox")) return 'W';
    if (ieq(b, "abilityinfo.sox")) return 'I';                 /* v1.3 Fire at Will (abil_fopen) */
    int n = strlen_(b);
    if (n == 17 && lc(b[0]) == 't' && b[9] == '_') { char t[16]; for (int i = 0; i < 10; i++) t[i] = b[i]; t[10] = 0; if (ieq(t, "troopinfo_")) return 'L'; }
    if (n == 19 && b[11] == '_') { char t[16]; for (int i = 0; i < 12; i++) t[i] = b[i]; t[12] = 0; if (ieq(t, "abilityinfo_")) return 'N'; }
    if (n == 26 && b[18] == '_') { char t[24]; for (int i = 0; i < 19; i++) t[i] = b[i]; t[19] = 0; if (ieq(t, "worldmap_troopinfo_")) return 'M'; }
    return 0;
}
static int sox_str(const unsigned char *p) { short n = *(const short *)p; return 2 + (n > 0 ? n : 0); }
static int row_len(int k, const unsigned char *p)
{
    switch (k) {
    case 'T': return 148;
    case 'A': return 24;
    case 'W': return 28;
    case 'J': { int l = 68; l += sox_str(p + l); l += sox_str(p + l); return l; }
    case 'L': return 4 + sox_str(p + 4);
    case 'M': { int l = 4; l += sox_str(p + l); l += sox_str(p + l); return l; }
    }
    return 0;
}
static unsigned char *put_str(unsigned char *o, const char *s) { int n = strlen_(s); *(short *)o = (short)n; for (int i = 0; i < n; i++) o[2 + i] = s[i]; return o + 2 + n; }
static unsigned char *sox_in, *sox_out;
/* write buf to %TEMP%\kufmp_<file name> and fopen that (0 on failure) */
KTEXT static void *temp_open(const char *path, const char *mode, const void *buf, u32 len)
{
    void *k32 = (*(fn_gmh2 *)0x988170)("kernel32.dll");
    if (!k32) return 0;
    fn_gtp gtp = (fn_gtp)(*(fn_gpa2 *)0x98829C)(k32, "GetTempPathA");
    fn_wf wf = (fn_wf)(*(fn_gpa2 *)0x98829C)(k32, "WriteFile");
    if (!gtp || !wf) return 0;
    char tp[300]; u32 tl = gtp(200, tp);
    if (!tl || tl > 200) return 0;
    SPRINTF(tp + tl, "kufmp_%s", base_name(path));
    void *h = (*(fn_cf *)0x988134)(tp, 0x40000000, 0, 0, 2, 0x80, 0);
    if (h == (void *)-1) return 0;
    u32 w = 0; int ok = wf(h, buf, len, &w, 0) && w == len;
    (*(fn_ch *)0x988144)(h);
    return ok ? C_FOPEN(tp, mode) : 0;
}
/* v1.3 Fire at Will: AbilityInfo row 10 (Pour Oil - no handler, never offered) -> immediate troop ability, 0 SP, Ranged 1,
   tag @(FireAtWill) (our icon, sfont_reg), illustration of Direct Arrow; language row 10 -> name / description. */
static const i32 afw_row[15] = { 10, 1, 0, 1, -1, -1, -1, 0, 0, 0, 0, 1, 1, -1, -1 };
static void *abil_fopen(const char *path, const char *mode, int k)
{
    void *f = C_FOPEN(path, mode);
    if (!f) return f;
    if (!sox_in) { sox_in = G_NEW(0x20000); sox_out = G_NEW(0x24000); }
    if (!sox_in || !sox_out) return f;
    u32 n = 0, r;
    while (n < 0x20000 && (r = C_FREAD(sox_in + n, 1, 0x20000 - n, f)) > 0) n += r;
    C_FCLOSE(f);
    f = C_FOPEN(path, mode);
    i32 rows = n >= 8 ? *(i32 *)(sox_in + 4) : 0;
    if (n < 8 || n >= 0x20000 || rows <= 10 || rows > 200) return f;
    const unsigned char *p = sox_in + 8, *r10 = 0, *r11 = 0;
    for (int i = 0; i < rows; i++) {
        if (i == 10) r10 = p;
        int l = k == 'I' ? 60 : 4; l += sox_str(p + l); l += sox_str(p + l);
        if (p + l > sox_in + n) return f;
        p += l;
        if (i == 10) r11 = p;
    }
    unsigned char *o = sox_out;
    for (const unsigned char *q = sox_in; q < r10; q++) *o++ = *q;
    if (k == 'I') {
        for (int i = 0; i < 15; i++) ((i32 *)o)[i] = afw_row[i];
        o = put_str(o + 60, "@(FireAtWill)"); o = put_str(o, "IL_ABL_DIRECT_ARROW.TGA");
    } else {
        *(i32 *)o = 10;
        o = put_str(o + 4, "Fire at Will");
        o = put_str(o, "Toggle. While on, this troop shoots any enemy that comes within range whenever it has no orders, and never moves to chase. Use it again to switch it off.");
    }
    for (const unsigned char *q = r11; q < sox_in + n; q++) *o++ = *q;
    void *g = temp_open(path, mode, sox_out, (u32)(o - sox_out));
    if (!g) return f;
    C_FCLOSE(f);
    return g;
}
KTEXT void *sox_fopen(const char *path, const char *mode)
{
    int k = path ? sox_kind(path) : 0;
    if (!k) return C_FOPEN(path, mode);
    if (k == 'I' || k == 'N') return abil_fopen(path, mode, k);
    void *f = C_FOPEN(path, mode);
    if (!f) return f;
    if (!sox_in) { sox_in = G_NEW(0x20000); sox_out = G_NEW(0x24000); }
    if (!sox_in || !sox_out) return f;
    u32 n = 0, r;
    while (n < 0x20000 && (r = C_FREAD(sox_in + n, 1, 0x20000 - n, f)) > 0) n += r;
    C_FCLOSE(f);
    f = C_FOPEN(path, mode);                            /* fallback: the original file, unchanged */
    if (n < 8 || n >= 0x20000 || *(i32 *)(sox_in + 4) != JOB0) return f;
    /* rows */
    const unsigned char *rows[JOB0]; const unsigned char *p = sox_in + 8;
    for (int i = 0; i < JOB0; i++) { rows[i] = p; int l = row_len(k, p); if (l <= 0 || p + l > sox_in + n) return f; p += l; }
    unsigned char *o = sox_out;
    for (const unsigned char *q = sox_in; q < p; q++) *o++ = *q;
    *(i32 *)(sox_out + 4) = JOB0 + NJOB;
    for (int j = 0; j < NJOB; j++) {
        const struct njob *J = &njobs[j]; const unsigned char *b = rows[J->base]; int bl = row_len(k, b);
        if (k == 'T' || k == 'A' || k == 'W') {
            for (int i = 0; i < bl; i++) o[i] = b[i];
            *(i32 *)o = J->id;
            if (k == 'T') for (int i = 0; i < 16 && njob_ti[j][i][0] >= 0; i++) ((i32 *)o)[njob_ti[j][i][0]] = njob_ti[j][i][1];
            /* v1.5 (USER): the soldiers' equipment lists = their character's own items (Elf sword/ring/bow/staff etc.) */
            if (k == 'W' && njob_eq[j][0] != -2) for (int i = 0; i < 6; i++) ((i32 *)o)[1 + i] = njob_eq[j][i];
            o += bl;
        } else if (k == 'J') {
            i32 *v = (i32 *)o; for (int i = 0; i < 17; i++) v[i] = ((const i32 *)b)[i];
            v[0] = J->id; v[1] = J->col; v[2] = J->row; for (int i = 3; i < 11; i++) v[i] = -1;
            for (int i = 0; i < 6; i++) v[11 + i] = J->req[i];
            o += 68; char tk[40]; SPRINTF(tk, "@%s", J->tok); o = put_str(o, tk);
            const unsigned char *ill = b + 68 + sox_str(b + 68); int il = sox_str(ill);
            for (int i = 0; i < il; i++) o[i] = ill[i];
            o += il;
        } else if (k == 'L') { *(i32 *)o = J->id; o = put_str(o + 4, J->name); }
        else if (k == 'M') { *(i32 *)o = J->id; o = put_str(o + 4, J->name); o = put_str(o, J->desc); }
    }
    if (k == 'A') {   /* v1.3 Fire at Will for Archer, Longbow, Dark Elf Archer (E: Enca Spitter) */
        for (unsigned char *rw = sox_out + 8; rw + 24 <= o; rw += 24) {
            i32 *v = (i32 *)rw;
            if (!(v[0] == 0 || v[0] == 1 || v[0] == 13 || v[0] == 18 || v[0] == 62)) continue;   /* v1.4: + Mortar 13 (USER) */
            for (int q = 1; q < 6; q++) { if (v[q] == 10) break; if (v[q] < 0) { v[q] = 10; break; } }
        }
    }
    for (const unsigned char *q = p; q < sox_in + n; q++) *o++ = *q;   /* "THEND" trailer */
    void *g = temp_open(path, mode, sox_out, (u32)(o - sox_out));
    if (!g) return f;
    C_FCLOSE(f);
    if (k == 'T') jobs_rows = 1;
    return g;
}

/* Font2.dds is read whole (0x88FB50) before the texture is made: write each icon's DXT5 blocks (11x11 blocks = 44x44 px) into
   its spot if every alpha there is 0 (spots chosen free in the shipped and community Font2). */
static int blk_clear(const unsigned char *a)
{
    if (a[0] > a[1]) return a[0] == 0;                  /* 8-value mode: all values between a0 and a1 */
    if (a[0] || a[1]) return 0;
    u32 lo = a[2] | (a[3] << 8) | (a[4] << 16), hi = a[5] | (a[6] << 8) | (a[7] << 16);
    for (int i = 0; i < 8; i++) { if (((lo >> (3 * i)) & 7) == 7 || ((hi >> (3 * i)) & 7) == 7) return 0; }   /* code 7 = 255 */
    return 1;
}
/* v1.2E packed block -> DXT5: alpha block a0 = 255, a1 = 0 (8-value mode), code 0 = opaque / 1 = transparent per texel.
   grey: colour endpoints to dimmed grey (the 'not usable' ability icon) */
#define AFW_UX 216
#define AFW_UY 176
#define AFW_GX 632
#define AFW_GY 364
static int afw_placed;
static unsigned short grey565(unsigned short c)
{
    int r = (c >> 11) * 255 / 31, g = ((c >> 5) & 63) * 255 / 63, b = (c & 31) * 255 / 31, l = (r * 30 + g * 59 + b * 11) / 100 * 3 / 5;
    return (unsigned short)(((l * 31 / 255) << 11) | ((l * 63 / 255) << 5) | (l * 31 / 255));
}
static void pk_put(unsigned char *d, const unsigned char *s2, int grey)
{
    u32 m = s2[0] | (s2[1] << 8), lo = 0, hi = 0;
    for (int t = 0; t < 8; t++) if (!(m >> t & 1)) lo |= 1u << (3 * t);
    for (int t = 0; t < 8; t++) if (!(m >> (t + 8) & 1)) hi |= 1u << (3 * t);
    d[0] = 255; d[1] = 0;
    d[2] = (unsigned char)lo; d[3] = (unsigned char)(lo >> 8); d[4] = (unsigned char)(lo >> 16);
    d[5] = (unsigned char)hi; d[6] = (unsigned char)(hi >> 8); d[7] = (unsigned char)(hi >> 16);
    for (int i = 0; i < 8; i++) d[8 + i] = s2[2 + i];
    if (grey) {   /* DXT5 colour blocks always use 4-colour interpolation, so the endpoint order does not matter */
        unsigned short c0 = grey565(d[8] | (d[9] << 8)), c1 = grey565(d[10] | (d[11] << 8));
        d[8] = (unsigned char)c0; d[9] = (unsigned char)(c0 >> 8); d[10] = (unsigned char)c1; d[11] = (unsigned char)(c1 >> 8);
    }
}
/* TEST25 (USER idea): six new team colours (game colours 6-11: black, white, turquoise, orange, pink, grey). The game asks for
   "<group>_<colour+1>" team textures (0x6BAE90); for 7-12 the loader (hook 0x87D646, tc_name) reads "<group>_1" instead, under
   the new name, on the whole-file path, and tex_fix recolours its DXT1 colour endpoints (luminance -> new colour). The game's
   per-group colour table only has 6 colours (0x6AB800 fill / 0x6AB8F0 lookup) -> colours 6+ go into tc_tab. */
#include "colicons.inc"
static int tc_pending = -1; static char tc_src[64] __attribute__((section(".data")));
static int ci_placed;
static const unsigned char tc_rgb[6][3] = { {0,0,0}, {255,255,255}, {38,196,186}, {244,132,26}, {242,112,182}, {128,128,128} };
KTEXT __attribute__((noinline)) static int lcase(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }
#ifdef KUF_ENCA
KTEXT __attribute__((noinline)) static int pfx_i(const char *s, const char *p) { while (*p) { if (lcase(*s) != *p) return 0; s++; p++; } return 1; }
#endif
KTEXT __attribute__((noinline)) int tc_name(char *dds)
{
    tc_pending = -1;
    if (!dds) return 0;
    int n = 0; while (dds[n] && n < 250) n++;
    if (n < 10 || lcase(dds[n - 4]) != '.' || lcase(dds[n - 3]) != 'd' || lcase(dds[n - 2]) != 'd' || lcase(dds[n - 1]) != 's') return 0;
#ifdef KUF_ENCA
    {   /* E.Regnier (look 1 of Regnier): C_DP_RNE_* -> C_DP_RN_*, UV_DU_HERO_REGNIER_1_* -> _0_; darkened (tc_tint 100) */
        char *bn = (char *)base_name(dds); int hit = 0;
        if (pfx_i(bn, "c_dp_rne_")) { for (char *p = bn + 7; (p[0] = p[1]) != 0; p++) continue; hit = 1; }
        else if (pfx_i(bn, "uv_du_hero_regnier_1_")) { bn[19] = '0'; hit = 1; }
        if (hit) { int k = 0; while (bn[k] && k < 63) { tc_src[k] = bn[k]; k++; } tc_src[k] = 0; tc_pending = 100; return 1; }
    }
#endif
#if 0   /* USER 2026-11-23: normal Regnier must keep his look - E.Regnier is darkened per unit by a light instead (rn_e_light) */
    {   /* E build (USER): E.Regnier, the Encablossa hero - Regnier's own textures (C_DP_RN*, E_DP_RN*; not the team-colour ones)
           are read on the whole-file path and darkened in tex_fix (tc_tint mode 100) */
        const char *bn = base_name(dds); int rn = 0;
        if ((lcase(bn[0]) == 'c' || lcase(bn[0]) == 'e') && bn[1] == '_' && lcase(bn[2]) == 'd' && lcase(bn[3]) == 'p' && bn[4] == '_' && lcase(bn[5]) == 'r' && lcase(bn[6]) == 'n') {
            rn = 1;
            for (int i = 0; bn[i + 3]; i++) if (lcase(bn[i]) == 't' && lcase(bn[i + 1]) == 'e' && lcase(bn[i + 2]) == 'a' && lcase(bn[i + 3]) == 'm') rn = 0;
        }
        if (rn) { int k = 0; while (bn[k] && k < 63) { tc_src[k] = bn[k]; k++; } tc_src[k] = 0; tc_pending = 100; return 1; }
    }
#endif
    int e = n - 4, b = e;
    while (b > 0 && dds[b - 1] >= '0' && dds[b - 1] <= '9') b--;
    if (b == e || b < 6 || dds[b - 1] != '_' || e - b > 2) return 0;
    int num = 0; for (int i = b; i < e; i++) num = num * 10 + dds[i] - '0';
    /* TEST25 (USER colour check, Test 24: yellow came out red, red and brown right): the game's textures sit one file further than
       colour+1 suggests - colour c shows "<group>_<c+2>" (c0 blue _2, c1 yellow/ochre _3, c2 red _4, c3 green _5, c4 purple _6).
       So colour 5 asks for _7 (no such file: was the mesh's own red) and colours 6-11 for _8.._13 - all made here from _1. */
    if (num < 7 || num > 13) return 0;
    if (lcase(dds[b - 5]) != 't' || lcase(dds[b - 4]) != 'e' || lcase(dds[b - 3]) != 'a' || lcase(dds[b - 2]) != 'm') return 0;
    dds[b] = '1'; dds[b + 1] = '.'; dds[b + 2] = 'd'; dds[b + 3] = 'd'; dds[b + 4] = 's'; dds[b + 5] = 0;
    const char *bn = base_name(dds); int k = 0; while (bn[k] && k < 63) { tc_src[k] = bn[k]; k++; } tc_src[k] = 0;
    tc_pending = num - 2;
    return 1;
}
KTEXT __attribute__((noinline)) static int c565_l(unsigned short c) { int r = (c >> 11) * 255 / 31, g = ((c >> 5) & 63) * 255 / 63, b = (c & 31) * 255 / 31; return (r * 30 + g * 59 + b * 11) / 100; }
KTEXT __attribute__((noinline)) static unsigned short tc_col(int L, int c)
{
    int r, g, b;
    if (c == 5) { int f = 40 + L * 215 / 255; r = 150 * f / 255; g = 92 * f / 255; b = 48 * f / 255; }   /* TEST25: brown (colour 5 has no file) */
    else if (c == 6) r = g = b = 16 + L * 72 / 255;          /* black: very dark, shading kept */
    else if (c == 7) r = g = b = 150 + L * 105 / 255;        /* white */
    else if (c == 11) r = g = b = 58 + L * 135 / 255;        /* grey */
    else {
        const unsigned char *t = tc_rgb[c - 6]; int f = 40 + L * 215 / 255, h = L > 190 ? (L - 190) * 2 : 0;
        r = t[0] * f / 255; g = t[1] * f / 255; b = t[2] * f / 255;
        r += h * (255 - r) / 160; g += h * (255 - g) / 160; b += h * (255 - b) / 160;
    }
    if (r > 255) r = 255; if (g > 255) g = 255; if (b > 255) b = 255;
    return (unsigned short)(((r * 31 / 255) << 11) | ((g * 63 / 255) << 5) | (b * 31 / 255));
}
/* TEST31 (USER, Iddy: the new colours look plain, no detailing next to the original ones): the original colour files are the
   blue one (_1) with only the hue changed - trims, metal and other parts keep their colours, light and dark stay. Same here:
   each colour endpoint whose hue is the texture's own team hue (found per texture) gets the new hue (or no saturation for
   black / white / grey) with its saturation and brightness scaled; everything else is left as it is. */
KTEXT __attribute__((noinline)) static void c565_hsv(unsigned short c, float *h, float *s, float *v)
{
    float r = (float)(c >> 11) / 31.0f, g = (float)((c >> 5) & 63) / 63.0f, b = (float)(c & 31) / 31.0f;
    float mx = r > g ? (r > b ? r : b) : (g > b ? g : b), mn = r < g ? (r < b ? r : b) : (g < b ? g : b), d = mx - mn;
    *v = mx; *s = mx > 0.0f ? d / mx : 0.0f;
    if (d <= 0.0f) { *h = 0.0f; return; }
    float x = mx == r ? (g - b) / d : mx == g ? (b - r) / d + 2.0f : (r - g) / d + 4.0f;
    if (x < 0.0f) x += 6.0f;
    *h = x / 6.0f;
}
KTEXT __attribute__((noinline)) static unsigned short hsv_565(float h, float s, float v)
{
    if (s < 0.0f) s = 0.0f; if (s > 1.0f) s = 1.0f; if (v < 0.0f) v = 0.0f; if (v > 1.0f) v = 1.0f;
    h -= (float)(int)h; if (h < 0.0f) h += 1.0f;
    float x = h * 6.0f; int k = (int)x; float f = x - (float)k;
    float p = v * (1.0f - s), q = v * (1.0f - f * s), t = v * (1.0f - (1.0f - f) * s), r, g, b;
    switch (k % 6) { case 0: r = v; g = t; b = p; break; case 1: r = q; g = v; b = p; break; case 2: r = p; g = v; b = t; break;
                     case 3: r = p; g = q; b = v; break; case 4: r = t; g = p; b = v; break; default: r = v; g = p; b = q; }
    return (unsigned short)(((int)(r * 31.0f + 0.5f) << 11) | ((int)(g * 63.0f + 0.5f) << 5) | (int)(b * 31.0f + 0.5f));
}
/* per colour 5..11: hue (-1 = grey), saturation factor, brightness factor, brightness offset */
static const float tc_hsv[7][4] = {
    { 0.075f, 1.00f, 0.75f, 0.00f },   /* 5 brown */
    { -1.0f,  0.00f, 0.45f, 0.02f },   /* 6 black: dark, shading kept */
    { -1.0f,  0.00f, 0.85f, 0.32f },   /* 7 white: a touch below pure white so white trims still show */
    { 0.48f,  1.00f, 1.05f, 0.00f },   /* 8 turquoise */
    { 0.07f,  1.15f, 1.20f, 0.00f },   /* 9 orange */
    { 0.92f,  0.75f, 1.25f, 0.00f },   /* 10 pink */
    { -1.0f,  0.00f, 0.75f, 0.12f },   /* 11 grey */
};
KTEXT __attribute__((noinline)) static unsigned short tc_rec(unsigned short c, float th, int col)
{
    float h, s, v;
    c565_hsv(c, &h, &s, &v);
    float dh = h - th; if (dh > 0.5f) dh -= 1.0f; if (dh < -0.5f) dh += 1.0f;
    if (s < 0.12f || dh > 0.14f || dh < -0.14f) return c;   /* not the team colour: keep */
    const float *t = tc_hsv[col - 5];
    float lift = v < 0.25f ? t[3] * v * 4.0f : t[3];            /* near-black stays near-black (background, deep folds) */
    return t[0] < 0.0f ? hsv_565(0.0f, 0.0f, lift + v * t[2]) : hsv_565(t[0], s * t[1], v * t[2]);
}
/* E build: E.Regnier - grey skin, cloth and armour (gold -> dark steel); strong reds (horns, glow) stay */
KTEXT __attribute__((noinline)) static unsigned short rn_dark(unsigned short c)
{
    float h, s, v; c565_hsv(c, &h, &s, &v);
    if ((h < 0.045f || h > 0.94f) && s > 0.45f && v > 0.22f) return c;
    return hsv_565(0.0f, 0.0f, v * 0.62f + 0.02f);
}
KTEXT __attribute__((noinline)) static void tc_tint(unsigned char *buf, int size, int c)
{
    int bs, co;
    if ((c < 5 || c > 11) && c != 100) return;
    if (buf[84] == 'D' && buf[85] == 'X' && buf[86] == 'T' && buf[87] == '1') { bs = 8; co = 0; }
    else if (buf[84] == 'D' && buf[85] == 'X' && buf[86] == 'T' && (buf[87] == '3' || buf[87] == '5')) { bs = 16; co = 8; }
    else return;
    int nb = (size - 128) / bs, hist[36];
    for (int k = 0; k < 36; k++) hist[k] = 0;
    for (int i = 0; i < nb; i++) {                           /* the texture's team hue: most common blue-ish hue of saturated endpoints */
        unsigned char *d = buf + 128 + i * bs + co;
        for (int e = 0; e < 2; e++) {
            float h, sa, v; c565_hsv((unsigned short)(d[e * 2] | d[e * 2 + 1] << 8), &h, &sa, &v);
            if (sa > 0.3f && v > 0.2f) { int k = (int)(h * 36.0f); if (k > 35) k = 35; hist[k]++; }
        }
    }
    int best = 20; for (int k = 15; k < 26; k++) if (hist[k] > hist[best]) best = k;   /* the source (_1) is always the blue one: look only around blue, or brown rock wins on C_DU_DG */
    float th = ((float)best + 0.5f) / 36.0f;
    for (int i = 0; i < nb; i++) {
        unsigned char *d = buf + 128 + i * bs + co;
        unsigned short c0 = d[0] | d[1] << 8, c1 = d[2] | d[3] << 8, n0, n1;
        u32 ix = d[4] | d[5] << 8 | d[6] << 16 | (u32)d[7] << 24;
        if (c == 100) { n0 = rn_dark(c0); n1 = rn_dark(c1); } else { n0 = tc_rec(c0, th, c); n1 = tc_rec(c1, th, c); }
        if (n0 == c0 && n1 == c1) continue;                     /* nothing of the team colour in this block */
        if (bs == 16 || c0 > c1) {                               /* 4-colour block: keep c0 > c1 */
            if (n0 < n1) { unsigned short t = n0; n0 = n1; n1 = t; ix ^= 0x55555555u; }
            if (n0 == n1) { n1 = 0; ix = 0; if (!n0) n0 = 1; }   /* flat block: every texel = c0 (a -1 would borrow into another channel) */
        } else if (n0 > n1) {                                    /* 3-colour + transparent block: keep c0 <= c1 */
            unsigned short t = n0; n0 = n1; n1 = t;
            for (int k = 0; k < 16; k++) { u32 v = ix >> (2 * k) & 3; if (v < 2) ix ^= 1u << (2 * k); }
        }
        d[0] = (unsigned char)n0; d[1] = (unsigned char)(n0 >> 8); d[2] = (unsigned char)n1; d[3] = (unsigned char)(n1 >> 8);
        d[4] = (unsigned char)ix; d[5] = (unsigned char)(ix >> 8); d[6] = (unsigned char)(ix >> 16); d[7] = (unsigned char)(ix >> 24);
    }
}
static struct { short key, tex[7]; } tc_tab[64] __attribute__((section(".data"))); static int tc_n;   /* TEST28: game colours 6..12 */
KTEXT __attribute__((noinline)) void tc_fill(int key, int tex, int c)
{
    if (c < 6 || c > 12) return;
    int i = 0; while (i < tc_n && tc_tab[i].key != (short)key) i++;
    if (i == tc_n) { if (tc_n >= 64) return; tc_tab[i].key = (short)key; for (int k = 0; k < 7; k++) tc_tab[i].tex[k] = -1; tc_n++; }
    tc_tab[i].tex[c - 6] = (short)tex;
}
KTEXT __attribute__((noinline)) int tc_get(int key, int c)
{
    if (c < 6 || c > 12) return -1;
    for (int i = 0; i < tc_n; i++) if (tc_tab[i].key == (short)key) return tc_tab[i].tex[c - 6];
    return -1;
}
KTEXT void tex_fix(const char *name, unsigned char *buf, int size)
{
    if (tc_pending >= 0 && name && buf && size > 128 && ieq(base_name(name), tc_src)) tc_tint(buf, size, tc_pending);
    tc_pending = -1;
    if (!name || !buf || size < 128 || !ieq(base_name(name), "font2.dds")) return;
    if (buf[84] != 'D' || buf[85] != 'X' || buf[86] != 'T' || buf[87] != '5') return;
    int h = *(i32 *)(buf + 12), w = *(i32 *)(buf + 16), stride = (w / 4) * 16;
    if (w < 256 || h < 256 || 128 + stride * (h / 4) > size) return;
    for (int j = 0; j < NJOB; j++) {
        int bx = njobs[j].ux / 4, by = njobs[j].uy / 4, ok = 1;
        if (njobs[j].ux + 44 > w || njobs[j].uy + 44 > h) continue;
        for (int y = 0; y < 11 && ok; y++) for (int x = 0; x < 11 && ok; x++) ok = blk_clear(buf + 128 + (by + y) * stride + (bx + x) * 16);
        if (!ok) continue;
        for (int y = 0; y < 11; y++) for (int x = 0; x < 11; x++) {
            pk_put(buf + 128 + (by + y) * stride + (bx + x) * 16, njob_dxt[j] + (y * 11 + x) * 10, 0);
        }
        jplaced[j] = 1;
    }
    /* TEST25: lobby icons of the 6 new colours, 7x7 blocks each at CI_X0 + 32 * k, CI_Y0 */
    ci_placed = 0;
    for (int k = 0; k < NCI && CI_X0 + 32 * k + 28 <= w && CI_Y0 + 28 <= h; k++) {
        int bx = (CI_X0 + 32 * k) / 4, by = CI_Y0 / 4, ok = 1;
        for (int y = 0; y < 7 && ok; y++) for (int x = 0; x < 7 && ok; x++) ok = blk_clear(buf + 128 + (by + y) * stride + (bx + x) * 16);
        if (!ok) continue;
        for (int y = 0; y < 7; y++) for (int x = 0; x < 7; x++) { unsigned char *o = buf + 128 + (by + y) * stride + (bx + x) * 16; for (int q = 0; q < 16; q++) o[q] = ci_dxt[k][y * 7 + x][q]; }
        ci_placed |= 1 << k;
    }
    /* v1.3 Fire at Will ability icon (36x36) and its greyed 'not usable' copy */
    afw_placed = 0;
    for (int g = 0; g < 2; g++) {
        int bx = (g ? AFW_GX : AFW_UX) / 4, by = (g ? AFW_GY : AFW_UY) / 4, ok = 1;
        for (int y = 0; y < 9 && ok; y++) for (int x = 0; x < 9 && ok; x++) ok = blk_clear(buf + 128 + (by + y) * stride + (bx + x) * 16);
        if (!ok) break;
        for (int y = 0; y < 9; y++) for (int x = 0; x < 9; x++) pk_put(buf + 128 + (by + y) * stride + (bx + x) * 16, ABIL_DXT + (y * 9 + x) * 10, g);
        afw_placed = g + 1;
    }
}
/* UV ids 600.. for the job tree (table 0xA76190 of 0x1C-byte entries, count 0xA7618C; texture handle from HARPOON's entry 106) */
static void uv_setup(void)
{
    void ***tp = (void ***)0xA76190; int *cnt = (int *)0xA7618C;
    if (!*tp || *cnt <= 106 || !(*tp)[106]) return;
    int need = 600 + NJOB;
    if (*cnt < need) {
        void **nt = ((void **(*)(u32))0x91668F)(need * 4);
        if (!nt) return;
        for (int i = 0; i < need; i++) nt[i] = i < *cnt ? (*tp)[i] : 0;
        ((void (*)(void *))0x916674)(*tp);
        *tp = nt; *cnt = need;
    }
    i32 tex = *(i32 *)(*tp)[106];
    /* v1.3 Fire at Will: ability UV ids 10 (_UVID_ABILITY_POUR_OIL, a 0,0,0,0 dummy) and 46 (its NOT_ variant) -> our icons */
    for (int g = 0; g < afw_placed && (*tp)[8]; g++) {
        int id = g ? 46 : 10;
        i32 *e = (i32 *)(*tp)[id], *ref = (i32 *)(*tp)[8];
        if (!e) { e = G_NEW(0x1c); if (!e) break; (*tp)[id] = e; }
        int x1 = g ? AFW_GX : AFW_UX, y1 = g ? AFW_GY : AFW_UY;
        e[0] = ref[0]; e[1] = ref[1]; e[2] = ref[2];
        ((float *)e)[3] = (float)x1 / 1024.0f; ((float *)e)[4] = (float)(y1 + 36) / 512.0f;
        ((float *)e)[5] = (float)(x1 + 36) / 1024.0f; ((float *)e)[6] = (float)y1 / 512.0f;
    }
    for (int j = 0; j < NJOB; j++) {
        if (!jplaced[j]) continue;
        i32 *e = (i32 *)(*tp)[600 + j];
        if (!e) { e = G_NEW(0x1c); if (!e) return; (*tp)[600 + j] = e; e[0] = 0; }
        if (e[0] == tex && e[1] == 43) { juv[j] = 600 + j; continue; }
        int x1 = njobs[j].ux, y1 = njobs[j].uy;
        e[0] = tex; e[1] = 43; e[2] = 42;
        ((float *)e)[3] = (float)x1 / 1024.0f; ((float *)e)[4] = (float)(y1 + 41) / 512.0f;
        ((float *)e)[5] = (float)(x1 + 42) / 1024.0f; ((float *)e)[6] = (float)y1 / 512.0f;
        juv[j] = 600 + j;
    }
}
/* inline icon tokens "@(J_Dwarf)" ... used by the job header and the barracks list (after K2UVSpecialFontTable.txt loads) */
void sfont_reg(void *font)
{
    if (!font) return;
    for (int j = 0; j < NJOB; j++) {
        i32 r[4] = { njobs[j].ux, njobs[j].uy, njobs[j].ux + 42, njobs[j].uy + 41 };
        ((void (TC *)(void *, const char *, i32 *))0x74B0E0)(font, njobs[j].tok, r);
    }
    { i32 r[4] = { AFW_UX, AFW_UY, AFW_UX + 36, AFW_UY + 36 }; ((void (TC *)(void *, const char *, i32 *))0x74B0E0)(font, "(FireAtWill)", r); }
    for (int k = 0; k < NCI; k++) { i32 r[4] = { CI_X0 + 32 * k, CI_Y0, CI_X0 + 32 * k + 28, CI_Y0 + 28 }; ((void (TC *)(void *, const char *, i32 *))0x74B0E0)(font, ci_tok[k], r); }   /* TEST25 */
}
static int job_base(int j) { return (j >= JOB0 && j < JOB0 + NJOB) ? njobs[j - JOB0].base : j; }
static void jobs_troops(void);
void jobs_inject(void)
{
    jobs_troops();
    uv_setup();
#ifdef KUF_ENCA
    enca_uv();                                 /* E build: barracks pictures for Encablossa troops (also for armies made earlier) */
#endif
}

/* v88: a troop keeps its formation (troop +0x3c width, +0x40 depth; set from TroopInfo by 0x7A8490 only when the troop is
   created), so a job change to a custom job kept 5x5. Force the custom job's formation on every troop of the 3 army sets.
   v93: Dwarves made with v85-v92 (job 15, Dwarf soldiers) move to job 56; slot 15 is the cut Harpoon again. */
static void jobs_troops(void)
{
    if (!jobs_rows) return;
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof) return;
    for (int set = 0; set < 3; set++) {
        void *vec = prof + 0x1dc + set * 0x3c + 0x30;
        int n = ((int (TC*)(void*))0x6B1D30)(vec);
        for (int i = 1; i < n && i < 16; i++) {
            char **pp = ((char **(TC*)(void*,int))0x554050)(vec, i);
            char *t = pp ? *pp : 0;
            if (!t) continue;
            int job = *(i32 *)(t + 0x48);
            if (job == 15) { *(i32 *)(t + 0x48) = job = 56; }
            if (job < JOB0 || job >= JOB0 + NJOB) continue;
            int c = 5, d = 5;
            for (int k = 0; k < 16 && njob_ti[job - JOB0][k][0] >= 0; k++) {
                if (njob_ti[job - JOB0][k][0] == 27) c = njob_ti[job - JOB0][k][1];
                if (njob_ti[job - JOB0][k][0] == 28) d = njob_ti[job - JOB0][k][1];
            }
            *(i32 *)(t + 0x3c) = c; *(i32 *)(t + 0x40) = d;
        }
    }
}

/* ---- v94: co-op support choice (host, Create Game "Support" row in Co-op; 0 = the mission's own).
   The stage loader's fopen (0x51A7AD, full loader 0x51A6C0) -> stg_fopen: for the co-op mission being started, every
   player-owned (+0x24 == 0) support placement except the mission-critical Battaloon (17) / Bone Dragon (34) becomes the chosen
   kind (human or Dark Legion version by the original's race), at least the mission's co-op level, with job skills.
   Placement: +0x5b level, +0x80 4x{skill,level}, +0x194 soldier char, +0x196 look, +0x198/+0x19c formation, +0x1c4 job,
   +0x1c8 troop type. The troop keeps its id, so the mission script still finds it. ---- */
int coop_sup;
/* ---- v95: network co-op (2 players, lobby Game Type Co-op). The host plays the mission as in solo co-op; P2 plays a free slot
   of the mission (stage slot type 0) with their own MP army at the mission's co-op level, allied with the host. Everything
   that changes the stage file happens here, identically on both PCs (same file + host's choices from the START packet). ---- */
static int coop_p2slot = -1;
int coop_net(void)
{
    u32 m = *(u32 *)0xAE1808;
    return net_nplayers >= 2 && want_mode == 2 && m >= 100 && m < 100 + COOP_N && *(unsigned char *)0xAE1BF1;
}
int coop_is_p2(void) { return coop_net() && net_local_slot > 0; }
int net_is_spec(void) { return net_spec && net_nplayers >= 2; }   /* v96 */
static signed char inv_st[6] = { -1, -1, -1, -1, -1, -1 };   /* v100: Invasion extra players -> free stage slots */
static int ls_stage(int s) { if (s >= 0 && s < 6 && inv_st[s] >= 0) return inv_st[s]; return (coop_net() && s == 1 && coop_p2slot > 0) ? coop_p2slot : s; }
int ls_p2stage(void) { return ls_stage(1); }
static int ls_lobby_of(int st) { for (int s = 0; s < 6; s++) if (ls_stage(s) == st) return s; return -1; }
static const signed char sup_jobs[5][2] = { {-1,-1}, {12,31}, {9,32}, {13,30}, {14,29} };   /* human, dark legion */
static int is_sup(int j) { return j == 9 || j == 12 || j == 13 || j == 14 || j == 16 || (j >= 29 && j <= 32); }
static const signed char sup_sk[64][4] = {   /* skill id, weight % (2 skills) */
    [9] = {3,50,2,50}, [12] = {6,60,4,40}, [13] = {6,60,1,40}, [14] = {4,60,1,40},
    [29] = {7,70,0,30}, [30] = {7,70,0,30}, [31] = {7,60,8,40}, [32] = {7,60,3,40} };
static unsigned char *stg_buf;
/* TEST16 (USER 2026-11-14: in co-op the armies were well above the enemies, e.g. Gerald 7: Gerald 28, troops 26 against enemies
   of 6-14 - the Crusaders levels came from a fixed curve. "roughly matched to the average level maybe plus 2 or 3"; "upgrade
   troops where possible based on their level"; enemies keep +15% with a second player).
   Crusaders co-op missions: army level = median level of the enemy troops in the mission's own stage file (computer slots on
   another team; the median ignores the few boss troops) + 2, hero +2 on top. Same file on both PCs -> same level.
   Upgrades: down the promotion tree while the level covers the sum of the new job's skill requirements (JobInfo). */
static int coop_p1min __attribute__((section(".data")));
KTEXT static int up_lv(int j, int lv)
{
    for (int g = 0; g < 4; g++) {
        int n = -1;
        switch (j) {
        case 0:  if (lv >= 10) n = 1;  break;   /* Archer -> Longbow (Melee 3, Ranged 7) */
        case 2:  if (lv >= 10) n = 4;  break;   /* Infantry -> H.Infantry (Melee 10) */
        case 4:  if (lv >= 15) n = 5;  break;   /* H.Infantry -> Knight (Melee 15) */
        case 7:  if (lv >= 17) n = 8;  break;   /* Cavalry -> H.Cavalry (7+5+5) */
        /* H.Cavalry stays: Storm Riders are a support troop (USER: Heavy Cavalry far more useful late on) */
        case 10: if (lv >= 13) n = 11; break;   /* Sapper -> PyroTech (4+4+5) */
        case 20: if (lv >= 7)  n = 21; break;   /* D.Fighter -> D.Knight (Melee 7) */
        case 23: if (lv >= 5)  n = 26; break;   /* O.Infantry -> Orc Axemen (2+3) */
        /* Orc Axemen stay: Orc Heavy are too slow (USER 2026-11-15: Lucretia 4 cavalry part nigh impossible) */
        case 24: if (lv >= 17) n = 25; break;   /* O.Rider -> Orc H.Rider (5+5+7) */
        }
        if (n < 0) break;
        j = n;
    }
    return j;
}
static int add_sk __attribute__((section(".data")));
/* USER 2026-11-25 (Keaton: Walter 5's archers have Fire and Holy 25 but no Melee/Ranged -> only the minimum 10 damage on a
   Wyvern; USER: "the host units lost their base skills ... make sure all those skills are enabled, and for player 2 keep their
   existing skills just level them appropriately"). Network battles run the Live rules, under which the troop set-up (0x56B9A0)
   gives a human player's troops only the skills in their placement - the job's own skills (0x4A23E0: TroopInfo auto-skill row,
   level = troop level x rate) are only given to computer troops (sk_live_c). job_sk gives a placement its job's skills the same
   way, on the stage data, so every PC gets the same: a job skill it already has is raised to the job's level if lower; add_sk
   also adds the missing ones (free slot first, else in place of the lowest non-job skill). Returns the number changed. */
KTEXT static int job_sk(unsigned char *P, int lv)
{
    int j = *(short *)(P + 0x1c4), ch = 0;
    char *ti = ((char *(TC *)(void *))0x4A2940)((void *)0xA75198);
    if (!ti || !*(char **)(ti + 8) || j < 0 || j > 55 || lv < 1) return 0;   /* our own jobs (56+) have no auto-skill row */
    char *row = ((char *(TC *)(void *, int))0x4A2920)(ti, j);
    i32 *sk = (i32 *)(P + 0x80), id[3]; int nv[3], nj = 0;
    for (int q = 0; q < 3; q++) {
        int s = *(i32 *)(row + 4 + q * 4); if (s < 0) break;
        int v = (int)((float)lv * *(float *)(row + 0x10 + q * 4)); if (v < 1) continue; if (v > 99) v = 99;   /* rate 0: nothing, as 0x4A23E0 */
        id[nj] = s; nv[nj++] = v;
    }
    for (int q = 0; q < nj; q++) {
        int k = 0; while (k < 4 && sk[k * 2] != id[q]) k++;
        if (k < 4) { if (sk[k * 2 + 1] < nv[q]) { sk[k * 2 + 1] = nv[q]; ch++; } continue; }
        if (!add_sk) continue;
        k = 0; while (k < 4 && sk[k * 2] >= 0) k++;
        if (k == 4) {                                    /* full: replace the lowest skill that is not one of the job's */
            int lo = -1;
            for (int m = 0; m < 4; m++) { int isj = 0; for (int r = 0; r < nj; r++) if (sk[m * 2] == id[r]) isj = 1;
                                          if (!isj && (lo < 0 || sk[m * 2 + 1] < sk[lo * 2 + 1])) lo = m; }
            if (lo < 0) continue;
            k = lo;
        }
        sk[k * 2] = id[q]; sk[k * 2 + 1] = nv[q]; ch++;
    }
    return ch;
}
KTEXT static int med_enemy(unsigned char *b)
{
    unsigned char *H = b + 8;
    u32 cnt = *(u32 *)(b + 584);
    int h[100], n = 0;
    for (int i = 0; i < 100; i++) h[i] = 0;
    for (u32 i = 0; i < cnt && i < 1000; i++) {
        unsigned char *t = b + 588 + i * 548; int o = t[0x24], l = t[0x5b];
        if (o < 8 && H[o] >= 3 && H[o] <= 5 && H[8 + o] != H[8] && l > 0 && l < 100) { h[l]++; n++; }
    }
    if (!n) return 0;
    for (int l = 1, c = 0; l < 100; l++) { c += h[l]; if (c * 2 >= n) return l; }
    return 0;
}
static unsigned char *ct_buf;
KTEXT int coop_tgt_lv(int idx)
{
    static int ci = -1, cl = 0;
    if (idx < 0 || idx >= COOP_N) return 1;
    if (!coop_rt[idx]) return coop_lvtab[idx];
    if (ci == idx) return cl;
    int lv = coop_lvtab[idx], m = 0;
    char p[48]; SPRINTF(p, "Data\\Mission\\%s.stg", coopf[idx]);
    void *f = C_FOPEN(p, "rb");
    if (f) {
        if (!ct_buf) ct_buf = G_NEW(0x80000);
        u32 n = 0, r;
        if (ct_buf) while (n < 0x80000 && (r = C_FREAD(ct_buf + n, 1, 0x80000 - n, f)) > 0) n += r;
        C_FCLOSE(f);
        if (ct_buf && n >= 592 && n < 0x80000 && *(i32 *)ct_buf == 0x3E9) m = med_enemy(ct_buf);
    }
    if (m > 0) { lv = m + 2; if (lv > 97) lv = 97; }
    ls_log("co-op level: %s enemies median %d -> army level %d (curve %d)", (int)coopf[idx], m, lv, (int)coop_lvtab[idx]);
    ci = idx; cl = lv;
    return lv;
}
/* TEST29 (USER: custom troops in the Invasion randomiser): soldier table entry {char, look min, look max} of a job. The game's
   table 0xA4A120 has the 56 original jobs only (kuf.asm copies it to jobsold + the new jobs); reading it for 56+ gave other
   data - a Lich troop got Gerald (char 32) as soldiers: wrong model, its equipment did not fit (crash), could not be killed. */
KTEXT __attribute__((noinline)) static int jsold(int j, int f)
{
    if (j >= JOB0 && j < JOB0 + NJOB) return f == 0 ? njobs[j - JOB0].ch : 0;
    if (j < 0 || j >= 56) return 0;
    return *(i32 *)(0xA4A120 + j * 12 + f * 4);
}
KTEXT static void stg_setjob(unsigned char *t, int nj)
{
    char **to = *(char ***)(0xA75198 + 0x5a0);
    if (!to || !to[0] || !to[1]) return;
    int oj = *(short *)(t + 0x1c4);
    *(short *)(t + 0x1c4) = (short)nj;
    *(short *)(t + 0x194) = (short)jsold(nj, 0);
    *(short *)(t + 0x196) = (short)jsold(nj, 1);
    unsigned char *b = (unsigned char *)to[1] + nj * 0x14;
    *(i32 *)(t + 0x198) = b[0xD]; *(i32 *)(t + 0x19c) = b[0xE];
    *(i32 *)(t + 0x1c8) = *(i32 *)(to[0] + nj * 0x60 + 4);
    /* TEST24 (USER: story heroes / officers missing in co-op): only a troop led by its job's plain leader gets the new job's
       plain leader; a named leader (story hero, officer) stays */
    if (t[0x58] && oj >= 0 && oj < 64 && t[0x58] == (unsigned char)jsold(oj, 0)) {
        t[0x58] = (unsigned char)jsold(nj, 0);   /* TEST30: Ogre troops keep the leader's look (look 1 of char 26 is not loaded in network battles) */
    }
}
int coop_army_lv;   /* TEST23: original-campaign co-op army level for both players (0 = use coop_tgt_lv) */
u32 coop_stg_crc;   /* v98c: checksum of the co-op stage file as installed (host sends it with its army) */
int ls_is_net(void);
void ls_hdr_colours(unsigned char *M);
/* USER 2026-11-26: the host's mission troops (stage slot 0) in co-op, network and solo alike - the hero's own troop keeps its
   earlier mission's troop, and every troop gets its job's look. Returns the number of troops changed. */
KTEXT static int coop_looks(u32 map, u32 cnt)
{
    int lk = 0;
    for (u32 i = 0; i < cnt; i++) {
        unsigned char *t = stg_buf + 588 + i * 548;
        int j = *(short *)(t + 0x1c4);
        if (t[0x24] != 0 || j < 0 || j > 34) continue;
              /* USER 2026-11-26 ("strange for walter to have paladins on walter 3 then knights on walter 5 - he should keep paladins
           from 3 onwards"): a hero's own troop never drops back to a lower troop than in an earlier mission of the same
           campaign - it takes that mission's troop (job, soldier type, look). From the mission files: Walter 2 H.Infantry
           (Walter 1), Walter 5 Paladins (Walter 3/4), Rupert 5/6 H.Infantry (Rupert 4), Urukubarr 4 heavy orc infantry
           (Urukubarr 3), Morene 4 Dark Knights (Morene 3). */
        if (t[0x25]) {
            static const struct { char f[5]; signed char j, ut, lk; } hc[6] = {
                {"4020", 4, 1, 2}, {"4070", 6, 13, 0}, {"1050", 4, 1, 2}, {"1060", 4, 1, 2}, {"2030", 27, 25, 2}, {"5060", 21, 18, 5} };
            for (int q = 0; q < 6; q++) if (ieq(coopf[map - 100], hc[q].f) && j != hc[q].j) {
                stg_setjob(t, hc[q].j); *(short *)(t + 0x194) = hc[q].ut; *(short *)(t + 0x196) = hc[q].lk;
                ls_log("co-op: the hero's own troop keeps its earlier troop - job %d -> %d", j, hc[q].j, 0, 0); lk++;
            }
        }
        /* USER 2026-11-26 ("units are upgraded but they have the wrong looks ... basic infantry holding a 2 handed sword"):
           many mission troops carry soldier look 0 (the plain C_HU_NR body) whatever their job - in single player the
           player's troops come from the saved army instead. A troop of its job's own soldier type gets at least the job's
           look (0xA4A120: Knight / H.Cavalry 3 = full plate, Longbow 1, Cavalry 1, D.Knight 1, orc heavies 2). */
        { int jj = *(short *)(t + 0x1c4);
          if (jj >= 0 && jj < 56 && *(short *)(t + 0x194) == jsold(jj, 0) && *(short *)(t + 0x196) < jsold(jj, 1)) {
              *(short *)(t + 0x196) = (short)jsold(jj, 1); lk++; } }
    }
    if (lk) ls_log("co-op: %d host troops given their job's look / earlier troop", lk, 0, 0, 0);
    return lk;
}
/* v98: network Versus / Invasion: put the lobby colours into the stage file's header (see ls_hdr_colours) */
/* v100 (USER): Invasion maps take one more player (Ruin 2 -> 3, IceStorm 3 -> 4). Each extra lobby player gets a free stage slot
   (type 0 -> 2 remote human, the human players' team group) with copies of the last human slot's start placements (new ids,
   moved 600 units), which ls_army then fills from that player's army. ls_stage maps the lobby slot to it. */
KTEXT static u32 inv_extra(u32 n)
{
    unsigned char *H = stg_buf + 8;                         /* +0 types[8], +8 groups[8], +0x10 colours[8] */
    int hum[8], nh = 0;
    for (int k = 0; k < 8; k++) if (H[k] == 1 || H[k] == 2) hum[nh++] = k;
    int np = (int)net_nplayers > 6 ? 6 : (int)net_nplayers;
    if (nh < 2 || np <= nh) return n;
    u32 cnt = *(u32 *)(stg_buf + 584);
    if (cnt > 1000 || 588 + cnt * 548 > n) return n;
    int maxid = 0;
    for (u32 i = 0; i < cnt; i++) { int id = *(i32 *)(stg_buf + 588 + i * 548 + 0x20); if (id > maxid) maxid = id; }
    static unsigned char *nw; int nn = 0;
    if (!nw) nw = G_NEW(16 * 548);
    if (!nw) return n;
    for (int L = nh; L < np; L++) {
        int f = -1;
        for (int k = 1; k < 8; k++) if (H[k] == 0) { f = k; break; }
        if (f < 0) break;
        H[f] = 2; H[8 + f] = H[8 + hum[0]]; inv_st[L] = (signed char)f;
        int e = L - nh, src = hum[nh - 1 - e % nh], rep = e / nh + 1;   /* v100i: extras start next to P3, P2, P1 in turn */
        for (u32 i = 0; i < cnt && nn < 16; i++) {
            unsigned char *t = stg_buf + 588 + i * 548;
            if (t[0x24] != src) continue;
            unsigned char *d = nw + nn * 548;
            for (int q = 0; q < 548; q++) d[q] = t[q];
            d[0x24] = (unsigned char)f; d[0x5a] = 0xFF;
            *(i32 *)(d + 0x20) = maxid + 1 + nn;
            *(float *)(d + 0x48) += 600.0f * (float)rep; *(float *)(d + 0x4c) += 600.0f * (float)rep;
            nn++;
        }
        ls_log("invasion: extra player %d plays stage slot %d", L, f, 0, 0);
    }
    if (!nn || n + nn * 548 >= 0x80000) return n;
    unsigned char *end = stg_buf + 588 + cnt * 548;
    u32 taillen = n - (u32)(end - stg_buf);
    for (u32 q = taillen; q-- > 0;) end[nn * 548 + q] = end[q];
    for (int q = 0; q < nn * 548; q++) end[q] = nw[q];
    *(u32 *)(stg_buf + 584) = cnt + nn;
    return n + nn * 548;
}
/* v100d: each human slot's main (hero) placement first among its placements - IceStorm lists sub_P3 1/2 before P3, so P3's
   hero troop was the player's 2nd troop (USER: Walter moved to his second unit with no control; extra player not on Morene) */
KTEXT static void main_first(u32 n)
{
    unsigned char *H = stg_buf + 8;
    u32 cnt = *(u32 *)(stg_buf + 584);
    if (cnt > 1000 || 588 + cnt * 548 > n) return;
    static unsigned char tmp[548];
    for (int k = 0; k < 8; k++) {
        if (H[k] != 1 && H[k] != 2) continue;
        int first = -1, mn = -1;
        for (u32 i = 0; i < cnt; i++) {
            unsigned char *t = stg_buf + 588 + i * 548;
            if (t[0x24] != k) continue;
            if (first < 0) first = (int)i;
            if (t[0x25]) { mn = (int)i; break; }
        }
        if (first < 0 || mn <= first) continue;
        unsigned char *base = stg_buf + 588;
        for (int q = 0; q < 548; q++) tmp[q] = base[mn * 548 + q];
        for (int i = mn; i > first; i--) for (int q = 0; q < 548; q++) base[i * 548 + q] = base[(i - 1) * 548 + q];
        for (int q = 0; q < 548; q++) base[first * 548 + q] = tmp[q];
        ls_log("stage slot %d: hero placement moved first", k, 0, 0, 0);
    }
}
/* v100q (USER: Invasion is very hard - give players a 4th unit, hero + 3 instead of hero + 2): every human stage slot gets one more
   placement, a copy of its last non-hero placement (new id, moved 150 units) that ls_army fills from the player's 4th army troop. */
KTEXT static u32 inv_more(u32 n)
{
    unsigned char *H = stg_buf + 8;
    u32 cnt = *(u32 *)(stg_buf + 584);
    if (cnt > 1000 || 588 + cnt * 548 > n) return n;
    int maxid = 0;
    for (u32 i = 0; i < cnt; i++) { int id = *(i32 *)(stg_buf + 588 + i * 548 + 0x20); if (id > maxid) maxid = id; }
    static unsigned char *nw; int nn = 0;
    if (!nw) nw = G_NEW(8 * 548);
    if (!nw) return n;
    for (int k = 0; k < 8 && nn < 8; k++) {
        if (H[k] != 1 && H[k] != 2) continue;
        int last = -1;
        for (u32 i = 0; i < cnt; i++) { unsigned char *t = stg_buf + 588 + i * 548; if (t[0x24] == k && !t[0x25]) last = (int)i; }
        if (last < 0) continue;
        unsigned char *d = nw + nn * 548, *t = stg_buf + 588 + last * 548;
        for (int q = 0; q < 548; q++) d[q] = t[q];
        d[0x5a] = 0xFF;
        *(i32 *)(d + 0x20) = maxid + 1 + nn;
        *(float *)(d + 0x48) += 150.0f;
        nn++;
    }
    if (!nn || n + nn * 548 >= 0x80000) return n;
    unsigned char *end = stg_buf + 588 + cnt * 548;
    u32 taillen = n - (u32)(end - stg_buf);
    for (u32 q = taillen; q-- > 0;) end[nn * 548 + q] = end[q];
    for (int q = 0; q < nn * 548; q++) end[q] = nw[q];
    *(u32 *)(stg_buf + 584) = cnt + nn;
    ls_log("invasion: 4th unit placement for %d stage slots", nn, 0, 0, 0);
    return n + nn * 548;
}
/* TEST25 (USER): Invasion variants, chosen in Create Game and sent to every PC with the start packet (want_dmg in Invasion).
   Randomiser: every enemy wave troop of a normal troop kind gets a random kind from inv_pool (custom jobs included again from Test 29), from a
   seed every PC shares (net_seed, new each game); the wave levels stay. Insanity: every enemy troop is level 99 (stage record
   and every wave spawn, 0x54A946), upgraded as far as up_lv allows (Spearmen, Axemen and Heavy Cavalry are never upgraded),
   and computer troops get their job's skills at that level (sk_live_c). Both can be on. Same file + same seed on every PC. */
int inv_var;
unsigned coop_enm;   /* TEST40 (USER: Swipez - enemy officers still die too quickly): stage slots of the co-op enemies (bit per slot) */
/* TEST28 (USER, Test 27 Invasion randomiser: an enemy troop turned into a Lich showed as a troop of Geralds, could not be
   killed, and the game crashed in the model code): our custom jobs (56-60) are only set up for players' armies, not for
   computer troops from the mission file - left out of the randomiser until they are. */
static const unsigned char inv_pool[] = { 0,1,2,3,4,5,6,7,8,9,10,11,18,19,20,21,22,23,24,25,26,27,28,33,56,57,58,59,60 };   /* TEST29: custom jobs back (jsold) */
/* TEST39 (USER: "a true randomiser should include all units"): the Randomiser's own pool - every troop kind: Human and Dark Legion
   troops, support and siege (Bomber Wing, Mortar, Ballistas, Catapult, Battaloon, Scorpion, Swamp Mammoth, Dirigible, Wyvern,
   Bone Dragon), Ghouls and Exploding Ghouls, the Encablossa (normal, Flyer, Spitter, Infested), the four elementals (one row each -
   the game has three identical rows per elemental, which would make them 3x as likely) and our custom troops. Left out: walls
   (35, 41), the Scout (36) and the Encablossa column (42) - fixed structures / special placements, not troops. inv_pool above stays
   the list for Roguelike Hero's reserve troops (the player's own army). Kinds come from the stage file, so each map loads their models. */
/* TEST52 (Swipez, Randomiser: Sappers stood still when they died): Sapper (10) and PyroTech (11) left out - work troops, not fighters */
static const unsigned char inv_rpool[] = { 0,1,2,3,4,5,6,7,8,9,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,37,
                                           38,39,40,43,44,45,46,55,56,57,58,59,60 };
KTEXT static int inv_inpool(int j) { for (u32 q = 0; q < sizeof inv_rpool; q++) if (inv_rpool[q] == j) return 1; return 0; }
KTEXT __attribute__((noinline)) static void inv_variant(u32 n)
{
    inv_var = want_mode == 1 ? (want_dmg & 7) : 0;   /* TEST36: 4 = Roguelike */
    if (inv_var >= 4 || !(inv_var & 3) || n < 592) return;   /* TEST36/38: 4, 5 = Roguelike (no randomiser) */
    unsigned char *H = stg_buf + 8;
    u32 cnt = *(u32 *)(stg_buf + 584), r = net_seed ^ 0x9E3779B9u, nr = 0, ni = 0;
    for (u32 i = 0; i < cnt && 588 + (i + 1) * 548 <= n; i++) {
        unsigned char *t = stg_buf + 588 + i * 548; int o = t[0x24], j = *(short *)(t + 0x1c4);
        if (o >= 8 || H[o] < 3 || H[o] > 5 || H[8 + o] == H[8]) continue;      /* computer players on another team */
        if (!inv_inpool(j)) { if (inv_var & 2) { t[0x5b] = 99; ni++; } continue; }   /* heroes and structures keep their kind */
        if (inv_var & 1) { r = r * 1103515245u + 12345u; j = inv_rpool[(r >> 16) % sizeof inv_rpool]; stg_setjob(t, j); nr++; }
        if (inv_var & 2) { int u = up_lv(j, 99); if (u != j) stg_setjob(t, u); t[0x5b] = 99; ni++; }
    }
    ls_log("invasion variant %d: %d troops randomised, %d at level 99", inv_var, (int)nr, (int)ni, 0);
}
KTEXT static void *stg_colours(const char *path, const char *mode)
{
    void *f = C_FOPEN(path, mode);
    if (!f) return f;
    if (!stg_buf) stg_buf = G_NEW(0x80000);
    if (!stg_buf) return f;
    u32 n = 0, r;
    while (n < 0x80000 && (r = C_FREAD(stg_buf + n, 1, 0x80000 - n, f)) > 0) n += r;
    C_FCLOSE(f);
    f = C_FOPEN(path, mode);
    if (n < 592 || n >= 0x80000 || *(i32 *)stg_buf != 0x3E9) return f;
    { u32 h = 2166136261u; for (u32 q = 0; q < n; q++) { h ^= stg_buf[q]; h *= 16777619u; } coop_stg_crc = h; }   /* v99: stage fingerprint */
    for (int k = 0; k < 6; k++) inv_st[k] = -1;
    main_first(n);                                          /* v100d */
    if (want_mode == 1) n = inv_extra(n);                   /* v100: Ruin 3 / IceStorm 4 players */
    if (want_mode == 1) n = inv_more(n);                    /* v100q: hero + 3 units */
    inv_var = 0;                                            /* TEST36: no variant left over from an earlier Invasion game */
    coop_enm = 0;                                           /* TEST40: co-op enemy slots (leader HP +50%), set below in co-op */
    if (want_mode == 1) inv_variant(n);                     /* TEST25: Randomiser / Insanity */
    /* v100n (USER dump, extra player put into watching at the start): the loader set the local player index G+0x704 from
       ls_load_slot BEFORE this stage file was read, i.e. before inv_st existed -> an extra player became stage slot 3 = the
       Invasion ENEMY player; the map's 'player 3 loses' then ended the extra player's battle. Set it again now. */
    { int ls = ls_load_slot(); if (ls > 0) *(i32 *)(0xA75198 + 0x704) = ls; }
    ls_hdr_colours(stg_buf + 4);
    void *g = temp_open(path, mode, stg_buf, n);
    if (!g) return f;
    C_FCLOSE(f);
    return g;
}
/* v100p multiplayer balance (USER), active only while a network battle is loaded (Versus, co-op, multiplayer Invasion) and for
   every troop in it, AI included. Set at each .stg open before the stage builds its troops, put back for any other stage and
   when the network battle ends. Same on every PC (the fingerprint hashes these arrays).
   TroopInfo in memory [G+0x5a0]: to[0] rows 0x60 (file int 11 = ranged attack -> float +0x2c; file ints 14-23 = damage taken
   x100 -> floats /100: +0x38 melee, +0x3c ranged, +0x44 FRONTAL (file 16), +0x40 explosion (file 17), ...), to[1] rows 0x14
   (+4 = soldier HP float, file int 25).
   - Longbow (row 1): ranged attack 26 -> 23 (~-10%; Archer 20). USER: "longbows slightly overtuned".
   - Thunder Rhino elementals (rows 45, 49, 53): soldier HP 5000 -> 6000 (+20%), frontal 2.5 (-150 resistance) -> 1.0 (neutral).
     USER: "far too weak in PvP". */
struct mpb { short tab, row, off; float from, to; };
static const struct mpb mpb_tab[] = {
    { 0,  1, 0x2c,   26.0f,   20.0f },   /* TEST18: Longbow 23 -> 20 (USER 2026-11-14 option 1, archers too strong in MP) */
    { 0,  0, 0x2c,    0.0f,   0.85f },   /* TEST18: Archer x0.85 (20 -> 17) - from 0 = multiply */
    { 0, 18, 0x2c,    0.0f,   0.85f },   /* TEST18: Dark Archer x0.85 */
    { 0, 19, 0x2c,    0.0f,   0.85f },   /* TEST18: Cavalry Archer x0.85 */
    { 1, 45, 0x04, 5000.0f, 6000.0f }, { 0, 45, 0x44, 2.5f, 1.0f },
    { 1, 49, 0x04, 5000.0f, 6000.0f }, { 0, 49, 0x44, 2.5f, 1.0f },
    { 1, 53, 0x04, 5000.0f, 6000.0f }, { 0, 53, 0x44, 2.5f, 1.0f },
};
#define MPB_N (int)(sizeof(mpb_tab) / sizeof(mpb_tab[0]))
static float mpb_orig[MPB_N]; static unsigned char mpb_set[MPB_N];
void lb_trim(int on)
{
    char **to = *(char ***)(0xA75198 + 0x5a0);
    if (!to || !to[0] || !to[1]) return;
    u32 n = ((u32 *)to)[3];
    for (int i = 0; i < MPB_N; i++) {
        const struct mpb *m = &mpb_tab[i];
        if ((u32)m->row >= n) continue;
        float *f = (float *)(to[m->tab] + m->row * (m->tab ? 0x14 : 0x60) + m->off);
        if (on && !mpb_set[i]) { mpb_orig[i] = *f; if (m->from == 0.0f) *f = *f * m->to; else if (*f == m->from) *f = m->to; mpb_set[i] = 1; }
        else if (!on && mpb_set[i]) { *f = mpb_orig[i]; mpb_set[i] = 0; }
    }
}
/* ---- v1.2 (USER: "generate a flat map for invasion ... same amount of slots ... plain grass"): Invasion map id 26 "Plains".
   Built at run time from the installed 9_IceStorm files into Data\Mission\Live\9_Plains.{map,dds,stg}, so no extra files ship:
   - .map: same 200x200 size, grass tileset (area02, base tiles 0-3 with random flips), one flat height, no trees (IceStorm's
     802 trees removed, its healing tombstone kept), all cells open (the game itself blocks 3000 units at each edge),
     no forest cover, no fire fuel.
   - .dds: plain grass minimap (512x512 DXT1), the blocked edge darker.
   - .stg: IceStorm's stage file (same 6 player slots, waves, enemy AI AI2_Ruin.txt, spawns) with MAPNAME / MINIMAPNAME
     pointing at the new files. Every PC builds the same bytes. ---- */
#define PL_DIR "Data\\Mission\\Live\\"
#define PL_H 300
static int pl_done;
static u32 pl_rng;
KTEXT static u32 pl_rand(void) { pl_rng = pl_rng * 1103515245u + 12345u; return pl_rng >> 16; }
KTEXT static void *pl_read(const char *path, u32 max, u32 *len)
{
    *len = 0;
    void *f = C_FOPEN(path, "rb");
    if (!f) return 0;
    unsigned char *b = G_NEW(max);
    if (b) { u32 r; while (*len < max && (r = C_FREAD(b + *len, 1, max - *len, f)) > 0) *len += r; }
    C_FCLOSE(f);
    return b;
}
KTEXT static int pl_write(const char *path, const void *buf, u32 len)
{
    void *k32 = (*(fn_gmh2 *)0x988170)("kernel32.dll");
    if (!k32) return 0;
    fn_wf wf = (fn_wf)(*(fn_gpa2 *)0x98829C)(k32, "WriteFile");
    if (!wf) return 0;
    void *h = (*(fn_cf *)0x988134)(path, 0x40000000, 0, 0, 2, 0x80, 0);
    if (h == (void *)-1) return 0;
    u32 w = 0; int ok = wf(h, buf, len, &w, 0) && w == len;
    (*(fn_ch *)0x988144)(h);
    return ok;
}
KTEXT static void pl_name(unsigned char *d, const char *s, int room)
{
    int i = 0;
    for (; s[i] && i < room - 1; i++) d[i] = (unsigned char)s[i];
    for (; i < room; i++) d[i] = 0;
}
KTEXT void plains_make(void)
{
    if (pl_done) return;
    pl_done = 1;
    const int W = 200, H = 200;
    u32 n = 0;
    unsigned char *src = pl_read(PL_DIR "9_IceStorm.map", 0x40000, &n);
    if (!src || n < 64 || *(u32 *)src != 0x6d) { ls_log("plains: 9_IceStorm.map not readable (%d)", (int)n, 0, 0, 0); return; }
    /* IceStorm's healing tombstone (prop 420) */
    unsigned char heal[24]; int has_heal = 0;
    {
        u32 nc = *(u32 *)(src + 4), c1 = 0;
        for (u32 k = 0; k < nc && k < 16; k++) if (*(u32 *)(src + 8 + k * 8) == 1) c1 = *(u32 *)(src + 12 + k * 8);
        if (c1 && c1 + 4 <= n) {
            u32 pc = *(u32 *)(src + c1);
            for (u32 i = 0; i < pc && c1 + 28 + i * 24 <= n; i++) {
                unsigned char *r = src + c1 + 4 + i * 24;
                if (*(i32 *)r == 420) { for (int q = 0; q < 24; q++) heal[q] = r[q]; *(float *)(heal + 12) = (float)PL_H; has_heal = 1; break; }
            }
        }
    }
    u32 sz0 = 264 + 2 * W * H + 2 * (W + 1) * (H + 1), sz1 = 4 + 24 * has_heal, sz2 = 8 + W * H, sz3 = 8 + 64 * 64, sz5 = sz3;
    u32 hdr = 8 + 5 * 8 + 8 + 4 * has_heal;
    u32 tot = hdr + sz0 + sz1 + sz2 + sz3 + sz5;
    unsigned char *m = G_NEW(tot);
    if (!m) return;
    for (u32 q = 0; q < tot; q++) m[q] = 0;
    u32 o0 = hdr, o1 = o0 + sz0, o2 = o1 + sz1, o3 = o2 + sz2, o5 = o3 + sz3;
    u32 *h32 = (u32 *)m;
    h32[0] = 0x6d; h32[1] = 5;
    h32[2] = 0; h32[3] = o0; h32[4] = 1; h32[5] = o1; h32[6] = 2; h32[7] = o2; h32[8] = 3; h32[9] = o3; h32[10] = 5; h32[11] = o5;
    m[48] = 'P'; m[49] = 'I'; m[50] = 'N'; m[51] = 'F'; *(u32 *)(m + 52) = (u32)has_heal;
    if (has_heal) *(i32 *)(m + 56) = 420;
    pl_name(m + o0, "area02_maptile.tga", 256);
    *(u32 *)(m + o0 + 256) = W; *(u32 *)(m + o0 + 260) = H;
    unsigned short *tl = (unsigned short *)(m + o0 + 264);
    pl_rng = 0x9A1215u;
    for (int i = 0; i < W * H; i++) {
        u32 r = pl_rand(), f = pl_rand() % 100;
        u32 fl = f < 60 ? 0 : f < 74 ? 1 : f < 88 ? 2 : 3;        /* flip mix of the grass Versus maps (Halmoral) */
        tl[i] = (unsigned short)((r & 3) | (fl << 14));
    }
    short *hg = (short *)(m + o0 + 264 + 2 * W * H);
    for (int i = 0; i < (W + 1) * (H + 1); i++) hg[i] = PL_H;
    *(u32 *)(m + o1) = (u32)has_heal;
    if (has_heal) for (int q = 0; q < 24; q++) m[o1 + 4 + q] = heal[q];
    *(u32 *)(m + o2) = W; *(u32 *)(m + o2 + 4) = H;                 /* cell attributes: all open */
    *(u32 *)(m + o3) = 64; *(u32 *)(m + o3 + 4) = 64;
    for (int i = 0; i < 4096; i++) m[o3 + 8 + i] = 255;            /* openness (unused by the game) */
    *(u32 *)(m + o5) = 64; *(u32 *)(m + o5 + 4) = 64;               /* fire fuel: none */
    int okm = pl_write(PL_DIR "9_Plains.map", m, tot);
    /* minimap: 512x512 DXT1 */
    u32 dl = 128 + 128 * 128 * 8;
    unsigned char *dd = G_NEW(dl);
    int okd = 0;
    if (dd) {
        for (u32 q = 0; q < 128; q++) dd[q] = 0;
        u32 *D = (u32 *)dd;
        D[0] = 0x20534444; D[1] = 124; D[2] = 0x81007; D[3] = 512; D[4] = 512; D[5] = 512 * 512 / 2;
        D[19] = 32; D[20] = 4; D[21] = 0x31545844; D[27] = 0x1000;
        int edge = 3000 * 512 / 25000 / 4;                          /* blocked border in 4x4 blocks */
        for (int by = 0; by < 128; by++) for (int bx = 0; bx < 128; bx++) {
            unsigned char *b = dd + 128 + (by * 128 + bx) * 8;
            int e = bx < edge || by < edge || bx >= 128 - edge || by >= 128 - edge;
            unsigned short c0 = e ? (unsigned short)((34 >> 3) << 11 | (58 >> 2) << 5 | (24 >> 3)) : (unsigned short)((98 >> 3) << 11 | (138 >> 2) << 5 | (58 >> 3));
            unsigned short c1 = e ? (unsigned short)((26 >> 3) << 11 | (44 >> 2) << 5 | (18 >> 3)) : (unsigned short)((78 >> 3) << 11 | (114 >> 2) << 5 | (44 >> 3));
            *(unsigned short *)b = c0; *(unsigned short *)(b + 2) = c1;
            *(u32 *)(b + 4) = (pl_rand() << 16 | pl_rand()) & 0x55555555u;   /* speckle between the two greens */
        }
        okd = pl_write(PL_DIR "9_Plains.dds", dd, dl);
    }
    /* stage file */
    u32 sl = 0; int oks = 0;
    unsigned char *st = pl_read(PL_DIR "9_IceStorm.stg", 0x80000, &sl);
    if (st && sl > 0x80 && *(i32 *)st == 0x3E9) {
        pl_name(st + 0x20, "9_Plains.map", 15);                   /* "9_IceStorm.map\0" */
        pl_name(st + 0x60, "9_Plains.bmp", 15);
        oks = pl_write(PL_DIR "9_Plains.stg", st, sl);
    }
    ls_log("plains: map %d (%d bytes) minimap %d stage %d", okm, (int)tot, okd, oks);
}
void afw_reset(void);
#ifdef KUF_ENCA
/* USER 2026-11-23 (E.Regnier: "his fire ability is still orange"): particle slot 123 (never used by the game) gets its own copy
   of slot 6 KUF2_PTL_REG_FIRE (data\Particle\Reg_Fire.spf, loaded the normal way through 0x87D160) with each member's start /
   end colour turned blue (red and blue swapped: orange 0.8,0.14,0 -> 0,0.14,0.8; the flame core 1,0.8,0.23 -> 0.23,0.8,1).
   Its textures are grey (L8), so the colour comes only from these values. Regnier's two REG_FIRE emitters (0x643FFC fire
   sword, 0x846D0A K2RegnierView) use slot 123 in the E build; Flame Wraiths keep slot 6. */
static int rn_blue_done;
void rn_blue(void);
/* USER 2026-11-23 (E.Regnier must be his own hero, normal Regnier unchanged; agreed: a hero-list entry using Regnier's body):
   E.Regnier = unit type 34 whose troop is an Encablossa troop (job 61, set by enca_build). Everything E-specific keys on that. */
KTEXT __attribute__((noinline)) int rn_is_e(char *u)
{
    if (!u || *(i32 *)(u + 0x14) != 34) return 0;
    char *t = *(char **)(u + 0x30);
    return t && *(i32 *)(t + 0x1890) == 61;
}
/* army record of E.Regnier (job 61, leader 34): name "E.Regnier" - 0x6B4C80 copies CharInfo's name into the record at every
   battle load (0x6B1E10), hooked at 0x6B4F53 */
KTEXT const char *rn_recname(char *rec, int ch)
{
    if (rec && ch == 34) { static int n __attribute__((section(".data"))); if (n++ < 6) ls_log("E.Regnier check: army record of Regnier, job %d, look %d", *(i32 *)(rec + 0x48), *(i32 *)(rec + 0x30), 0, 0); }
    if (rec && ch == 34 && *(i32 *)(rec + 0x48) == 61) { *(i32 *)(rec + 0x30) = 1; rn_variant(); return "E.Regnier"; }
    return ((const char *(TC *)(void *, int))0x6AAE40)((void *)0xA75198, ch);
}
/* his look: kernel special light 40 (table 0xA861B8 has 60 records; the game uses 0-19) = black, blended 45 % into the scene
   light for his model only (object message 0x1F7, applied before each draw by 0x898260) - 55 % brightness, same colours.
   Re-sent from his sword-flame update (0x846D12), so a hit flash (which resets the light) only lasts until the next one. */
KTEXT __attribute__((noinline)) static void rn_e_light(char *view)
{
    char *mdl = *(char **)(view + 8); void *k = *(void **)0xA75BC4;
    if (!mdl || !k) return;
    float *L = (float *)(0xA861B8 + 40 * 0x68);
    for (int i = 0; i < 26; i++) L[i] = 0;
    *(i32 *)L = 3; L[17] = -1.0f;                                  /* directional, straight down; no colour */
    static float w = 0.45f;
    ((void (TC *)(void *, int, int, int, int, float *))0x87FEB0)(k, *(i32 *)(mdl + 0x54), 0x1F7, 40, -1, &w);
}
/* K2RegnierView sword flame (0x846D12 call 0x87A470(slot 6 REG_FIRE ...)): E.Regnier's uses slot 123 (blue copy) */
KTEXT int rn_fire(char *view, int slot)
{
    {   /* USER 2026-11-25 (E.Regnier: no blue fire, solo log had no "blue fire" line): log which Regnier the flame is for */
        static int n __attribute__((section(".data"))); char *u = view ? *(char **)(view + 0x10) : 0, *t = u ? *(char **)(u + 0x30) : 0;
        if (u && n < 6) { n++; ls_log("E.Regnier check: sword flame for unit type %d, troop job %d -> %s", *(i32 *)(u + 0x14), t ? *(i32 *)(t + 0x1890) : -1,
                                      (int)(rn_is_e(u) ? "E.Regnier (blue)" : "normal Regnier"), 0); }
    }
    if (!view || !rn_is_e(*(char **)(view + 0x10))) return slot;
    rn_blue();
    if (!rn_blue_done) { static int m __attribute__((section(".data"))); if (m++ < 3) ls_log("E.Regnier: blue fire not ready (particle slot 6 not loaded yet)", 0, 0, 0, 0); }
    return *(void **)(*(char **)0xA75BC4 + 0x44D8 + 123 * 4) ? 123 : slot;
}
/* USER 2026-11-25 (E.Regnier "still orange flames", solo log had no sword-flame line at all - 0x846D12 only runs while view+0x7c is
   set): his burning sword is KUF2FX_FireSword (made by K2RegnierView's ctor, handle at view+0x60), whose 0x643F40 starts the
   REG_FIRE (6) emitters. The ctor (0x845B10, rn_fsw) records which view owns each FireSword object; 0x643FFC (rn_fsp) asks here
   and E.Regnier's gets the blue copy (123). The attack flame in K2RegnierView::vfunction3 (0x846175, rn_fw2) goes to rn_fire too. */
static char *rn_sw_obj[16] __attribute__((section(".data"))), *rn_sw_view[16] __attribute__((section(".data")));
static int rn_sw_next __attribute__((section(".data")));
KTEXT void rn_sword(char *view, int h)
{
    char *mgr = *(char **)0xA75100;
    if (!mgr || h < 0) return;
    char **pp = ((char **(TC *)(void *, int))0x5E4050)(mgr + 0x2c, h);
    char *o = pp ? *pp : 0;
    if (!o) return;
    int i = 0;
    while (i < 16 && rn_sw_obj[i] != o) i++;
    if (i == 16) { i = rn_sw_next; rn_sw_next = (rn_sw_next + 1) & 15; }
    rn_sw_obj[i] = o; rn_sw_view[i] = view;
}
KTEXT int rn_swslot(char *o)
{
    for (int i = 0; i < 16; i++)
        if (o && rn_sw_obj[i] == o) { char *v = rn_sw_view[i]; return v && *(u32 *)v == 0x9A685C ? rn_fire(v, 6) : 6; }   /* still a K2RegnierView */
    return 6;
}
/* USER 2026-11-23 (E.Regnier his own hero, normal Regnier unchanged; "duplicate Regnier's files and edit them"): E.Regnier is
   Regnier with look 1 - a second mesh variant of Regnier's body type (K2UnitType row 17, job 32 C_REGNIER), "C_DP_RNE.SMF" in
   mesh slot RN_E_MESH. Nothing is shipped: that mesh is read from the player's own C_DP_RN.SMFX (rn_mesh_name) and its body
   texture renamed to C_DP_RNE_Body (rn_mesh_fix); tc_name reads C_DP_RN_Body for it and darkens it (rn_dark). His Body /
   Portrait pictures ask for UV_DU_HERO_REGNIER_1_* (the look is in the file name) -> Regnier's _0_ pictures, darkened. The
   army record's leader look (+0x30, copied to the stage troop +0x59 by 0x6B45A0) is 1. Look 0 (normal Regnier) is untouched. */
#define RN_E_MESH 1095
KTEXT __attribute__((noinline)) void rn_variant(void)
{
    void *mgr = ((void *(*)(void))0x6A4820)();
    if (!mgr) return;
    char *bt = ((char *(TC *)(void *, int))0x6C30D0)(mgr, 32);
    if (!bt) return;
    char **v = (char **)(bt + 0x8c);
    if (!v[0] || v[1] - v[0] != 0x44 || !pfx_i(v[0] + 4, "c_dp_rn.")) return;   /* already done (2 variants), not loaded, not his */
    char *n = ((char *(*)(unsigned))0x8D5E88)(2 * 0x44);     /* the game's operator new: the vector may free it */
    if (!n) return;
    for (int i = 0; i < 0x44; i++) { n[i] = v[0][i]; n[0x44 + i] = 0; }
    *(i32 *)(n + 0x44) = RN_E_MESH;
    { const char *m = "C_DP_RNE.SMF"; for (int i = 0; m[i]; i++) n[0x48 + i] = m[i]; }
    v[0] = n; v[1] = n + 0x88; v[2] = n + 0x88;              /* the old buffer is left alone (never freed) */
    ls_log("E.Regnier: mesh variant 1 added to Regnier's body type (mesh %d, look 0 = mesh %d)", RN_E_MESH, *(i32 *)n, 0, 0);
}
static int rn_mesh_pend;
KTEXT int rn_mesh_name(char *nm)       /* 0x87CEC9: mesh file name [ebp-0x218] ("...C_DP_RNE.SMFX") -> C_DP_RN.SMFX, sync read */
{
    rn_mesh_pend = 0;
    if (!nm) return 0;
    char *b = (char *)base_name(nm);
    if (!pfx_i(b, "c_dp_rne.smf")) return 0;
    for (char *p = b + 7; (p[0] = p[1]) != 0; p++) continue;
    rn_mesh_pend = 1;
    return 1;
}
KTEXT void rn_mesh_fix(unsigned char *buf, int size)   /* after the whole-file read of that mesh: C_DP_RN_Body.* -> C_DP_RNE_Body.* */
{
    if (!rn_mesh_pend) return;
    rn_mesh_pend = 0;
    int c = 0;
    for (int i = 0; buf && i + 13 < size; i++) {
        if (!pfx_i((const char *)buf + i, "c_dp_rn_body.")) continue;
        int e = i; while (e < size && buf[e]) e++;
        if (e + 1 >= size || buf[e + 1] || e - i > 200) continue;   /* room for one more character in the name field */
        for (int j = e; j > i + 7; j--) buf[j] = buf[j - 1];
        buf[i + 7] = 'E'; c++;
    }
    ls_log("E.Regnier: his mesh read from C_DP_RN.SMFX, %d texture names renamed", c, 0, 0, 0);
}
KTEXT __attribute__((noinline)) void rn_blue(void)
{
    char *k = *(char **)0xA75BC4;
    if (!k || rn_blue_done) return;
    char *g6 = *(char **)(k + 0x44D8 + 6 * 4), *g = *(char **)(k + 0x44D8 + 123 * 4);
    if (!g6) return;
    if (!g) { char nm[40]; int i = 0; for (; i < 32 && g6[4 + i]; i++) nm[i] = g6[4 + i]; nm[i] = 0;
              /* USER 2026-11-25 (still orange; log: "blue fire not ready" every time): the name is relative ("reg_fire.spf") - the
                 game reads particle files with the current folder at data\particle (script player 0x5FFDA4..0x5FFF34 does the same) */
              typedef void (TC *fn_cd)(void *, const char *);
              ((fn_cd)0x880700)(k, "data"); ((fn_cd)0x880700)(k, "particle");
              int r = ((int (TC *)(void *, int, const char *))0x87D160)(k, 123, nm);
              ((fn_cd)0x880700)(k, ".."); ((fn_cd)0x880700)(k, "..");
              g = *(char **)(k + 0x44D8 + 123 * 4);
              { static int q __attribute__((section(".data"))); if (q++ < 3) ls_log("E.Regnier: loading blue fire copy of %s -> %d (%s)", (int)nm, r, (int)(g ? "ok" : "failed"), 0); } }
    if (!g) return;
    int n = *(i32 *)(g + 0x24); char *m = *(char **)(g + 0x30);
    if (n <= 0 || n > 8 || !m) return;                     /* not loaded yet (background loader) - next time */
    for (int i = 0; i < n; i++) {
        float *c = (float *)(m + i * 0x94 + 0x54), t;
        t = c[0]; c[0] = c[2]; c[2] = t; t = c[4]; c[4] = c[6]; c[6] = t;
    }
    rn_blue_done = 1;
    ls_log("E.Regnier: blue fire in particle slot 123 (%d members)", n, 0, 0, 0);
}
#endif
KTEXT void *stg_fopen(const char *path, const char *mode)
{
    { extern int fly_n, fls_n; fly_n = 0; fls_n = 0; }
#ifdef KUF_ENCA
    rn_blue(); rn_variant();
#endif
    u32 map = *(u32 *)0xAE1808;
    int net = coop_net();
    if (path && map == 26) plains_make();                  /* v1.2: make sure the flat map exists before its stage loads */
    if (path && strlen_(path) > 4 && ieq(path + strlen_(path) - 4, ".stg")) { lb_trim(ls_is_net()); afw_reset(); }
    if (path && ls_is_net() && strlen_(path) > 4 && ieq(path + strlen_(path) - 4, ".stg")) ls_seed();   /* v100c: same RNG state on every PC before the stage builds its troops */
    if (path && !net && map < 100 && ls_is_net()) return stg_colours(path, mode);
    if (!path || coop_sup > 4 || map < 100 || map >= 100 + COOP_N || !*(unsigned char *)0xAE1BFE) return C_FOPEN(path, mode);
    char want[24]; SPRINTF(want, "%s.stg", coopf[map - 100]);
    if (!ieq(base_name(path), want)) return C_FOPEN(path, mode);
    void *f = C_FOPEN(path, mode);
    if (!f) return f;
    if (!stg_buf) stg_buf = G_NEW(0x80000);
    if (!stg_buf) return f;
    u32 n = 0, r;
    while (n < 0x80000 && (r = C_FREAD(stg_buf + n, 1, 0x80000 - n, f)) > 0) n += r;
    C_FCLOSE(f);
    f = C_FOPEN(path, mode);
    if (n < 592 || n >= 0x80000 || *(i32 *)stg_buf != 0x3E9) return f;
    u32 cnt = *(u32 *)(stg_buf + 584);
    if (cnt > 1000 || 588 + cnt * 548 > n) return f;
    if (net) { coop_p2slot = -1; coop_army_lv = 0; coop_enm = 0; }   /* TEST40: coop_enm */
    if (net) { u32 h = 2166136261u; for (u32 q = 0; q < n; q++) { h ^= stg_buf[q]; h *= 16777619u; } coop_stg_crc = h; }   /* v98c: file check */
    char **to = *(char ***)(0xA75198 + 0x5a0);
    if (!to || !to[0] || !to[1]) return f;
    int lv = coop_lvtab[map - 100], done = 0;
    for (u32 i = 0; i < cnt; i++) {
        unsigned char *t = stg_buf + 588 + i * 548;
        int j = *(short *)(t + 0x1c4);
        if (t[0x24] != 0 || !is_sup(j)) continue;
        int nj = coop_sup ? sup_jobs[coop_sup][j >= 29 ? 1 : 0] : -1;
        if (nj < 0) continue;
        *(short *)(t + 0x1c4) = (short)nj;
        *(short *)(t + 0x194) = (short)*(i32 *)(0xA4A120 + nj * 12);
        *(short *)(t + 0x196) = (short)*(i32 *)(0xA4A120 + nj * 12 + 4);
        unsigned char *b = (unsigned char *)to[1] + nj * 0x14;
        *(i32 *)(t + 0x198) = b[0xD]; *(i32 *)(t + 0x19c) = b[0xE];
        *(i32 *)(t + 0x1c8) = *(i32 *)(to[0] + nj * 0x60 + 4);
        if (t[0x5b] < lv) t[0x5b] = (unsigned char)lv;
        i32 *sk = (i32 *)(t + 0x80);
        for (int k = 0; k < 8; k++) sk[k] = -1;
        for (int k = 0; k < 2; k++) if (sup_sk[nj][k * 2 + 1]) { sk[k * 2] = sup_sk[nj][k * 2]; sk[k * 2 + 1] = t[0x5b] * sup_sk[nj][k * 2 + 1] / 100; if (sk[k * 2 + 1] < 1) sk[k * 2 + 1] = 1; }
        done++;
    }
    if (!net) done += coop_looks(map, cnt);                 /* USER 2026-11-26: solo co-op too */
    if (net) {
        unsigned char *H = stg_buf + 8;                     /* header: +8 slot types[8], +0x10 team groups[8], +0x18 colours[8] */
        /* v100q (USER: Ellen's final mission too hard in co-op, scale her to ~30, the one before a little): host's army levels */
        /* v1.3 (USER test): Rupert 3/4 at least 25, Rupert 5 at least 35, Cirith 3 a little higher */
        { static const char p1f[][5] = { "0070", "0080", "1020", "1030", "1050", "6030", "6020" }; static const unsigned char p1l[] = { 25, 30, 25, 25, 35, 35, 30 };   /* v1.5 USER: Cirith 2 under-levelled */
          int mn = 0; for (int q = 0; q < 7; q++) if (ieq(coopf[map - 100], p1f[q])) mn = p1l[q];
          coop_p1min = mn;
          if (mn) for (u32 i = 0; i < cnt; i++) { unsigned char *t = stg_buf + 588 + i * 548; if (t[0x24] == 0 && t[0x5b] < mn) t[0x5b] = (unsigned char)mn; } }
        /* TEST16: the host's mission troops - Crusaders: level from the enemies (hero +2); all co-op: level-based upgrades */
        { int tl = coop_tgt_lv(map - 100), up = 0, rl = 0;
          for (u32 i = 0; i < cnt; i++) {
              unsigned char *t = stg_buf + 588 + i * 548;
              int j = *(short *)(t + 0x1c4);
              if (t[0x24] != 0 || j < 0 || j > 34 || t[0x5b] <= 1) continue;
              if (coop_rt[map - 100]) {
                  int nl = t[0x25] ? tl + 2 : tl, ol = t[0x5b];
                  if (nl > 99) nl = 99;
                  if (nl != ol) {
                      i32 *sk = (i32 *)(t + 0x80);
                      for (int k = 0; k < 4; k++) if (sk[k * 2] >= 0 && sk[k * 2 + 1] > 0) { int v = sk[k * 2 + 1] * nl / ol; if (v < 1) v = 1; if (v > 99) v = 99; sk[k * 2 + 1] = v; }
                      t[0x5b] = (unsigned char)nl; rl++;
                  }
              }
              if (!t[0x25]) { int u = up_lv(j, t[0x5b]); if (u != j) { stg_setjob(t, u); up++; } }   /* TEST53 (USER: Ellen's knights use the wrong model): the heroes' own troops keep their own job and look again (TEST46 undone) */
          }
          ls_log("co-op: host's mission troops - %d set to level %d, %d upgraded", rl, tl, up, 0); }
        coop_looks(map, cnt);   /* USER 2026-11-26: before P2's copies are made */
        int p2 = -1;
        /* v100q (USER go-ahead): missions with every slot holding troops. Cirith 4 (6040): enemy slot 7 (3 troops, team 1, never
           named by the script) merged into enemy slot 6 (team 1) -> slot 7 free. Morene 1 (5000): P2 takes allied AI slot 5
           (team of the host, never named by the script) together with its 5 troops. */
        if (ieq(coopf[map - 100], "6040") && H[7] == 3 && H[6] == 3 && H[8 + 7] == H[8 + 6]) {
            for (u32 i = 0; i < cnt; i++) { unsigned char *t = stg_buf + 588 + i * 548; if (t[0x24] == 7) t[0x24] = 6; }
            H[7] = 0;
        }
        if (ieq(coopf[map - 100], "5000") && H[5] == 3 && H[8 + 5] == H[8]) p2 = 5;
        /* v1.5 (USER: Rupert 6, Morene 2, Cirith 2 missing - all 8 slots used): free a slot that no script line names as a player
           by moving its troops to a slot of the same side. Rupert 6: enemy slot 7 holds only intro/cut-scene actors (disabled
           after their scenes) -> enemy slot 3 (named only by "all disabled", which disabled actors satisfy). Morene 2: slot 7 is the
           friendly cut-scene actors (Leinhart scene) -> the host's allied slot 1 (never named). Cirith 2: enemy slot 4 (5 troops,
           renewed out of sight) -> enemy slot 3 (named only as the observer of such renewals). */
        { int from = -1, to = -1;
          if (ieq(coopf[map - 100], "1060") && H[7] == 3 && H[3] == 3 && H[8 + 7] == H[8 + 3]) { from = 7; to = 3; }
          if (ieq(coopf[map - 100], "5020") && H[7] == 4 && H[1] == 3 && H[8 + 1] == H[8]) { from = 7; to = 1; }
          if (ieq(coopf[map - 100], "6020") && H[4] == 3 && H[3] == 3 && H[8 + 4] == H[8 + 3]) { from = 4; to = 3; }
          if (from > 0) {
              int mv = 0;
              for (u32 i = 0; i < cnt; i++) { unsigned char *t = stg_buf + 588 + i * 548; if (t[0x24] == from) { t[0x24] = (unsigned char)to; mv++; } }
              H[from] = 0;
              ls_log("co-op: stage slot %d (%d troops) merged into slot %d", from, mv, to, 0);
          } }
        /* v1.3 (USER: Cirith 4's orc sappers are useless, dark elf cavalry instead): the host's Orc Sapper troops -> Dark Elf Cavalry */
        if (ieq(coopf[map - 100], "6040")) for (u32 i = 0; i < cnt; i++) {
            unsigned char *t = stg_buf + 588 + i * 548;
            if (t[0x24] != 0 || *(short *)(t + 0x1c4) != 28) continue;
            int nj = 22;
            *(short *)(t + 0x1c4) = (short)nj;
            *(short *)(t + 0x194) = (short)*(i32 *)(0xA4A120 + nj * 12);
            *(short *)(t + 0x196) = (short)*(i32 *)(0xA4A120 + nj * 12 + 4);
            unsigned char *b = (unsigned char *)to[1] + nj * 0x14;
            *(i32 *)(t + 0x198) = b[0xD]; *(i32 *)(t + 0x19c) = b[0xE];
            *(i32 *)(t + 0x1c8) = *(i32 *)(to[0] + nj * 0x60 + 4);
            if (t[0x58]) t[0x58] = (unsigned char)*(i32 *)(0xA4A120 + nj * 12);
            i32 *sk = (i32 *)(t + 0x80); for (int k = 0; k < 8; k++) sk[k] = -1;
            ls_log("co-op: Cirith 4 sappers -> dark elf cavalry (placement %d)", (int)i, 0, 0, 0);
        }
        for (int k = 1; k < 8 && p2 < 0; k++) if (H[k] == 0) p2 = k;
        /* v100q (USER: Rupert 3/4/6, Walter 2, Morene 1/2/5, Cirith 2/3/4 missing): missions with all 8 slots in use - P2 takes
           an AI slot that has no troops at all (highest first) */
        for (int k = 7; k >= 1 && p2 < 0; k--) {
            if (H[k] != 3) continue;
            int used = 0;
            for (u32 i = 0; i < cnt && !used; i++) if (stg_buf[588 + i * 548 + 0x24] == k) used = 1;
            if (!used) p2 = k;
        }
        if (p2 > 0) {
            for (int o = 1; o < 8; o++) if (o != p2 && H[o] >= 3 && H[o] <= 5 && H[8 + o] != H[8]) coop_enm |= 1u << o;   /* TEST40 */
            /* enemies: +15% levels (at least +1; TEST26 USER: was +2) with two players (USER) */
            for (u32 i = 0; i < cnt; i++) {
                unsigned char *t = stg_buf + 588 + i * 548; int o = t[0x24];
                if (o < 8 && H[o] >= 3 && H[o] <= 5 && H[8 + o] != H[8]) {
                    int l = t[0x5b], l2 = l * 109 / 100;   /* TEST35 (USER: too difficult now): +9% (was +15%) */ if (l2 < l + 3) l2 = l + 3;   /* TEST33 (USER: officers now levelled - harder again): at least +3 */ if (l2 > 99) l2 = 99; t[0x5b] = (unsigned char)l2;
                }
            }
            /* TEST23 (USER): co-op levels. Hero = the enemies' average level (after the two-player raise above) + 2, player 2
               matches the host; Crusaders keep their own player levels (troops enemy median + 2, hero + 4). Every enemy troop
               is then at least the players' hero level + 2.
               TEST26 (USER 2026-11-21: levels felt high): + 1 instead of + 2 for the hero and for the enemies' minimum (Crusaders'
               own player levels unchanged), and the computer's ALLIED troops (same team as the host) are raised to the hero level. */
            {   int sum = 0, ne = 0, rt = coop_rt[map - 100], raised = 0, hr = 0, al = 0;
                /* TEST26 (USER: Gerald 5's Regnier is level 50+ on purpose - the player must run): the enemies' MEDIAN level, so one
                   very strong story troop does not lift the players' levels */
                unsigned char lvs[256];
                for (u32 i = 0; i < cnt; i++) {
                    unsigned char *t = stg_buf + 588 + i * 548; int o = t[0x24];
                    if (o < 8 && H[o] >= 3 && H[o] <= 5 && H[8 + o] != H[8] && t[0x5b] > 0 && t[0x5b] < 100 && ne < 256) { sum += t[0x5b]; lvs[ne++] = t[0x5b]; }
                }
                for (int a = 1; a < ne; a++) { unsigned char v = lvs[a]; int b = a - 1; while (b >= 0 && lvs[b] > v) { lvs[b + 1] = lvs[b]; b--; } lvs[b + 1] = v; }
                int E = ne ? lvs[ne / 2] : 1, HL = rt ? coop_tgt_lv(map - 100) + 2 : E + 1;
                if (HL > 97) HL = 97;
                /* TEST46 (USER, Swipez: Ellen 9 - player 2 died at once): where the host's army has a minimum level (Ellen 9: 30),
                   player 2 and the computer's allies get it too (P2 was at 24 next to a level 30 host); the enemies' floor stays
                   from their own levels */
                int HE = HL; if (!rt && coop_p1min > HL) HL = coop_p1min;
                if (!rt) {
                    for (u32 i = 0; i < cnt; i++) {
                        unsigned char *t = stg_buf + 588 + i * 548; int j = *(short *)(t + 0x1c4), ol = t[0x5b];
                        if (t[0x24] != 0 || j < 0 || j > 34 || ol < 1 || ol >= HL) continue;
                        i32 *sk = (i32 *)(t + 0x80);
                        for (int k = 0; k < 4; k++) if (sk[k * 2] >= 0 && sk[k * 2 + 1] > 0) { int v = sk[k * 2 + 1] * HL / ol; if (v > 99) v = 99; sk[k * 2 + 1] = v; }
                        t[0x5b] = (unsigned char)HL; hr++;
                    }
                    coop_army_lv = HL;
                }
                for (u32 i = 0; i < cnt; i++) {
                    unsigned char *t = stg_buf + 588 + i * 548; int o = t[0x24];
                    if (o < 8 && H[o] >= 3 && H[o] <= 5 && H[8 + o] != H[8] && t[0x5b] > 0 && t[0x5b] < HE + 3) { t[0x5b] = (unsigned char)(HE + 3 > 99 ? 99 : HE + 3); raised++; }   /* TEST33 (USER): hero + 3 (was + 1) */
                    else if (o > 0 && o < 8 && H[o] >= 3 && H[o] <= 5 && H[8 + o] == H[8] && t[0x5b] > 0 && t[0x5b] < HL) {   /* ally */
                        int ol = t[0x5b]; i32 *sk = (i32 *)(t + 0x80);
                        for (int k = 0; k < 4; k++) if (sk[k * 2] >= 0 && sk[k * 2 + 1] > 0) { int v = sk[k * 2 + 1] * HL / ol; if (v > 99) v = 99; sk[k * 2 + 1] = v; }
                        t[0x5b] = (unsigned char)HL; al++;
                    }
                }
                ls_log("co-op levels: enemies median %d, hero %d (%d host troops), enemies raised %d", E, HL, hr, raised); ls_log("co-op levels: %d allied troops raised to %d", al, HL, 0, 0);
                /* TEST33 (USER, Swipez: Ellen 2/3 officers die in 3-4 hits; Keaton: "once officer created, match troop level"): an officer's
                   level is its own byte in the stage record (+0xC4 + k*0x68 + 3, read at 0x55D541) and is 1 in every shipped mission, so
                   the raises above only reached the troop leaders. Every officer in the mission (enemies, computer allies and the
                   players' own troops' assist officers) now has its troop's leader level, skills scaled the same way. P2's troops get
                   theirs where they are filled (ls_army). Co-op only: Versus, Invasion and Hero Battle have no officers (USER). */
                {   int of = 0;
                    for (u32 i = 0; i < cnt; i++) {
                        unsigned char *t = stg_buf + 588 + i * 548; int o = t[0x24], L = t[0x5b], no = *(i32 *)(t + 0xc0);
                        if (o >= 8 || L < 2 || L > 99 || no < 1 || no > 2) continue;
                        for (int k = 0; k < no; k++) {
                            unsigned char *f = t + 0xc4 + k * 0x68; int ol = f[3];
                            if (f[0] == 0xff || ol >= L) continue;
                            if (ol < 1) ol = 1;
                            i32 *sk = (i32 *)(f + 0x28);
                            for (int q = 0; q < 4; q++) if (sk[q * 2] >= 0 && sk[q * 2 + 1] > 0) { int v = sk[q * 2 + 1] * L / ol; if (v > 99) v = 99; sk[q * 2 + 1] = v; }
                            f[3] = (unsigned char)L; of++;
                        }
                    }
                    ls_log("co-op levels: %d mission officers raised to their troop's level", of, 0, 0, 0);
                }
            }
            {   int hs = 0, ht = 0; add_sk = 1;   /* USER 2026-11-25: the host's mission troops get their job's skills (job_sk) */
                for (u32 i = 0; i < cnt; i++) { unsigned char *t = stg_buf + 588 + i * 548; int j = *(short *)(t + 0x1c4);
                    if (t[0x24] != 0 || j < 0 || j > 34 || t[0x5b] < 1) continue;
                    int c = job_sk(t, t[0x5b]); if (c) { hs += c; ht++; } }
                add_sk = 0;
                ls_log("co-op: host's mission troops - %d skills given or raised to their job's level (%d troops)", hs, ht, 0, 0); }
            /* P2's start placements: copies of the host's army placements (owner 0, a leader, present at start) plus up to two
               support placements (USER: up to 2 supports, e.g. in the finals), next to the host's; filled from P2's army by ls_army */
            int maxid = 0, add = 0, sup = 0;
            for (u32 i = 0; i < cnt; i++) { int id = *(i32 *)(stg_buf + 588 + i * 548 + 0x20); if (id > maxid) maxid = id; }
            unsigned char *end = stg_buf + 588 + cnt * 548, *tail = end;
            u32 taillen = n - (u32)(end - stg_buf);
            unsigned char nw[10 * 548]; int nn = 0;
            for (u32 i = 0; i < cnt && nn < 10; i++) {
                unsigned char *t = stg_buf + 588 + i * 548;
                if (t[0x24] != 0 || !t[0x26]) continue;
                int j = *(short *)(t + 0x1c4);
                /* v1.3 (USER: Morene 5 gave P2 only the hero): the host's troops without an officer (leader 0) count too - hero
                   + up to 4 troops, plus up to two supports */
                if (is_sup(j) || j == 17 || j == 34) { if (sup >= 2) continue; sup++; }
                else if (nn - sup >= 5) continue;
                unsigned char *d = nw + nn * 548;
                for (int q = 0; q < 548; q++) d[q] = t[q];
                d[0x24] = (unsigned char)p2; d[0x25] = nn == 0 ? 1 : 0; d[0x5a] = 0xFF;
                /* TEST27: the network load no longer clears mission leaders in co-op (Keaton's fix), so P2's copies drop the
                   host troops' leaders here - P2's troops are led as before (by their own army), no second story hero */
                if (nn) d[0x58] = 0;
                *(i32 *)(d + 0x20) = maxid + 1 + nn;
                *(float *)(d + 0x48) += 500.0f; *(float *)(d + 0x4c) += 500.0f;
                nn++;
            }
            if (nn && n + nn * 548 < 0x80000) {
                for (u32 q = taillen; q-- > 0;) tail[nn * 548 + q] = tail[q];          /* make room after the troop list */
                for (int q = 0; q < nn * 548; q++) end[q] = nw[q];
                *(u32 *)(stg_buf + 584) = cnt + nn; n += nn * 548; add = nn;
                H[p2] = 2; H[8 + p2] = H[8]; if (!H[0x10 + p2]) H[0x10 + p2] = H[0x10] == 2 ? 3 : 2;
                coop_p2slot = p2; done++;
            }
            ls_log("co-op stage: P2 slot %d, %d placements (%d support)", p2, add, sup, 0);
        }
    }
    if (!done) return f;
    void *g = temp_open(path, mode, stg_buf, n);
    if (!g) return f;
    C_FCLOSE(f);
    return g;
}

/* v95: co-op dialogue in sync. Act_CharSay / CharSayEx / OfficerSay / 0x548A00 wait for their message box, whose end depends on
   each PC's own voice playback -> the script would run differently on the two PCs. In network co-op the HOST decides: when a
   line is over on the host it puts marker (pid 0xFFFE, line number) into its next order bundle; every PC finishes that line
   on the turn the marker is executed (ls_can_step). Lines are numbered in the order they start (the same on both PCs). */
static void *say_act[32]; static i32 say_num[32];
int say_exec(char *act, int off, u32 orig)
{
    int (TC *f)(void *) = (int (TC *)(void *))orig;
    if (!ls_on || !ls_coop) return f(act);
    int k = -1, slot = -1;
    for (int i = 0; i < 32; i++) if (say_act[i] == act) { slot = i; break; }
    int fresh = *(i32 *)(act + off) == -1;
    if (slot < 0 || fresh) {
        if (slot < 0) { slot = say_started & 31; say_act[slot] = act; }
        say_num[slot] = say_started++;
    }
    k = say_num[slot];
    int r = f(act);
    if (k < say_done) return 0;                       /* the host's marker for this line has run */
    if (r == 0 && net_local_slot == 0 && say_sent <= k) {
        i32 v = k; ls_add(0xFFFE, (const unsigned char *)&v, 4); say_sent = k + 1;
    }
    return 1;
}

/* ---- v100p (USER: Invasion waves start immediately, no time to reposition): Act_Invasion_Cnt_End (0x54A840) starts the next
   wave. The maps' "count" (var Time = 4, counted down by a trigger with no delay) is over in a few frames. While the map is in its
   count phase (TimeCntSet == 1) the action now waits 20 s of battle frames (0xA62390 = lockstep turn on every PC). Mission end
   and skill set (TimeCntSet 0/2) are not delayed. Returning 1 = action still running (as Act_DelayTick). ---- */
static void *ivw_act; static u32 ivw_from; static int ivw_shown;
u32 inv_won_at;   /* v1.2: battle turn of the Invasion win (0 = not yet), reset by vic_check at each new battle */
/* ---- TEST36 Roguelike Invasion, phase 1 (USER + Iddy 2026-11-22): Invasion variant 4. Player armies start the battle at level 1
   (skills scaled the same way) and gain 2 levels at every wave start; every wave each player's whole army gets one random upgrade
   and the enemy gets one random trait. All of it follows from (game seed, wave, player) - every PC works out the same picks on
   the same turn, nothing extra is sent. Upgrades stack: a troop's stats = its stats when first seen x the product of every pick
   so far. Phase 2 = the 3-choice overlay; until then the pick is automatic and shown on the "Wave N in" sign + the netlog. ---- */
#define RG_ON (want_mode == 1 && (inv_var & 4))
enum { RG_ATK, RG_DEF, RG_RES, RG_HP, RG_SIEGE, RG_GLASS, RG_LV3, RG_TROOP, RG_NALL };
#define RG_HERO (RG_ON && inv_var == 5)
#define RG_NP (RG_HERO ? RG_NALL : RG_TROOP)      /* Reinforcements only in the hero start variant */
static const char *const rg_pname[RG_NALL] = { "Attack +10%", "Defence +8%", "Elemental resist +15%", "HP +12%", "Charge/blast resist +12%", "Glass cannon", "+3 levels", "Reinforcements: a new troop joins" };
enum { RT_IRON, RT_BERSERK, RT_WARD, RT_HARD, RT_BRUTAL, RT_NT };
static const char *const rg_tname[RT_NT] = { "Ironclad", "Berserk", "Warded", "Hardened", "Brutal" };
KTEXT __attribute__((noinline)) static u32 rg_rand(u32 w, u32 who)
{
    u32 r = net_seed ^ 0x52474C4Bu ^ (w * 0x9E3779B1u) ^ (who * 0x85EBCA77u);
    r ^= r >> 15; r *= 0x2C1B3C6Du; r ^= r >> 12; r *= 0x297A2D39u; r ^= r >> 15;
    return r;
}
/* TEST36 phase 2: each wave a player is offered 3 different upgrades (from the seed) and picks one in the overlay; the pick travels
   as an order (pid 0xFFFC: player, wave, choice) so every PC stores it on the same turn. No pick by the time the wave starts =
   a random one of the three (also from the seed). The wave start freezes every player's pick for that wave. */
static signed char (*rg_pk)[64]; static int rg_frozen; static char *rg_pkbm;   /* [8][64], on the game heap (no .bss room) */
KTEXT __attribute__((noinline)) static void rg_chk(void)
{
    char *bm = *(char **)0xA62334;
    if (!rg_pk) { rg_pk = (signed char (*)[64])G_NEW(8 * 64); rg_pkbm = (char *)1; }
    if (bm == rg_pkbm) return;
    rg_pkbm = bm; rg_frozen = 0;
    for (int p = 0; p < 8; p++) for (int w = 0; w < 64; w++) rg_pk[p][w] = -1;
}
static int rg_ntroop(int w, int pl);
KTEXT __attribute__((noinline)) static void rg_offers(int w, int pl, int *o)
{
    int idx[RG_NP]; u32 r = rg_rand((u32)w, (u32)pl + 1);
    for (int i = 0; i < RG_NP; i++) idx[i] = i;
    for (int i = RG_NP - 1; i > 0; i--) { r = r * 1103515245u + 12345u; int j = (int)((r >> 16) % (u32)(i + 1)), x = idx[i]; idx[i] = idx[j]; idx[j] = x; }
    o[0] = idx[0]; o[1] = idx[1]; o[2] = idx[2];
    if (RG_HERO && (w & 1) && w > 0 && rg_ntroop(w - 1, pl) < 3 && o[0] != RG_TROOP && o[1] != RG_TROOP && o[2] != RG_TROOP) o[0] = RG_TROOP;   /* odd waves: one card brings a troop while any wait */
}
KTEXT static int rg_defc(int w, int pl) { return (int)(rg_rand((u32)w, (u32)pl + 50) % 3); }
KTEXT __attribute__((noinline)) static int rg_pick(int w, int pl)
{
    int o[3]; rg_chk(); rg_offers(w, pl, o);
    int c = (rg_pk && pl >= 0 && pl < 8 && w >= 0 && w < 64) ? rg_pk[pl][w] : -1;
    if (c < 0 || c > 2) c = rg_defc(w, pl);
    return o[c];
}
void rg_recv(int from, const unsigned char *d)       /* net.c bundle apply: pid 0xFFFC */
{
    rg_chk();
    if (!rg_pk) return;
    int pl = d[0], w = d[1], c = d[2];
    if (pl >= 8 || w >= 64 || c > 2 || w <= rg_frozen || rg_pk[pl][w] >= 0) return;
    rg_pk[pl][w] = (signed char)c;
    int o[3]; rg_offers(w, pl, o);
    ls_log("roguelike: player %d (slot %d) picked %s for wave %d", pl, from, (int)rg_pname[o[c]], w);
}
KTEXT static int rg_trait(int w) { return (int)(rg_rand((u32)w, 99) % RT_NT); }
/* multipliers after waves 1..w: m[0] attack, m[1] melee/missile taken, m[2] blast/charge taken, m[3] elements taken, m[4] HP */
KTEXT __attribute__((noinline)) static void rg_mult(int w, int pl, float *m)
{
    m[0] = m[1] = m[2] = m[3] = m[4] = 1.0f;
    for (int i = 1; i <= w && i <= 60; i++) {
        if (pl >= 0) switch (rg_pick(i, pl)) {
            case RG_ATK: m[0] *= 1.10f; break;
            case RG_DEF: m[1] *= 0.92f; break;
            case RG_RES: m[3] *= 0.85f; break;
            case RG_HP: m[4] *= 1.12f; break;
            case RG_SIEGE: m[2] *= 0.88f; break;
            case RG_LV3: case RG_TROOP: break;                  /* levels: rg_bonuslv; troops: rg_wave_start */
            default: m[0] *= 1.25f; m[1] *= 1.12f; m[2] *= 1.12f; m[3] *= 1.12f; break;   /* glass cannon */
        } else switch (rg_trait(i)) {
            case RT_IRON: m[1] *= 0.88f; break;
            case RT_BERSERK: m[0] *= 1.12f; break;
            case RT_WARD: m[3] *= 0.85f; break;
            case RT_HARD: m[2] *= 0.85f; break;
            default: m[0] *= 1.06f; m[1] *= 0.94f; break;   /* brutal */
        }
    }
    for (int k = 1; k <= 3; k++) if (m[k] < 0.25f) m[k] = 0.25f;   /* never more than 75% resistance (X13: past 100% breaks immunity) */
    if (m[0] > 6.0f) m[0] = 6.0f;
}
KTEXT __attribute__((noinline)) static int rg_ntroop(int w, int pl)   /* Reinforcements picked in waves 1..w (max 3) */
{
    int n = 0;
    for (int i = 1; i <= w && i <= 60 && n < 3; i++) if (rg_pick(i, pl) == RG_TROOP) n++;
    return n;
}
/* TEST38 hero start: a held-back troop - not on the field at the start (+0x26 = 0, enabled later like ACT_TROOP_ENABLE) and a random kind */
KTEXT __attribute__((noinline)) void rg_reserve(unsigned char *P, int r)
{
    P[0x26] = 0;
    stg_setjob(P, inv_pool[r % (int)sizeof inv_pool]);
}
KTEXT __attribute__((noinline)) static int rg_bonuslv(int w, int pl)   /* USER: a "+3 levels" card (in case the wave levels have to go) */
{
    int n = 0;
    for (int i = 1; i <= w && i <= 60; i++) if (rg_pick(i, pl) == RG_LV3) n += 3;
    return n;
}
/* troop stats when first seen: +0x18B8 melee attack, +0x18BC missile attack, +0x18C8..+0x18EC ten damage-taken factors */
#define RG_MAX 256
typedef struct { void *t; float a[2], d[10]; } rg_base;
static rg_base *rg_b; static int rg_n, rg_wave; static char *rg_bm;
KTEXT __attribute__((noinline)) static void rg_apply(char *t, const float *m)
{
    int k = -1;
    for (int i = 0; i < rg_n; i++) if (rg_b[i].t == t) { k = i; break; }
    if (k < 0) {
        if (rg_n >= RG_MAX) return;
        k = rg_n++; rg_b[k].t = t;
        rg_b[k].a[0] = *(float *)(t + 0x18B8); rg_b[k].a[1] = *(float *)(t + 0x18BC);
        for (int q = 0; q < 10; q++) rg_b[k].d[q] = *(float *)(t + 0x18C8 + q * 4);
    }
    *(float *)(t + 0x18B8) = rg_b[k].a[0] * m[0]; *(float *)(t + 0x18BC) = rg_b[k].a[1] * m[0];
    *(float *)(t + 0x18C8) = rg_b[k].d[0] * m[1]; *(float *)(t + 0x18CC) = rg_b[k].d[1] * m[1];   /* melee, missile */
    *(float *)(t + 0x18D0) = rg_b[k].d[2] * m[2]; *(float *)(t + 0x18D4) = rg_b[k].d[3] * m[2];   /* explosion, charge */
    for (int q = 4; q < 10; q++) *(float *)(t + 0x18C8 + q * 4) = rg_b[k].d[q] * m[3];             /* fire .. curse */
}
KTEXT static int rg_wavenum(void)
{
    void *sm = *(void **)0xA73540;
    return sm ? ((int (TC *)(void *, const char *))0x53C0E0)(sm, "WaveNum") : 0;
}
KTEXT __attribute__((noinline)) static int rg_owner(char *t)     /* index of the troop's player in the battle's player list */
{
    char *bm = *(char **)0xA62334;
    if (!bm) return 0;
    int np = ((int (TC *)(void *))0x466B10)(bm);
    for (int p = 0; p < np && p < 8; p++) {
        void *pl = ((void *(TC *)(void *, int))0x466AD0)(bm, p);
        int nt = pl ? ((int (TC *)(void *))0x4DE970)(pl) : 0;
        for (int k = 0; k < nt; k++) if (((char *(TC *)(void *, int))0x4DE990)(pl, k) == t) return p;
    }
    return 0;
}
/* once per wave start (first player troop levelled by the wave action): every troop on the map gets its side's stacked multipliers */
KTEXT __attribute__((noinline)) static void rg_wave_start(void)
{
    char *bm = *(char **)0xA62334;
    int w = rg_wavenum();
    if (!bm || w < 1) return;
    if (bm != rg_bm) { rg_bm = bm; rg_n = 0; rg_wave = 0; }
    if (w == rg_wave) return;
    rg_wave = w;
    rg_chk(); if (rg_pk && w < 64) for (int q = 0; q < 8; q++) if (rg_pk[q][w] < 0) rg_pk[q][w] = (signed char)rg_defc(w, q);
    if (w > rg_frozen) rg_frozen = w;
    if (!rg_b) rg_b = (rg_base *)G_NEW(sizeof(rg_base) * RG_MAX);
    if (!rg_b) return;
    float em[5]; rg_mult(w, -1, em);
    int np = ((int (TC *)(void *))0x466B10)(bm), ne = 0, nh = 0;
    for (int p = 0; p < np && p < 8; p++) {
        void *pl = ((void *(TC *)(void *, int))0x466AD0)(bm, p);
        if (!pl) continue;
        int nt = ((int (TC *)(void *))0x4DE970)(pl), human = -1;
        for (int k = 0; k < nt; k++) {
            char *t = ((char *(TC *)(void *, int))0x4DE990)(pl, k);
            if (!t) continue;
            int id = ((int (TC *)(void *))0x4235F0)(t);
            if (human < 0) human = (((unsigned char (TC *)(void *, int))0x6AD270)((void *)0xA75198, id) & 0xFF) ? 1 : 0;
            if (human) { float m[5]; rg_mult(w, p, m); rg_apply(t, m); nh++; }
            else { rg_apply(t, em); ne++; }
        }
        if (human == 1 && RG_HERO) {                       /* TEST38: bring in as many held-back troops as Reinforcements were picked */
            int want = rg_ntroop(w, p), have = 0;
            for (int k = 0; k < nt; k++) {
                char *t = ((char *(TC *)(void *, int))0x4DE990)(pl, k);
                if (!t || k == 0) continue;                  /* the hero troop is the player's first */
                if (have < want) { if (!t[0x1932]) { ((void (TC *)(void *, int))0x55DA70)(t, 1); ls_log("roguelike wave %d: player %d reinforcement %d arrives", w, p, have + 1, 0); } have++; }
            }
        }
        if (human == 1) ls_log("roguelike wave %d: player %d gets %s", w, p, (int)rg_pname[rg_pick(w, p)], 0);
    }
    ls_log("roguelike wave %d: enemy trait %s (%d enemy / %d player troops updated)", w, (int)rg_tname[rg_trait(w)], ne, nh);
}
static char ivw_txt[200] = "Next wave in";
static int key_hit(int vk);
int ls_add(int pid, const unsigned char *data, int len);
static int rg_uiw = -1, rg_sel = -1, rg_uisec = -1, rg_uilast = -2;
#define RG_TXT 0x5247        /* HUD entry with this "variable" shows its text only (asm rg_hudv) */
KTEXT __attribute__((noinline)) static void rg_ui(void *sm, int el, int win)
{
    int w = ((int (TC *)(void *, const char *))0x53C0E0)(sm, "WaveNum");
    int lp = *(i32 *)(0xA75198 + 0x704); if (lp < 0 || lp > 7) lp = 0;
    typedef short (__attribute__((stdcall)) *fn_aks2)(int);
    fn_aks2 aks = *(fn_aks2 *)0x988404;                        /* GetAsyncKeyState: bit 0 = pressed since the last call */
    if (w != rg_uiw) { rg_uiw = w; rg_sel = -1; rg_uisec = -1; rg_uilast = -2; for (int k = 0; k < 3; k++) { aks('1' + k); aks(0x61 + k); } }
    int open = el < win - 30 - net_delay;          /* last second: the pick must reach every PC before the wave starts */
    if (open && rg_sel < 0 && w >= 0 && w < 64)
        for (int k = 0; k < 3; k++) if (((aks('1' + k) | aks(0x61 + k)) & 0x8001) != 0) {   /* the wave action only runs now and then: catch presses in between */
            unsigned char m[4] = { (unsigned char)lp, (unsigned char)w, (unsigned char)k, 0 };
            if (ls_add(0xFFFC, m, 4)) rg_sel = k;
            break;
        }
    int sec = (win - el + 29) / 30, st = rg_sel * 2 + open;
    if (sec == rg_uisec && st == rg_uilast) return;
    rg_uisec = sec; rg_uilast = st;
    char *hud = ((char *(TC *)(void *))0x54E800)(((void *(*)(void))0x426310)());
    if (!hud) return;
    void (TC *add)(void *, int, int, int, char *) = (void (TC *)(void *, int, int, int, char *))0x4531D0;
    ((void (TC *)(void *))0x54E750)(hud);
    int o[3]; rg_offers(w, lp, o);
    /* the HUD shows ONE variable box (the seconds) with its text under it, at a fixed place - so one entry, several lines (LF), smaller text */
    char *b = ivw_txt; int L;
    L = SPRINTF(b, "@(scale=0.7,0.7)Wave %d - enemy: %s\n", w, rg_tname[rg_trait(w)]);
    if (rg_sel >= 0) L += SPRINTF(b + L, "Chosen: %s", rg_pname[o[rg_sel]]);
    else {
        L += SPRINTF(b + L, open ? "Press 1, 2 or 3 to upgrade your army:" : "Random pick coming up:");
        for (int k = 0; k < 3; k++) L += SPRINTF(b + L, "\n%d  %s", k + 1, rg_pname[o[k]]);
    }
    char rg_hb[240]; int q = 0;                         /* the HUD runs the text through sprintf: double every % */
    for (int i = 0; ivw_txt[i] && q < 236; i++) { rg_hb[q++] = ivw_txt[i]; if (ivw_txt[i] == '%') rg_hb[q++] = '%'; }
    rg_hb[q] = 0;
    add(hud, 3, 228, 80, rg_hb);
}
static void ivw_setvar(void *sm, int id, int v)
{
    i32 *var = ((i32 *(TC *)(void *, int))0x53B4D0)(sm, id);
    if (var && *var == 0) ((void (TC *)(void *, int))0x517640)(var, v);
}
int TC iv_wait(void *act)
{
    u32 fc = *(u32 *)0xA62390;
    void *sm = *(void **)0xA73540;
    int (TC *getv)(void *, const char *) = (int (TC *)(void *, const char *))0x53C0E0;
    int cnt = sm ? getv(sm, "TimeCntSet") : 0;
    if (cnt == 1) {
        if (ivw_act != act || fc < ivw_from || fc - ivw_from > 100000) { ivw_act = act; ivw_from = fc; ivw_shown = 0; }
        u32 el = fc - ivw_from;
        u32 win = RG_ON ? 450 : 300;   /* TEST36 (USER): Roguelike +5 s to choose */
        if (RG_ON && el < win && getv(sm, "WaveSign") == 0) { ivw_setvar(sm, 3, (int)((win - el + 29) / 30)); rg_ui(sm, (int)el, (int)win); ivw_shown = 1; return 1; }
        if (el < win) {   /* v100q: 10 s (USER) */
            /* on-screen countdown once the map's own "Next Wave N" sign is over (WaveSign back to 0): script var 3 "Time"
               (the map's own counter, unused now) holds the seconds left, shown by the Act_VarDisplay HUD (0x4531D0).
               v100q: at the bottom centre above the SP bar (y 392 of 480) - at the top it covered the wave number (USER) */
            if (getv(sm, "WaveSign") == 0) {
                ivw_setvar(sm, 3, (int)((300 - el + 29) / 30));
                if (!ivw_shown) {
                    /* v100q (USER: timer and wave number still overlap - the position is not taken): take the map's own
                       "Next Wave N" sign down first and show one sign, "Wave N in <seconds>" */
                    char *hud = ((char *(TC *)(void *))0x54E800)(((void *(*)(void))0x426310)());
                    if (hud) {
                        ((void (TC *)(void *))0x54E750)(hud);
                        if (RG_ON) { int nw = getv(sm, "WaveNum"); int lp = *(i32 *)(0xA75198 + 0x704); if (lp < 0 || lp > 7) lp = 0;   /* TEST36: next wave's picks */
                            SPRINTF(ivw_txt, "Wave %d - foes %s, you %s - in", nw, rg_tname[rg_trait(nw)], rg_pname[rg_pick(nw, lp)]); }
                        else SPRINTF(ivw_txt, "Wave %d in", getv(sm, "WaveNum"));
                        ((void (TC *)(void *, int, int, int, char *))0x4531D0)(hud, 3, 228, 80, ivw_txt);
                    }
                    ivw_shown = 1;
                }
            }
            return 1;
        }
        if (ivw_shown) {
            ivw_setvar(sm, 3, 0);
            char *hud = ((char *(TC *)(void *))0x54E800)(((void *(*)(void))0x426310)());
            if (hud) ((void (TC *)(void *))0x54E750)(hud);
            ivw_shown = 0;
        }
    }
    ivw_act = 0;
    /* v1.2 (USER: Ruin completed to wave 30, "congratulations" shown, but no end / EXP screen): the maps' last trigger
       ("미션종료", WaveNum == 30 and Time == 0) shows text 9228 and runs this action once more. On the Xbox the wave counter
       then reached 31 and the Live end check (Xbox FUN_0021cd50) sent N_CMD_MISSION_COMPLETE - PC has no such check.
       Note it here (outside the count phase, wave 30+); vic_check sends the win ~5 s later. */
    if (sm && cnt == 0 && getv(sm, "WaveNum") >= 30 && !inv_won_at) { inv_won_at = fc ? fc : 1; ls_log("invasion: final wave cleared at turn %d - mission complete", (int)fc, 0, 0, 0); }
    return ((int (TC *)(void *))0x54A840)(act);
}

/* ---- v97: Invasion levelling for custom armies (USER: "hybrid"). The Xbox Invasion action 0x54A840 (IceStorm script) levels each
   player's troops to  P<n>exp / divisor + base  from InvasionHeroTable, which only knows the 11 Xbox heroes and 3 troops each.
   For any other hero or troop (v81 skipped them) a troop now gains the same number of levels relative to the level it started
   the battle with: start + P<n>exp / 15. Same steps as the action: leader level, troop HP, troop level, refresh, notify. ---- */
static void *iv_tp[64]; static int iv_start[64], iv_n; static char *iv_bm; static u32 iv_fc;
void iv_troop(char *t, int pexp, int cur)
{
    char *bm = *(char **)0xA62334;
    u32 fc = *(u32 *)0xA62390;
    if (!t || !bm) return;
    if (bm != iv_bm || fc < iv_fc) { iv_bm = bm; iv_n = 0; }
    iv_fc = fc;
    int k = -1;
    for (int i = 0; i < iv_n; i++) if (iv_tp[i] == t) { k = i; break; }
    if (k < 0) { if (iv_n >= 64) return; k = iv_n++; iv_tp[k] = t; iv_start[k] = cur; }
    /* v100: the team's best P<n>exp - an extra Invasion player (no P<n>exp in the map script) levels with the others */
    void *sm = *(void **)0xA73540;
    if (sm) for (int p = 1; p <= 6; p++) { char nm[12]; SPRINTF(nm, "P%dexp", p); int v = ((int (TC *)(void *, const char *))0x53C0E0)(sm, nm); if (v > pexp) pexp = v; }
    if (pexp < 0) pexp = 0;
    int L = iv_start[k] + pexp / 15;
    float hpm = 1.0f, rgm[5];
    if (RG_ON) {                                   /* TEST36: Roguelike - level 1 + 2 per wave, stacked HP upgrades */
        rg_wave_start();
        int w = rg_wavenum(), ow = rg_owner(t); L = 1 + 2 * (w > 1 ? w - 1 : 0) + rg_bonuslv(w, ow); if (L > 99) L = 99;
        rg_mult(w, ow, rgm); hpm = rgm[4];
    }
    if (L > 99) L = 99;
    if (L <= cur) return;
    void *plo = ((void *(TC *)(void *, int))0x597A80)(bm, ((int (TC *)(void *))0x4235F0)(t));
    void *ld = ((void *(TC *)(void *))0x55D940)(t);
    if (ld) ((void (TC *)(void *, int))0x4D4FD0)(ld, L);
    void *ti = ((void *(TC *)(void *))0x4A2940)((void *)0xA75198);
    int job = ((int (TC *)(void *))0x431F00)(t);
    int ns = ((int (TC *)(void *))0x488C30)(t);
    float hp = ((float (TC *)(void *, int, int, int, int))0x7A84D0)(ti, job, L, 1, 1) * (float)ns * hpm;
    ((void (TC *)(void *, float))0x54E880)(t, hp);
    ((void (TC *)(void *, int))0x56CB80)(t, L);
    ((void (TC *)(void *))0x56C9B0)(t);
    ((void (TC *)(void *, void *, int))0x56C850)(t, plo, L);
    if (RG_ON && rg_b) rg_apply(t, rgm);           /* TEST36: again after the level-up refresh, in case it rebuilt the stats */
}

/* ---- Battle Order prototype (USER idea 2026-11-20, build flag KUF_BO): the campaign briefing troop menu (menu 10,
   K2MenuBattle) opened over the MP army screen. Its roster = the selected army's troop vector, its slots = ours:
   hero fixed + 3 normal slots + 1 support slot (USER 2026-11-26). Opening/closing is done from the menu manager's result router
   (0x6F0CC0, outside the form loop); see notes/battle-order-briefing-menu.md. Menu-only, nothing in battle changes. */
volatile i32 bo_proto = 1;   /* USER 2026-11-20: tested OK (Iddy) -> on in every build from Test 25 */
volatile i32 bo_on __attribute__((section(".data")));     /* overlay requested/open: army screen input off */
volatile i32 bo_req __attribute__((section(".data")));    /* open on the next router tick */
volatile i32 bo_close __attribute__((section(".data")));  /* 1 cancel, 2 save: close on the next router tick */
volatile i32 bo_cool __attribute__((section(".data")));   /* router ticks the army screen still ignores keys after closing */
static i32 bo_handle __attribute__((section(".data"))); static i32 bo_wait __attribute__((section(".data")));
u32 *bo_rost __attribute__((section(".data")));             /* asm (menu 10 ctor): the roster passed instead of the campaign's */
static unsigned char bo_slots[5*8] __attribute__((section(".data")));
u32 bo_vec[3] __attribute__((section(".data")));
#define BO_SUP(ty) (((unsigned char (*)(int))0x5839F0)(ty) & 1)
KTEXT __attribute__((noinline)) static u32 *bo_army(void)
{
    char *pr = *(char **)0xA75ED0; i32 s = *(i32 *)0xAE1D7C;
    if (!pr || s < 0 || s > 2) return 0;
    return (u32 *)(pr + 0x1DC + s * 0x3C + 0x30);          /* {begin, end, cap} of troop pointers, [0] = hero */
}
KTEXT __attribute__((noinline)) static char *bo_troop(i32 rid)
{
    u32 *v = bo_army(); if (!v || rid < 0) return 0;
    for (char **p = (char **)v[0]; p < (char **)v[1]; p++) if (*p && *(i32 *)(*p + 0x20) == rid) return *p;
    return 0;
}
KTEXT __attribute__((noinline)) void bo_request(void)       /* asm: Battle Order pressed on the army screen */
{
    if (bo_on) return;
    bo_on = 1; bo_req = 1;
}
KTEXT __attribute__((noinline)) static void bo_open(void *mgr)
{
    u32 *v = bo_army(); i32 n = v ? (i32)((v[1] - v[0]) / 4) : 0;
    if (n < 2) { bo_on = 0; return; }
    char **t = (char **)v[0];
    for (i32 i = 0; i < n; i++) { *(i32 *)(t[i] + 0x20) = i; t[i][0x5C] = 1; }   /* roster ids 0.. (hero 0), all listed */
    for (i32 j = 0; j < 5; j++) {
        unsigned char *s = bo_slots + j * 8;
        *(short *)s = (short)j; *(short *)(s + 2) = j ? -1 : 0; *(short *)(s + 4) = -1; s[6] = j == 4; s[7] = j ? 1 : 0;   /* USER 2026-11-26: slot 4 = support (yellow) */
    }
    bo_rost = v; bo_wait = 0;
    bo_vec[0] = (u32)bo_slots; bo_vec[1] = bo_vec[2] = (u32)(bo_slots + 5 * 8);
    bo_handle = (i32)((void *(TC *)(void *, int, int, int, int, int, int, int, int))0x6EFDC0)(mgr, 10, 0, 0, 0, 0, 0, 0, 0);
    if (*(i32 *)((char *)mgr + 0x10) != 10) { bo_on = 0; return; }
    /* show today's order as calc_deploy takes it: the first 3 normal troops in slots 1-3, the first support in slot 4 */
    i32 k = 1, sup = 0;
    for (i32 i = 1; i < n; i++) {
        if (BO_SUP(*(i32 *)(t[i] + 0x44))) { if (!sup) { sup = 1; *(short *)(bo_slots + 4 * 8 + 2) = (short)i; } }
        else if (k < 4) { *(short *)(bo_slots + k * 8 + 2) = (short)i; k++; }
    }
}
/* USER 2026-11-26 ("reduce the normal slots to 4 (normal troop only) and make 1 yellow support unit only troop slot", hero +
   3 normal + 1 support): slots 1-3 take normal troops only, slot 4 a support troop only */
KTEXT __attribute__((noinline)) int bo_ok(unsigned char *slot, char *troop)   /* asm: may this troop go on this slot */
{
    return !BO_SUP(*(i32 *)(troop + 0x44)) == (slot != bo_slots + 4 * 8);
}
KTEXT __attribute__((noinline)) void bo_apply(void)          /* asm: "yes" on the save question */
{
    u32 *v = bo_army(); if (!v) { bo_close = 1; return; }
    char **t = (char **)v[0]; i32 n = (i32)((v[1] - v[0]) / 4), m = 1;
    char *nw[64]; if (n > 64 || n < 2) { bo_close = 1; return; }
    nw[0] = t[0];
    for (i32 j = 1; j < 5; j++) { char *o = bo_troop(*(short *)(bo_slots + j * 8 + 2)); if (o && o != t[0]) nw[m++] = o; }
    for (i32 i = 1; i < n; i++) {
        i32 used = 0; for (i32 q = 1; q < m; q++) if (nw[q] == t[i]) used = 1;
        if (!used && m < n) nw[m++] = t[i];
    }
    if (m != n) { bo_close = 1; return; }
    for (i32 i = 0; i < n; i++) { t[i] = nw[i]; *(i32 *)(t[i] + 0x20) = i; }
    bo_close = 2;
}
KTEXT __attribute__((noinline)) void bo_msgtxt(void *form, i32 msg)   /* asm: our wording for the menu's questions */
{
    if (msg == 2) {   /* B (USER: message 12 "Save" did nothing) -> message 2's OK / Cancel, worded Save / Don't Save */
        settext(((void *(TC *)(void *, int))0x819BA0)(form, 5), "Save");
        settext(((void *(TC *)(void *, int))0x819BA0)(form, 6), "Don't Save");
    }
    const char *s = msg == 2 ? "Save the new battle order?" : msg == 59 ? "Save this battle order?"
                  : msg == 61 ? "Some slots are empty. Keep going?" : msg == 60 ? "No other troop can go in this slot." : 0;
    if (!s) return;
    void *c = ((void *(TC *)(void *, int))0x819BA0)(form, 4);
    settext(c, s);
}
KTEXT __attribute__((noinline)) int bo_block(void)            /* asm: army screen ignores input while the overlay is up */
{
    if (!bo_on && bo_cool <= 0) return 0;
    if (bo_req && ++bo_wait > 90) { bo_on = bo_req = 0; bo_wait = 0; return 0; }   /* router never ran: give the screen back */
    return 1;
}
KTEXT __attribute__((noinline)) int bo_tick(void *mgr)       /* asm: menu manager router entry; 1 = closed, 2 = closed + saved */
{
    if (bo_cool > 0) bo_cool--;
    if (bo_req) { bo_req = 0; bo_open(mgr); return 0; }
    if (!bo_close) return 0;
    i32 r = bo_close; bo_close = 0;
    if (*(i32 *)((char *)mgr + 0x10) == 10 || *(i32 *)((char *)mgr + 0x14) == bo_handle)   /* top form is menu 10 */
        ((int (TC *)(void *, int))0x6F0870)(mgr, *(i32 *)((char *)mgr + 0x14));
    bo_on = 0; bo_cool = 15;
    return r == 2 ? 2 : 1;
}
