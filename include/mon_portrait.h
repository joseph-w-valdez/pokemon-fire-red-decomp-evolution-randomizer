#ifndef GUARD_MON_PORTRAIT_H
#define GUARD_MON_PORTRAIT_H

#include "global.h"

#define MON_PORTRAIT_NONE 0xFF
#define MON_PORTRAIT_ICON_SIZE 32
#define MON_PORTRAIT_NO_WINDOW 0xFF

enum
{
    MON_PORTRAIT_ALIGN_ABS = 0,       // use config->x / y as screen coords
    MON_PORTRAIT_ALIGN_RIGHT_MIDDLE,
    MON_PORTRAIT_ALIGN_LEFT_MIDDLE,
    MON_PORTRAIT_ALIGN_CENTER,
};

enum
{
    MON_PORTRAIT_MODE_ICON = 0,       // 32x32 animated party icon
    MON_PORTRAIT_MODE_FRONT_PIC,      // 64x64 static front pic
};

struct MonPortraitConfig
{
    bool8 usePartySlot;
    u8 partySlot;
    u16 species;       // used when usePartySlot is FALSE
    u32 personality;
    u32 otId;
    u8 windowId;       // MON_PORTRAIT_NO_WINDOW => absolute x/y
    u8 align;
    s16 x;             // absolute, or ignored when align places from window
    s16 y;
    s16 nudgeX;
    s16 nudgeY;
    u8 pad;            // inset from window edge when aligning
    u8 oamPriority;
    u8 subpriority;
    bool8 animated;    // ICON mode only
    u8 mode;
    u8 paletteSlot;    // FRONT_PIC mode
};

void MonPortrait_SetDefaults(struct MonPortraitConfig *config);
u8 MonPortrait_Show(const struct MonPortraitConfig *config);

// Convenience wrappers
u8 MonPortrait_ShowPartySlot(u8 partySlot, s16 x, s16 y);
u8 MonPortrait_ShowPartySlotInWindow(u8 partySlot, u8 windowId, s16 nudgeX, s16 nudgeY);

void MonPortrait_Hide(bool8 hide);
void MonPortrait_Destroy(void);
u8 MonPortrait_GetSpriteId(void);

#endif // GUARD_MON_PORTRAIT_H
