#ifndef GUARD_RH_LOG_H
#define GUARD_RH_LOG_H

#include "global.h"

// Session ring buffer for in-game debug Log viewer (Debug Menu → Log).
// Storage is heap-allocated on first RhLog (not a permanent EWRAM slab).
#define RH_LOG_LINES    24
#define RH_LOG_LINE_LEN 36

// msg / fmt are game-encoded strings (use _("...")).
// RhLogf supports %d, %x, %s / %S (u8 * string). Unknown specs print literally.
void RhLog(const u8 *msg);
void RhLogf(const u8 *fmt, ...);
void RhLog_Clear(void); // drops lines and frees the heap buffer

u16 RhLog_GetCount(void);
// index 0 = oldest, GetCount()-1 = newest.
const u8 *RhLog_GetLine(u16 index);

#endif // GUARD_RH_LOG_H
