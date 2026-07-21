#!/usr/bin/env python3
"""Generate src/data/rh_debug_events.h for the RH debug Events menu."""

from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "src" / "data" / "rh_debug_events.h"

# Shared description keys -> short _("...") text
DESCS = {
    "badge": "Gym badge",
    "defeat": "Defeated",
    "hide": "Hide object",
    "got": "Got item",
    "sys": "System flag",
    "fought": "Fought",
    "flew": "Flew away",
    "door": "Door unlocked",
    "boulder": "Boulder hide",
    "current": "Current stopped",
    "map": "Map unlock",
    "tutor": "Move tutor",
    "trade": "In-game trade",
    "quiz": "Gym quiz",
    "shop": "Shop progress",
}


def E(flag: str, name: str, desc: str) -> tuple[str, str, str]:
    """(FLAG_*, display name, desc key)."""
    assert desc in DESCS, desc
    return (flag, name, desc)


# ---------------------------------------------------------------------------
# Event arrays (no duplicate flags across arrays)
# ---------------------------------------------------------------------------

EVENTS: dict[str, list[tuple[str, str, str]]] = {}

EVENTS["early"] = [
    E("FLAG_SYS_POKEMON_GET", "HAS POKEMON", "sys"),
    E("FLAG_SYS_POKEDEX_GET", "HAS POKEDEX", "sys"),
    E("FLAG_SYS_B_DASH", "B DASH", "sys"),
    E("FLAG_HIDE_BULBASAUR_BALL", "HIDE BULBASAUR BALL", "hide"),
    E("FLAG_HIDE_SQUIRTLE_BALL", "HIDE SQUIRTLE BALL", "hide"),
    E("FLAG_HIDE_CHARMANDER_BALL", "HIDE CHARMANDER BALL", "hide"),
    E("FLAG_HIDE_OAK_IN_HIS_LAB", "HIDE OAK IN LAB", "hide"),
    E("FLAG_HIDE_OAK_IN_PALLET_TOWN", "HIDE OAK IN PALLET", "hide"),
    E("FLAG_HIDE_RIVAL_IN_LAB", "HIDE RIVAL IN LAB", "hide"),
    E("FLAG_HIDE_POKEDEX", "HIDE POKEDEX", "hide"),
    E("FLAG_HIDE_TOWN_MAP", "HIDE TOWN MAP", "hide"),
    E("FLAG_VISITED_OAKS_LAB", "VISITED OAKS LAB", "sys"),
    E("FLAG_BEAT_RIVAL_IN_OAKS_LAB", "BEAT RIVAL IN LAB", "defeat"),
    E("FLAG_GOT_POKEBALLS_FROM_OAK_AFTER_22_RIVAL", "GOT BALLS AFTER 22", "got"),
    E("FLAG_OAK_SKIP_22_RIVAL_CHECK", "OAK SKIP 22 CHECK", "sys"),
    E("FLAG_PALLET_LADY_NOT_BLOCKING_SIGN", "PALLET LADY MOVED", "sys"),
    E("FLAG_HIDE_VIRIDIAN_CITY_TUTORIAL_MAN", "HIDE VIRIDIAN BLOCK", "hide"),
    E("FLAG_HIDE_ROUTE_22_RIVAL", "HIDE ROUTE 22 RIVAL", "hide"),
    E("FLAG_HIDE_PEWTER_CITY_GYM_GUIDE", "HIDE PEWTER GYM GUIDE", "hide"),
    E("FLAG_HIDE_PEWTER_MUSEUM_GUIDE", "HIDE PEWTER MUSEUM", "hide"),
    E("FLAG_HIDE_PEWTER_CITY_RUNNING_SHOES_GUY", "HIDE RUNNING SHOES", "hide"),
    E("FLAG_GOT_POTION_ON_ROUTE_1", "GOT ROUTE 1 POTION", "got"),
]

EVENTS["badges"] = [
    E("FLAG_BADGE01_GET", "BADGE 1 BROCK", "badge"),
    E("FLAG_BADGE02_GET", "BADGE 2 MISTY", "badge"),
    E("FLAG_BADGE03_GET", "BADGE 3 SURGE", "badge"),
    E("FLAG_BADGE04_GET", "BADGE 4 ERIKA", "badge"),
    E("FLAG_BADGE05_GET", "BADGE 5 KOGA", "badge"),
    E("FLAG_BADGE06_GET", "BADGE 6 SABRINA", "badge"),
    E("FLAG_BADGE07_GET", "BADGE 7 BLAINE", "badge"),
    E("FLAG_BADGE08_GET", "BADGE 8 GIOVANNI", "badge"),
    E("FLAG_DEFEATED_BROCK", "DEFEATED BROCK", "defeat"),
    E("FLAG_DEFEATED_MISTY", "DEFEATED MISTY", "defeat"),
    E("FLAG_DEFEATED_LT_SURGE", "DEFEATED LT SURGE", "defeat"),
    E("FLAG_DEFEATED_ERIKA", "DEFEATED ERIKA", "defeat"),
    E("FLAG_DEFEATED_KOGA", "DEFEATED KOGA", "defeat"),
    E("FLAG_DEFEATED_SABRINA", "DEFEATED SABRINA", "defeat"),
    E("FLAG_DEFEATED_BLAINE", "DEFEATED BLAINE", "defeat"),
    E("FLAG_DEFEATED_LEADER_GIOVANNI", "DEFEATED GIOVANNI", "defeat"),
    E("FLAG_DEFEATED_LORELEI", "DEFEATED LORELEI", "defeat"),
    E("FLAG_DEFEATED_BRUNO", "DEFEATED BRUNO", "defeat"),
    E("FLAG_DEFEATED_AGATHA", "DEFEATED AGATHA", "defeat"),
    E("FLAG_DEFEATED_LANCE", "DEFEATED LANCE", "defeat"),
    E("FLAG_DEFEATED_CHAMP", "DEFEATED CHAMP", "defeat"),
    E("FLAG_SYS_GAME_CLEAR", "GAME CLEAR", "sys"),
    E("FLAG_HIDE_OAK_IN_CHAMP_ROOM", "HIDE OAK CHAMP ROOM", "hide"),
    E("FLAG_HIDE_POSTGAME_GOSSIPERS", "HIDE POSTGAME GOSSIP", "hide"),
    E("FLAG_HIDE_CERULEAN_CAVE_GUARD", "HIDE CERULEAN CAVE", "hide"),
]

EVENTS["hms"] = [
    E("FLAG_GOT_HM01", "GOT HM01 CUT", "got"),
    E("FLAG_GOT_HM02", "GOT HM02 FLY", "got"),
    E("FLAG_GOT_HM03", "GOT HM03 SURF", "got"),
    E("FLAG_GOT_HM04", "GOT HM04 STRENGTH", "got"),
    E("FLAG_GOT_HM05", "GOT HM05 FLASH", "got"),
    E("FLAG_GOT_HM06", "GOT HM06 ROCK SMASH", "got"),
    E("FLAG_GOT_BIKE_VOUCHER", "GOT BIKE VOUCHER", "got"),
    E("FLAG_GOT_BICYCLE", "GOT BICYCLE", "got"),
    E("FLAG_GOT_OLD_ROD", "GOT OLD ROD", "got"),
    E("FLAG_GOT_GOOD_ROD", "GOT GOOD ROD", "got"),
    E("FLAG_GOT_SUPER_ROD", "GOT SUPER ROD", "got"),
    E("FLAG_GOT_ITEMFINDER", "GOT ITEMFINDER", "got"),
    E("FLAG_GOT_COIN_CASE", "GOT COIN CASE", "got"),
    E("FLAG_GOT_VS_SEEKER", "GOT VS SEEKER", "got"),
    E("FLAG_GOT_FAME_CHECKER", "GOT FAME CHECKER", "got"),
    E("FLAG_GOT_POWDER_JAR", "GOT POWDER JAR", "got"),
    E("FLAG_SYS_GOT_BERRY_POUCH", "GOT BERRY POUCH", "sys"),
    E("FLAG_GOT_EEVEE", "GOT EEVEE", "got"),
    E("FLAG_HIDE_EEVEE_BALL", "HIDE EEVEE BALL", "hide"),
    E("FLAG_GOT_HITMON_FROM_DOJO", "GOT HITMON FROM DOJO", "got"),
    E("FLAG_HIDE_DOJO_HITMONLEE_BALL", "HIDE HITMONLEE BALL", "hide"),
    E("FLAG_HIDE_DOJO_HITMONCHAN_BALL", "HIDE HITMONCHAN BALL", "hide"),
    E("FLAG_BOUGHT_MAGIKARP", "BOUGHT MAGIKARP", "got"),
    E("FLAG_GOT_RECORD_SETTING_MAGIKARP", "GOT RECORD MAGIKARP", "got"),
    E("FLAG_GOT_POKE_FLUTE", "GOT POKE FLUTE", "got"),
    E("FLAG_GOT_TEA", "GOT TEA", "got"),
]

EVENTS["legends"] = [
    E("FLAG_HIDE_ARTICUNO", "HIDE ARTICUNO", "hide"),
    E("FLAG_FOUGHT_ARTICUNO", "FOUGHT ARTICUNO", "fought"),
    E("FLAG_HIDE_ZAPDOS", "HIDE ZAPDOS", "hide"),
    E("FLAG_FOUGHT_ZAPDOS", "FOUGHT ZAPDOS", "fought"),
    E("FLAG_HIDE_MOLTRES", "HIDE MOLTRES", "hide"),
    E("FLAG_FOUGHT_MOLTRES", "FOUGHT MOLTRES", "fought"),
    E("FLAG_HIDE_MEWTWO", "HIDE MEWTWO", "hide"),
    E("FLAG_FOUGHT_MEWTWO", "FOUGHT MEWTWO", "fought"),
    E("FLAG_HIDE_LUGIA", "HIDE LUGIA", "hide"),
    E("FLAG_FOUGHT_LUGIA", "FOUGHT LUGIA", "fought"),
    E("FLAG_LUGIA_FLEW_AWAY", "LUGIA FLEW AWAY", "flew"),
    E("FLAG_HIDE_HO_OH", "HIDE HO-OH", "hide"),
    E("FLAG_FOUGHT_HO_OH", "FOUGHT HO-OH", "fought"),
    E("FLAG_HO_OH_FLEW_AWAY", "HO-OH FLEW AWAY", "flew"),
    E("FLAG_HIDE_DEOXYS", "HIDE DEOXYS", "hide"),
    E("FLAG_FOUGHT_DEOXYS", "FOUGHT DEOXYS", "fought"),
    E("FLAG_DEOXYS_FLEW_AWAY", "DEOXYS FLEW AWAY", "flew"),
    E("FLAG_HIDE_BIRTH_ISLAND_METEORITE", "HIDE METEORITE", "hide"),
    E("FLAG_SYS_DEOXYS_AWAKENED", "DEOXYS AWAKENED", "sys"),
]

EVENTS["sysmisc"] = [
    E("FLAG_SYS_NOT_SOMEONES_PC", "NOT SOMEONES PC", "sys"),
    E("FLAG_SYS_RIBBON_GET", "GOT RIBBON", "sys"),
    E("FLAG_SYS_SAW_HELP_SYSTEM_INTRO", "HELP INTRO SEEN", "sys"),
    E("FLAG_OPENED_START_MENU", "OPENED START MENU", "sys"),
    E("FLAG_SYS_SET_TRAINER_CARD_PROFILE", "TRAINER CARD PROFILE", "sys"),
    E("FLAG_SYS_MYSTERY_GIFT_ENABLED", "MYSTERY GIFT ON", "sys"),
    E("FLAG_SYS_NUZLOCKE", "NUZLOCKE", "sys"),
    E("FLAG_SYS_ALL_EXP_SHARE", "ALL EXP SHARE", "sys"),
    E("FLAG_SYS_SHINY_CHARM", "SHINY CHARM SYS", "sys"),
]

# --- Kanto story ---
EVENTS["k_mtmoon"] = [
    E("FLAG_HIDE_DOME_FOSSIL", "HIDE DOME FOSSIL", "hide"),
    E("FLAG_HIDE_HELIX_FOSSIL", "HIDE HELIX FOSSIL", "hide"),
    E("FLAG_GOT_FOSSIL_FROM_MT_MOON", "GOT FOSSIL MT MOON", "got"),
    E("FLAG_GOT_DOME_FOSSIL", "GOT DOME FOSSIL", "got"),
    E("FLAG_GOT_HELIX_FOSSIL", "GOT HELIX FOSSIL", "got"),
    E("FLAG_HIDE_OLD_AMBER", "HIDE OLD AMBER", "hide"),
    E("FLAG_GOT_OLD_AMBER", "GOT OLD AMBER", "got"),
    E("FLAG_REVIVED_DOME", "REVIVED DOME", "sys"),
    E("FLAG_REVIVED_HELIX", "REVIVED HELIX", "sys"),
    E("FLAG_REVIVED_AMBER", "REVIVED AMBER", "sys"),
]

EVENTS["k_nugget"] = [
    E("FLAG_HIDE_NUGGET_BRIDGE_ROCKET", "HIDE NUGGET ROCKET", "hide"),
    E("FLAG_GOT_TM28_FROM_ROCKET", "GOT TM28 FROM ROCKET", "got"),
]

EVENTS["k_bill"] = [
    E("FLAG_HIDE_BILL_CLEFAIRY", "HIDE BILL CLEFAIRY", "hide"),
    E("FLAG_HIDE_BILL_HUMAN_SEA_COTTAGE", "HIDE BILL HUMAN", "hide"),
    E("FLAG_HELPED_BILL_IN_SEA_COTTAGE", "HELPED BILL", "sys"),
]

EVENTS["k_cerulean"] = [
    E("FLAG_HIDE_CERULEAN_ROCKET", "HIDE CERULEAN ROCKET", "hide"),
    E("FLAG_HIDE_CERULEAN_RIVAL", "HIDE CERULEAN RIVAL", "hide"),
]

EVENTS["k_anne"] = [
    E("FLAG_HIDE_SS_ANNE_RIVAL", "HIDE SS ANNE RIVAL", "hide"),
    E("FLAG_HIDE_SS_ANNE", "HIDE SS ANNE", "hide"),
    E("FLAG_GOT_SS_TICKET", "GOT SS TICKET", "got"),
    E("FLAG_FOUND_BOTH_VERMILION_GYM_SWITCHES", "VERMILION SWITCHES", "sys"),
]

EVENTS["k_rockets"] = [
    E("FLAG_HIDE_GAME_CORNER_ROCKET", "HIDE GAME CORNER", "hide"),
    E("FLAG_HIDE_CELADON_ROCKETS", "HIDE CELADON ROCKETS", "hide"),
    E("FLAG_HIDE_LIFT_KEY", "HIDE LIFT KEY", "hide"),
    E("FLAG_HIDE_SILPH_SCOPE", "HIDE SILPH SCOPE", "hide"),
    E("FLAG_HIDE_HIDEOUT_GIOVANNI", "HIDE HIDEOUT GIOVANNI", "hide"),
    E("FLAG_OPENED_ROCKET_HIDEOUT", "OPENED ROCKET HIDEOUT", "sys"),
    E("FLAG_CAN_USE_ROCKET_HIDEOUT_LIFT", "ROCKET LIFT WORKS", "sys"),
    E("FLAG_HIDE_TOWER_ROCKET_1", "HIDE TOWER ROCKET 1", "hide"),
    E("FLAG_HIDE_TOWER_ROCKET_2", "HIDE TOWER ROCKET 2", "hide"),
    E("FLAG_HIDE_TOWER_ROCKET_3", "HIDE TOWER ROCKET 3", "hide"),
    E("FLAG_GOT_10_COINS_FROM_GAMBLER", "GOT 10 COINS", "got"),
    E("FLAG_GOT_20_COINS_FROM_GAMBLER", "GOT 20 COINS", "got"),
    E("FLAG_GOT_20_COINS_FROM_GAMBLER_2", "GOT 20 COINS 2", "got"),
]

EVENTS["k_fuji"] = [
    E("FLAG_HIDE_TOWER_FUJI", "HIDE TOWER FUJI", "hide"),
    E("FLAG_HIDE_POKEHOUSE_FUJI", "HIDE POKEHOUSE FUJI", "hide"),
    E("FLAG_HIDE_TOWER_RIVAL", "HIDE TOWER RIVAL", "hide"),
    E("FLAG_RESCUED_MR_FUJI", "RESCUED MR FUJI", "sys"),
]

EVENTS["k_snorlax"] = [
    E("FLAG_HIDE_ROUTE_12_SNORLAX", "HIDE R12 SNORLAX", "hide"),
    E("FLAG_WOKE_UP_ROUTE_12_SNORLAX", "WOKE R12 SNORLAX", "sys"),
    E("FLAG_HIDE_ROUTE_16_SNORLAX", "HIDE R16 SNORLAX", "hide"),
]

EVENTS["k_silph"] = [
    E("FLAG_HIDE_SAFFRON_ROCKETS", "HIDE SAFFRON ROCKETS", "hide"),
    E("FLAG_HIDE_SAFFRON_CIVILIANS", "HIDE SAFFRON CIVILIANS", "hide"),
    E("FLAG_HIDE_SILPH_ROCKETS", "HIDE SILPH ROCKETS", "hide"),
    E("FLAG_HIDE_SILPH_RIVAL", "HIDE SILPH RIVAL", "hide"),
    E("FLAG_GOT_LAPRAS_FROM_SILPH", "GOT LAPRAS", "got"),
    E("FLAG_GOT_MASTER_BALL_FROM_SILPH", "GOT MASTER BALL", "got"),
    E("FLAG_SILPH_2F_DOOR_1", "SILPH 2F DOOR 1", "door"),
    E("FLAG_SILPH_2F_DOOR_2", "SILPH 2F DOOR 2", "door"),
    E("FLAG_SILPH_3F_DOOR_1", "SILPH 3F DOOR 1", "door"),
    E("FLAG_SILPH_3F_DOOR_2", "SILPH 3F DOOR 2", "door"),
    E("FLAG_SILPH_4F_DOOR_1", "SILPH 4F DOOR 1", "door"),
    E("FLAG_SILPH_4F_DOOR_2", "SILPH 4F DOOR 2", "door"),
    E("FLAG_SILPH_5F_DOOR_1", "SILPH 5F DOOR 1", "door"),
    E("FLAG_SILPH_5F_DOOR_2", "SILPH 5F DOOR 2", "door"),
    E("FLAG_SILPH_5F_DOOR_3", "SILPH 5F DOOR 3", "door"),
    E("FLAG_SILPH_6F_DOOR", "SILPH 6F DOOR", "door"),
    E("FLAG_SILPH_7F_DOOR_1", "SILPH 7F DOOR 1", "door"),
    E("FLAG_SILPH_7F_DOOR_2", "SILPH 7F DOOR 2", "door"),
    E("FLAG_SILPH_7F_DOOR_3", "SILPH 7F DOOR 3", "door"),
    E("FLAG_SILPH_8F_DOOR", "SILPH 8F DOOR", "door"),
    E("FLAG_SILPH_9F_DOOR_1", "SILPH 9F DOOR 1", "door"),
    E("FLAG_SILPH_9F_DOOR_2", "SILPH 9F DOOR 2", "door"),
    E("FLAG_SILPH_9F_DOOR_3", "SILPH 9F DOOR 3", "door"),
    E("FLAG_SILPH_9F_DOOR_4", "SILPH 9F DOOR 4", "door"),
    E("FLAG_SILPH_10F_DOOR", "SILPH 10F DOOR", "door"),
    E("FLAG_SILPH_11F_DOOR", "SILPH 11F DOOR", "door"),
]

EVENTS["k_plant"] = [
    E("FLAG_HIDE_POWER_PLANT_ELECTRODE_1", "HIDE ELECTRODE 1", "hide"),
    E("FLAG_HIDE_POWER_PLANT_ELECTRODE_2", "HIDE ELECTRODE 2", "hide"),
    E("FLAG_FOUGHT_POWER_PLANT_ELECTRODE_1", "FOUGHT ELECTRODE 1", "fought"),
    E("FLAG_FOUGHT_POWER_PLANT_ELECTRODE_2", "FOUGHT ELECTRODE 2", "fought"),
]

EVENTS["k_seafoam"] = [
    E("FLAG_HIDE_SEAFOAM_1F_BOULDER_1", "SEAFOAM 1F B1", "boulder"),
    E("FLAG_HIDE_SEAFOAM_1F_BOULDER_2", "SEAFOAM 1F B2", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B1F_BOULDER_1", "SEAFOAM B1F B1", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B1F_BOULDER_2", "SEAFOAM B1F B2", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B2F_BOULDER_1", "SEAFOAM B2F B1", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B2F_BOULDER_2", "SEAFOAM B2F B2", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B3F_BOULDER_1", "SEAFOAM B3F B1", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B3F_BOULDER_2", "SEAFOAM B3F B2", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B3F_BOULDER_3", "SEAFOAM B3F B3", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B3F_BOULDER_4", "SEAFOAM B3F B4", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B3F_BOULDER_5", "SEAFOAM B3F B5", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B3F_BOULDER_6", "SEAFOAM B3F B6", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B4F_BOULDER_1", "SEAFOAM B4F B1", "boulder"),
    E("FLAG_HIDE_SEAFOAM_B4F_BOULDER_2", "SEAFOAM B4F B2", "boulder"),
    E("FLAG_STOPPED_SEAFOAM_B3F_CURRENT", "SEAFOAM B3F CURRENT", "current"),
    E("FLAG_STOPPED_SEAFOAM_B4F_CURRENT", "SEAFOAM B4F CURRENT", "current"),
]

EVENTS["k_cinnabar"] = [
    E("FLAG_HIDE_CINNABAR_BILL", "HIDE CINNABAR BILL", "hide"),
    E("FLAG_HIDE_CINNABAR_SEAGALLOP", "HIDE CINNABAR BOAT", "hide"),
    E("FLAG_HIDE_CINNABAR_POKECENTER_BILL", "HIDE CINNABAR PC BILL", "hide"),
    E("FLAG_POKEMON_MANSION_SWITCH_STATE", "MANSION SWITCH", "sys"),
    E("FLAG_CINNABAR_GYM_QUIZ_1", "CINNABAR QUIZ 1", "quiz"),
    E("FLAG_CINNABAR_GYM_QUIZ_2", "CINNABAR QUIZ 2", "quiz"),
    E("FLAG_CINNABAR_GYM_QUIZ_3", "CINNABAR QUIZ 3", "quiz"),
    E("FLAG_CINNABAR_GYM_QUIZ_4", "CINNABAR QUIZ 4", "quiz"),
    E("FLAG_CINNABAR_GYM_QUIZ_5", "CINNABAR QUIZ 5", "quiz"),
    E("FLAG_CINNABAR_GYM_QUIZ_6", "CINNABAR QUIZ 6", "quiz"),
]

EVENTS["k_viridian"] = [
    E("FLAG_HIDE_VIRIDIAN_GIOVANNI", "HIDE VIRIDIAN GIOVANNI", "hide"),
]

EVENTS["k_vr"] = [
    E("FLAG_HIDE_VICTORY_ROAD_2F_BOULDER", "VR 2F BOULDER", "boulder"),
    E("FLAG_HIDE_VICTORY_ROAD_3F_BOULDER", "VR 3F BOULDER", "boulder"),
]

EVENTS["k_miscrocket"] = [
    E("FLAG_HIDE_MISC_KANTO_ROCKETS", "HIDE MISC ROCKETS", "hide"),
]

# --- Side content ---
EVENTS["s_trades"] = [
    E("FLAG_DID_MIMIEN_TRADE", "TRADE MIMIEN", "trade"),
    E("FLAG_DID_ZYNX_TRADE", "TRADE ZYNX", "trade"),
    E("FLAG_DID_MS_NIDO_TRADE", "TRADE MS NIDO", "trade"),
    E("FLAG_DID_CH_DING_TRADE", "TRADE CH DING", "trade"),
    E("FLAG_DID_NINA_TRADE", "TRADE NINA", "trade"),
    E("FLAG_DID_MARC_TRADE", "TRADE MARC", "trade"),
    E("FLAG_DID_ESPHERE_TRADE", "TRADE ESPHERE", "trade"),
    E("FLAG_DID_TANGENY_TRADE", "TRADE TANGENY", "trade"),
    E("FLAG_DID_SEELOR_TRADE", "TRADE SEELOR", "trade"),
]

EVENTS["s_tutors"] = [
    E("FLAG_TUTOR_DOUBLE_EDGE", "TUTOR DOUBLE EDGE", "tutor"),
    E("FLAG_TUTOR_THUNDER_WAVE", "TUTOR THUNDER WAVE", "tutor"),
    E("FLAG_TUTOR_ROCK_SLIDE", "TUTOR ROCK SLIDE", "tutor"),
    E("FLAG_TUTOR_EXPLOSION", "TUTOR EXPLOSION", "tutor"),
    E("FLAG_TUTOR_MEGA_PUNCH", "TUTOR MEGA PUNCH", "tutor"),
    E("FLAG_TUTOR_MEGA_KICK", "TUTOR MEGA KICK", "tutor"),
    E("FLAG_TUTOR_DREAM_EATER", "TUTOR DREAM EATER", "tutor"),
    E("FLAG_TUTOR_SOFT_BOILED", "TUTOR SOFTBOILED", "tutor"),
    E("FLAG_TUTOR_SUBSTITUTE", "TUTOR SUBSTITUTE", "tutor"),
    E("FLAG_TUTOR_SWORDS_DANCE", "TUTOR SWORDS DANCE", "tutor"),
    E("FLAG_TUTOR_SEISMIC_TOSS", "TUTOR SEISMIC TOSS", "tutor"),
    E("FLAG_TUTOR_COUNTER", "TUTOR COUNTER", "tutor"),
    E("FLAG_TUTOR_METRONOME", "TUTOR METRONOME", "tutor"),
    E("FLAG_TUTOR_MIMIC", "TUTOR MIMIC", "tutor"),
    E("FLAG_TUTOR_BODY_SLAM", "TUTOR BODY SLAM", "tutor"),
    E("FLAG_TUTOR_FRENZY_PLANT", "TUTOR FRENZY PLANT", "tutor"),
    E("FLAG_TUTOR_BLAST_BURN", "TUTOR BLAST BURN", "tutor"),
    E("FLAG_TUTOR_HYDRO_CANNON", "TUTOR HYDRO CANNON", "tutor"),
    E("FLAG_LEARNED_ALL_MOVES_AT_CAPE_BRINK", "CAPE BRINK ALL", "tutor"),
]

EVENTS["s_gymtms"] = [
    E("FLAG_GOT_TM39_FROM_BROCK", "GOT TM39 BROCK", "got"),
    E("FLAG_GOT_TM03_FROM_MISTY", "GOT TM03 MISTY", "got"),
    E("FLAG_GOT_TM34_FROM_SURGE", "GOT TM34 SURGE", "got"),
    E("FLAG_GOT_TM19_FROM_ERIKA", "GOT TM19 ERIKA", "got"),
    E("FLAG_GOT_TM06_FROM_KOGA", "GOT TM06 KOGA", "got"),
    E("FLAG_GOT_TM04_FROM_SABRINA", "GOT TM04 SABRINA", "got"),
    E("FLAG_GOT_TM38_FROM_BLAINE", "GOT TM38 BLAINE", "got"),
    E("FLAG_GOT_TM26_FROM_GIOVANNI", "GOT TM26 GIOVANNI", "got"),
]

EVENTS["s_aides"] = [
    E("FLAG_HIDE_VERMILION_CITY_OAKS_AIDE", "HIDE VERMILION AIDE", "hide"),
    E("FLAG_TALKED_TO_OAKS_AIDE_IN_VERMILION", "TALKED VERMILION AIDE", "sys"),
    E("FLAG_GOT_EXP_SHARE_FROM_OAKS_AIDE", "GOT EXP SHARE AIDE", "got"),
    E("FLAG_GOT_EVERSTONE_FROM_OAKS_AIDE", "GOT EVERSTONE AIDE", "got"),
    E("FLAG_GOT_AMULET_COIN_FROM_OAKS_AIDE", "GOT AMULET COIN AIDE", "got"),
    E("FLAG_OAK_SAW_DEX_COMPLETION", "OAK SAW DEX DONE", "sys"),
    E("FLAG_GOT_SHINY_CHARM", "GOT SHINY CHARM", "got"),
]

EVENTS["s_fanclub"] = [
    E("FLAG_HIDE_SAFFRON_FAN_CLUB_BLACK_BELT", "HIDE FAN BLACK BELT", "hide"),
    E("FLAG_HIDE_SAFFRON_FAN_CLUB_ROCKER", "HIDE FAN ROCKER", "hide"),
    E("FLAG_HIDE_SAFFRON_FAN_CLUB_WOMAN", "HIDE FAN WOMAN", "hide"),
    E("FLAG_HIDE_SAFFRON_FAN_CLUB_BEAUTY", "HIDE FAN BEAUTY", "hide"),
    E("FLAG_GOT_TITAN", "GOT TITAN", "got"),
]

EVENTS["s_daycare"] = [
    E("FLAG_PENDING_DAYCARE_EGG", "PENDING DAYCARE EGG", "sys"),
]

EVENTS["s_misc"] = [
    E("FLAG_GOT_TM29_FROM_MR_PSYCHIC", "GOT TM29 PSYCHIC", "got"),
    E("FLAG_GOT_TM33_FROM_THIRSTY_GIRL", "GOT TM33 THIRSTY", "got"),
    E("FLAG_GOT_TM20_FROM_THIRSTY_GIRL", "GOT TM20 THIRSTY", "got"),
    E("FLAG_GOT_TM16_FROM_THIRSTY_GIRL", "GOT TM16 THIRSTY", "got"),
    E("FLAG_GOT_TM27", "GOT TM27", "got"),
    E("FLAG_TALKED_TO_TEA_LADY_AFTER_HOF", "TEA LADY AFTER HOF", "sys"),
    E("FLAG_MET_STICKER_MAN", "MET STICKER MAN", "sys"),
    E("FLAG_LEARNED_YES_NAH_CHANSEY", "YES NAH CHANSEY", "sys"),
]

# --- Sevii / postgame ---
EVENTS["v_sys"] = [
    E("FLAG_SYS_SEVII_MAP_123", "SEVII MAP 123", "sys"),
    E("FLAG_SYS_SEVII_MAP_4567", "SEVII MAP 4567", "sys"),
    E("FLAG_SYS_NATIONAL_DEX", "NATIONAL DEX", "sys"),
    E("FLAG_SYS_CAN_LINK_WITH_RS", "CAN LINK WITH RS", "sys"),
    E("FLAG_SYS_UNLOCKED_TANOBY_RUINS", "TANOBY RUINS UNLOCK", "sys"),
    E("FLAG_SEVII_DETOUR_FINISHED", "SEVII DETOUR DONE", "sys"),
]

EVENTS["v_tickets"] = [
    E("FLAG_RECEIVED_AURORA_TICKET", "GOT AURORA TICKET", "got"),
    E("FLAG_SHOWN_AURORA_TICKET", "SHOWN AURORA TICKET", "sys"),
    E("FLAG_RECEIVED_MYSTIC_TICKET", "GOT MYSTIC TICKET", "got"),
    E("FLAG_SHOWN_MYSTIC_TICKET", "SHOWN MYSTIC TICKET", "sys"),
    E("FLAG_RECEIVED_OLD_SEA_MAP", "GOT OLD SEA MAP", "got"),
    E("FLAG_HIDE_MG_DELIVERYMEN", "HIDE MG DELIVERYMEN", "hide"),
]

EVENTS["v_celio"] = [
    E("FLAG_HIDE_ONE_ISLAND_BILL", "HIDE 1I BILL", "hide"),
    E("FLAG_HIDE_ONE_ISLAND_POKECENTER_BILL", "HIDE 1I PC BILL", "hide"),
    E("FLAG_HIDE_ONE_ISLAND_POKECENTER_CELIO", "HIDE 1I PC CELIO", "hide"),
    E("FLAG_HIDE_RUBY", "HIDE RUBY", "hide"),
    E("FLAG_GOT_RUBY", "GOT RUBY", "got"),
    E("FLAG_HIDE_SAPPHIRE", "HIDE SAPPHIRE", "hide"),
    E("FLAG_RECOVERED_SAPPHIRE", "RECOVERED SAPPHIRE", "sys"),
    E("FLAG_HIDE_MT_EMBER_EXTERIOR_ROCKETS", "HIDE MT EMBER ROCKETS", "hide"),
]

EVENTS["v_lostelle"] = [
    E("FLAG_HIDE_TWO_ISLAND_GAME_CORNER_BIKER", "HIDE 2I CORNER BIKER", "hide"),
    E("FLAG_HIDE_TWO_ISLAND_GAME_CORNER_LOSTELLE", "HIDE 2I CORNER LOSTELLE", "hide"),
    E("FLAG_HIDE_LOSTELLE_IN_HER_HOME", "HIDE LOSTELLE HOME", "hide"),
    E("FLAG_HIDE_LOSTELLE_IN_BERRY_FOREST", "HIDE LOSTELLE FOREST", "hide"),
    E("FLAG_HIDE_THREE_ISLAND_BIKERS", "HIDE 3I BIKERS", "hide"),
    E("FLAG_HIDE_THREE_ISLAND_ANTIBIKERS", "HIDE 3I ANTIBIKERS", "hide"),
    E("FLAG_HIDE_THREE_ISLAND_LONE_BIKER", "HIDE 3I LONE BIKER", "hide"),
    E("FLAG_HIDE_TWO_ISLAND_WOMAN", "HIDE 2I WOMAN", "hide"),
    E("FLAG_HIDE_TWO_ISLAND_BEAUTY", "HIDE 2I BEAUTY", "hide"),
    E("FLAG_HIDE_TWO_ISLAND_POKE_MANIAC", "HIDE 2I MANIAC", "hide"),
    E("FLAG_RESCUED_LOSTELLE", "RESCUED LOSTELLE", "sys"),
    E("FLAG_VISITED_TWO_ISLAND", "VISITED TWO ISLAND", "sys"),
    E("FLAG_GOT_FULL_RESTORE_FROM_THREE_ISLAND_DEFENDER", "GOT 3I FULL RESTORE", "got"),
]

EVENTS["v_rockets"] = [
    E("FLAG_HIDE_FIVE_ISLAND_ROCKETS", "HIDE 5I ROCKETS", "hide"),
    E("FLAG_HIDE_ICEFALL_CAVE_LORELEI", "HIDE ICEFALL LORELEI", "hide"),
    E("FLAG_HIDE_LORELEI_IN_HER_HOUSE", "HIDE LORELEI HOUSE", "hide"),
    E("FLAG_HIDE_ICEFALL_CAVE_ROCKETS", "HIDE ICEFALL ROCKETS", "hide"),
    E("FLAG_DEFEATED_ROCKETS_IN_WAREHOUSE", "DEFEATED WAREHOUSE", "defeat"),
    E("FLAG_UNLOCKED_ROCKET_WAREHOUSE", "UNLOCKED WAREHOUSE", "sys"),
    E("FLAG_TALKED_TO_LORELEI_AFTER_WAREHOUSE", "LORELEI AFTER WH", "sys"),
]

EVENTS["v_selphy"] = [
    E("FLAG_HIDE_LOST_CAVE_SELPHY", "HIDE LOST CAVE SELPHY", "hide"),
    E("FLAG_HIDE_RESORT_GORGEOUS_SELPHY", "HIDE RESORT SELPHY", "hide"),
    E("FLAG_HIDE_RESORT_GORGEOUS_INSIDE_SELPHY", "HIDE INSIDE SELPHY", "hide"),
    E("FLAG_HIDE_SELPHYS_BUTLER", "HIDE SELPHY BUTLER", "hide"),
    E("FLAG_GOT_TOGEPI_EGG", "GOT TOGEPI EGG", "got"),
]

EVENTS["v_ruin"] = [
    E("FLAG_HIDE_RUIN_VALLEY_SCIENTIST", "HIDE RUIN SCIENTIST", "hide"),
    E("FLAG_HIDE_DOTTED_HOLE_SCIENTIST", "HIDE DOTTED SCIENTIST", "hide"),
    E("FLAG_USED_CUT_ON_RUIN_VALLEY_BRAILLE", "CUT RUIN BRAILLE", "sys"),
]

EVENTS["v_rivals"] = [
    E("FLAG_HIDE_FOUR_ISLAND_RIVAL", "HIDE 4I RIVAL", "hide"),
    E("FLAG_HIDE_SIX_ISLAND_POKECENTER_RIVAL", "HIDE 6I PC RIVAL", "hide"),
]

EVENTS["v_misc"] = [
    E("FLAG_TWO_ISLAND_SHOP_INTRODUCED", "2I SHOP INTRO", "shop"),
    E("FLAG_TWO_ISLAND_SHOP_EXPANDED_1", "2I SHOP EXPAND 1", "shop"),
    E("FLAG_TWO_ISLAND_SHOP_EXPANDED_2", "2I SHOP EXPAND 2", "shop"),
    E("FLAG_TWO_ISLAND_SHOP_EXPANDED_3", "2I SHOP EXPAND 3", "shop"),
    E("FLAG_GOT_TM42_AT_MEMORIAL_PILLAR", "GOT TM42 MEMORIAL", "got"),
    E("FLAG_GOT_MOON_STONE_FROM_JOYFUL_GAME_CORNER", "GOT JOYFUL MOON STONE", "got"),
    E("FLAG_GOT_NEST_BALL_FROM_WATER_PATH_HOUSE_1", "GOT NEST BALL WATER", "got"),
    E("FLAG_GOT_NUGGET_FROM_DUNSPARCE_TUNNEL", "GOT DUNSPARCE NUGGET", "got"),
]

# --- World map ---
EVENTS["w_towns"] = [
    E("FLAG_WORLD_MAP_PALLET_TOWN", "MAP PALLET", "map"),
    E("FLAG_WORLD_MAP_VIRIDIAN_CITY", "MAP VIRIDIAN", "map"),
    E("FLAG_WORLD_MAP_PEWTER_CITY", "MAP PEWTER", "map"),
    E("FLAG_WORLD_MAP_CERULEAN_CITY", "MAP CERULEAN", "map"),
    E("FLAG_WORLD_MAP_LAVENDER_TOWN", "MAP LAVENDER", "map"),
    E("FLAG_WORLD_MAP_VERMILION_CITY", "MAP VERMILION", "map"),
    E("FLAG_WORLD_MAP_CELADON_CITY", "MAP CELADON", "map"),
    E("FLAG_WORLD_MAP_FUCHSIA_CITY", "MAP FUCHSIA", "map"),
    E("FLAG_WORLD_MAP_CINNABAR_ISLAND", "MAP CINNABAR", "map"),
    E("FLAG_WORLD_MAP_INDIGO_PLATEAU_EXTERIOR", "MAP INDIGO", "map"),
    E("FLAG_WORLD_MAP_SAFFRON_CITY", "MAP SAFFRON", "map"),
    E("FLAG_WORLD_MAP_ROUTE4_POKEMON_CENTER_1F", "MAP ROUTE 4 PC", "map"),
    E("FLAG_WORLD_MAP_ROUTE10_POKEMON_CENTER_1F", "MAP ROUTE 10 PC", "map"),
]

EVENTS["w_dung"] = [
    E("FLAG_WORLD_MAP_VIRIDIAN_FOREST", "MAP VIRIDIAN FOREST", "map"),
    E("FLAG_WORLD_MAP_MT_MOON_1F", "MAP MT MOON", "map"),
    E("FLAG_WORLD_MAP_SSANNE_EXTERIOR", "MAP SS ANNE", "map"),
    E("FLAG_WORLD_MAP_UNDERGROUND_PATH_NORTH_SOUTH_TUNNEL", "MAP UG NS", "map"),
    E("FLAG_WORLD_MAP_UNDERGROUND_PATH_EAST_WEST_TUNNEL", "MAP UG EW", "map"),
    E("FLAG_WORLD_MAP_DIGLETTS_CAVE_B1F", "MAP DIGLETT CAVE", "map"),
    E("FLAG_WORLD_MAP_VICTORY_ROAD_1F", "MAP VICTORY ROAD", "map"),
    E("FLAG_WORLD_MAP_ROCKET_HIDEOUT_B1F", "MAP ROCKET HIDEOUT", "map"),
    E("FLAG_WORLD_MAP_SILPH_CO_1F", "MAP SILPH CO", "map"),
    E("FLAG_WORLD_MAP_POKEMON_MANSION_1F", "MAP MANSION", "map"),
    E("FLAG_WORLD_MAP_SAFARI_ZONE_CENTER", "MAP SAFARI ZONE", "map"),
    E("FLAG_WORLD_MAP_POKEMON_LEAGUE_LORELEIS_ROOM", "MAP LEAGUE", "map"),
    E("FLAG_WORLD_MAP_ROCK_TUNNEL_1F", "MAP ROCK TUNNEL", "map"),
    E("FLAG_WORLD_MAP_SEAFOAM_ISLANDS_1F", "MAP SEAFOAM", "map"),
    E("FLAG_WORLD_MAP_POKEMON_TOWER_1F", "MAP POKE TOWER", "map"),
    E("FLAG_WORLD_MAP_CERULEAN_CAVE_1F", "MAP CERULEAN CAVE", "map"),
    E("FLAG_WORLD_MAP_POWER_PLANT", "MAP POWER PLANT", "map"),
]

EVENTS["w_sevii"] = [
    E("FLAG_WORLD_MAP_ONE_ISLAND", "MAP ONE ISLAND", "map"),
    E("FLAG_WORLD_MAP_TWO_ISLAND", "MAP TWO ISLAND", "map"),
    E("FLAG_WORLD_MAP_THREE_ISLAND", "MAP THREE ISLAND", "map"),
    E("FLAG_WORLD_MAP_FOUR_ISLAND", "MAP FOUR ISLAND", "map"),
    E("FLAG_WORLD_MAP_FIVE_ISLAND", "MAP FIVE ISLAND", "map"),
    E("FLAG_WORLD_MAP_SIX_ISLAND", "MAP SIX ISLAND", "map"),
    E("FLAG_WORLD_MAP_SEVEN_ISLAND", "MAP SEVEN ISLAND", "map"),
    E("FLAG_WORLD_MAP_NAVEL_ROCK_EXTERIOR", "MAP NAVEL ROCK", "map"),
    E("FLAG_WORLD_MAP_MT_EMBER_EXTERIOR", "MAP MT EMBER", "map"),
    E("FLAG_WORLD_MAP_THREE_ISLAND_BERRY_FOREST", "MAP BERRY FOREST", "map"),
    E("FLAG_WORLD_MAP_FOUR_ISLAND_ICEFALL_CAVE_ENTRANCE", "MAP ICEFALL CAVE", "map"),
    E("FLAG_WORLD_MAP_FIVE_ISLAND_ROCKET_WAREHOUSE", "MAP ROCKET WH", "map"),
    E("FLAG_WORLD_MAP_TRAINER_TOWER_LOBBY", "MAP TRAINER TOWER", "map"),
    E("FLAG_WORLD_MAP_SIX_ISLAND_DOTTED_HOLE_1F", "MAP DOTTED HOLE", "map"),
    E("FLAG_WORLD_MAP_FIVE_ISLAND_LOST_CAVE_ENTRANCE", "MAP LOST CAVE", "map"),
    E("FLAG_WORLD_MAP_SIX_ISLAND_PATTERN_BUSH", "MAP PATTERN BUSH", "map"),
    E("FLAG_WORLD_MAP_SIX_ISLAND_ALTERING_CAVE", "MAP ALTERING CAVE", "map"),
    E("FLAG_WORLD_MAP_SEVEN_ISLAND_TANOBY_RUINS_MONEAN_CHAMBER", "MAP TANOBY", "map"),
    E("FLAG_WORLD_MAP_THREE_ISLAND_DUNSPARCE_TUNNEL", "MAP DUNSPARCE", "map"),
    E("FLAG_WORLD_MAP_SEVEN_ISLAND_SEVAULT_CANYON_TANOBY_KEY", "MAP TANOBY KEY", "map"),
    E("FLAG_WORLD_MAP_BIRTH_ISLAND_EXTERIOR", "MAP BIRTH ISLAND", "map"),
]

# Folder / subfolder taxonomy for menus
FOLDER_ORDER = [
    ("EARLY", "EARLY GAME", "early", None),
    ("KANTO", "KANTO STORY", None, "kanto"),
    ("BADGES", "BADGES / LEAGUE", "badges", None),
    ("HMS", "HMs / KEY ITEMS", "hms", None),
    ("SIDE", "SIDE CONTENT", None, "side"),
    ("LEGENDS", "LEGENDARIES", "legends", None),
    ("SEVII", "SEVII / POSTGAME", None, "sevii"),
    ("WORLDMAP", "WORLD MAP", None, "world"),
    ("SYSMISC", "SYS MISC", "sysmisc", None),
]

KANTO_SUBS = [
    ("MTMOON", "MT MOON / FOSSILS", "k_mtmoon"),
    ("NUGGET", "NUGGET BRIDGE", "k_nugget"),
    ("BILL", "BILL", "k_bill"),
    ("CERULEAN", "CERULEAN", "k_cerulean"),
    ("ANNE", "SS ANNE", "k_anne"),
    ("ROCKETS", "ROCKETS / CORNER", "k_rockets"),
    ("FUJI", "LAVENDER / FUJI", "k_fuji"),
    ("SNORLAX", "SNORLAX", "k_snorlax"),
    ("SILPH", "SILPH / SAFFRON", "k_silph"),
    ("PLANT", "POWER PLANT", "k_plant"),
    ("SEAFOAM", "SEAFOAM", "k_seafoam"),
    ("CINNABAR", "CINNABAR", "k_cinnabar"),
    ("VIRIDIAN", "VIRIDIAN GIOVANNI", "k_viridian"),
    ("VR", "VICTORY ROAD", "k_vr"),
    ("MISCROCKET", "MISC KANTO ROCKETS", "k_miscrocket"),
]

SIDE_SUBS = [
    ("TRADES", "IN-GAME TRADES", "s_trades"),
    ("TUTORS", "MOVE TUTORS", "s_tutors"),
    ("GYMTMS", "GYM LEADER TMs", "s_gymtms"),
    ("AIDES", "OAKS AIDES", "s_aides"),
    ("FANCLUB", "FAN CLUB", "s_fanclub"),
    ("DAYCARE", "DAY CARE", "s_daycare"),
    ("MISC", "MISC SIDE", "s_misc"),
]

SEVII_SUBS = [
    ("SYS", "SYS / UNLOCK", "v_sys"),
    ("TICKETS", "TICKETS / MG", "v_tickets"),
    ("CELIO", "ONE ISLAND / CELIO", "v_celio"),
    ("LOSTELLE", "LOSTELLE", "v_lostelle"),
    ("ROCKETS", "FIVE ISLAND ROCKETS", "v_rockets"),
    ("SELPHY", "SELPHY", "v_selphy"),
    ("RUIN", "RUIN / DOTTED HOLE", "v_ruin"),
    ("RIVALS", "ISLAND RIVALS", "v_rivals"),
    ("MISC", "SEVII MISC", "v_misc"),
]

WORLD_SUBS = [
    ("TOWNS", "KANTO TOWNS", "w_towns"),
    ("DUNG", "KANTO DUNGEONS", "w_dung"),
    ("SEVII", "SEVII", "w_sevii"),
]


def flag_to_ident(flag: str) -> str:
    assert flag.startswith("FLAG_")
    return flag[len("FLAG_") :]


def validate() -> None:
    seen: dict[str, str] = {}
    for arr_name, rows in EVENTS.items():
        for flag, _name, _desc in rows:
            if flag in seen:
                raise SystemExit(f"Duplicate flag {flag} in {seen[flag]} and {arr_name}")
            seen[flag] = arr_name

    flags_h = (ROOT / "include" / "constants" / "flags.h").read_text(encoding="utf-8", errors="replace")
    missing = [f for f in seen if f"#define {f}" not in flags_h and f"\n{f} " not in flags_h]
    # Allow aliases defined as FLAG_X FLAG_0x...
    still = []
    for f in missing:
        if f"#define {f}" not in flags_h:
            still.append(f)
    if still:
        raise SystemExit(f"Flags not found in flags.h: {still}")


def emit_events_array(lines: list[str], arr_key: str, c_name: str) -> None:
    rows = EVENTS[arr_key]
    lines.append(f"static const struct RhDebugEvent {c_name}[] =")
    lines.append("{")
    for flag, _name, desc in rows:
        ident = flag_to_ident(flag)
        lines.append(f"    {{{flag}, sEvtName_{ident}, sEvtDesc_{desc}}},")
    lines.append("};")
    lines.append("")


def main() -> None:
    validate()

    lines: list[str] = []
    lines.append("// Auto-generated by tools/gen_rh_debug_events.py — do not edit by hand.")
    lines.append("// Included only from rh_debug_menu.c.")
    lines.append("")
    lines.append("struct RhDebugEvent")
    lines.append("{")
    lines.append("    u16 flag;")
    lines.append("    const u8 *name;")
    lines.append("    const u8 *desc;")
    lines.append("};")
    lines.append("")
    lines.append("#define EVENT_SUB_NONE 0xFF")
    lines.append("")

    # Enums
    lines.append("enum")
    lines.append("{")
    for key, _label, _flat, _subs in FOLDER_ORDER:
        lines.append(f"    EVENT_FOLDER_{key},")
    lines.append("};")
    lines.append("")

    lines.append("enum")
    lines.append("{")
    for key, _label, _arr in KANTO_SUBS:
        lines.append(f"    EVENT_KANTO_{key},")
    lines.append("};")
    lines.append("")

    lines.append("enum")
    lines.append("{")
    for key, _label, _arr in SIDE_SUBS:
        lines.append(f"    EVENT_SIDE_{key},")
    lines.append("};")
    lines.append("")

    lines.append("enum")
    lines.append("{")
    for key, _label, _arr in SEVII_SUBS:
        lines.append(f"    EVENT_SEVII_{key},")
    lines.append("};")
    lines.append("")

    lines.append("enum")
    lines.append("{")
    for key, _label, _arr in WORLD_SUBS:
        lines.append(f"    EVENT_MAP_{key},")
    lines.append("};")
    lines.append("")

    # Shared desc strings
    for key, text in DESCS.items():
        lines.append(f'static const u8 sEvtDesc_{key}[] = _("{text}");')
    lines.append("")

    # Unique name strings
    for arr_name in EVENTS:
        for flag, name, _desc in EVENTS[arr_name]:
            ident = flag_to_ident(flag)
            # Escape nothing special; names are plain ASCII
            lines.append(f'static const u8 sEvtName_{ident}[] = _("{name}");')
    lines.append("")

    # Event arrays
    flat_cnames = {
        "early": "sEvents_early",
        "badges": "sEvents_badges",
        "hms": "sEvents_hms",
        "legends": "sEvents_legends",
        "sysmisc": "sEvents_sysmisc",
    }
    for key, cname in flat_cnames.items():
        emit_events_array(lines, key, cname)

    for _ek, _label, arr in KANTO_SUBS:
        emit_events_array(lines, arr, f"sEvents_{arr}")
    for _ek, _label, arr in SIDE_SUBS:
        emit_events_array(lines, arr, f"sEvents_{arr}")
    for _ek, _label, arr in SEVII_SUBS:
        emit_events_array(lines, arr, f"sEvents_{arr}")
    for _ek, _label, arr in WORLD_SUBS:
        emit_events_array(lines, arr, f"sEvents_{arr}")

    # Folder label strings
    for key, label, _flat, _subs in FOLDER_ORDER:
        lines.append(f'static const u8 sText_EvtFolder_{key}[] = _("{label}");')
    for key, label, _arr in KANTO_SUBS:
        lines.append(f'static const u8 sText_EvtKanto_{key}[] = _("{label}");')
    for key, label, _arr in SIDE_SUBS:
        lines.append(f'static const u8 sText_EvtSide_{key}[] = _("{label}");')
    for key, label, _arr in SEVII_SUBS:
        lines.append(f'static const u8 sText_EvtSevii_{key}[] = _("{label}");')
    for key, label, _arr in WORLD_SUBS:
        lines.append(f'static const u8 sText_EvtMap_{key}[] = _("{label}");')
    lines.append("")

    # List menus (sText_Back exists in rh_debug_menu.c before include)
    lines.append("static const struct ListMenuItem sEventFolderItems[] =")
    lines.append("{")
    for key, _label, _flat, _subs in FOLDER_ORDER:
        lines.append(f"    {{sText_EvtFolder_{key}, EVENT_FOLDER_{key}}},")
    lines.append("    {sText_Back, LIST_CANCEL},")
    lines.append("};")
    lines.append("")

    lines.append("static const struct ListMenuItem sEventKantoItems[] =")
    lines.append("{")
    for key, _label, _arr in KANTO_SUBS:
        lines.append(f"    {{sText_EvtKanto_{key}, EVENT_KANTO_{key}}},")
    lines.append("    {sText_Back, LIST_CANCEL},")
    lines.append("};")
    lines.append("")

    lines.append("static const struct ListMenuItem sEventSideItems[] =")
    lines.append("{")
    for key, _label, _arr in SIDE_SUBS:
        lines.append(f"    {{sText_EvtSide_{key}, EVENT_SIDE_{key}}},")
    lines.append("    {sText_Back, LIST_CANCEL},")
    lines.append("};")
    lines.append("")

    lines.append("static const struct ListMenuItem sEventSeviiItems[] =")
    lines.append("{")
    for key, _label, _arr in SEVII_SUBS:
        lines.append(f"    {{sText_EvtSevii_{key}, EVENT_SEVII_{key}}},")
    lines.append("    {sText_Back, LIST_CANCEL},")
    lines.append("};")
    lines.append("")

    lines.append("static const struct ListMenuItem sEventMapItems[] =")
    lines.append("{")
    for key, _label, _arr in WORLD_SUBS:
        lines.append(f"    {{sText_EvtMap_{key}, EVENT_MAP_{key}}},")
    lines.append("    {sText_Back, LIST_CANCEL},")
    lines.append("};")
    lines.append("")

    # Helper tables for the menu
    lines.append("// Used by menu: folder has subs if KANTO/SIDE/SEVII/WORLDMAP")
    lines.append("static const bool8 sEventFolderHasSubs[] =")
    lines.append("{")
    for _key, _label, flat, subs in FOLDER_ORDER:
        lines.append(f"    {(1 if subs else 0)}, // EVENT_FOLDER_{_key}")
    lines.append("};")
    lines.append("")

    # Flat folder -> events table (NULL if has subs)
    lines.append("static const struct RhDebugEvent * const sEventFolderFlatTables[] =")
    lines.append("{")
    for key, _label, flat, _subs in FOLDER_ORDER:
        if flat:
            lines.append(f"    sEvents_{flat},")
        else:
            lines.append("    NULL,")
    lines.append("};")
    lines.append("")

    lines.append("static const u16 sEventFolderFlatCounts[] =")
    lines.append("{")
    for key, _label, flat, _subs in FOLDER_ORDER:
        if flat:
            lines.append(f"    ARRAY_COUNT(sEvents_{flat}),")
        else:
            lines.append("    0,")
    lines.append("};")
    lines.append("")

    def emit_sub_tables(prefix: str, enum_prefix: str, subs: list[tuple[str, str, str]]) -> None:
        lines.append(f"static const struct RhDebugEvent * const sEvent{prefix}Tables[] =")
        lines.append("{")
        for key, _label, arr in subs:
            lines.append(f"    sEvents_{arr},")
        lines.append("};")
        lines.append("")
        lines.append(f"static const u16 sEvent{prefix}Counts[] =")
        lines.append("{")
        for key, _label, arr in subs:
            lines.append(f"    ARRAY_COUNT(sEvents_{arr}),")
        lines.append("};")
        lines.append("")

    emit_sub_tables("Kanto", "KANTO", KANTO_SUBS)
    emit_sub_tables("Side", "SIDE", SIDE_SUBS)
    emit_sub_tables("Sevii", "SEVII", SEVII_SUBS)
    emit_sub_tables("Map", "MAP", WORLD_SUBS)

    total = sum(len(v) for v in EVENTS.values())
    lines.append(f"// Total events: {total}")
    lines.append("")

    OUT.parent.mkdir(parents=True, exist_ok=True)
    text = "\n".join(lines)
    OUT.write_text(text, encoding="utf-8", newline="\n")
    print(f"Wrote {OUT} ({total} events, {OUT.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
