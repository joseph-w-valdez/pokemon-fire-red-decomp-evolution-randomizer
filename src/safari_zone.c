#include "global.h"
#include "battle.h"
#include "event_scripts.h"
#include "overworld.h"
#include "script.h"
#include "event_data.h"
#include "field_screen_effect.h"

EWRAM_DATA u8 gNumSafariBalls = 0;
EWRAM_DATA u16 gSafariZoneStepCounter = 0;

bool32 GetSafariZoneFlag(void)
{
    // Safari Zone uses normal wild battles — session mode permanently disabled.
    return FALSE;
}

void SetSafariZoneFlag(void)
{
}

void ResetSafariZoneFlag(void)
{
    FlagClear(FLAG_SYS_SAFARI_MODE);
}

void EnterSafariMode(void)
{
    // No-op: no balls, no step limit, no Safari battles.
}

void ExitSafariMode(void)
{
    ResetSafariZoneFlag();
    gNumSafariBalls = 0;
    gSafariZoneStepCounter = 0;
}

bool8 SafariZoneTakeStep(void)
{
    if (GetSafariZoneFlag() == FALSE)
        return FALSE;
    gSafariZoneStepCounter--;
    if (gSafariZoneStepCounter == 0)
    {
        ScriptContext_SetupScript(SafariZone_EventScript_TimesUp);
        return TRUE;
    }
    return FALSE;
}

void SafariZoneRetirePrompt(void)
{
    ScriptContext_SetupScript(SafariZone_EventScript_RetirePrompt);
}

void CB2_EndSafariBattle(void)
{
    if (gNumSafariBalls != 0)
    {
        SetMainCallback2(CB2_ReturnToField);
    }
    else if (gBattleOutcome == B_OUTCOME_NO_SAFARI_BALLS)
    {
        RunScriptImmediately(SafariZone_EventScript_OutOfBallsMidBattle);
        WarpIntoMap();
        gFieldCallback = FieldCB_SafariZoneRanOutOfBalls;
        SetMainCallback2(CB2_LoadMap);
    }
    else if (gBattleOutcome == B_OUTCOME_CAUGHT)
    {
        ScriptContext_SetupScript(SafariZone_EventScript_OutOfBalls);
        ScriptContext_Stop();
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
    }
}
