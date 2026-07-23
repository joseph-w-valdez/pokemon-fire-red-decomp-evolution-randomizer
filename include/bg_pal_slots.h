#ifndef GUARD_BG_PAL_SLOTS_H
#define GUARD_BG_PAL_SLOTS_H

#include "global.h"

// Inject discrete 15-bit colors into a BG 4bpp palette bank.
// indices[i] is 0–15; colors[i] is RGB()/RGB8()/RGB_HEX().
// Does not touch other slots in that bank.
void LoadBgPalSlots(u8 bgPalNum, const u8 *indices, const u16 *colors, u8 count);

// Contiguous run: load `count` colors starting at `startIndex` (0–15).
void LoadBgPalSlotRange(u8 bgPalNum, u8 startIndex, const u16 *colors, u8 count);

#endif // GUARD_BG_PAL_SLOTS_H
