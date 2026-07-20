#ifndef GUARD_NUZLOCKE_H
#define GUARD_NUZLOCKE_H

#include "global.h"

bool8 IsNuzlockeActive(void);
void Nuzlocke_SetEnabledFromIntro(bool8 enabled);
void Nuzlocke_ApplyIntroChoice(void);
void Nuzlocke_OnWildBattleStart(void);
bool8 Nuzlocke_CanThrowBall(void);
bool8 Nuzlocke_AreaEncounterUsed(void);

#endif // GUARD_NUZLOCKE_H
