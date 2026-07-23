#include "global.h"
#include "bg_pal_slots.h"
#include "palette.h"

void LoadBgPalSlots(u8 bgPalNum, const u8 *indices, const u16 *colors, u8 count)
{
    u8 i;
    u8 idx;

    if (indices == NULL || colors == NULL || count == 0)
        return;

    for (i = 0; i < count; i++)
    {
        idx = indices[i];
        if (idx > 15)
            continue;
        LoadPalette(&colors[i], BG_PLTT_ID(bgPalNum) + idx, PLTT_SIZEOF(1));
    }
}

void LoadBgPalSlotRange(u8 bgPalNum, u8 startIndex, const u16 *colors, u8 count)
{
    u8 n;

    if (colors == NULL || count == 0 || startIndex > 15)
        return;

    n = count;
    if (startIndex + n > 16)
        n = 16 - startIndex;

    LoadPalette(colors, BG_PLTT_ID(bgPalNum) + startIndex, PLTT_SIZEOF(n));
}
