#ifndef GUARD_TYPE_ICON_H
#define GUARD_TYPE_ICON_H

#include "global.h"

// Type-pill preset on UiChip (include/ui_chip.h): silhouette / label / surround /
// Commit are generic; this module owns type fills, stock bake, HP helpers.
// Blank procedural pills: TypeIcon_DrawBlank* / BlitBlank* (fill + gTypeNames).
// Preview: python tools/preview_type_pills.py
//
// Prefer same-pal window when possible (Moves / Skills detail HP):
//   window paletteNum = type bank; FillWindowPixelBuffer(0);
//   TypeIcon_Draw / DrawBlank / BlitMenuInfoIcon; PutWindowTilemap; CopyVram.
//   Idx 0 punches through to page chrome (stripes OK). No surround / Commit.
//
// Mixed-palette (shared foreign / themed window — MAKEOVER footer):
//   TypeIcon_LoadPalette(TYPE_PAL);          // once at screen init
//   … paint other window content …
//   TypeIcon_Blit / BlitBlank / DrawHiddenPowerBlock…  // before PutWindowTilemap
//   PutWindowTilemap(window);
//   TypeIcon_Commit / CommitSized(...); // or MAP if GFX copies later
//   Tile-align; fill unused index 10 with surround RGB15 (BG idx 0 is transparent).
//
// Surround: UiTheme fill via TypeIcon_Blit / DrawHiddenPowerBlock, or explicit
// RGB15 via *WithSurround. Prefer SurroundFromBgPal; blit sanitizes RGB_MAGENTA.
// Opaque surround cannot match striped chrome — use same-pal window there.
// Call LoadPalette at init — blit only patches surround idx 10 (does not reload).
//
// IV order for Hidden Power helpers: HP, ATK, DEF, SPE, SPA, SPD (stat order).
// Each IV is 0–31 (u16 arrays from draft/save are fine).

#define TYPE_ICON_WIDTH         32 // stock lettered menu_info badge
#define TYPE_ICON_BLANK_WIDTH   32 // default procedural pill (matches stock)
#define TYPE_ICON_HEIGHT  12
#define TYPE_ICON_TILE_W  4
#define TYPE_ICON_TILE_H  2
#define TYPE_ICON_TILE_PX 8
// Fine-tune after ink-box center. Stock menu_info is geometric (T=B=2, L≈R);
// keep 0 so AABB center matches the sheet — nonzero here was pushing labels down-right.
#define TYPE_ICON_LABEL_NUDGE_X    0
#define TYPE_ICON_LABEL_NUDGE_Y    0
// FormatTypeLabel dest size: prefix + TYPE_NAME_LENGTH + EOS.
#define TYPE_ICON_LABEL_BUF_SIZE  16

// Indices inside pokemon_types (after TypeIcon_LoadPalette / ListMenuLoadStdPalAt …, 1).
// Stock badges: body = type fill; letters use outline+text. Blank pills omit letters.
#define TYPE_ICON_IDX_KEY      0
#define TYPE_ICON_IDX_OUTLINE 14
#define TYPE_ICON_IDX_TEXT    15

// CSS-like width for procedural blank pills (ResolveWidth).
enum {
    TYPE_ICON_WIDTH_FIXED, // exact fixed px
    TYPE_ICON_WIDTH_FIT,   // label advance + 2*padX, then min/max
};

struct TypeIconWidth
{
    u8 mode;       // TYPE_ICON_WIDTH_FIXED | FIT
    u16 fixed;     // FIXED mode value
    u16 min;       // 0 = none; floor after FIT (or ignored for FIXED)
    u16 max;       // 0 = none; ceiling
    u8 padX;       // FIT: padding each side
    bool8 tileAlign; // ceil to TYPE_ICON_TILE_PX (mixed-palette)
};

u16 TypeIcon_AlignX(u16 x);
// Snap down to tile (right-align helpers).
u16 TypeIcon_AlignXFloor(u16 x);
// Snap to nearest tile (centering before mixed-pal blit/Commit).
u16 TypeIcon_AlignXNearest(u16 x);
// Center `width` in the open span [leftEdge, rightEdge). If too wide, pin to leftEdge.
u16 TypeIcon_CenterXInSpan(u16 leftEdge, u16 rightEdge, u16 width);

// Shared "HP " prefix for Hidden Power blank pills (MAKEOVER / Skills detail).
extern const u8 gText_TypeIconHpPrefix[];

void TypeIcon_WidthSetFixed(struct TypeIconWidth *cfg, u16 px, bool8 tileAlign);
void TypeIcon_WidthSetFit(struct TypeIconWidth *cfg, u8 padX, u16 min, u16 max, bool8 tileAlign);
// FIT: max(labelW + 2*padX, min) then min(..., max) then optional tile ceil. FIXED: fixed (+ tile).
u16 TypeIcon_ResolveWidth(const struct TypeIconWidth *cfg, const u8 *label);

// dest[TYPE_ICON_LABEL_BUF_SIZE]. prefix NULL → type name only.
void TypeIcon_FormatTypeLabel(u8 *dest, u8 type, const u8 *prefix);

// Load pokemon_types into a free BG bank. Call once per screen (or when the bank
// was stomped). Blit does not reload the full bank.
void TypeIcon_LoadPalette(u8 bgPalNum);
u8 TypeIcon_CalcHiddenPowerType(const u16 ivs[6]);
u8 TypeIcon_CalcHiddenPowerPower(const u16 ivs[6]);
u8 TypeIcon_CalcHiddenPowerTypeFromMon(struct Pokemon *mon);

// pokemon_types body indices for TYPE_* (matches stock badge; solid types have top==bottom).
// Two-tone stock badges split horizontally at mid-height (top 6px / bottom 6px).
u8 TypeIcon_GetFillColor(u8 type); // top half (solid types: the only fill)
void TypeIcon_GetFillColors(u8 type, u8 *topColor, u8 *bottomColor);

// Reject chroma-key magenta (and similar). FromBgPal skips idx 0 and prefers panel slots.
bool8 TypeIcon_IsUnsafeSurroundColor(u16 color);
u16 TypeIcon_SanitizeSurroundColor(u16 color, u16 fallback);
u16 TypeIcon_SurroundFromBgPal(u8 bgPalNum);

// Blit a TYPE_* constant (TYPE_FIRE, etc.) with the stock badge gfx.
void TypeIcon_Draw(u8 windowId, u8 type, u16 x, u16 y);

// Procedural pill (stock silhouette + gTypeNames via FONT_SMALL).
// Uses type fill colors (incl. top/bottom half-and-half). Label: fg 15 / shadow 14 / bg 0.
// Same-pal windows: DrawBlank* (corner idx 0 punches through to under-BGs — Moves/Skills).
// Mixed-palette hosts (MAKEOVER): BlitBlank* — surround under corners, then Commit.
// width < 2 is clamped to 2. DrawBlankWithFills: body only (no label).
void TypeIcon_DrawBlankWithFills(u8 windowId, u8 fillTop, u8 fillBottom, u16 x, u16 y, u16 width, u8 cornerIdx);
void TypeIcon_DrawBlankSized(u8 windowId, u8 type, u16 x, u16 y, u16 width);
void TypeIcon_DrawBlank(u8 windowId, u8 type, u16 x, u16 y);
// Same-pal blank with optional prefix (NULL = gTypeNames only). widthCfg NULL → FIXED 32.
void TypeIcon_DrawBlankEx(u8 windowId, u8 type, u16 x, u16 y, const struct TypeIconWidth *widthCfg, const u8 *prefix);

// Center any FONT_SMALL str in a pill rect from measured white-ink bounds.
// Y may be negative (font cell taller than pill).
void TypeIcon_GetLabelOrigin(u8 fontId, const u8 *str, u16 pillX, u16 pillY, u16 pillW, u16 pillH, s16 *outX, s16 *outY);
void TypeIcon_PrintLabel(u8 windowId, const u8 *str, u16 pillX, u16 pillY, u16 width);
void TypeIcon_PrintTypeName(u8 windowId, u8 type, u16 pillX, u16 pillY, u16 width);

// Tile-aligned badge blit for mixed-palette windows. Call before PutWindowTilemap.
// TypeIcon_Blit uses the host window's UiTheme fill as surround.
void TypeIcon_Blit(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y);
void TypeIcon_BlitWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 surroundColor);

// Same mixed-palette path for procedural blank pills (stock or custom width).
void TypeIcon_BlitBlank(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y);
void TypeIcon_BlitBlankWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 surroundColor);
void TypeIcon_BlitBlankSizedWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 width, u16 surroundColor);
// Returns resolved width. widthCfg NULL → FIXED blank width. prefix NULL → type name only.
u16 TypeIcon_BlitBlankEx(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y,
                         const struct TypeIconWidth *widthCfg, const u8 *prefix, u16 surroundColor);

// Tilemap palette override for a prior blit. Call after PutWindowTilemap.
void TypeIcon_ApplyPaletteOverride(u8 windowId, u8 bgPalNum, u16 x, u16 y);
void TypeIcon_ApplyPaletteOverrideSized(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width);

// ApplyPaletteOverride + optional CopyWindowToVram.
// copyMode: COPYWIN_NONE (override only), COPYWIN_MAP, COPYWIN_GFX, or COPYWIN_FULL.
// Does not PutWindowTilemap — caller maps the whole window first on shared canvases.
void TypeIcon_Commit(u8 windowId, u8 bgPalNum, u16 x, u16 y, u8 copyMode);
void TypeIcon_CommitSized(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width, u8 copyMode);

// Hidden Power helpers: stock lettered badge, sized/aligned for mixed-palette windows.
u16 TypeIcon_HiddenPowerWidth(void);
u16 TypeIcon_DrawHiddenPowerBlock(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y);
u16 TypeIcon_DrawHiddenPowerBlockWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y, u16 surroundColor);

// Hidden Power procedural-pill helpers. Sized variants use an explicit width.
u16 TypeIcon_DrawHiddenPowerBlankBlock(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y);
u16 TypeIcon_DrawHiddenPowerBlankBlockSized(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y, u16 width);
u16 TypeIcon_DrawHiddenPowerBlankBlockSizedWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y, u16 width, u16 surroundColor);
// Right edge = rightX (exclusive). Places pill with floor tile-align. Writes *outWidth if non-NULL.
// Returns icon X. prefix e.g. _("HP-"); widthCfg NULL → stock fixed blank width.
// Theme surround (UiTheme fill). Use *WithSurround for foreign hosts.
u16 TypeIcon_DrawHiddenPowerBlankEx(u8 windowId, u8 bgPalNum, u8 type, u16 rightX, u16 y,
                                    const struct TypeIconWidth *widthCfg, const u8 *prefix, u16 *outWidth);
u16 TypeIcon_DrawHiddenPowerBlankExWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 rightX, u16 y,
                                                const struct TypeIconWidth *widthCfg, const u8 *prefix,
                                                u16 surroundColor, u16 *outWidth);

#endif // GUARD_TYPE_ICON_H
