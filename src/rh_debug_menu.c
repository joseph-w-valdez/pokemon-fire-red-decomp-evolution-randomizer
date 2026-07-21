#include "global.h"
#include "gflib.h"
#include "bg.h"
#include "field_fadetransition.h"
#include "gpu_regs.h"
#include "item.h"
#include "list_menu.h"
#include "menu.h"
#include "new_menu_helpers.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "task.h"
#include "text_window.h"
#include "rh_debug_menu.h"
#include "scrollbar.h"
#include "rh_log.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "event_data.h"
#include "money.h"

enum
{
    WIN_LIST,
    WIN_SCROLL,
    WIN_FOOTER,
};

enum
{
    PAGE_ROOT,
    PAGE_GIVE_FOLDERS,
    PAGE_GIVE_SUBFOLDERS,
    PAGE_GIVE_ITEMS,
    PAGE_GIVE_QTY,
    PAGE_WARP_FOLDERS,
    PAGE_WARP_ROUTE_GROUPS,
    PAGE_WARP_DEST,
    PAGE_CHEAT_FOLDERS,
    PAGE_CHEATS,
    PAGE_CHEAT_RESET_CONFIRM,
    PAGE_EVENT_FOLDERS,
    PAGE_EVENT_SUBFOLDERS,
    PAGE_EVENTS,
    PAGE_LOG,
};

enum
{
    DEBUG_MENU_LOG,
    DEBUG_MENU_GIVE,
    DEBUG_MENU_WARP,
    DEBUG_MENU_CHEATS,
    DEBUG_MENU_EVENTS,
    DEBUG_MENU_CLOSE = LIST_CANCEL,
};

enum
{
    CHEAT_GOD_MODE,
    CHEAT_ALWAYS_SHINY,
    CHEAT_PLAYER_ACCURACY,
    CHEAT_ENEMY_MISS,
    CHEAT_CATCH_RATE,
    CHEAT_CATCH_TRAINERS,
    CHEAT_WALK_WALLS,
    CHEAT_NO_ENCOUNTERS,
    CHEAT_INFINITE_PP,
    CHEAT_INSTANT_HATCH,
    CHEAT_MAX_IVS,
    CHEAT_FREE_MARTS,
    CHEAT_ALWAYS_OBEY,
    CHEAT_EXP_MULT,
};

enum
{
    CHEAT_FOLDER_PLAYER,
    CHEAT_FOLDER_ENEMY,
    CHEAT_FOLDER_CATCHING,
    CHEAT_FOLDER_MISC,
    CHEAT_ACTION_RESET_ALL = 0x80,
    CHEAT_ACTION_MAX_MONEY,
    CHEAT_ACTION_NAT_DEX,
};

enum
{
    WARP_FOLDER_TOWNS,
    WARP_FOLDER_ROUTES,
    WARP_FOLDER_SPECIAL,
    WARP_FOLDER_SEVII,
};

enum
{
    ROUTE_GROUP_1_10,
    ROUTE_GROUP_11_20,
    ROUTE_GROUP_21_PLUS,
};

enum
{
    STATE_FADE_IN,
    STATE_WAIT_FADE_IN,
    STATE_HANDLE_INPUT,
    STATE_WAIT_FADE_OUT,
    STATE_EXIT,
    STATE_EXIT_WARP,
};

#define tState       data[0]
#define tListTaskId  data[1]
#define tPage        data[2]
#define tGiveFolder  data[3]
#define tWarpDest    data[4]  // heal id, or index into sSpecialWarps
#define tWarpFolder  data[5]
#define tGiveSub     data[6]  // GIVE_SUB_NONE, or subfolder / cheat cursor
#define tCheatScroll data[7]

#define LIST_NONE 0xFF
#define GIVE_SUB_NONE 0xFF
#define WARP_LIST_MAX_SHOWED 5
#define GIVE_LIST_MAX_SHOWED 5
#define GIVE_MENU_ITEMS_MAX 56
#define CHEAT_BOOL_COUNT CHEAT_EXP_MULT
#define CHEAT_LIST_SHOW 3  // each cheat is 2 text lines; window fits ~3 rows
#define CHEAT_FALSE_X 120
#define CHEAT_TRUE_X 168
#define CHEAT_EXP_MULT_COUNT 5
#define CHEAT_LINE_H 14
#define EVENT_LIST_SHOW 2  // name + TRUE/FALSE; room left for desc line
#define EVENT_DESC_Y 62
#define LOG_LIST_SHOW 5
#define tCheatFolder tGiveFolder
#define tEventFolder tGiveFolder
#define tEventSub    tWarpFolder
#define tEventCursor tGiveSub
#define tEventScroll tCheatScroll
#define tLogScroll tCheatScroll

struct RhDebugMapWarp
{
    u16 map;
    s8 x;
    s8 y;
};

static EWRAM_DATA u16 sGiveItemId = ITEM_MASTER_BALL;
static EWRAM_DATA u8 sGiveQty = 1;
static EWRAM_DATA struct ListMenuItem sGiveMenuItems[GIVE_MENU_ITEMS_MAX];
static EWRAM_DATA u8 sScrollbarTaskId = SCROLLBAR_NONE;
static EWRAM_DATA u16 sScrollbarScroll = 0;

static const u8 sText_FooterRoot[] = _("Press {L_BUTTON} to close the menu");
static const u8 sText_FooterGiveFolders[] = _("{A_BUTTON} Open    {B_BUTTON} Back    {L_BUTTON} Close");
static const u8 sText_FooterGiveItems[] = _("{DPAD_UPDOWN} Scroll    {A_BUTTON} Select    {B_BUTTON} Back");
static const u8 sText_FooterGiveQty[] = _("{DPAD_UPDOWN} Qty    {A_BUTTON} Give    {B_BUTTON} Back");
static const u8 sText_FooterWarpFolders[] = _("{A_BUTTON} Open    {B_BUTTON} Back    {L_BUTTON} Close");
static const u8 sText_FooterWarpDest[] = _("{DPAD_UPDOWN} Scroll    {A_BUTTON} Warp    {B_BUTTON} Back");
static const u8 sText_FooterCheatFolders[] = _("{A_BUTTON} Open    {B_BUTTON} Back    {L_BUTTON} Close");
static const u8 sText_FooterCheats[] = _("{DPAD_UPDOWN} Select    {DPAD_LEFTRIGHT} Toggle    {B_BUTTON} Back");
static const u8 sText_FooterEventFolders[] = _("{A_BUTTON} Open    {B_BUTTON} Back    {L_BUTTON} Close");
static const u8 sText_FooterEvents[] = _("{DPAD_UPDOWN} Select    {DPAD_LEFTRIGHT} Toggle    {B_BUTTON} Back");
static const u8 sText_FooterLog[] = _("{DPAD_UPDOWN} Scroll    {A_BUTTON} Clear    {B_BUTTON} Back");
static const u8 sText_LogEventOnFmt[] = _("%S ON");
static const u8 sText_LogEventOffFmt[] = _("%S OFF");
static const u8 sText_LogEmpty[] = _("NO LOG ENTRIES");
static const u8 sText_LogOpened[] = _("opened debug menu");
static const u8 sText_LogGiveFail[] = _("give failed");
static const u8 sText_LogGaveFmt[] = _("gave %S x%d");
static const u8 sText_LogGiveFullFmt[] = _("bag full %S x%d");
static const u8 sText_LogWarpMapFmt[] = _("warp map=%d,%d xy=%d,%d");
static const u8 sText_LogWarpHealFmt[] = _("warp heal loc=%d");
static const u8 sText_LogCheatOnFmt[] = _("%S ON");
static const u8 sText_LogCheatOffFmt[] = _("%S OFF");
static const u8 sText_LogCheatsReset[] = _("reset all cheats");
static const u8 sText_LogExpMultFmt[] = _("EXP MULT %S");
static const u8 sText_LogMaxMoney[] = _("applied MAX MONEY");
static const u8 sText_LogNatDex[] = _("unlocked NAT DEX");
static const u8 sText_CheatResetAll[] = _("RESET ALL");
static const u8 sText_CheatResetPrompt[] = _("RESET ALL CHEATS?");
static const u8 sText_Yes[] = _("YES");
static const u8 sText_No[] = _("NO");
static const u8 sText_Log[] = _("LOG");
static const u8 sText_Give[] = _("GIVE");
static const u8 sText_Warp[] = _("WARP");
static const u8 sText_Cheats[] = _("CHEATS");
static const u8 sText_Events[] = _("EVENTS");
static const u8 sText_Close[] = _("CLOSE");
static const u8 sText_Back[] = _("BACK");
static const u8 sText_Times[] = _("x");
static const u8 sText_True[] = _("TRUE");
static const u8 sText_False[] = _("FALSE");
static const u8 sText_CheatGod[] = _("GOD MODE");
static const u8 sText_CheatShiny[] = _("100% SHINY");
static const u8 sText_CheatPlayerAcc[] = _("100% PLAYER ACC");
static const u8 sText_CheatEnemyMiss[] = _("0% ENEMY ACC");
static const u8 sText_CheatCatchRate[] = _("100% CATCH RATE");
static const u8 sText_CheatCatchTrainers[] = _("CATCH TRAINER POKEMON");
static const u8 sText_CheatWalkWalls[] = _("WALK THRU WALLS");
static const u8 sText_CheatNoEncounters[] = _("NO WILD ENCOUNTERS");
static const u8 sText_CheatInfinitePP[] = _("INFINITE PP");
static const u8 sText_CheatInstantHatch[] = _("INSTANT EGG HATCH");
static const u8 sText_CheatMaxIVs[] = _("MAX IVS");
static const u8 sText_CheatFreeMarts[] = _("FREE POKE MARTS");
static const u8 sText_CheatAlwaysObey[] = _("ALWAYS OBEY");
static const u8 sText_CheatMaxMoney[] = _("MAX MONEY");
static const u8 sText_CheatNatDex[] = _("UNLOCK NATIONAL DEX");
static const u8 sText_CheatActionHint[] = _("A: APPLY");
static const u8 sText_CheatExpMult[] = _("EXP MULT");
static const u8 sText_Exp1x[] = _("1x");
static const u8 sText_Exp2x[] = _("2x");
static const u8 sText_Exp5x[] = _("5x");
static const u8 sText_Exp10x[] = _("10x");
static const u8 sText_Exp15x[] = _("15x");
static const u8 sText_CheatFolderPlayer[] = _("PLAYER");
static const u8 sText_CheatFolderEnemy[] = _("ENEMY");
static const u8 sText_CheatFolderCatching[] = _("CATCHING");
static const u8 sText_CheatFolderMisc[] = _("MISC");
static const u8 sText_FolderTowns[] = _("TOWNS / CITIES");
static const u8 sText_FolderRoutes[] = _("ROUTES");
static const u8 sText_FolderSpecial[] = _("SPECIAL");
static const u8 sText_FolderSevii[] = _("SEVII ISLANDS");
static const u8 sText_RouteGroup1_10[] = _("ROUTES 1 - 10");
static const u8 sText_RouteGroup11_20[] = _("ROUTES 11 - 20");
static const u8 sText_RouteGroup21Plus[] = _("ROUTES 21+");
// Fly destinations (same heal spots as the Town Map fly list)
static const u8 sText_WarpPallet[] = _("PALLET TOWN");
static const u8 sText_WarpViridian[] = _("VIRIDIAN CITY");
static const u8 sText_WarpPewter[] = _("PEWTER CITY");
static const u8 sText_WarpCerulean[] = _("CERULEAN CITY");
static const u8 sText_WarpLavender[] = _("LAVENDER TOWN");
static const u8 sText_WarpVermilion[] = _("VERMILION CITY");
static const u8 sText_WarpCeladon[] = _("CELADON CITY");
static const u8 sText_WarpFuchsia[] = _("FUCHSIA CITY");
static const u8 sText_WarpCinnabar[] = _("CINNABAR ISLAND");
static const u8 sText_WarpIndigo[] = _("INDIGO PLATEAU");
static const u8 sText_WarpSaffron[] = _("SAFFRON CITY");
static const u8 sText_WarpRoute1[] = _("ROUTE 1");
static const u8 sText_WarpRoute2[] = _("ROUTE 2");
static const u8 sText_WarpRoute3[] = _("ROUTE 3");
static const u8 sText_WarpRoute4[] = _("ROUTE 4");
static const u8 sText_WarpRoute5[] = _("ROUTE 5");
static const u8 sText_WarpRoute6[] = _("ROUTE 6");
static const u8 sText_WarpRoute7[] = _("ROUTE 7");
static const u8 sText_WarpRoute8[] = _("ROUTE 8");
static const u8 sText_WarpRoute9[] = _("ROUTE 9");
static const u8 sText_WarpRoute10[] = _("ROUTE 10");
static const u8 sText_WarpRoute11[] = _("ROUTE 11");
static const u8 sText_WarpRoute12[] = _("ROUTE 12");
static const u8 sText_WarpRoute13[] = _("ROUTE 13");
static const u8 sText_WarpRoute14[] = _("ROUTE 14");
static const u8 sText_WarpRoute15[] = _("ROUTE 15");
static const u8 sText_WarpRoute16[] = _("ROUTE 16");
static const u8 sText_WarpRoute17[] = _("ROUTE 17");
static const u8 sText_WarpRoute18[] = _("ROUTE 18");
static const u8 sText_WarpRoute19[] = _("ROUTE 19");
static const u8 sText_WarpRoute20[] = _("ROUTE 20");
static const u8 sText_WarpRoute21N[] = _("ROUTE 21 NORTH");
static const u8 sText_WarpRoute21S[] = _("ROUTE 21 SOUTH");
static const u8 sText_WarpRoute22[] = _("ROUTE 22");
static const u8 sText_WarpRoute23[] = _("ROUTE 23");
static const u8 sText_WarpRoute24[] = _("ROUTE 24");
static const u8 sText_WarpRoute25[] = _("ROUTE 25");
static const u8 sText_WarpOneIsland[] = _("ONE ISLAND");
static const u8 sText_WarpTwoIsland[] = _("TWO ISLAND");
static const u8 sText_WarpThreeIsland[] = _("THREE ISLAND");
static const u8 sText_WarpFourIsland[] = _("FOUR ISLAND");
static const u8 sText_WarpFiveIsland[] = _("FIVE ISLAND");
static const u8 sText_WarpSixIsland[] = _("SIX ISLAND");
static const u8 sText_WarpSevenIsland[] = _("SEVEN ISLAND");
// Special / static encounters (stand one tile in front of the mon)
static const u8 sText_SpecialArticuno[] = _("ARTICUNO");
static const u8 sText_SpecialZapdos[] = _("ZAPDOS");
static const u8 sText_SpecialMoltres[] = _("MOLTRES");
static const u8 sText_SpecialMewtwo[] = _("MEWTWO");
static const u8 sText_SpecialDeoxys[] = _("DEOXYS");
static const u8 sText_SpecialLugia[] = _("LUGIA");
static const u8 sText_SpecialHoOh[] = _("HO-OH");
static const u8 sText_SpecialSnorlax12[] = _("SNORLAX (R12)");
static const u8 sText_SpecialSnorlax16[] = _("SNORLAX (R16)");
static const u8 sTextColors[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static const struct ListMenuItem sRootItems[] =
{
    {sText_Log,     DEBUG_MENU_LOG},
    {sText_Give,    DEBUG_MENU_GIVE},
    {sText_Warp,    DEBUG_MENU_WARP},
    {sText_Cheats,  DEBUG_MENU_CHEATS},
    {sText_Events,  DEBUG_MENU_EVENTS},
    {sText_Close,   DEBUG_MENU_CLOSE},
};

static const u16 sCheatFlags[CHEAT_BOOL_COUNT] =
{
    [CHEAT_GOD_MODE]         = FLAG_SYS_CHEAT_GOD_MODE,
    [CHEAT_ALWAYS_SHINY]     = FLAG_SYS_CHEAT_ALWAYS_SHINY,
    [CHEAT_PLAYER_ACCURACY]  = FLAG_SYS_CHEAT_PLAYER_ACCURACY,
    [CHEAT_ENEMY_MISS]       = FLAG_SYS_CHEAT_ENEMY_MISS,
    [CHEAT_CATCH_RATE]       = FLAG_SYS_CHEAT_CATCH_RATE,
    [CHEAT_CATCH_TRAINERS]   = FLAG_SYS_CHEAT_CATCH_TRAINERS,
    [CHEAT_WALK_WALLS]       = FLAG_SYS_CHEAT_WALK_WALLS,
    [CHEAT_NO_ENCOUNTERS]    = FLAG_SYS_CHEAT_NO_ENCOUNTERS,
    [CHEAT_INFINITE_PP]      = FLAG_SYS_CHEAT_INFINITE_PP,
    [CHEAT_INSTANT_HATCH]    = FLAG_SYS_CHEAT_INSTANT_HATCH,
    [CHEAT_MAX_IVS]          = FLAG_SYS_CHEAT_MAX_IVS,
    [CHEAT_FREE_MARTS]       = FLAG_SYS_CHEAT_FREE_MARTS,
    [CHEAT_ALWAYS_OBEY]      = FLAG_SYS_CHEAT_ALWAYS_OBEY,
};

static const u8 *const sCheatNames[CHEAT_BOOL_COUNT] =
{
    [CHEAT_GOD_MODE]        = sText_CheatGod,
    [CHEAT_ALWAYS_SHINY]    = sText_CheatShiny,
    [CHEAT_PLAYER_ACCURACY] = sText_CheatPlayerAcc,
    [CHEAT_ENEMY_MISS]      = sText_CheatEnemyMiss,
    [CHEAT_CATCH_RATE]      = sText_CheatCatchRate,
    [CHEAT_CATCH_TRAINERS]  = sText_CheatCatchTrainers,
    [CHEAT_WALK_WALLS]      = sText_CheatWalkWalls,
    [CHEAT_NO_ENCOUNTERS]   = sText_CheatNoEncounters,
    [CHEAT_INFINITE_PP]     = sText_CheatInfinitePP,
    [CHEAT_INSTANT_HATCH]   = sText_CheatInstantHatch,
    [CHEAT_MAX_IVS]         = sText_CheatMaxIVs,
    [CHEAT_FREE_MARTS]      = sText_CheatFreeMarts,
    [CHEAT_ALWAYS_OBEY]     = sText_CheatAlwaysObey,
};

static const u8 *const sExpMultLabels[CHEAT_EXP_MULT_COUNT] =
{
    sText_Exp1x,
    sText_Exp2x,
    sText_Exp5x,
    sText_Exp10x,
    sText_Exp15x,
};

static const u8 sCheatFolderPlayer[] =
{
    CHEAT_GOD_MODE,
    CHEAT_PLAYER_ACCURACY,
    CHEAT_INFINITE_PP,
    CHEAT_EXP_MULT,
};

static const u8 sCheatFolderEnemy[] =
{
    CHEAT_ENEMY_MISS,
};

static const u8 sCheatFolderCatching[] =
{
    CHEAT_CATCH_RATE,
    CHEAT_CATCH_TRAINERS,
    CHEAT_MAX_IVS,
};

static const u8 sCheatFolderMisc[] =
{
    CHEAT_ALWAYS_SHINY,
    CHEAT_WALK_WALLS,
    CHEAT_NO_ENCOUNTERS,
    CHEAT_INSTANT_HATCH,
    CHEAT_FREE_MARTS,
    CHEAT_ALWAYS_OBEY,
    CHEAT_ACTION_MAX_MONEY,
    CHEAT_ACTION_NAT_DEX,
};

static const struct ListMenuItem sCheatFolderItems[] =
{
    {sText_CheatFolderPlayer,   CHEAT_FOLDER_PLAYER},
    {sText_CheatFolderEnemy,    CHEAT_FOLDER_ENEMY},
    {sText_CheatFolderCatching, CHEAT_FOLDER_CATCHING},
    {sText_CheatFolderMisc,     CHEAT_FOLDER_MISC},
    {sText_CheatResetAll,       CHEAT_ACTION_RESET_ALL},
    {sText_Back,                LIST_CANCEL},
};

static const struct ListMenuItem sCheatResetConfirmItems[] =
{
    {sText_Yes, 1},
    {sText_No,  LIST_CANCEL},
};

static const struct ListMenuItem sWarpFolderItems[] =
{
    {sText_FolderTowns,   WARP_FOLDER_TOWNS},
    {sText_FolderRoutes,  WARP_FOLDER_ROUTES},
    {sText_FolderSpecial, WARP_FOLDER_SPECIAL},
    {sText_FolderSevii,   WARP_FOLDER_SEVII},
    {sText_Back,          LIST_CANCEL},
};

// Dest list indexes are HEAL_LOCATION_*; BACK cancels like B.
static const struct ListMenuItem sWarpTownItems[] =
{
    {sText_WarpPallet,    HEAL_LOCATION_PALLET_TOWN},
    {sText_WarpViridian,  HEAL_LOCATION_VIRIDIAN_CITY},
    {sText_WarpPewter,    HEAL_LOCATION_PEWTER_CITY},
    {sText_WarpCerulean,  HEAL_LOCATION_CERULEAN_CITY},
    {sText_WarpLavender,  HEAL_LOCATION_LAVENDER_TOWN},
    {sText_WarpVermilion, HEAL_LOCATION_VERMILION_CITY},
    {sText_WarpCeladon,   HEAL_LOCATION_CELADON_CITY},
    {sText_WarpFuchsia,   HEAL_LOCATION_FUCHSIA_CITY},
    {sText_WarpSaffron,   HEAL_LOCATION_SAFFRON_CITY},
    {sText_WarpCinnabar,  HEAL_LOCATION_CINNABAR_ISLAND},
    {sText_WarpIndigo,    HEAL_LOCATION_INDIGO_PLATEAU},
    {sText_Back,          LIST_CANCEL},
};

// Indexes into sRouteWarps[]; BACK cancels like B.
enum
{
    ROUTE_WARP_1,
    ROUTE_WARP_2,
    ROUTE_WARP_3,
    ROUTE_WARP_4,
    ROUTE_WARP_5,
    ROUTE_WARP_6,
    ROUTE_WARP_7,
    ROUTE_WARP_8,
    ROUTE_WARP_9,
    ROUTE_WARP_10,
    ROUTE_WARP_11,
    ROUTE_WARP_12,
    ROUTE_WARP_13,
    ROUTE_WARP_14,
    ROUTE_WARP_15,
    ROUTE_WARP_16,
    ROUTE_WARP_17,
    ROUTE_WARP_18,
    ROUTE_WARP_19,
    ROUTE_WARP_20,
    ROUTE_WARP_21_NORTH,
    ROUTE_WARP_21_SOUTH,
    ROUTE_WARP_22,
    ROUTE_WARP_23,
    ROUTE_WARP_24,
    ROUTE_WARP_25,
};

// Outdoor landings: near route signs, or R4/R10 Pokémon Center fly spots.
static const struct RhDebugMapWarp sRouteWarps[] =
{
    [ROUTE_WARP_1]        = {MAP_ROUTE1, 12, 28},
    [ROUTE_WARP_2]        = {MAP_ROUTE2, 14, 13},
    [ROUTE_WARP_3]        = {MAP_ROUTE3, 72, 12},
    [ROUTE_WARP_4]        = {MAP_ROUTE4, 12, 6},
    [ROUTE_WARP_5]        = {MAP_ROUTE5, 32, 33},
    [ROUTE_WARP_6]        = {MAP_ROUTE6, 21, 16},
    [ROUTE_WARP_7]        = {MAP_ROUTE7, 5, 15},
    [ROUTE_WARP_8]        = {MAP_ROUTE8, 16, 6},
    [ROUTE_WARP_9]        = {MAP_ROUTE9, 29, 8},
    [ROUTE_WARP_10]       = {MAP_ROUTE10, 13, 21},
    [ROUTE_WARP_11]       = {MAP_ROUTE11, 3, 8},
    [ROUTE_WARP_12]       = {MAP_ROUTE12, 17, 14},
    [ROUTE_WARP_13]       = {MAP_ROUTE13, 41, 13},
    [ROUTE_WARP_14]       = {MAP_ROUTE14, 17, 13},
    [ROUTE_WARP_15]       = {MAP_ROUTE15, 41, 12},
    [ROUTE_WARP_16]       = {MAP_ROUTE16, 6, 18},
    [ROUTE_WARP_17]       = {MAP_ROUTE17, 12, 50},
    [ROUTE_WARP_18]       = {MAP_ROUTE18, 37, 8},
    [ROUTE_WARP_19]       = {MAP_ROUTE19, 13, 13},
    [ROUTE_WARP_20]       = {MAP_ROUTE20, 64, 9},
    [ROUTE_WARP_21_NORTH] = {MAP_ROUTE21_NORTH, 12, 28},
    [ROUTE_WARP_21_SOUTH] = {MAP_ROUTE21_SOUTH, 12, 12},
    [ROUTE_WARP_22]       = {MAP_ROUTE22, 7, 13},
    [ROUTE_WARP_23]       = {MAP_ROUTE23, 3, 32},
    [ROUTE_WARP_24]       = {MAP_ROUTE24, 12, 16},
    [ROUTE_WARP_25]       = {MAP_ROUTE25, 48, 5},
};

static const struct ListMenuItem sWarpRouteGroupItems[] =
{
    {sText_RouteGroup1_10,   ROUTE_GROUP_1_10},
    {sText_RouteGroup11_20,  ROUTE_GROUP_11_20},
    {sText_RouteGroup21Plus, ROUTE_GROUP_21_PLUS},
    {sText_Back,             LIST_CANCEL},
};

static const struct ListMenuItem sWarpRouteItems_1_10[] =
{
    {sText_WarpRoute1,  ROUTE_WARP_1},
    {sText_WarpRoute2,  ROUTE_WARP_2},
    {sText_WarpRoute3,  ROUTE_WARP_3},
    {sText_WarpRoute4,  ROUTE_WARP_4},
    {sText_WarpRoute5,  ROUTE_WARP_5},
    {sText_WarpRoute6,  ROUTE_WARP_6},
    {sText_WarpRoute7,  ROUTE_WARP_7},
    {sText_WarpRoute8,  ROUTE_WARP_8},
    {sText_WarpRoute9,  ROUTE_WARP_9},
    {sText_WarpRoute10, ROUTE_WARP_10},
    {sText_Back,        LIST_CANCEL},
};

static const struct ListMenuItem sWarpRouteItems_11_20[] =
{
    {sText_WarpRoute11, ROUTE_WARP_11},
    {sText_WarpRoute12, ROUTE_WARP_12},
    {sText_WarpRoute13, ROUTE_WARP_13},
    {sText_WarpRoute14, ROUTE_WARP_14},
    {sText_WarpRoute15, ROUTE_WARP_15},
    {sText_WarpRoute16, ROUTE_WARP_16},
    {sText_WarpRoute17, ROUTE_WARP_17},
    {sText_WarpRoute18, ROUTE_WARP_18},
    {sText_WarpRoute19, ROUTE_WARP_19},
    {sText_WarpRoute20, ROUTE_WARP_20},
    {sText_Back,        LIST_CANCEL},
};

static const struct ListMenuItem sWarpRouteItems_21Plus[] =
{
    {sText_WarpRoute21N, ROUTE_WARP_21_NORTH},
    {sText_WarpRoute21S, ROUTE_WARP_21_SOUTH},
    {sText_WarpRoute22,  ROUTE_WARP_22},
    {sText_WarpRoute23,  ROUTE_WARP_23},
    {sText_WarpRoute24,  ROUTE_WARP_24},
    {sText_WarpRoute25,  ROUTE_WARP_25},
    {sText_Back,         LIST_CANCEL},
};

static const struct ListMenuItem sWarpSeviiItems[] =
{
    {sText_WarpOneIsland,   HEAL_LOCATION_ONE_ISLAND},
    {sText_WarpTwoIsland,   HEAL_LOCATION_TWO_ISLAND},
    {sText_WarpThreeIsland, HEAL_LOCATION_THREE_ISLAND},
    {sText_WarpFourIsland,  HEAL_LOCATION_FOUR_ISLAND},
    {sText_WarpFiveIsland,  HEAL_LOCATION_FIVE_ISLAND},
    {sText_WarpSixIsland,   HEAL_LOCATION_SIX_ISLAND},
    {sText_WarpSevenIsland, HEAL_LOCATION_SEVEN_ISLAND},
    {sText_Back,            LIST_CANCEL},
};

// Indexes into sSpecialWarps[]; BACK cancels like B.
enum
{
    SPECIAL_WARP_ARTICUNO,
    SPECIAL_WARP_ZAPDOS,
    SPECIAL_WARP_MOLTRES,
    SPECIAL_WARP_MEWTWO,
    SPECIAL_WARP_DEOXYS,
    SPECIAL_WARP_LUGIA,
    SPECIAL_WARP_HO_OH,
    SPECIAL_WARP_SNORLAX_R12,
    SPECIAL_WARP_SNORLAX_R16,
};

static const struct RhDebugMapWarp sSpecialWarps[] =
{
    [SPECIAL_WARP_ARTICUNO]    = {MAP_SEAFOAM_ISLANDS_B4F, 9, 3},
    [SPECIAL_WARP_ZAPDOS]      = {MAP_POWER_PLANT, 5, 12},
    [SPECIAL_WARP_MOLTRES]     = {MAP_MT_EMBER_SUMMIT, 9, 7},
    [SPECIAL_WARP_MEWTWO]      = {MAP_CERULEAN_CAVE_B1F, 7, 13},
    [SPECIAL_WARP_DEOXYS]      = {MAP_BIRTH_ISLAND_EXTERIOR, 15, 4},
    [SPECIAL_WARP_LUGIA]       = {MAP_NAVEL_ROCK_BASE, 10, 16},
    [SPECIAL_WARP_HO_OH]       = {MAP_NAVEL_ROCK_SUMMIT, 9, 7},
    [SPECIAL_WARP_SNORLAX_R12] = {MAP_ROUTE12, 14, 71},
    [SPECIAL_WARP_SNORLAX_R16] = {MAP_ROUTE16, 31, 14},
};

static const struct ListMenuItem sWarpSpecialItems[] =
{
    {sText_SpecialArticuno,  SPECIAL_WARP_ARTICUNO},
    {sText_SpecialZapdos,    SPECIAL_WARP_ZAPDOS},
    {sText_SpecialMoltres,   SPECIAL_WARP_MOLTRES},
    {sText_SpecialMewtwo,    SPECIAL_WARP_MEWTWO},
    {sText_SpecialDeoxys,    SPECIAL_WARP_DEOXYS},
    {sText_SpecialLugia,     SPECIAL_WARP_LUGIA},
    {sText_SpecialHoOh,      SPECIAL_WARP_HO_OH},
    {sText_SpecialSnorlax12, SPECIAL_WARP_SNORLAX_R12},
    {sText_SpecialSnorlax16, SPECIAL_WARP_SNORLAX_R16},
    {sText_Back,             LIST_CANCEL},
};

#include "data/rh_debug_give.h"
#include "data/rh_debug_events.h"

static const struct ListMenuTemplate sListTemplateBase =
{
    .items = sRootItems,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .itemPrintFunc = NULL,
    .totalItems = ARRAY_COUNT(sRootItems),
    .maxShowed = ARRAY_COUNT(sRootItems),
    .windowId = WIN_LIST,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 0,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = FONT_NORMAL,
    .cursorKind = 0,
};

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sWinTemplates[] =
{
    [WIN_LIST] =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 26,
        .height = 11,
        .paletteNum = 15,
        .baseBlock = 1
    },
    [WIN_SCROLL] =
    {
        // Std frame's right border is at tilemapLeft+width (col 28). Sit past it.
        // Only mapped while scrolling is needed.
        .bg = 0,
        .tilemapLeft = 29,
        .tilemapTop = 2,
        .width = 1,
        .height = 11,
        .paletteNum = 15,
        .baseBlock = 287
    },
    [WIN_FOOTER] =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 298
    },
    DUMMY_WIN_TEMPLATE
};

static void VBlankCB_RhDebugMenu(void);
static void CB2_RhDebugMenu(void);
static void Task_RhDebugMenu(u8 taskId);
static void InitRhDebugMenuGfx(void);
static void RhDebugMenu_BeginClose(u8 taskId);
static void RhDebugMenu_PrintFooter(const u8 *str);
static void RhDebugMenu_DestroyScrollbar(void);
static void RhDebugMenu_CreateScrollbar(u16 totalItems, u16 visibleItems);
static void RhDebugMenu_UpdateScrollbar(u8 taskId);
static void RhDebugMenu_DestroyList(u8 taskId);
static void RhDebugMenu_ShowRoot(u8 taskId);
static void RhDebugMenu_ShowGiveFolders(u8 taskId);
static void RhDebugMenu_ShowGiveSubfolders(u8 taskId, u8 folder);
static void RhDebugMenu_ShowGiveItems(u8 taskId, u8 folder, u8 sub);
static void RhDebugMenu_ShowGiveQty(u8 taskId);
static void RhDebugMenu_ShowWarpFolders(u8 taskId);
static void RhDebugMenu_ShowWarpRouteGroups(u8 taskId);
static void RhDebugMenu_ShowWarpDest(u8 taskId, u8 folder);
static void RhDebugMenu_ShowWarpRouteDest(u8 taskId, u8 group);
static void RhDebugMenu_InitList(u8 taskId, u8 page, const struct ListMenuItem *items, u16 totalItems, u16 maxShowed, const u8 *footer);
static void RhDebugMenu_RedrawGiveQty(u8 taskId);
static void RhDebugMenu_HandleRootInput(u8 taskId);
static void RhDebugMenu_HandleGiveFoldersInput(u8 taskId);
static void RhDebugMenu_HandleGiveSubfoldersInput(u8 taskId);
static void RhDebugMenu_HandleGiveItemsInput(u8 taskId);
static void RhDebugMenu_HandleGiveQtyInput(u8 taskId);
static void RhDebugMenu_HandleWarpFoldersInput(u8 taskId);
static void RhDebugMenu_HandleWarpRouteGroupsInput(u8 taskId);
static void RhDebugMenu_HandleWarpDestInput(u8 taskId);
static void RhDebugMenu_ShowCheatFolders(u8 taskId);
static void RhDebugMenu_ShowCheats(u8 taskId, u8 folder);
static void RhDebugMenu_ShowCheatResetConfirm(u8 taskId);
static void RhDebugMenu_RedrawCheats(u8 taskId);
static void RhDebugMenu_HandleCheatFoldersInput(u8 taskId);
static void RhDebugMenu_HandleCheatsInput(u8 taskId);
static void RhDebugMenu_HandleCheatResetConfirmInput(u8 taskId);
static void RhDebugMenu_ShowEventFolders(u8 taskId);
static void RhDebugMenu_ShowEventSubfolders(u8 taskId, u8 folder);
static void RhDebugMenu_ShowEvents(u8 taskId, u8 folder, u8 sub);
static void RhDebugMenu_RedrawEvents(u8 taskId);
static void RhDebugMenu_HandleEventFoldersInput(u8 taskId);
static void RhDebugMenu_HandleEventSubfoldersInput(u8 taskId);
static void RhDebugMenu_HandleEventsInput(u8 taskId);
static void RhDebugMenu_GetEventList(u8 folder, u8 sub, const struct RhDebugEvent **events, u16 *count);
static bool8 RhDebugMenu_EventFolderHasSubs(u8 folder);
static void RhDebugMenu_GetEventSubList(u8 folder, const struct ListMenuItem **items, u16 *count);
static void RhDebugMenu_OpenEventCategory(u8 taskId, u8 folder);
static void RhDebugMenu_ShowLog(u8 taskId);
static void RhDebugMenu_RedrawLog(u8 taskId);
static void RhDebugMenu_HandleLogInput(u8 taskId);
static void RhDebugMenu_GetCheatFolder(u8 folder, const u8 **ids, u8 *count);
static void RhDebugMenu_SetCheat(u8 cheatId, bool8 enabled);
static u8 RhDebugMenu_GetExpMultIndex(void);
static void RhDebugMenu_SetExpMultIndex(u8 idx);
static void RhDebugMenu_EnsureCheatCursorVisible(u8 taskId, u8 rowCount);
static const u8 *RhDebugMenu_GetCheatActionName(u8 actionId);
static void RhDebugMenu_RunCheatAction(u8 actionId);
static bool8 RhDebugMenu_GiveFolderHasSubs(u8 folder);
static void RhDebugMenu_GetGiveSubList(u8 folder, const struct ListMenuItem **items, u16 *count);
static void RhDebugMenu_GetGiveItemIds(u8 folder, u8 sub, const u16 **ids, u16 *count);
static void RhDebugMenu_BuildGiveItemMenu(const u16 *ids, u16 count);
static void RhDebugMenu_OpenGiveCategory(u8 taskId, u8 folder);
static void RhDebugMenu_AdjustGiveQty(s8 dir);
static void RhDebugMenu_TryGiveItem(void);
static void RhDebugMenu_StartWarp(u8 taskId, u8 destId);
static void RhDebugMenu_ApplyWarp(u8 folder, u8 destId);

static void VBlankCB_RhDebugMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_RhDebugMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void RhDebugMenu_BeginClose(u8 taskId)
{
    PlaySE(SE_SELECT);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].tState = STATE_WAIT_FADE_OUT;
}

static void RhDebugMenu_PrintFooter(const u8 *str)
{
    FillWindowPixelBuffer(WIN_FOOTER, PIXEL_FILL(1));
    DrawStdFrameWithCustomTileAndPalette(WIN_FOOTER, FALSE, 0x1C0, 14);
    AddTextPrinterParameterized3(WIN_FOOTER, FONT_SMALL, 4, 4, sTextColors, TEXT_SKIP_DRAW, str);
    PutWindowTilemap(WIN_FOOTER);
    CopyWindowToVram(WIN_FOOTER, COPYWIN_FULL);
}

static const struct ScrollbarConfig sScrollbarConfig =
{
    .windowId = WIN_SCROLL,
    .trackColor = 2,
    .thumbColor = 1,
    // Match std window frame outer border (stdpal_3 index 13).
    .borderColor = 13,
    .highlightColor = 1,
    .minThumbPx = 12,
    .taskPriority = 0, // default (after ListMenu)
    .padTop = 2,
    .padBottom = 2,
    .padLeft = 0,
    .padRight = 0,
};

static void RhDebugMenu_HideScrollStrip(void)
{
    ClearWindowTilemap(WIN_SCROLL);
    CopyWindowToVram(WIN_SCROLL, COPYWIN_MAP);
}

static void RhDebugMenu_DestroyScrollbar(void)
{
    if (sScrollbarTaskId != SCROLLBAR_NONE)
    {
        Scrollbar_Destroy(sScrollbarTaskId);
        sScrollbarTaskId = SCROLLBAR_NONE;
    }
    RhDebugMenu_HideScrollStrip();
}

static void RhDebugMenu_CreateScrollbar(u16 totalItems, u16 visibleItems)
{
    RhDebugMenu_DestroyScrollbar();
    sScrollbarScroll = 0;
    if (totalItems <= visibleItems)
        return;

    sScrollbarTaskId = Scrollbar_Create(&sScrollbarConfig, &sScrollbarScroll, totalItems, visibleItems);
    if (sScrollbarTaskId == SCROLLBAR_NONE)
        return;

    // Black fill matches the screen bg; only the track/thumb read as UI.
    FillWindowPixelBuffer(WIN_SCROLL, PIXEL_FILL(0));
    PutWindowTilemap(WIN_SCROLL);
    CopyWindowToVram(WIN_SCROLL, COPYWIN_FULL);
}

static void RhDebugMenu_UpdateScrollbar(u8 taskId)
{
    if (sScrollbarTaskId == SCROLLBAR_NONE)
        return;

    if (gTasks[taskId].tListTaskId != LIST_NONE)
        Scrollbar_SyncFromListMenu(sScrollbarTaskId, gTasks[taskId].tListTaskId);
    else if (gTasks[taskId].tPage == PAGE_CHEATS || gTasks[taskId].tPage == PAGE_EVENTS || gTasks[taskId].tPage == PAGE_LOG)
        sScrollbarScroll = gTasks[taskId].tCheatScroll;
}

static void RhDebugMenu_DestroyList(u8 taskId)
{
    if (gTasks[taskId].tListTaskId != LIST_NONE)
    {
        DestroyListMenuTask(gTasks[taskId].tListTaskId, NULL, NULL);
        gTasks[taskId].tListTaskId = LIST_NONE;
    }
    RhDebugMenu_DestroyScrollbar();
}

static void RhDebugMenu_ShowRoot(u8 taskId)
{
    struct ListMenuTemplate template = sListTemplateBase;

    RhDebugMenu_DestroyList(taskId);
    gTasks[taskId].tPage = PAGE_ROOT;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, 0x1C0, 14);
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
    RhDebugMenu_PrintFooter(sText_FooterRoot);

    template.items = sRootItems;
    template.totalItems = ARRAY_COUNT(sRootItems);
    template.maxShowed = ARRAY_COUNT(sRootItems);
    gTasks[taskId].tListTaskId = ListMenuInit(&template, 0, 0);
}

static void RhDebugMenu_RedrawGiveQty(u8 taskId)
{
    u8 buf[32];
    u8 line[64];
    const u8 *name = ItemId_GetName(sGiveItemId);

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, 0x1C0, 14);

    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 8, 8, sTextColors, TEXT_SKIP_DRAW, name);

    ConvertIntToDecimalStringN(buf, sGiveQty, STR_CONV_MODE_LEADING_ZEROS, 2);
    line[0] = CHAR_RIGHT_ARROW;
    line[1] = CHAR_SPACE;
    StringCopy(&line[2], sText_Times);
    StringAppend(&line[2], buf);
    AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 8, 24, sTextColors, TEXT_SKIP_DRAW, line);

    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static bool8 RhDebugMenu_GiveFolderHasSubs(u8 folder)
{
    switch (folder)
    {
    case GIVE_CAT_MEDICINE:
    case GIVE_CAT_HOLD:
    case GIVE_CAT_BERRIES:
    case GIVE_CAT_TMS:
    case GIVE_CAT_KEY:
        return TRUE;
    default:
        return FALSE;
    }
}

static void RhDebugMenu_GetGiveSubList(u8 folder, const struct ListMenuItem **items, u16 *count)
{
    switch (folder)
    {
    case GIVE_CAT_MEDICINE:
        *items = sGiveMedSubItems;
        *count = ARRAY_COUNT(sGiveMedSubItems);
        break;
    case GIVE_CAT_HOLD:
        *items = sGiveHoldSubItems;
        *count = ARRAY_COUNT(sGiveHoldSubItems);
        break;
    case GIVE_CAT_BERRIES:
        *items = sGiveBerrySubItems;
        *count = ARRAY_COUNT(sGiveBerrySubItems);
        break;
    case GIVE_CAT_TMS:
        *items = sGiveTMSubItems;
        *count = ARRAY_COUNT(sGiveTMSubItems);
        break;
    case GIVE_CAT_KEY:
        *items = sGiveKeySubItems;
        *count = ARRAY_COUNT(sGiveKeySubItems);
        break;
    default:
        *items = sGiveMedSubItems;
        *count = ARRAY_COUNT(sGiveMedSubItems);
        break;
    }
}

static void RhDebugMenu_GetGiveItemIds(u8 folder, u8 sub, const u16 **ids, u16 *count)
{
    *ids = sGiveItems_Balls;
    *count = ARRAY_COUNT(sGiveItems_Balls);

    switch (folder)
    {
    case GIVE_CAT_BALLS:
        *ids = sGiveItems_Balls;
        *count = ARRAY_COUNT(sGiveItems_Balls);
        break;
    case GIVE_CAT_MEDICINE:
        switch (sub)
        {
        case GIVE_MED_POTIONS:
            *ids = sGiveItems_MedPotions;
            *count = ARRAY_COUNT(sGiveItems_MedPotions);
            break;
        case GIVE_MED_STATUS:
            *ids = sGiveItems_MedStatus;
            *count = ARRAY_COUNT(sGiveItems_MedStatus);
            break;
        case GIVE_MED_PP:
            *ids = sGiveItems_MedPP;
            *count = ARRAY_COUNT(sGiveItems_MedPP);
            break;
        case GIVE_MED_REVIVES:
            *ids = sGiveItems_MedRevives;
            *count = ARRAY_COUNT(sGiveItems_MedRevives);
            break;
        }
        break;
    case GIVE_CAT_BATTLE:
        *ids = sGiveItems_Battle;
        *count = ARRAY_COUNT(sGiveItems_Battle);
        break;
    case GIVE_CAT_VITAMINS:
        *ids = sGiveItems_Vitamins;
        *count = ARRAY_COUNT(sGiveItems_Vitamins);
        break;
    case GIVE_CAT_EVOLUTION:
        *ids = sGiveItems_Evolution;
        *count = ARRAY_COUNT(sGiveItems_Evolution);
        break;
    case GIVE_CAT_HOLD:
        switch (sub)
        {
        case GIVE_HOLD_BATTLE:
            *ids = sGiveItems_HoldBattle;
            *count = ARRAY_COUNT(sGiveItems_HoldBattle);
            break;
        case GIVE_HOLD_TYPE:
            *ids = sGiveItems_HoldType;
            *count = ARRAY_COUNT(sGiveItems_HoldType);
            break;
        case GIVE_HOLD_SPECIES:
            *ids = sGiveItems_HoldSpecies;
            *count = ARRAY_COUNT(sGiveItems_HoldSpecies);
            break;
        case GIVE_HOLD_OTHER:
            *ids = sGiveItems_HoldOther;
            *count = ARRAY_COUNT(sGiveItems_HoldOther);
            break;
        }
        break;
    case GIVE_CAT_BERRIES:
        switch (sub)
        {
        case GIVE_BERRY_STATUS:
            *ids = sGiveItems_BerryStatus;
            *count = ARRAY_COUNT(sGiveItems_BerryStatus);
            break;
        case GIVE_BERRY_HEAL:
            *ids = sGiveItems_BerryHeal;
            *count = ARRAY_COUNT(sGiveItems_BerryHeal);
            break;
        case GIVE_BERRY_EV:
            *ids = sGiveItems_BerryEV;
            *count = ARRAY_COUNT(sGiveItems_BerryEV);
            break;
        case GIVE_BERRY_GROWTH:
            *ids = sGiveItems_BerryGrowth;
            *count = ARRAY_COUNT(sGiveItems_BerryGrowth);
            break;
        case GIVE_BERRY_PINCH:
            *ids = sGiveItems_BerryPinch;
            *count = ARRAY_COUNT(sGiveItems_BerryPinch);
            break;
        }
        break;
    case GIVE_CAT_MAIL:
        *ids = sGiveItems_Mail;
        *count = ARRAY_COUNT(sGiveItems_Mail);
        break;
    case GIVE_CAT_TREASURES:
        *ids = sGiveItems_Treasures;
        *count = ARRAY_COUNT(sGiveItems_Treasures);
        break;
    case GIVE_CAT_TMS:
        switch (sub)
        {
        case GIVE_TM_01_25:
            *ids = sGiveItems_TM01_25;
            *count = ARRAY_COUNT(sGiveItems_TM01_25);
            break;
        case GIVE_TM_26_50:
            *ids = sGiveItems_TM26_50;
            *count = ARRAY_COUNT(sGiveItems_TM26_50);
            break;
        }
        break;
    case GIVE_CAT_HMS:
        *ids = sGiveItems_HMs;
        *count = ARRAY_COUNT(sGiveItems_HMs);
        break;
    case GIVE_CAT_KEY:
        switch (sub)
        {
        case GIVE_KEY_STORY:
            *ids = sGiveItems_KeyStory;
            *count = ARRAY_COUNT(sGiveItems_KeyStory);
            break;
        case GIVE_KEY_TOOLS:
            *ids = sGiveItems_KeyTools;
            *count = ARRAY_COUNT(sGiveItems_KeyTools);
            break;
        case GIVE_KEY_PASSES:
            *ids = sGiveItems_KeyPasses;
            *count = ARRAY_COUNT(sGiveItems_KeyPasses);
            break;
        case GIVE_KEY_OTHER:
            *ids = sGiveItems_KeyOther;
            *count = ARRAY_COUNT(sGiveItems_KeyOther);
            break;
        }
        break;
    case GIVE_CAT_ROMHACK:
        *ids = sGiveItems_Romhack;
        *count = ARRAY_COUNT(sGiveItems_Romhack);
        break;
    }
}

static void RhDebugMenu_BuildGiveItemMenu(const u16 *ids, u16 count)
{
    u16 i;

    for (i = 0; i < count; i++)
    {
        sGiveMenuItems[i].label = ItemId_GetName(ids[i]);
        sGiveMenuItems[i].index = ids[i];
    }
    sGiveMenuItems[count].label = sText_Back;
    sGiveMenuItems[count].index = LIST_CANCEL;
}

static void RhDebugMenu_ShowGiveFolders(u8 taskId)
{
    gTasks[taskId].tGiveSub = GIVE_SUB_NONE;
    RhDebugMenu_InitList(taskId,
                         PAGE_GIVE_FOLDERS,
                         sGiveFolderItems,
                         ARRAY_COUNT(sGiveFolderItems),
                         (ARRAY_COUNT(sGiveFolderItems) > GIVE_LIST_MAX_SHOWED)
                             ? GIVE_LIST_MAX_SHOWED
                             : ARRAY_COUNT(sGiveFolderItems),
                         sText_FooterGiveFolders);
}

static void RhDebugMenu_ShowGiveSubfolders(u8 taskId, u8 folder)
{
    const struct ListMenuItem *items;
    u16 count;

    gTasks[taskId].tGiveFolder = folder;
    gTasks[taskId].tGiveSub = GIVE_SUB_NONE;
    RhDebugMenu_GetGiveSubList(folder, &items, &count);
    RhDebugMenu_InitList(taskId,
                         PAGE_GIVE_SUBFOLDERS,
                         items,
                         count,
                         (count > GIVE_LIST_MAX_SHOWED) ? GIVE_LIST_MAX_SHOWED : count,
                         sText_FooterGiveFolders);
}

static void RhDebugMenu_ShowGiveItems(u8 taskId, u8 folder, u8 sub)
{
    const u16 *ids;
    u16 count;

    gTasks[taskId].tGiveFolder = folder;
    gTasks[taskId].tGiveSub = sub;
    RhDebugMenu_GetGiveItemIds(folder, sub, &ids, &count);
    RhDebugMenu_BuildGiveItemMenu(ids, count);
    RhDebugMenu_InitList(taskId,
                         PAGE_GIVE_ITEMS,
                         sGiveMenuItems,
                         count + 1,
                         (count + 1 > GIVE_LIST_MAX_SHOWED) ? GIVE_LIST_MAX_SHOWED : (count + 1),
                         sText_FooterGiveItems);
}

static void RhDebugMenu_ShowGiveQty(u8 taskId)
{
    RhDebugMenu_DestroyList(taskId);
    gTasks[taskId].tPage = PAGE_GIVE_QTY;
    RhDebugMenu_PrintFooter(sText_FooterGiveQty);
    RhDebugMenu_RedrawGiveQty(taskId);
}

static void RhDebugMenu_OpenGiveCategory(u8 taskId, u8 folder)
{
    if (RhDebugMenu_GiveFolderHasSubs(folder))
        RhDebugMenu_ShowGiveSubfolders(taskId, folder);
    else
        RhDebugMenu_ShowGiveItems(taskId, folder, GIVE_SUB_NONE);
}

static void RhDebugMenu_InitList(u8 taskId, u8 page, const struct ListMenuItem *items, u16 totalItems, u16 maxShowed, const u8 *footer)
{
    struct ListMenuTemplate template = sListTemplateBase;

    RhDebugMenu_DestroyList(taskId);
    gTasks[taskId].tPage = page;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, 0x1C0, 14);
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
    RhDebugMenu_PrintFooter(footer);

    template.items = items;
    template.totalItems = totalItems;
    template.maxShowed = maxShowed;
    gTasks[taskId].tListTaskId = ListMenuInit(&template, 0, 0);
    RhDebugMenu_CreateScrollbar(totalItems, maxShowed);
}

static void RhDebugMenu_ShowWarpFolders(u8 taskId)
{
    RhDebugMenu_InitList(taskId,
                         PAGE_WARP_FOLDERS,
                         sWarpFolderItems,
                         ARRAY_COUNT(sWarpFolderItems),
                         ARRAY_COUNT(sWarpFolderItems),
                         sText_FooterWarpFolders);
}

static void RhDebugMenu_ShowWarpRouteGroups(u8 taskId)
{
    gTasks[taskId].tWarpFolder = WARP_FOLDER_ROUTES;
    RhDebugMenu_InitList(taskId,
                         PAGE_WARP_ROUTE_GROUPS,
                         sWarpRouteGroupItems,
                         ARRAY_COUNT(sWarpRouteGroupItems),
                         ARRAY_COUNT(sWarpRouteGroupItems),
                         sText_FooterWarpFolders);
}

static void RhDebugMenu_ShowWarpRouteDest(u8 taskId, u8 group)
{
    const struct ListMenuItem *items;
    u16 count;

    gTasks[taskId].tWarpFolder = WARP_FOLDER_ROUTES;
    gTasks[taskId].tGiveSub = group; // remember group for Back

    switch (group)
    {
    default:
    case ROUTE_GROUP_1_10:
        items = sWarpRouteItems_1_10;
        count = ARRAY_COUNT(sWarpRouteItems_1_10);
        break;
    case ROUTE_GROUP_11_20:
        items = sWarpRouteItems_11_20;
        count = ARRAY_COUNT(sWarpRouteItems_11_20);
        break;
    case ROUTE_GROUP_21_PLUS:
        items = sWarpRouteItems_21Plus;
        count = ARRAY_COUNT(sWarpRouteItems_21Plus);
        break;
    }

    RhDebugMenu_InitList(taskId,
                         PAGE_WARP_DEST,
                         items,
                         count,
                         (count > WARP_LIST_MAX_SHOWED) ? WARP_LIST_MAX_SHOWED : count,
                         sText_FooterWarpDest);
}

static void RhDebugMenu_ShowWarpDest(u8 taskId, u8 folder)
{
    const struct ListMenuItem *items;
    u16 count;

    gTasks[taskId].tWarpFolder = folder;
    switch (folder)
    {
    default:
    case WARP_FOLDER_TOWNS:
        items = sWarpTownItems;
        count = ARRAY_COUNT(sWarpTownItems);
        break;
    case WARP_FOLDER_SPECIAL:
        items = sWarpSpecialItems;
        count = ARRAY_COUNT(sWarpSpecialItems);
        break;
    case WARP_FOLDER_SEVII:
        items = sWarpSeviiItems;
        count = ARRAY_COUNT(sWarpSeviiItems);
        break;
    }

    RhDebugMenu_InitList(taskId,
                         PAGE_WARP_DEST,
                         items,
                         count,
                         (count > WARP_LIST_MAX_SHOWED) ? WARP_LIST_MAX_SHOWED : count,
                         sText_FooterWarpDest);
}

static void RhDebugMenu_AdjustGiveQty(s8 dir)
{
    s32 qty = sGiveQty + dir;

    if (qty < 0)
        qty = 99;
    else if (qty > 99)
        qty = 0;
    sGiveQty = qty;
}

static void RhDebugMenu_TryGiveItem(void)
{
    if (sGiveQty == 0 || sGiveItemId == ITEM_NONE)
    {
        PlaySE(SE_BOO);
        RhLog(sText_LogGiveFail);
        return;
    }

    if (AddBagItem(sGiveItemId, sGiveQty))
    {
        PlaySE(SE_SUCCESS);
        RhLogf(sText_LogGaveFmt, ItemId_GetName(sGiveItemId), sGiveQty);
    }
    else
    {
        PlaySE(SE_FAILURE);
        RhLogf(sText_LogGiveFullFmt, ItemId_GetName(sGiveItemId), sGiveQty);
    }
}

static void RhDebugMenu_StartWarp(u8 taskId, u8 destId)
{
    PlaySE(SE_SELECT);
    gTasks[taskId].tWarpDest = destId;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].tState = STATE_EXIT_WARP;
}

static void RhDebugMenu_ApplyWarp(u8 folder, u8 destId)
{
    const struct RhDebugMapWarp *table = NULL;

    if (folder == WARP_FOLDER_SPECIAL)
        table = sSpecialWarps;
    else if (folder == WARP_FOLDER_ROUTES)
        table = sRouteWarps;

    if (table != NULL)
    {
        const struct RhDebugMapWarp *warp = &table[destId];

        RhLogf(sText_LogWarpMapFmt, MAP_GROUP(warp->map), MAP_NUM(warp->map), warp->x, warp->y);
        SetWarpDestination(MAP_GROUP(warp->map), MAP_NUM(warp->map), WARP_ID_NONE, warp->x, warp->y);
    }
    else
    {
        RhLogf(sText_LogWarpHealFmt, destId);
        SetWarpDestinationToHealLocation(destId);
    }
}

static void RhDebugMenu_GetCheatFolder(u8 folder, const u8 **ids, u8 *count)
{
    switch (folder)
    {
    case CHEAT_FOLDER_PLAYER:
        *ids = sCheatFolderPlayer;
        *count = ARRAY_COUNT(sCheatFolderPlayer);
        break;
    case CHEAT_FOLDER_ENEMY:
        *ids = sCheatFolderEnemy;
        *count = ARRAY_COUNT(sCheatFolderEnemy);
        break;
    case CHEAT_FOLDER_CATCHING:
        *ids = sCheatFolderCatching;
        *count = ARRAY_COUNT(sCheatFolderCatching);
        break;
    case CHEAT_FOLDER_MISC:
    default:
        *ids = sCheatFolderMisc;
        *count = ARRAY_COUNT(sCheatFolderMisc);
        break;
    }
}

static void RhDebugMenu_SetCheat(u8 cheatId, bool8 enabled)
{
    if (cheatId >= CHEAT_BOOL_COUNT)
        return;

    if (enabled)
        FlagSet(sCheatFlags[cheatId]);
    else
        FlagClear(sCheatFlags[cheatId]);

    if (enabled)
        RhLogf(sText_LogCheatOnFmt, sCheatNames[cheatId]);
    else
        RhLogf(sText_LogCheatOffFmt, sCheatNames[cheatId]);
}

void RhDebugMenu_ResetAllCheats(void)
{
    u8 i;

    for (i = 0; i < CHEAT_BOOL_COUNT; i++)
        FlagClear(sCheatFlags[i]);
    VarSet(VAR_CHEAT_EXP_MULT, 0);
    RhLog(sText_LogCheatsReset);
}

static u8 RhDebugMenu_GetExpMultIndex(void)
{
    u16 idx = VarGet(VAR_CHEAT_EXP_MULT);

    if (idx >= CHEAT_EXP_MULT_COUNT)
        return 0;
    return idx;
}

static void RhDebugMenu_SetExpMultIndex(u8 idx)
{
    if (idx >= CHEAT_EXP_MULT_COUNT)
        idx = 0;
    VarSet(VAR_CHEAT_EXP_MULT, idx);
    RhLogf(sText_LogExpMultFmt, sExpMultLabels[idx]);
}

static const u8 *RhDebugMenu_GetCheatActionName(u8 actionId)
{
    switch (actionId)
    {
    case CHEAT_ACTION_MAX_MONEY:
        return sText_CheatMaxMoney;
    case CHEAT_ACTION_NAT_DEX:
        return sText_CheatNatDex;
    default:
        return sText_Back;
    }
}

static void RhDebugMenu_RunCheatAction(u8 actionId)
{
    switch (actionId)
    {
    case CHEAT_ACTION_MAX_MONEY:
        SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
        PlaySE(SE_SUCCESS);
        RhLog(sText_LogMaxMoney);
        break;
    case CHEAT_ACTION_NAT_DEX:
        EnableNationalPokedex();
        PlaySE(SE_SUCCESS);
        RhLog(sText_LogNatDex);
        break;
    default:
        PlaySE(SE_BOO);
        break;
    }
}

static void RhDebugMenu_EnsureCheatCursorVisible(u8 taskId, u8 rowCount)
{
    u8 cursor = gTasks[taskId].tGiveSub;
    u8 scroll = gTasks[taskId].tCheatScroll;

    if (cursor >= rowCount)
        cursor = rowCount - 1;
    if (cursor < scroll)
        gTasks[taskId].tCheatScroll = cursor;
    else if (cursor >= scroll + CHEAT_LIST_SHOW)
        gTasks[taskId].tCheatScroll = cursor - CHEAT_LIST_SHOW + 1;
}

static void RhDebugMenu_RedrawCheats(u8 taskId)
{
    u8 i;
    u8 j;
    u8 y = 2;
    u8 line[48];
    u8 cursor = gTasks[taskId].tGiveSub;
    u8 scroll;
    u8 drawn;
    u8 expIdx;
    u8 x;
    bool8 enabled;
    const u8 *ids;
    u8 cheatCount;
    u8 rowCount;
    u8 cheatId;
    u8 rowBack;

    RhDebugMenu_GetCheatFolder(gTasks[taskId].tCheatFolder, &ids, &cheatCount);
    rowCount = cheatCount + 1;
    rowBack = cheatCount;

    RhDebugMenu_EnsureCheatCursorVisible(taskId, rowCount);
    scroll = gTasks[taskId].tCheatScroll;
    cursor = gTasks[taskId].tGiveSub;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, 0x1C0, 14);

    for (i = scroll, drawn = 0; i < rowCount && drawn < CHEAT_LIST_SHOW; i++, drawn++)
    {
        if (i == rowBack)
        {
            line[0] = (cursor == rowBack) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
            line[1] = CHAR_SPACE;
            StringCopy(&line[2], sText_Back);
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, y, sTextColors, TEXT_SKIP_DRAW, line);
            break;
        }

        cheatId = ids[i];
        if (cheatId >= CHEAT_ACTION_RESET_ALL)
        {
            line[0] = (cursor == i) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
            line[1] = CHAR_SPACE;
            StringCopy(&line[2], RhDebugMenu_GetCheatActionName(cheatId));
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, y, sTextColors, TEXT_SKIP_DRAW, line);
            y += CHEAT_LINE_H;
            line[0] = CHAR_SPACE;
            line[1] = CHAR_SPACE;
            StringCopy(&line[2], sText_CheatActionHint);
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 8, y, sTextColors, TEXT_SKIP_DRAW, line);
            y += CHEAT_LINE_H;
        }
        else if (cheatId == CHEAT_EXP_MULT)
        {
            line[0] = (cursor == i) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
            line[1] = CHAR_SPACE;
            StringCopy(&line[2], sText_CheatExpMult);
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, y, sTextColors, TEXT_SKIP_DRAW, line);
            y += CHEAT_LINE_H;

            expIdx = RhDebugMenu_GetExpMultIndex();
            x = 8;
            for (j = 0; j < CHEAT_EXP_MULT_COUNT; j++)
            {
                line[0] = (j == expIdx) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
                line[1] = CHAR_SPACE;
                StringCopy(&line[2], sExpMultLabels[j]);
                AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, x, y, sTextColors, TEXT_SKIP_DRAW, line);
                x += 36;
            }
            y += CHEAT_LINE_H;
        }
        else
        {
            enabled = FlagGet(sCheatFlags[cheatId]);

            line[0] = (cursor == i) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
            line[1] = CHAR_SPACE;
            StringCopy(&line[2], sCheatNames[cheatId]);
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, y, sTextColors, TEXT_SKIP_DRAW, line);
            y += CHEAT_LINE_H;

            // Values on the next line (right-aligned) so long labels don't cover them.
            line[0] = (!enabled) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
            line[1] = CHAR_SPACE;
            StringCopy(&line[2], sText_False);
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, CHEAT_FALSE_X, y, sTextColors, TEXT_SKIP_DRAW, line);

            line[0] = (enabled) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
            line[1] = CHAR_SPACE;
            StringCopy(&line[2], sText_True);
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, CHEAT_TRUE_X, y, sTextColors, TEXT_SKIP_DRAW, line);
            y += CHEAT_LINE_H;
        }
    }

    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void RhDebugMenu_ShowCheatFolders(u8 taskId)
{
    RhDebugMenu_InitList(taskId,
                         PAGE_CHEAT_FOLDERS,
                         sCheatFolderItems,
                         ARRAY_COUNT(sCheatFolderItems),
                         ARRAY_COUNT(sCheatFolderItems),
                         sText_FooterCheatFolders);
}

static void RhDebugMenu_ShowCheatResetConfirm(u8 taskId)
{
    struct ListMenuTemplate template = sListTemplateBase;

    RhDebugMenu_DestroyList(taskId);
    gTasks[taskId].tPage = PAGE_CHEAT_RESET_CONFIRM;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, 0x1C0, 14);
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
    RhDebugMenu_PrintFooter(sText_CheatResetPrompt);

    template.items = sCheatResetConfirmItems;
    template.totalItems = ARRAY_COUNT(sCheatResetConfirmItems);
    template.maxShowed = ARRAY_COUNT(sCheatResetConfirmItems);
    // cursorPos=0 (first visible row index), itemsAbove=1 → cursor on NO.
    gTasks[taskId].tListTaskId = ListMenuInit(&template, 0, 1);
}

static void RhDebugMenu_ShowCheats(u8 taskId, u8 folder)
{
    const u8 *ids;
    u8 cheatCount;

    RhDebugMenu_DestroyList(taskId);
    gTasks[taskId].tPage = PAGE_CHEATS;
    gTasks[taskId].tCheatFolder = folder;
    gTasks[taskId].tGiveSub = 0;
    gTasks[taskId].tCheatScroll = 0;
    RhDebugMenu_PrintFooter(sText_FooterCheats);
    RhDebugMenu_RedrawCheats(taskId);
    RhDebugMenu_GetCheatFolder(folder, &ids, &cheatCount);
    RhDebugMenu_CreateScrollbar(cheatCount + 1, CHEAT_LIST_SHOW);
}

static bool8 RhDebugMenu_EventFolderHasSubs(u8 folder)
{
    if (folder >= ARRAY_COUNT(sEventFolderHasSubs))
        return FALSE;
    return sEventFolderHasSubs[folder];
}

static void RhDebugMenu_GetEventSubList(u8 folder, const struct ListMenuItem **items, u16 *count)
{
    switch (folder)
    {
    case EVENT_FOLDER_KANTO:
        *items = sEventKantoItems;
        *count = ARRAY_COUNT(sEventKantoItems);
        break;
    case EVENT_FOLDER_SIDE:
        *items = sEventSideItems;
        *count = ARRAY_COUNT(sEventSideItems);
        break;
    case EVENT_FOLDER_SEVII:
        *items = sEventSeviiItems;
        *count = ARRAY_COUNT(sEventSeviiItems);
        break;
    case EVENT_FOLDER_WORLDMAP:
        *items = sEventMapItems;
        *count = ARRAY_COUNT(sEventMapItems);
        break;
    default:
        *items = NULL;
        *count = 0;
        break;
    }
}

static void RhDebugMenu_GetEventList(u8 folder, u8 sub, const struct RhDebugEvent **events, u16 *count)
{
    *events = NULL;
    *count = 0;

    if (!RhDebugMenu_EventFolderHasSubs(folder))
    {
        if (folder < ARRAY_COUNT(sEventFolderFlatTables) && sEventFolderFlatTables[folder] != NULL)
        {
            *events = sEventFolderFlatTables[folder];
            *count = sEventFolderFlatCounts[folder];
        }
        return;
    }

    switch (folder)
    {
    case EVENT_FOLDER_KANTO:
        if (sub < ARRAY_COUNT(sEventKantoTables))
        {
            *events = sEventKantoTables[sub];
            *count = sEventKantoCounts[sub];
        }
        break;
    case EVENT_FOLDER_SIDE:
        if (sub < ARRAY_COUNT(sEventSideTables))
        {
            *events = sEventSideTables[sub];
            *count = sEventSideCounts[sub];
        }
        break;
    case EVENT_FOLDER_SEVII:
        if (sub < ARRAY_COUNT(sEventSeviiTables))
        {
            *events = sEventSeviiTables[sub];
            *count = sEventSeviiCounts[sub];
        }
        break;
    case EVENT_FOLDER_WORLDMAP:
        if (sub < ARRAY_COUNT(sEventMapTables))
        {
            *events = sEventMapTables[sub];
            *count = sEventMapCounts[sub];
        }
        break;
    }
}

static void RhDebugMenu_OpenEventCategory(u8 taskId, u8 folder)
{
    if (RhDebugMenu_EventFolderHasSubs(folder))
        RhDebugMenu_ShowEventSubfolders(taskId, folder);
    else
        RhDebugMenu_ShowEvents(taskId, folder, EVENT_SUB_NONE);
}

static void RhDebugMenu_ShowEventFolders(u8 taskId)
{
    RhDebugMenu_InitList(taskId,
                         PAGE_EVENT_FOLDERS,
                         sEventFolderItems,
                         ARRAY_COUNT(sEventFolderItems),
                         (ARRAY_COUNT(sEventFolderItems) > GIVE_LIST_MAX_SHOWED)
                             ? GIVE_LIST_MAX_SHOWED
                             : ARRAY_COUNT(sEventFolderItems),
                         sText_FooterEventFolders);
}

static void RhDebugMenu_ShowEventSubfolders(u8 taskId, u8 folder)
{
    const struct ListMenuItem *items;
    u16 count;

    RhDebugMenu_GetEventSubList(folder, &items, &count);
    gTasks[taskId].tEventFolder = folder;
    RhDebugMenu_InitList(taskId,
                         PAGE_EVENT_SUBFOLDERS,
                         items,
                         count,
                         (count > GIVE_LIST_MAX_SHOWED) ? GIVE_LIST_MAX_SHOWED : count,
                         sText_FooterEventFolders);
}

static void RhDebugMenu_EnsureEventCursorVisible(u8 taskId, u16 rowCount)
{
    u8 cursor = gTasks[taskId].tEventCursor;
    u8 scroll = gTasks[taskId].tEventScroll;

    if (rowCount == 0)
    {
        gTasks[taskId].tEventCursor = 0;
        gTasks[taskId].tEventScroll = 0;
        return;
    }
    if (cursor >= rowCount)
        cursor = rowCount - 1;
    if (cursor < scroll)
        gTasks[taskId].tEventScroll = cursor;
    else if (cursor >= scroll + EVENT_LIST_SHOW)
        gTasks[taskId].tEventScroll = cursor - EVENT_LIST_SHOW + 1;
    gTasks[taskId].tEventCursor = cursor;
}

static void RhDebugMenu_RedrawEvents(u8 taskId)
{
    u8 i;
    u8 y = 2;
    u8 line[48];
    u8 cursor;
    u8 scroll;
    u8 drawn;
    bool8 enabled;
    const struct RhDebugEvent *events;
    u16 eventCount;
    u16 rowCount;
    u16 rowBack;
    const struct RhDebugEvent *ev;

    RhDebugMenu_GetEventList(gTasks[taskId].tEventFolder, gTasks[taskId].tEventSub, &events, &eventCount);
    rowCount = eventCount + 1;
    rowBack = eventCount;

    RhDebugMenu_EnsureEventCursorVisible(taskId, rowCount);
    scroll = gTasks[taskId].tEventScroll;
    cursor = gTasks[taskId].tEventCursor;
    sScrollbarScroll = scroll;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, 0x1C0, 14);

    for (i = scroll, drawn = 0; i < rowCount && drawn < EVENT_LIST_SHOW; i++, drawn++)
    {
        if (i == rowBack)
        {
            line[0] = (cursor == rowBack) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
            line[1] = CHAR_SPACE;
            StringCopy(&line[2], sText_Back);
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, y, sTextColors, TEXT_SKIP_DRAW, line);
            break;
        }

        ev = &events[i];
        enabled = FlagGet(ev->flag);

        line[0] = (cursor == i) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
        line[1] = CHAR_SPACE;
        StringCopy(&line[2], ev->name);
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, y, sTextColors, TEXT_SKIP_DRAW, line);
        y += CHEAT_LINE_H;

        line[0] = (!enabled) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
        line[1] = CHAR_SPACE;
        StringCopy(&line[2], sText_False);
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, CHEAT_FALSE_X, y, sTextColors, TEXT_SKIP_DRAW, line);

        line[0] = (enabled) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
        line[1] = CHAR_SPACE;
        StringCopy(&line[2], sText_True);
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, CHEAT_TRUE_X, y, sTextColors, TEXT_SKIP_DRAW, line);
        y += CHEAT_LINE_H;
    }

    if (cursor < eventCount && events != NULL)
    {
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 4, EVENT_DESC_Y, sTextColors, TEXT_SKIP_DRAW,
                                     events[cursor].desc);
    }

    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void RhDebugMenu_ShowEvents(u8 taskId, u8 folder, u8 sub)
{
    const struct RhDebugEvent *events;
    u16 eventCount;

    RhDebugMenu_DestroyList(taskId);
    gTasks[taskId].tPage = PAGE_EVENTS;
    gTasks[taskId].tEventFolder = folder;
    gTasks[taskId].tEventSub = sub;
    gTasks[taskId].tEventCursor = 0;
    gTasks[taskId].tEventScroll = 0;
    RhDebugMenu_PrintFooter(sText_FooterEvents);
    RhDebugMenu_RedrawEvents(taskId);
    RhDebugMenu_GetEventList(folder, sub, &events, &eventCount);
    RhDebugMenu_CreateScrollbar(eventCount + 1, EVENT_LIST_SHOW);
}

static void RhDebugMenu_HandleEventFoldersInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowRoot(taskId);
        return;
    }

    PlaySE(SE_SELECT);
    RhDebugMenu_OpenEventCategory(taskId, (u8)input);
}

static void RhDebugMenu_HandleEventSubfoldersInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowEventFolders(taskId);
        return;
    }

    PlaySE(SE_SELECT);
    RhDebugMenu_ShowEvents(taskId, gTasks[taskId].tEventFolder, (u8)input);
}

static void RhDebugMenu_HandleEventsInput(u8 taskId)
{
    u8 cursor = gTasks[taskId].tEventCursor;
    const struct RhDebugEvent *events;
    u16 eventCount;
    u16 rowBack;
    bool8 enabled;

    RhDebugMenu_GetEventList(gTasks[taskId].tEventFolder, gTasks[taskId].tEventSub, &events, &eventCount);
    rowBack = eventCount;

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (RhDebugMenu_EventFolderHasSubs(gTasks[taskId].tEventFolder))
            RhDebugMenu_ShowEventSubfolders(taskId, gTasks[taskId].tEventFolder);
        else
            RhDebugMenu_ShowEventFolders(taskId);
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        if (cursor == rowBack)
        {
            PlaySE(SE_SELECT);
            if (RhDebugMenu_EventFolderHasSubs(gTasks[taskId].tEventFolder))
                RhDebugMenu_ShowEventSubfolders(taskId, gTasks[taskId].tEventFolder);
            else
                RhDebugMenu_ShowEventFolders(taskId);
        }
        else if (events != NULL)
        {
            enabled = FlagGet(events[cursor].flag);
            if (enabled)
            {
                FlagClear(events[cursor].flag);
                RhLogf(sText_LogEventOffFmt, events[cursor].name);
            }
            else
            {
                FlagSet(events[cursor].flag);
                RhLogf(sText_LogEventOnFmt, events[cursor].name);
            }
            PlaySE(SE_SELECT);
            RhDebugMenu_RedrawEvents(taskId);
        }
        return;
    }

    if (JOY_REPT(DPAD_UP))
    {
        if (cursor == 0)
            cursor = rowBack;
        else
            cursor--;
        gTasks[taskId].tEventCursor = cursor;
        PlaySE(SE_SELECT);
        RhDebugMenu_RedrawEvents(taskId);
        return;
    }

    if (JOY_REPT(DPAD_DOWN))
    {
        if (cursor >= rowBack)
            cursor = 0;
        else
            cursor++;
        gTasks[taskId].tEventCursor = cursor;
        PlaySE(SE_SELECT);
        RhDebugMenu_RedrawEvents(taskId);
        return;
    }

    if (cursor == rowBack || events == NULL)
        return;

    if (JOY_NEW(DPAD_LEFT) || JOY_NEW(DPAD_RIGHT))
    {
        enabled = FlagGet(events[cursor].flag);
        if (enabled)
        {
            FlagClear(events[cursor].flag);
            RhLogf(sText_LogEventOffFmt, events[cursor].name);
        }
        else
        {
            FlagSet(events[cursor].flag);
            RhLogf(sText_LogEventOnFmt, events[cursor].name);
        }
        PlaySE(SE_SELECT);
        RhDebugMenu_RedrawEvents(taskId);
    }
}

static void RhDebugMenu_ShowLog(u8 taskId)
{
    u16 count;

    RhDebugMenu_DestroyList(taskId);
    gTasks[taskId].tPage = PAGE_LOG;
    gTasks[taskId].tLogScroll = 0;
    RhDebugMenu_PrintFooter(sText_FooterLog);
    RhDebugMenu_RedrawLog(taskId);
    count = RhLog_GetCount();
    RhDebugMenu_CreateScrollbar(count, LOG_LIST_SHOW);
}

static void RhDebugMenu_RedrawLog(u8 taskId)
{
    u16 count = RhLog_GetCount();
    u16 scroll = gTasks[taskId].tLogScroll;
    u16 shown;
    u16 lineIdx;
    u8 y = 2;

    // scroll 0 = newest at top; higher scroll = older entries further down.
    if (count > LOG_LIST_SHOW)
    {
        if (scroll + LOG_LIST_SHOW > count)
            scroll = count - LOG_LIST_SHOW;
    }
    else
    {
        scroll = 0;
    }
    gTasks[taskId].tLogScroll = scroll;
    sScrollbarScroll = scroll;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, 0x1C0, 14);

    if (count == 0)
    {
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 8, 40, sTextColors, TEXT_SKIP_DRAW, sText_LogEmpty);
    }
    else
    {
        for (shown = 0; shown < LOG_LIST_SHOW && scroll + shown < count; shown++)
        {
            lineIdx = count - 1 - scroll - shown;
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 4, y, sTextColors, TEXT_SKIP_DRAW, RhLog_GetLine(lineIdx));
            y += 14;
        }
    }

    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void RhDebugMenu_HandleLogInput(u8 taskId)
{
    u16 count = RhLog_GetCount();
    u16 scroll = gTasks[taskId].tLogScroll;

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowRoot(taskId);
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SUCCESS);
        RhLog_Clear();
        gTasks[taskId].tLogScroll = 0;
        RhDebugMenu_DestroyScrollbar();
        RhDebugMenu_RedrawLog(taskId);
        return;
    }

    if (JOY_REPT(DPAD_UP))
    {
        if (scroll > 0)
        {
            gTasks[taskId].tLogScroll = scroll - 1;
            PlaySE(SE_SELECT);
            RhDebugMenu_RedrawLog(taskId);
        }
    }
    else if (JOY_REPT(DPAD_DOWN))
    {
        if (count > LOG_LIST_SHOW && scroll + LOG_LIST_SHOW < count)
        {
            gTasks[taskId].tLogScroll = scroll + 1;
            PlaySE(SE_SELECT);
            RhDebugMenu_RedrawLog(taskId);
        }
    }
}

static void RhDebugMenu_HandleRootInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    switch (input)
    {
    case DEBUG_MENU_CLOSE:
        RhDebugMenu_BeginClose(taskId);
        break;
    case DEBUG_MENU_GIVE:
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowGiveFolders(taskId);
        break;
    case DEBUG_MENU_WARP:
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowWarpFolders(taskId);
        break;
    case DEBUG_MENU_CHEATS:
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowCheatFolders(taskId);
        break;
    case DEBUG_MENU_LOG:
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowLog(taskId);
        break;
    case DEBUG_MENU_EVENTS:
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowEventFolders(taskId);
        break;
    }
}

static void RhDebugMenu_HandleCheatFoldersInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowRoot(taskId);
        return;
    }

    PlaySE(SE_SELECT);
    if (input == CHEAT_ACTION_RESET_ALL)
        RhDebugMenu_ShowCheatResetConfirm(taskId);
    else
        RhDebugMenu_ShowCheats(taskId, input);
}

static void RhDebugMenu_HandleCheatResetConfirmInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowCheatFolders(taskId);
        return;
    }

    // YES
    PlaySE(SE_SUCCESS);
    RhDebugMenu_ResetAllCheats();
    RhDebugMenu_ShowCheatFolders(taskId);
}

static void RhDebugMenu_HandleCheatsInput(u8 taskId)
{
    u8 cursor = gTasks[taskId].tGiveSub;
    u8 expIdx;
    const u8 *ids;
    u8 cheatCount;
    u8 rowBack;
    u8 cheatId;

    RhDebugMenu_GetCheatFolder(gTasks[taskId].tCheatFolder, &ids, &cheatCount);
    rowBack = cheatCount;

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowCheatFolders(taskId);
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        if (cursor == rowBack)
        {
            PlaySE(SE_SELECT);
            RhDebugMenu_ShowCheatFolders(taskId);
        }
        else
        {
            cheatId = ids[cursor];
            if (cheatId >= CHEAT_ACTION_RESET_ALL)
            {
                RhDebugMenu_RunCheatAction(cheatId);
            }
            else
            {
                PlaySE(SE_SELECT);
                if (cheatId == CHEAT_EXP_MULT)
                {
                    expIdx = RhDebugMenu_GetExpMultIndex() + 1;
                    if (expIdx >= CHEAT_EXP_MULT_COUNT)
                        expIdx = 0;
                    RhDebugMenu_SetExpMultIndex(expIdx);
                }
                else
                {
                    RhDebugMenu_SetCheat(cheatId, !FlagGet(sCheatFlags[cheatId]));
                }
                RhDebugMenu_RedrawCheats(taskId);
            }
        }
        return;
    }

    if (JOY_REPT(DPAD_UP))
    {
        if (cursor == 0)
            cursor = rowBack;
        else
            cursor--;
        gTasks[taskId].tGiveSub = cursor;
        PlaySE(SE_SELECT);
        RhDebugMenu_RedrawCheats(taskId);
        return;
    }

    if (JOY_REPT(DPAD_DOWN))
    {
        if (cursor >= rowBack)
            cursor = 0;
        else
            cursor++;
        gTasks[taskId].tGiveSub = cursor;
        PlaySE(SE_SELECT);
        RhDebugMenu_RedrawCheats(taskId);
        return;
    }

    if (cursor == rowBack)
        return;

    if (JOY_NEW(DPAD_LEFT) || JOY_NEW(DPAD_RIGHT))
    {
        cheatId = ids[cursor];
        if (cheatId >= CHEAT_ACTION_RESET_ALL)
            return;
        PlaySE(SE_SELECT);
        if (cheatId == CHEAT_EXP_MULT)
        {
            expIdx = RhDebugMenu_GetExpMultIndex();
            if (JOY_NEW(DPAD_LEFT))
            {
                if (expIdx == 0)
                    expIdx = CHEAT_EXP_MULT_COUNT - 1;
                else
                    expIdx--;
            }
            else
            {
                expIdx++;
                if (expIdx >= CHEAT_EXP_MULT_COUNT)
                    expIdx = 0;
            }
            RhDebugMenu_SetExpMultIndex(expIdx);
        }
        else
        {
            RhDebugMenu_SetCheat(cheatId, !FlagGet(sCheatFlags[cheatId]));
        }
        RhDebugMenu_RedrawCheats(taskId);
    }
}

static void RhDebugMenu_HandleGiveFoldersInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowRoot(taskId);
        return;
    }

    PlaySE(SE_SELECT);
    RhDebugMenu_OpenGiveCategory(taskId, (u8)input);
}

static void RhDebugMenu_HandleGiveSubfoldersInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowGiveFolders(taskId);
        return;
    }

    PlaySE(SE_SELECT);
    RhDebugMenu_ShowGiveItems(taskId, gTasks[taskId].tGiveFolder, (u8)input);
}

static void RhDebugMenu_HandleGiveItemsInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        if (RhDebugMenu_GiveFolderHasSubs(gTasks[taskId].tGiveFolder))
            RhDebugMenu_ShowGiveSubfolders(taskId, gTasks[taskId].tGiveFolder);
        else
            RhDebugMenu_ShowGiveFolders(taskId);
        return;
    }

    PlaySE(SE_SELECT);
    sGiveItemId = (u16)input;
    if (sGiveQty == 0)
        sGiveQty = 1;
    RhDebugMenu_ShowGiveQty(taskId);
}

static void RhDebugMenu_HandleGiveQtyInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowGiveItems(taskId, gTasks[taskId].tGiveFolder, gTasks[taskId].tGiveSub);
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        RhDebugMenu_TryGiveItem();
        return;
    }

    if (JOY_REPT(DPAD_UP))
    {
        RhDebugMenu_AdjustGiveQty(1);
        PlaySE(SE_SELECT);
        RhDebugMenu_RedrawGiveQty(taskId);
    }
    else if (JOY_REPT(DPAD_DOWN))
    {
        RhDebugMenu_AdjustGiveQty(-1);
        PlaySE(SE_SELECT);
        RhDebugMenu_RedrawGiveQty(taskId);
    }
}

static void RhDebugMenu_HandleWarpFoldersInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowRoot(taskId);
        return;
    }

    PlaySE(SE_SELECT);
    if (input == WARP_FOLDER_ROUTES)
        RhDebugMenu_ShowWarpRouteGroups(taskId);
    else
        RhDebugMenu_ShowWarpDest(taskId, (u8)input);
}

static void RhDebugMenu_HandleWarpRouteGroupsInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        RhDebugMenu_ShowWarpFolders(taskId);
        return;
    }

    PlaySE(SE_SELECT);
    RhDebugMenu_ShowWarpRouteDest(taskId, (u8)input);
}

static void RhDebugMenu_HandleWarpDestInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        if (gTasks[taskId].tWarpFolder == WARP_FOLDER_ROUTES)
            RhDebugMenu_ShowWarpRouteGroups(taskId);
        else
            RhDebugMenu_ShowWarpFolders(taskId);
        return;
    }

    RhDebugMenu_StartWarp(taskId, (u8)input);
}

static void Task_RhDebugMenu(u8 taskId)
{
    switch (gTasks[taskId].tState)
    {
    case STATE_FADE_IN:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gTasks[taskId].tState = STATE_WAIT_FADE_IN;
        break;
    case STATE_WAIT_FADE_IN:
        if (!gPaletteFade.active)
            gTasks[taskId].tState = STATE_HANDLE_INPUT;
        break;
    case STATE_HANDLE_INPUT:
        if (JOY_NEW(L_BUTTON))
        {
            RhDebugMenu_BeginClose(taskId);
            break;
        }

        switch (gTasks[taskId].tPage)
        {
        case PAGE_ROOT:
            RhDebugMenu_HandleRootInput(taskId);
            break;
        case PAGE_GIVE_FOLDERS:
            RhDebugMenu_HandleGiveFoldersInput(taskId);
            break;
        case PAGE_GIVE_SUBFOLDERS:
            RhDebugMenu_HandleGiveSubfoldersInput(taskId);
            break;
        case PAGE_GIVE_ITEMS:
            RhDebugMenu_HandleGiveItemsInput(taskId);
            break;
        case PAGE_GIVE_QTY:
            RhDebugMenu_HandleGiveQtyInput(taskId);
            break;
        case PAGE_WARP_FOLDERS:
            RhDebugMenu_HandleWarpFoldersInput(taskId);
            break;
        case PAGE_WARP_ROUTE_GROUPS:
            RhDebugMenu_HandleWarpRouteGroupsInput(taskId);
            break;
        case PAGE_WARP_DEST:
            RhDebugMenu_HandleWarpDestInput(taskId);
            break;
        case PAGE_CHEAT_FOLDERS:
            RhDebugMenu_HandleCheatFoldersInput(taskId);
            break;
        case PAGE_CHEATS:
            RhDebugMenu_HandleCheatsInput(taskId);
            break;
        case PAGE_CHEAT_RESET_CONFIRM:
            RhDebugMenu_HandleCheatResetConfirmInput(taskId);
            break;
        case PAGE_EVENT_FOLDERS:
            RhDebugMenu_HandleEventFoldersInput(taskId);
            break;
        case PAGE_EVENT_SUBFOLDERS:
            RhDebugMenu_HandleEventSubfoldersInput(taskId);
            break;
        case PAGE_EVENTS:
            RhDebugMenu_HandleEventsInput(taskId);
            break;
        case PAGE_LOG:
            RhDebugMenu_HandleLogInput(taskId);
            break;
        }
        RhDebugMenu_UpdateScrollbar(taskId);
        break;
    case STATE_WAIT_FADE_OUT:
        if (!gPaletteFade.active)
            gTasks[taskId].tState = STATE_EXIT;
        break;
    case STATE_EXIT:
        RhDebugMenu_DestroyList(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
        DestroyTask(taskId);
        break;
    case STATE_EXIT_WARP:
        if (!gPaletteFade.active)
        {
            u8 folder = gTasks[taskId].tWarpFolder;
            u8 destId = gTasks[taskId].tWarpDest;

            RhDebugMenu_DestroyList(taskId);
            FreeAllWindowBuffers();
            DestroyTask(taskId);

            RhDebugMenu_ApplyWarp(folder, destId);
            Overworld_ResetStateAfterTeleport();
            WarpIntoMap();
            gFieldCallback = FieldCB_DefaultWarpExit;
            SetMainCallback2(CB2_LoadMap);
        }
        break;
    }
}

static void InitRhDebugMenuGfx(void)
{
    u8 taskId;

    SetVBlankCallback(NULL);
    SetHBlankCallback(NULL);

    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaClear32(3, (void *)OAM, OAM_SIZE);
    DmaClear16(3, (void *)PLTT, PLTT_SIZE);

    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    InitWindows(sWinTemplates);
    DeactivateAllTextPrinters();

    ResetPaletteFade();
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ScanlineEffect_Stop();

    LoadStdWindowGfx(WIN_LIST, 0x1C0, BG_PLTT_ID(14));
    // Same std palette on 15 so list text and scrollbar can use frame colors (e.g. border 13).
    LoadPalette(GetTextWindowPalette(3), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(0);

    ShowBg(0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG0_ON);
    SetVBlankCallback(VBlankCB_RhDebugMenu);

    taskId = CreateTask(Task_RhDebugMenu, 0);
    gTasks[taskId].tState = STATE_FADE_IN;
    gTasks[taskId].tListTaskId = LIST_NONE;
    sScrollbarTaskId = SCROLLBAR_NONE;
    RhLog(sText_LogOpened);
    RhDebugMenu_ShowRoot(taskId);
    SetMainCallback2(CB2_RhDebugMenu);
}

void CB2_OpenRhDebugMenu(void)
{
    if (gMain.savedCallback == NULL)
        gMain.savedCallback = CB2_ReturnToField;
    InitRhDebugMenuGfx();
}

void RhDebugMenu_SyncKeyItem(void)
{
#if RH_DEBUG_MENU
    if (!CheckBagHasItem(ITEM_DEBUG_MENU, 1))
        AddBagItem(ITEM_DEBUG_MENU, 1);
#else
    if (CheckBagHasItem(ITEM_DEBUG_MENU, 1))
        RemoveBagItem(ITEM_DEBUG_MENU, 1);
    if (gSaveBlock1Ptr->registeredItem == ITEM_DEBUG_MENU)
        gSaveBlock1Ptr->registeredItem = ITEM_NONE;
#endif
}
