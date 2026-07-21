#ifndef GUARD_UI_THEME_H
#define GUARD_UI_THEME_H

#include "global.h"

// Shared std-window themes (16 colors). Index roles: docs/ui-themes.md
// Used by MAKEOVER / Stat Editor and Debug Menu (BG pals 14+15 + gap on 0).

enum
{
    UI_THEME_PORCELAIN = 0,
    UI_THEME_CARBON,
    UI_THEME_STEEL,
    UI_THEME_INK,
    UI_THEME_SNOW,
    UI_THEME_BLUSH,
    UI_THEME_CRIMSON,
    UI_THEME_HONEY,
    UI_THEME_COUNT
};

// Semantic slots (same for every theme).
#define UI_THEME_IDX_BG            0
#define UI_THEME_IDX_FILL          1
#define UI_THEME_IDX_TEXT          2
#define UI_THEME_IDX_SHADOW        3
#define UI_THEME_IDX_FRAME_HI      11
#define UI_THEME_IDX_FRAME_MID     12
#define UI_THEME_IDX_FRAME_DARK    13
#define UI_THEME_IDX_ACCENT        14
#define UI_THEME_IDX_FRAME_SHADOW  15

u8 UiTheme_GetCount(void);
u8 UiTheme_GetIndex(void);
void UiTheme_SetIndex(u8 index);

// 16× u16 palette, or NULL if index out of range.
const u16 *UiTheme_GetPalette(u8 index);

// Short English name for footers / logs (never NULL; "?" if bad index).
const u8 *UiTheme_GetName(u8 index);

// Load current theme into BG pals 14 + 15 and color 0 onto pal 0 (gap).
void UiTheme_ApplyStdWindow(void);

// Advance index (wrap), ApplyStdWindow, optional SE_SELECT. Returns new index.
u8 UiTheme_Cycle(bool8 playSe);

#endif // GUARD_UI_THEME_H
