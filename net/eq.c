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
void enca_build(int set)
{
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof || set < 0 || set > 2) return;
    char *sp = prof + 0x1dc + set * 0x3c;
    typedef void (TC *fn_add)(void*, void*, int, int, const char*, int, int);
    fn_add add = (fn_add)0x71BFE0;
    *(short*)(sp + 0x20) = 39;
    *(i32*)(sp + 0x28) = 0x3E8; *(i32*)(sp + 0x2c) = 0x3E8;
    add(prof, sp, 39, 0, "Encablossa", 0, 0);
    {   /* v58: the hero record is the hero's own Leader troop - created as plain Infantry; make it an Encablossa troop */
        void *vec = sp + 0x30;
        int n = ((int (TC*)(void*))0x6B1D30)(vec);
#ifdef KUF_ENCA
        if (n >= 1) { char *t = *((char **(TC*)(void*,int))0x554050)(vec, n - 1); if (t) { enca_set_type(t, 61); *(i32*)(t + 0x28) = 39; *(i32*)(t + 0x2c) = 39; } }
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
int leader_learns(int c) { return human_leader(c) || spell_set(c) != 0; }
static int ll_arr[32];
static int *ll_vec[2];
void *learn_list_c(char *troop, int **list)
{
    if (!troop || *(int *)(troop + 0x4c) != 0) return list;
    int c = *(int *)(troop + 0x28);
    if (!leader_learns(c)) return list;
    int n = 0;
    /* list may be NULL: the game has no learn list for some Leader chars (Gerald's army) */
    if (list) for (int *p = list[0]; p && p < list[1] && n < 24; p++) ll_arr[n++] = *p;
    const signed char *add = human_leader(c) ? (const signed char *)"\x0b" : spell_set(c);
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
void coop_level(int set, int idx)
{
    char *prof = ((char *(*)(void))0x4372D0)();
    if (!prof || set < 0 || set > 2 || idx < 0 || idx >= COOP_N) return;
    int lv = coop_lvtab[idx];
    void *vec = prof + 0x1dc + set * 0x3c + 0x30;
    int n = ((int (TC*)(void*))0x6B1D30)(vec);
    for (int i = 0; i < n && i <= 8; i++) {
        char **pp = ((char **(TC*)(void*,int))0x554050)(vec, i);
        char *t = pp ? *pp : 0;
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
KTEXT void tex_fix(const char *name, unsigned char *buf, int size)
{
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
}
static int job_base(int j) { return (j >= JOB0 && j < JOB0 + NJOB) ? njobs[j - JOB0].base : j; }
static void jobs_troops(void);
void jobs_inject(void)
{
    jobs_troops();
    uv_setup();
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
static int ls_lobby_of(int st) { for (int s = 0; s < 6; s++) if (ls_stage(s) == st) return s; return -1; }
static const signed char sup_jobs[5][2] = { {-1,-1}, {12,31}, {9,32}, {13,30}, {14,29} };   /* human, dark legion */
static int is_sup(int j) { return j == 9 || j == 12 || j == 13 || j == 14 || j == 16 || (j >= 29 && j <= 32); }
static const signed char sup_sk[64][4] = {   /* skill id, weight % (2 skills) */
    [9] = {3,50,2,50}, [12] = {6,60,4,40}, [13] = {6,60,1,40}, [14] = {4,60,1,40},
    [29] = {7,70,0,30}, [30] = {7,70,0,30}, [31] = {7,60,8,40}, [32] = {7,60,3,40} };
static unsigned char *stg_buf;
u32 coop_stg_crc;   /* v98c: checksum of the co-op stage file as installed (host sends it with its army) */
int ls_is_net(void);
void ls_hdr_colours(unsigned char *M);
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
    { 0,  1, 0x2c,   26.0f,   23.0f },
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
        if (on && !mpb_set[i]) { mpb_orig[i] = *f; if (*f == m->from) *f = m->to; mpb_set[i] = 1; }
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
KTEXT void *stg_fopen(const char *path, const char *mode)
{
    u32 map = *(u32 *)0xAE1808;
    int net = coop_net();
    if (path && map == 26) plains_make();                  /* v1.2: make sure the flat map exists before its stage loads */
    if (path && strlen_(path) > 4 && ieq(path + strlen_(path) - 4, ".stg")) { lb_trim(ls_is_net()); afw_reset(); }
    if (path && ls_is_net() && strlen_(path) > 4 && ieq(path + strlen_(path) - 4, ".stg")) ls_seed();   /* v100c: same RNG state on every PC before the stage builds its troops */
    if (path && !net && map < 100 && ls_is_net()) return stg_colours(path, mode);
    if (!path || (!coop_sup && !net) || coop_sup > 4 || map < 100 || map >= 100 + COOP_N || !*(unsigned char *)0xAE1BFE) return C_FOPEN(path, mode);
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
    if (net) coop_p2slot = -1;
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
    if (net) {
        unsigned char *H = stg_buf + 8;                     /* header: +8 slot types[8], +0x10 team groups[8], +0x18 colours[8] */
        /* v100q (USER: Ellen's final mission too hard in co-op, scale her to ~30, the one before a little): host's army levels */
        /* v1.3 (USER test): Rupert 3/4 at least 25, Rupert 5 at least 35, Cirith 3 a little higher */
        { static const char p1f[][5] = { "0070", "0080", "1020", "1030", "1050", "6030" }; static const unsigned char p1l[] = { 25, 30, 25, 25, 35, 35 };
          int mn = 0; for (int q = 0; q < 6; q++) if (ieq(coopf[map - 100], p1f[q])) mn = p1l[q];
          if (mn) for (u32 i = 0; i < cnt; i++) { unsigned char *t = stg_buf + 588 + i * 548; if (t[0x24] == 0 && t[0x5b] < mn) t[0x5b] = (unsigned char)mn; } }
        int p2 = -1;
        /* v100q (USER go-ahead): missions with every slot holding troops. Cirith 4 (6040): enemy slot 7 (3 troops, team 1, never
           named by the script) merged into enemy slot 6 (team 1) -> slot 7 free. Morene 1 (5000): P2 takes allied AI slot 5
           (team of the host, never named by the script) together with its 5 troops. */
        if (ieq(coopf[map - 100], "6040") && H[7] == 3 && H[6] == 3 && H[8 + 7] == H[8 + 6]) {
            for (u32 i = 0; i < cnt; i++) { unsigned char *t = stg_buf + 588 + i * 548; if (t[0x24] == 7) t[0x24] = 6; }
            H[7] = 0;
        }
        if (ieq(coopf[map - 100], "5000") && H[5] == 3 && H[8 + 5] == H[8]) p2 = 5;
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
            /* enemies: +15% levels (at least +2) with two players (USER) */
            for (u32 i = 0; i < cnt; i++) {
                unsigned char *t = stg_buf + 588 + i * 548; int o = t[0x24];
                if (o < 8 && H[o] >= 3 && H[o] <= 5 && H[8 + o] != H[8]) {
                    int l = t[0x5b], l2 = l * 115 / 100; if (l2 < l + 2) l2 = l + 2; if (l2 > 99) l2 = 99; t[0x5b] = (unsigned char)l2;
                }
            }
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
static char ivw_txt[32] = "Next wave in";
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
        if (el < 300) {   /* v100q: 10 s (USER) */
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
                        SPRINTF(ivw_txt, "Wave %d in", getv(sm, "WaveNum"));
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
    if (L > 99) L = 99;
    if (L <= cur) return;
    void *plo = ((void *(TC *)(void *, int))0x597A80)(bm, ((int (TC *)(void *))0x4235F0)(t));
    void *ld = ((void *(TC *)(void *))0x55D940)(t);
    if (ld) ((void (TC *)(void *, int))0x4D4FD0)(ld, L);
    void *ti = ((void *(TC *)(void *))0x4A2940)((void *)0xA75198);
    int job = ((int (TC *)(void *))0x431F00)(t);
    int ns = ((int (TC *)(void *))0x488C30)(t);
    float hp = ((float (TC *)(void *, int, int, int, int))0x7A84D0)(ti, job, L, 1, 1) * (float)ns;
    ((void (TC *)(void *, float))0x54E880)(t, hp);
    ((void (TC *)(void *, int))0x56CB80)(t, L);
    ((void (TC *)(void *))0x56C9B0)(t);
    ((void (TC *)(void *, void *, int))0x56C850)(t, plo, L);
}
