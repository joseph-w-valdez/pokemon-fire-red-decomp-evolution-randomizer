#ifndef GUARD_RH_STAT_EDITOR_H
#define GUARD_RH_STAT_EDITOR_H

#include "global.h"

// Called from party menu after FULL MAKEOVER item selection.
void RhStatEditor_SetPending(u8 partySlot, u16 itemId);
void CB2_OpenStatEditor(void);

#endif // GUARD_RH_STAT_EDITOR_H
