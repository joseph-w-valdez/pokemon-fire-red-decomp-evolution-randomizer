#ifndef GUARD_UI_CHIP_H
#define GUARD_UI_CHIP_H

#include "global.h"
#include "constants/rgb.h"

// Generic small chrome painter (pill / chip / bar silhouette + optional label).
// Caller owns the window, palette bank load, and PutWindowTilemap order.
//
// Same-pal: clear to 0, UiChip_Draw (+ PrintLabel), map/copy. Corner idx 0 punches through.
// Mixed-palette: load bank once, UiChip_BlitLabeled* before PutWindowTilemap, then
// UiChip_Commit* after. Surround uses idx UI_CHIP_SURROUND_IDX (default 10).
//
// Type pills: TypeIcon_* presets consume this API (stock bake stays in type_icon).

#define UI_CHIP_TILE_PX           8
#define UI_CHIP_DEFAULT_HEIGHT   12
#define UI_CHIP_SURROUND_IDX     10
// Summary chrome panel lavender when surround is chroma-key magenta.
#define UI_CHIP_FALLBACK_SURROUND RGB(28, 25, 31)

enum
{
    UI_CHIP_WIDTH_FIXED,
    UI_CHIP_WIDTH_FIT,
};

struct UiChipWidth
{
    u8 mode;       // UI_CHIP_WIDTH_FIXED | FIT
    u16 fixed;     // FIXED mode value
    u16 min;       // 0 = none
    u16 max;       // 0 = none
    u8 padX;       // FIT: padding each side
    bool8 tileAlign;
};

// Label paint roles inside the chip's palette bank.
struct UiChipLabelStyle
{
    u8 fontId;    // FONT_SMALL only for ink measure today
    u8 fgIdx;     // white ink counted for AABB center
    u8 bgIdx;     // glyph empty / skip-write (usually 0)
    u8 shadowIdx; // outline / SE shadow
    s8 nudgeX;
    s8 nudgeY;
};

u16 UiChip_AlignX(u16 x);
u16 UiChip_AlignXFloor(u16 x);
u16 UiChip_AlignXNearest(u16 x);
u16 UiChip_CenterXInSpan(u16 leftEdge, u16 rightEdge, u16 width);
void UiChip_AlignPos(u16 *x, u16 *y);

void UiChip_WidthSetFixed(struct UiChipWidth *cfg, u16 px, bool8 tileAlign);
void UiChip_WidthSetFit(struct UiChipWidth *cfg, u8 padX, u16 min, u16 max, bool8 tileAlign);
// defaultFixed used when cfg is NULL. FIT uses FONT_SMALL GetStringWidth for label.
u16 UiChip_ResolveWidth(const struct UiChipWidth *cfg, const u8 *label, u16 defaultFixed);

// Mid-split fill (solid = same top/bottom) + 1px corners. width < 2 clamped to 2.
// height < 2 clamped to 2. cornerIdx: 0 = punch-through; surround idx for mixed-pal.
void UiChip_Draw(u8 windowId, u8 fillTop, u8 fillBottom, u16 x, u16 y, u16 width, u16 height, u8 cornerIdx);

void UiChip_GetLabelOrigin(u8 fontId, const u8 *str, u16 chipX, u16 chipY, u16 chipW, u16 chipH,
                           const struct UiChipLabelStyle *style, s16 *outX, s16 *outY);
void UiChip_PrintLabel(u8 windowId, const u8 *str, u16 chipX, u16 chipY, u16 width, u16 height,
                       const struct UiChipLabelStyle *style);

bool8 UiChip_IsUnsafeSurroundColor(u16 color);
u16 UiChip_SanitizeSurroundColor(u16 color, u16 fallback);
u16 UiChip_SurroundFromBgPal(u8 bgPalNum);
void UiChip_ApplySurroundColor(u8 bgPalNum, u16 surroundColor);

// Mixed-palette: align, patch surround, fill tile pad, draw body (corner = surround), print label.
// Returns resolved width. label may be NULL (body only). style required if label non-NULL.
u16 UiChip_BlitLabeled(u8 windowId, u8 bgPalNum, u8 fillTop, u8 fillBottom,
                       u16 x, u16 y, u16 width, u16 height, const u8 *label,
                       const struct UiChipLabelStyle *style, u16 surroundColor);

void UiChip_ApplyPaletteOverride(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width, u16 height);
// copyMode: COPYWIN_NONE / MAP / GFX / FULL. Does not PutWindowTilemap.
void UiChip_Commit(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width, u16 height, u8 copyMode);

#endif // GUARD_UI_CHIP_H
