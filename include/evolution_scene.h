#ifndef GUARD_EVOLUTION_SCENE_H
#define GUARD_EVOLUTION_SCENE_H

#include "global.h"

extern void (*gCB2_AfterEvolution)(void);

void BeginEvolutionScene(struct Pokemon* mon, u16 speciesToEvolve, u8, u8 partyId);
void EvolutionScene(struct Pokemon* mon, u16 speciesToEvolve, u8, u8 partyId);
void TradeEvolutionScene(struct Pokemon* mon, u16 speciesToEvolve, u8 preEvoSpriteId, u8 partyId);
void BeginRandomLevelEvolutionScene(struct Pokemon *mon, bool8 canStopEvo, u8 partyId);
void EvolutionSceneRandomLevel(struct Pokemon *mon, bool8 canStopEvo, u8 partyId);
void IsMovingBackgroundTaskRunning(void);

extern bool8 gRandomLevelEvoActive;

#endif // GUARD_EVOLUTION_SCENE_H
