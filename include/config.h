#ifndef GUARD_CONFIG_H
#define GUARD_CONFIG_H

// global.h includes config.h; do not include global.h here so map scripts can
// #include "config.h" without pulling C headers into the assembler pipeline.
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// In the Generation 3 games, Asserts were used in various debug builds.
// Ruby/Sapphire and Emerald do not have these asserts while Fire Red
// still has them in the ROM. This is because the developers forgot
// to define NDEBUG before release, however this has been changed as
// Ruby's actual debug build does not use the AGBPrint features.

// Revision 10 disabled asserts in some other way, comment out NDEBUG there to bring them back.
#if REVISION >= 0xA
#define NDEBUG
#endif

// Fire Red likely forgot to define NDEBUG/NOAGBPRN before release, leading
// to the inclusion of asserts in the retail ROM.
// Revision 10 disabled asserts in some other way, but isagbprint/etc is still present there.

#if !defined(NDEBUG) || REVISION >= 0xA
#define PRETTY_PRINT_OFF (0)
#define PRETTY_PRINT_MINI_PRINTF (1)
#define PRETTY_PRINT_LIBC (2)

#define LOG_HANDLER_AGB_PRINT (0)
#define LOG_HANDLER_NOCASH_PRINT (1)
#define LOG_HANDLER_MGBA_PRINT (2)

// Use this switch to choose a handler for pretty printing.
// NOTE: mini_printf supports a custom pretty printing formatter to display preproc encoded strings. (%S)
//       some libc distributions (especially dkp arm-libc) will fail to link pretty printing.
#define PRETTY_PRINT_HANDLER (PRETTY_PRINT_OFF)

// Use this switch to choose a handler for printf output.
// NOTE: These will only work on the respective emulators and should not be used in a productive environment.
//       Some emulators or real hardware might (and is allowed to) crash if they are used.
//       AGB_PRINT is supported on respective debug units.

#define LOG_HANDLER (LOG_HANDLER_AGB_PRINT)
#endif // NDEBUG

// Define the game version for use elsewhere
#if defined(FIRERED)
#define GAME_VERSION VERSION_FIRE_RED
#else // Default version seems to be LeafGreen
#define GAME_VERSION VERSION_LEAF_GREEN
#endif // GAME_VERSION

// rev1 renamed the source folder for reasons
#if REVISION == 0
#define CODE_ROOT "C:/WORK/POKeFRLG/src/pm_lgfr_ose/source/"
#else
#define CODE_ROOT "C:/WORK/POKeFRLG/Src/pm_lgfr_ose/source/"
#endif // REVISION

#define ABSPATH(x) (CODE_ROOT x)

#ifdef ENGLISH
#define UNITS_IMPERIAL
#else
#define UNITS_METRIC
#endif // ENGLISH

// Crashes may occur due to section reordering in the modern build,
// so we force BUGFIX here.
#if MODERN
#ifndef BUGFIX
#define BUGFIX
#endif // BUGFIX
#ifndef UBFIX
#define UBFIX
#endif // UBFIX
#endif // MODERN

// ---- Romhack feature toggles ----
// Defaults match the current hack. Set any to FALSE to restore nearer-to-vanilla
// behavior at the gated call sites (see CHANGES.md).

// HGSS-style party Pokémon follower. Engine is implemented, but FireRed only ships
// OW sprites for a small set of species (others currently fall back to Pikachu).
// Keep FALSE until a fuller overworld sprite set is added.
#define OW_FOLLOWERS_ENABLED       FALSE

#define RH_RANDOM_EVOLUTION        TRUE
#define RH_NUZLOCKE                TRUE
#define RH_FAST_MOVEMENT           TRUE
#define RH_RUN_ANYWHERE            TRUE
#define RH_REUSABLE_TMS            TRUE
#define RH_FORGETTABLE_HMS         TRUE
#define RH_HM_KEY_ITEMS            TRUE
#define RH_SAFARI_NORMAL_BATTLES   TRUE
#define RH_VS_SEEKER_QOL           TRUE
#define RH_ALL_EXP_SHARE           TRUE
#define RH_FAST_TEXT_ONLY          TRUE
#define RH_SKIP_QUEST_LOG_INTRO    TRUE
#define RH_FAST_BATTLES            TRUE
#define RH_SKIP_CONTROLS_GUIDE     TRUE
// When TRUE, skip Viridian catch tutorial (remove old man after Oak's Parcel)
// AND grant Teachy TV at new game. When FALSE, restore vanilla tutorial flow
// and do not auto-grant Teachy TV.
#define RH_SKIP_CATCH_TUTORIAL     TRUE

// ---- Tunable values (used when the matching RH_* toggle is TRUE) ----
// Tweak these instead of hunting magic numbers through src/.

// Overworld movement in pixels/frame. Valid: 1, 2, 4, 8
// (maps onto pret walk / run / slide step tables).
#define RH_WALK_PX_PER_FRAME       2   // PlayerWalkNormal (vanilla was 1)
#define RH_BIKE_PX_PER_FRAME       8   // bike + PlayerWalkFastest
#define RH_SURF_PX_PER_FRAME       8   // surfing
#define RH_BIKE_DOWNHILL_PX        8   // cycling road downhill
// Run step-table index in event_object_movement.c (0=1px … 3=4px … 4=8px).
// Vanilla run is 1 (MOVE_SPEED_FAST_1 / 2px); hack default is 3 (4px).
#define RH_RUN_MOVE_SPEED          3
// GetPlayerSpeed() while moving — use 3 (FASTER), not 4 (FASTEST), or Start/A lock out.
#define RH_REPORTED_SPEED_MOVING   3

// Text printer frame delays (lower = faster). Shown as Fast / Faster in the options menu.
#define RH_TEXT_DELAY_FAST         8
#define RH_TEXT_DELAY_FASTER       2

// Battle pacing
#define RH_BATTLE_ANIM_SPEED           2  // divide anim script waits
#define RH_BATTLE_INTRO_SLIDE_SPEED    3  // curtain intro ticks/frame
#define RH_BATTLE_TRANSITION_SPEED     3  // field transition ticks/frame
#define RH_BATTLE_DOUBLE_TICK_ANIMS    TRUE // BattleMainCB2 double-runs anim tasks
#define RH_CRY_TEMPO_MULT              3  // battle cry tempo = 256 * this

// Pokéball / send-out timing (used when RH_FAST_BATTLES)
#define RH_POKEBALL_BOUNCE_DELTA   864   // vanilla 288
#define RH_POKEBALL_ARC_FRAMES     8     // vanilla 25
#define RH_POKEBALL_STAGGER_FRAMES 8     // vanilla 24 (data[0]++ > N)
#define RH_POKEBALL_RELEASE_FRAMES 5     // vanilla 15
#define RH_POKEBALL_DELAY_FRAMES   7     // vanilla 20
#define RH_POKEBALL_HEALTHBOX_TICKS 3
#define RH_POKEBALL_CRY_FRAMES_A   1     // was 3
#define RH_POKEBALL_CRY_FRAMES_B   2     // was 6

// Pokérus strain byte applied by the Cerulean Gentleman (high nibble=strain, low=days)
#define RH_POKERUS_INFECT_VALUE    0xF4

#endif // GUARD_CONFIG_H
