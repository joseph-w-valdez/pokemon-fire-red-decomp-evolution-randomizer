// Item catalogs for the RH debug Give menu.
// Included only from rh_debug_menu.c.

enum
{
    GIVE_CAT_BALLS,
    GIVE_CAT_MEDICINE,
    GIVE_CAT_BATTLE,
    GIVE_CAT_VITAMINS,
    GIVE_CAT_EVOLUTION,
    GIVE_CAT_HOLD,
    GIVE_CAT_BERRIES,
    GIVE_CAT_MAIL,
    GIVE_CAT_TREASURES,
    GIVE_CAT_TMS,
    GIVE_CAT_HMS,
    GIVE_CAT_KEY,
    GIVE_CAT_ROMHACK,
};

enum
{
    GIVE_MED_POTIONS,
    GIVE_MED_STATUS,
    GIVE_MED_PP,
    GIVE_MED_REVIVES,
};

enum
{
    GIVE_HOLD_BATTLE,
    GIVE_HOLD_TYPE,
    GIVE_HOLD_SPECIES,
    GIVE_HOLD_OTHER,
};

enum
{
    GIVE_BERRY_STATUS,
    GIVE_BERRY_HEAL,
    GIVE_BERRY_EV,
    GIVE_BERRY_GROWTH,
    GIVE_BERRY_PINCH,
};

enum
{
    GIVE_TM_01_25,
    GIVE_TM_26_50,
};

enum
{
    GIVE_KEY_STORY,
    GIVE_KEY_TOOLS,
    GIVE_KEY_PASSES,
    GIVE_KEY_OTHER,
};

static const u8 sText_GiveBalls[] = _("POKé BALLS");
static const u8 sText_GiveMedicine[] = _("MEDICINE");
static const u8 sText_GiveBattle[] = _("BATTLE / FIELD");
static const u8 sText_GiveVitamins[] = _("VITAMINS");
static const u8 sText_GiveEvolution[] = _("EVOLUTION");
static const u8 sText_GiveHold[] = _("HOLD ITEMS");
static const u8 sText_GiveBerries[] = _("BERRIES");
static const u8 sText_GiveMail[] = _("MAIL");
static const u8 sText_GiveTreasures[] = _("TREASURES");
static const u8 sText_GiveTMs[] = _("TMs");
static const u8 sText_GiveHMs[] = _("HMs");
static const u8 sText_GiveKey[] = _("KEY ITEMS");
static const u8 sText_GiveRomhack[] = _("ROMHACK");

static const u8 sText_MedPotions[] = _("POTIONS / DRINKS");
static const u8 sText_MedStatus[] = _("STATUS HEALS");
static const u8 sText_MedPP[] = _("PP RESTORE");
static const u8 sText_MedRevives[] = _("REVIVES");

static const u8 sText_HoldBattle[] = _("BATTLE");
static const u8 sText_HoldType[] = _("TYPE BOOSTERS");
static const u8 sText_HoldSpecies[] = _("SPECIES");
static const u8 sText_HoldOther[] = _("OTHER");

static const u8 sText_BerryStatus[] = _("STATUS");
static const u8 sText_BerryHeal[] = _("HEAL / PINCH HP");
static const u8 sText_BerryEV[] = _("EV BERRIES");
static const u8 sText_BerryGrowth[] = _("GROWTH / CATCH");
static const u8 sText_BerryPinch[] = _("PINCH STATS");

static const u8 sText_TM01_25[] = _("TM01 - TM25");
static const u8 sText_TM26_50[] = _("TM26 - TM50");

static const u8 sText_KeyStory[] = _("STORY");
static const u8 sText_KeyTools[] = _("TOOLS");
static const u8 sText_KeyPasses[] = _("PASSES / TICKETS");
static const u8 sText_KeyOther[] = _("OTHER / UNUSED");

static const u16 sGiveItems_Balls[] =
{
    ITEM_MASTER_BALL,
    ITEM_ULTRA_BALL,
    ITEM_GREAT_BALL,
    ITEM_POKE_BALL,
    ITEM_SAFARI_BALL,
    ITEM_NET_BALL,
    ITEM_DIVE_BALL,
    ITEM_NEST_BALL,
    ITEM_REPEAT_BALL,
    ITEM_TIMER_BALL,
    ITEM_LUXURY_BALL,
    ITEM_PREMIER_BALL,
};

static const u16 sGiveItems_MedPotions[] =
{
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_MAX_POTION,
    ITEM_FULL_RESTORE,
    ITEM_FRESH_WATER,
    ITEM_SODA_POP,
    ITEM_LEMONADE,
    ITEM_MOOMOO_MILK,
    ITEM_ENERGY_POWDER,
    ITEM_ENERGY_ROOT,
    ITEM_BERRY_JUICE,
    ITEM_LAVA_COOKIE,
};

static const u16 sGiveItems_MedStatus[] =
{
    ITEM_ANTIDOTE,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_AWAKENING,
    ITEM_PARALYZE_HEAL,
    ITEM_FULL_HEAL,
    ITEM_HEAL_POWDER,
    ITEM_BLUE_FLUTE,
    ITEM_YELLOW_FLUTE,
    ITEM_RED_FLUTE,
};

static const u16 sGiveItems_MedPP[] =
{
    ITEM_ETHER,
    ITEM_MAX_ETHER,
    ITEM_ELIXIR,
    ITEM_MAX_ELIXIR,
};

static const u16 sGiveItems_MedRevives[] =
{
    ITEM_REVIVE,
    ITEM_MAX_REVIVE,
    ITEM_REVIVAL_HERB,
    ITEM_SACRED_ASH,
};

static const u16 sGiveItems_Battle[] =
{
    ITEM_GUARD_SPEC,
    ITEM_DIRE_HIT,
    ITEM_X_ATTACK,
    ITEM_X_DEFEND,
    ITEM_X_SPEED,
    ITEM_X_ACCURACY,
    ITEM_X_SPECIAL,
    ITEM_POKE_DOLL,
    ITEM_FLUFFY_TAIL,
    ITEM_ESCAPE_ROPE,
    ITEM_REPEL,
    ITEM_SUPER_REPEL,
    ITEM_MAX_REPEL,
    ITEM_BLACK_FLUTE,
    ITEM_WHITE_FLUTE,
};

static const u16 sGiveItems_Vitamins[] =
{
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CARBOS,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_PP_UP,
    ITEM_PP_MAX,
    ITEM_RARE_CANDY,
};

static const u16 sGiveItems_Evolution[] =
{
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_FIRE_STONE,
    ITEM_THUNDER_STONE,
    ITEM_WATER_STONE,
    ITEM_LEAF_STONE,
    ITEM_EVERSTONE,
    ITEM_KINGS_ROCK,
    ITEM_METAL_COAT,
    ITEM_DRAGON_SCALE,
    ITEM_UP_GRADE,
    ITEM_DEEP_SEA_TOOTH,
    ITEM_DEEP_SEA_SCALE,
};

static const u16 sGiveItems_HoldBattle[] =
{
    ITEM_LEFTOVERS,
    ITEM_CHOICE_BAND,
    ITEM_FOCUS_BAND,
    ITEM_SCOPE_LENS,
    ITEM_QUICK_CLAW,
    ITEM_BRIGHT_POWDER,
    ITEM_WHITE_HERB,
    ITEM_MENTAL_HERB,
    ITEM_SHELL_BELL,
    ITEM_MACHO_BRACE,
};

static const u16 sGiveItems_HoldType[] =
{
    ITEM_SOFT_SAND,
    ITEM_HARD_STONE,
    ITEM_MIRACLE_SEED,
    ITEM_BLACK_GLASSES,
    ITEM_BLACK_BELT,
    ITEM_MAGNET,
    ITEM_MYSTIC_WATER,
    ITEM_SHARP_BEAK,
    ITEM_POISON_BARB,
    ITEM_NEVER_MELT_ICE,
    ITEM_SPELL_TAG,
    ITEM_TWISTED_SPOON,
    ITEM_CHARCOAL,
    ITEM_DRAGON_FANG,
    ITEM_SILK_SCARF,
    ITEM_SILVER_POWDER,
    ITEM_METAL_COAT,
};

static const u16 sGiveItems_HoldSpecies[] =
{
    ITEM_LIGHT_BALL,
    ITEM_LUCKY_PUNCH,
    ITEM_METAL_POWDER,
    ITEM_THICK_CLUB,
    ITEM_STICK,
    ITEM_SOUL_DEW,
};

static const u16 sGiveItems_HoldOther[] =
{
    ITEM_EXP_SHARE,
    ITEM_AMULET_COIN,
    ITEM_CLEANSE_TAG,
    ITEM_SMOKE_BALL,
    ITEM_SOOTHE_BELL,
    ITEM_LUCKY_EGG,
    ITEM_SEA_INCENSE,
    ITEM_LAX_INCENSE,
    ITEM_RED_SCARF,
    ITEM_BLUE_SCARF,
    ITEM_PINK_SCARF,
    ITEM_GREEN_SCARF,
    ITEM_YELLOW_SCARF,
};

static const u16 sGiveItems_BerryStatus[] =
{
    ITEM_CHERI_BERRY,
    ITEM_CHESTO_BERRY,
    ITEM_PECHA_BERRY,
    ITEM_RAWST_BERRY,
    ITEM_ASPEAR_BERRY,
    ITEM_PERSIM_BERRY,
    ITEM_LUM_BERRY,
};

static const u16 sGiveItems_BerryHeal[] =
{
    ITEM_ORAN_BERRY,
    ITEM_SITRUS_BERRY,
    ITEM_LEPPA_BERRY,
    ITEM_FIGY_BERRY,
    ITEM_WIKI_BERRY,
    ITEM_MAGO_BERRY,
    ITEM_AGUAV_BERRY,
    ITEM_IAPAPA_BERRY,
};

static const u16 sGiveItems_BerryEV[] =
{
    ITEM_POMEG_BERRY,
    ITEM_KELPSY_BERRY,
    ITEM_QUALOT_BERRY,
    ITEM_HONDEW_BERRY,
    ITEM_GREPA_BERRY,
    ITEM_TAMATO_BERRY,
};

static const u16 sGiveItems_BerryGrowth[] =
{
    ITEM_RAZZ_BERRY,
    ITEM_BLUK_BERRY,
    ITEM_NANAB_BERRY,
    ITEM_WEPEAR_BERRY,
    ITEM_PINAP_BERRY,
    ITEM_CORNN_BERRY,
    ITEM_MAGOST_BERRY,
    ITEM_RABUTA_BERRY,
    ITEM_NOMEL_BERRY,
    ITEM_SPELON_BERRY,
    ITEM_PAMTRE_BERRY,
    ITEM_WATMEL_BERRY,
    ITEM_DURIN_BERRY,
    ITEM_BELUE_BERRY,
};

static const u16 sGiveItems_BerryPinch[] =
{
    ITEM_LIECHI_BERRY,
    ITEM_GANLON_BERRY,
    ITEM_SALAC_BERRY,
    ITEM_PETAYA_BERRY,
    ITEM_APICOT_BERRY,
    ITEM_LANSAT_BERRY,
    ITEM_STARF_BERRY,
    ITEM_ENIGMA_BERRY,
};

static const u16 sGiveItems_Mail[] =
{
    ITEM_ORANGE_MAIL,
    ITEM_HARBOR_MAIL,
    ITEM_GLITTER_MAIL,
    ITEM_MECH_MAIL,
    ITEM_WOOD_MAIL,
    ITEM_WAVE_MAIL,
    ITEM_BEAD_MAIL,
    ITEM_SHADOW_MAIL,
    ITEM_TROPIC_MAIL,
    ITEM_DREAM_MAIL,
    ITEM_FAB_MAIL,
    ITEM_RETRO_MAIL,
};

static const u16 sGiveItems_Treasures[] =
{
    ITEM_TINY_MUSHROOM,
    ITEM_BIG_MUSHROOM,
    ITEM_PEARL,
    ITEM_BIG_PEARL,
    ITEM_STARDUST,
    ITEM_STAR_PIECE,
    ITEM_NUGGET,
    ITEM_HEART_SCALE,
    ITEM_SHOAL_SALT,
    ITEM_SHOAL_SHELL,
    ITEM_RED_SHARD,
    ITEM_BLUE_SHARD,
    ITEM_YELLOW_SHARD,
    ITEM_GREEN_SHARD,
    ITEM_OLD_AMBER,
    ITEM_HELIX_FOSSIL,
    ITEM_DOME_FOSSIL,
    ITEM_ROOT_FOSSIL,
    ITEM_CLAW_FOSSIL,
    ITEM_RUBY,
    ITEM_SAPPHIRE,
};

static const u16 sGiveItems_TM01_25[] =
{
    ITEM_TM01, ITEM_TM02, ITEM_TM03, ITEM_TM04, ITEM_TM05,
    ITEM_TM06, ITEM_TM07, ITEM_TM08, ITEM_TM09, ITEM_TM10,
    ITEM_TM11, ITEM_TM12, ITEM_TM13, ITEM_TM14, ITEM_TM15,
    ITEM_TM16, ITEM_TM17, ITEM_TM18, ITEM_TM19, ITEM_TM20,
    ITEM_TM21, ITEM_TM22, ITEM_TM23, ITEM_TM24, ITEM_TM25,
};

static const u16 sGiveItems_TM26_50[] =
{
    ITEM_TM26, ITEM_TM27, ITEM_TM28, ITEM_TM29, ITEM_TM30,
    ITEM_TM31, ITEM_TM32, ITEM_TM33, ITEM_TM34, ITEM_TM35,
    ITEM_TM36, ITEM_TM37, ITEM_TM38, ITEM_TM39, ITEM_TM40,
    ITEM_TM41, ITEM_TM42, ITEM_TM43, ITEM_TM44, ITEM_TM45,
    ITEM_TM46, ITEM_TM47, ITEM_TM48, ITEM_TM49, ITEM_TM50,
};

static const u16 sGiveItems_HMs[] =
{
    ITEM_HM01,
    ITEM_HM02,
    ITEM_HM03,
    ITEM_HM04,
    ITEM_HM05,
    ITEM_HM06,
    ITEM_HM07,
    ITEM_HM08,
};

static const u16 sGiveItems_KeyStory[] =
{
    ITEM_OAKS_PARCEL,
    ITEM_BIKE_VOUCHER,
    ITEM_BICYCLE,
    ITEM_POKE_FLUTE,
    ITEM_SILPH_SCOPE,
    ITEM_CARD_KEY,
    ITEM_LIFT_KEY,
    ITEM_GOLD_TEETH,
    ITEM_SECRET_KEY,
    ITEM_TEA,
};

static const u16 sGiveItems_KeyTools[] =
{
    ITEM_TOWN_MAP,
    ITEM_VS_SEEKER,
    ITEM_FAME_CHECKER,
    ITEM_ITEMFINDER,
    ITEM_COIN_CASE,
    ITEM_OLD_ROD,
    ITEM_GOOD_ROD,
    ITEM_SUPER_ROD,
    ITEM_TM_CASE,
    ITEM_BERRY_POUCH,
    ITEM_TEACHY_TV,
    ITEM_POWDER_JAR,
};

static const u16 sGiveItems_KeyPasses[] =
{
    ITEM_SS_TICKET,
    ITEM_TRI_PASS,
    ITEM_RAINBOW_PASS,
    ITEM_MYSTIC_TICKET,
    ITEM_AURORA_TICKET,
    ITEM_EON_TICKET,
};

static const u16 sGiveItems_KeyOther[] =
{
    ITEM_MACH_BIKE,
    ITEM_ACRO_BIKE,
    ITEM_CONTEST_PASS,
    ITEM_WAILMER_PAIL,
    ITEM_DEVON_GOODS,
    ITEM_SOOT_SACK,
    ITEM_BASEMENT_KEY,
    ITEM_POKEBLOCK_CASE,
    ITEM_LETTER,
    ITEM_RED_ORB,
    ITEM_BLUE_ORB,
    ITEM_SCANNER,
    ITEM_GO_GOGGLES,
    ITEM_METEORITE,
    ITEM_ROOM_1_KEY,
    ITEM_ROOM_2_KEY,
    ITEM_ROOM_4_KEY,
    ITEM_ROOM_6_KEY,
    ITEM_STORAGE_KEY,
    ITEM_DEVON_SCOPE,
};

static const u16 sGiveItems_Romhack[] =
{
    ITEM_WAYMO,
    ITEM_EDGELORD,
    ITEM_POOL_NOODLE,
    ITEM_ANCHOR_ARMS,
    ITEM_GEOCIDE,
    ITEM_FISH_LADDER,
    ITEM_TITAN,
    ITEM_RING_LIGHT,
    ITEM_ALL_EXP_SHARE,
    ITEM_SHINY_CHARM,
    ITEM_DEBUG_MENU,
};

static const struct ListMenuItem sGiveFolderItems[] =
{
    {sText_GiveBalls,     GIVE_CAT_BALLS},
    {sText_GiveMedicine,  GIVE_CAT_MEDICINE},
    {sText_GiveBattle,    GIVE_CAT_BATTLE},
    {sText_GiveVitamins,  GIVE_CAT_VITAMINS},
    {sText_GiveEvolution, GIVE_CAT_EVOLUTION},
    {sText_GiveHold,      GIVE_CAT_HOLD},
    {sText_GiveBerries,   GIVE_CAT_BERRIES},
    {sText_GiveMail,      GIVE_CAT_MAIL},
    {sText_GiveTreasures, GIVE_CAT_TREASURES},
    {sText_GiveTMs,       GIVE_CAT_TMS},
    {sText_GiveHMs,       GIVE_CAT_HMS},
    {sText_GiveKey,       GIVE_CAT_KEY},
    {sText_GiveRomhack,   GIVE_CAT_ROMHACK},
    {sText_Back,          LIST_CANCEL},
};

static const struct ListMenuItem sGiveMedSubItems[] =
{
    {sText_MedPotions, GIVE_MED_POTIONS},
    {sText_MedStatus,  GIVE_MED_STATUS},
    {sText_MedPP,      GIVE_MED_PP},
    {sText_MedRevives, GIVE_MED_REVIVES},
    {sText_Back,       LIST_CANCEL},
};

static const struct ListMenuItem sGiveHoldSubItems[] =
{
    {sText_HoldBattle,  GIVE_HOLD_BATTLE},
    {sText_HoldType,    GIVE_HOLD_TYPE},
    {sText_HoldSpecies, GIVE_HOLD_SPECIES},
    {sText_HoldOther,   GIVE_HOLD_OTHER},
    {sText_Back,        LIST_CANCEL},
};

static const struct ListMenuItem sGiveBerrySubItems[] =
{
    {sText_BerryStatus, GIVE_BERRY_STATUS},
    {sText_BerryHeal,   GIVE_BERRY_HEAL},
    {sText_BerryEV,     GIVE_BERRY_EV},
    {sText_BerryGrowth, GIVE_BERRY_GROWTH},
    {sText_BerryPinch,  GIVE_BERRY_PINCH},
    {sText_Back,        LIST_CANCEL},
};

static const struct ListMenuItem sGiveTMSubItems[] =
{
    {sText_TM01_25, GIVE_TM_01_25},
    {sText_TM26_50, GIVE_TM_26_50},
    {sText_Back,    LIST_CANCEL},
};

static const struct ListMenuItem sGiveKeySubItems[] =
{
    {sText_KeyStory,  GIVE_KEY_STORY},
    {sText_KeyTools,  GIVE_KEY_TOOLS},
    {sText_KeyPasses, GIVE_KEY_PASSES},
    {sText_KeyOther,  GIVE_KEY_OTHER},
    {sText_Back,      LIST_CANCEL},
};
