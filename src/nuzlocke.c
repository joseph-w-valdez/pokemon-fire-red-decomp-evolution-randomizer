#include "global.h"
#include "nuzlocke.h"
#include "event_data.h"
#include "fieldmap.h"
#include "overworld.h"
#include "constants/flags.h"
#include "constants/region_map_sections.h"
#include "constants/vars.h"

// Intro choice lives in SaveBlock2 filler so NewGameInitData does not wipe it
// the way it does unkFlag2.
#define sNuzlockeIntroChoice (gSaveBlock2Ptr->filler_90[0])

// Set to 2 when Oak's Parcel is delivered and Poké Balls are received.
#define NUZLOCKE_ENCOUNTERS_START_VAR_VALUE 2

EWRAM_DATA static bool8 sNuzlockeCatchAllowedThisBattle = FALSE;

bool8 IsNuzlockeActive(void)
{
#if RH_NUZLOCKE
    return FlagGet(FLAG_SYS_NUZLOCKE);
#else
    return FALSE;
#endif
}

// Once-per-area catches only apply after parcel delivery (when balls are given).
static bool8 Nuzlocke_EncounterRulesActive(void)
{
    return IsNuzlockeActive()
        && VarGet(VAR_MAP_SCENE_VIRIDIAN_CITY_MART) >= NUZLOCKE_ENCOUNTERS_START_VAR_VALUE;
}

void Nuzlocke_SetEnabledFromIntro(bool8 enabled)
{
    sNuzlockeIntroChoice = enabled;
}

void Nuzlocke_ApplyIntroChoice(void)
{
    if (sNuzlockeIntroChoice)
        FlagSet(FLAG_SYS_NUZLOCKE);
    else
        FlagClear(FLAG_SYS_NUZLOCKE);
    sNuzlockeIntroChoice = FALSE;
}

static u8 GetCurrentNuzlockeMapsec(void)
{
    return gMapHeader.regionMapSectionId;
}

bool8 Nuzlocke_AreaEncounterUsed(void)
{
    u8 mapsec = GetCurrentNuzlockeMapsec();

    if (mapsec >= MAPSEC_COUNT)
        return TRUE;
    return (gSaveBlock1Ptr->nuzlockeEncounters[mapsec / 8] >> (mapsec % 8)) & 1;
}

static void SetAreaEncounterUsed(void)
{
    u8 mapsec = GetCurrentNuzlockeMapsec();

    if (mapsec >= MAPSEC_COUNT)
        return;
    gSaveBlock1Ptr->nuzlockeEncounters[mapsec / 8] |= 1 << (mapsec % 8);
}

void Nuzlocke_OnWildBattleStart(void)
{
    if (!Nuzlocke_EncounterRulesActive())
    {
        // Pre-parcel (or nuzlocke off): don't burn areas; balls unrestricted.
        sNuzlockeCatchAllowedThisBattle = TRUE;
        return;
    }

    // Classic: this first wild battle is the catch opportunity; flee/faint still burns the area.
    sNuzlockeCatchAllowedThisBattle = !Nuzlocke_AreaEncounterUsed();
    SetAreaEncounterUsed();
}

bool8 Nuzlocke_CanThrowBall(void)
{
    if (!Nuzlocke_EncounterRulesActive())
        return TRUE;
    return sNuzlockeCatchAllowedThisBattle;
}
