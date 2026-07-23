#include "global.h"
#include "type_icon.h"
#include "ui_chip.h"
#include "battle_main.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
#include "ui_theme.h"
#include "window.h"
#include "constants/pokemon.h"
#include "constants/rgb.h"
#include "characters.h"

// Type-pill preset on UiChip: fill tables, stock bake, HP helpers.
// Blank silhouette / label / surround / Commit live in ui_chip.c.

#define TYPE_ICON_SURROUND_IDX UI_CHIP_SURROUND_IDX
#define TYPE_ICON_FALLBACK_SURROUND UI_CHIP_FALLBACK_SURROUND

static const struct UiChipLabelStyle sTypeLabelStyle = {
    .fontId = FONT_SMALL,
    .fgIdx = TYPE_ICON_IDX_TEXT,
    .bgIdx = TYPE_ICON_IDX_KEY,
    .shadowIdx = TYPE_ICON_IDX_OUTLINE,
    .nudgeX = TYPE_ICON_LABEL_NUDGE_X,
    .nudgeY = TYPE_ICON_LABEL_NUDGE_Y,
};

static void TypeIcon_BlitBlankSizedLabelWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 width, const u8 *label, u16 surroundColor);

const u8 gText_TypeIconHpPrefix[] = _("HP ");

// Body indices per TYPE_* in pokemon_types (from menu_info.4bpp non-letter pixels).
static const u8 sTypeFillTop[NUMBER_OF_MON_TYPES] = {
    [TYPE_NORMAL]   = 9,
    [TYPE_FIGHTING] = 1,
    [TYPE_FLYING]   = 6,
    [TYPE_POISON]   = 12,
    [TYPE_GROUND]   = 4,
    [TYPE_ROCK]     = 11,
    [TYPE_BUG]      = 4,
    [TYPE_GHOST]    = 8,
    [TYPE_STEEL]    = 9,
    [TYPE_MYSTERY]  = 6,
    [TYPE_FIRE]     = 2,
    [TYPE_WATER]    = 7,
    [TYPE_GRASS]    = 5,
    [TYPE_ELECTRIC] = 3,
    [TYPE_PSYCHIC]  = 12,
    [TYPE_ICE]      = 6,
    [TYPE_DRAGON]   = 7,
    [TYPE_DARK]     = 13,
};

static const u8 sTypeFillBottom[NUMBER_OF_MON_TYPES] = {
    [TYPE_NORMAL]   = 9,
    [TYPE_FIGHTING] = 1,
    [TYPE_FLYING]   = 9,
    [TYPE_POISON]   = 8,
    [TYPE_GROUND]   = 11,
    [TYPE_ROCK]     = 11,
    [TYPE_BUG]      = 4,
    [TYPE_GHOST]    = 8,
    [TYPE_STEEL]    = 13,
    [TYPE_MYSTERY]  = 12,
    [TYPE_FIRE]     = 2,
    [TYPE_WATER]    = 7,
    [TYPE_GRASS]    = 5,
    [TYPE_ELECTRIC] = 3,
    [TYPE_PSYCHIC]  = 12,
    [TYPE_ICE]      = 6,
    [TYPE_DRAGON]   = 1,
    [TYPE_DARK]     = 13,
};

void TypeIcon_LoadPalette(u8 bgPalNum)
{
    ListMenuLoadStdPalAt(BG_PLTT_ID(bgPalNum), 1);
}

u8 TypeIcon_GetFillColor(u8 type)
{
    if (type >= NUMBER_OF_MON_TYPES)
        return sTypeFillTop[TYPE_NORMAL];
    return sTypeFillTop[type];
}

void TypeIcon_GetFillColors(u8 type, u8 *topColor, u8 *bottomColor)
{
    if (type >= NUMBER_OF_MON_TYPES)
        type = TYPE_NORMAL;
    *topColor = sTypeFillTop[type];
    *bottomColor = sTypeFillBottom[type];
}

void TypeIcon_DrawBlankWithFills(u8 windowId, u8 fillTop, u8 fillBottom, u16 x, u16 y, u16 width, u8 cornerIdx)
{
    UiChip_Draw(windowId, fillTop, fillBottom, x, y, width, TYPE_ICON_HEIGHT, cornerIdx);
}

void TypeIcon_GetLabelOrigin(u8 fontId, const u8 *str, u16 pillX, u16 pillY, u16 pillW, u16 pillH, s16 *outX, s16 *outY)
{
    struct UiChipLabelStyle style = sTypeLabelStyle;

    style.fontId = fontId;
    UiChip_GetLabelOrigin(fontId, str, pillX, pillY, pillW, pillH, &style, outX, outY);
}

void TypeIcon_PrintLabel(u8 windowId, const u8 *str, u16 pillX, u16 pillY, u16 width)
{
    UiChip_PrintLabel(windowId, str, pillX, pillY, width, TYPE_ICON_HEIGHT, &sTypeLabelStyle);
}

void TypeIcon_PrintTypeName(u8 windowId, u8 type, u16 pillX, u16 pillY, u16 width)
{
    if (type >= NUMBER_OF_MON_TYPES)
        return;
    TypeIcon_PrintLabel(windowId, gTypeNames[type], pillX, pillY, width);
}

void TypeIcon_FormatTypeLabel(u8 *dest, u8 type, const u8 *prefix)
{
    if (dest == NULL)
        return;

    if (prefix != NULL)
        StringCopy(dest, prefix);
    else
        dest[0] = EOS;

    if (type < NUMBER_OF_MON_TYPES)
        StringAppend(dest, gTypeNames[type]);
}

u16 TypeIcon_AlignXFloor(u16 x)
{
    return UiChip_AlignXFloor(x);
}

u16 TypeIcon_AlignXNearest(u16 x)
{
    return UiChip_AlignXNearest(x);
}

u16 TypeIcon_CenterXInSpan(u16 leftEdge, u16 rightEdge, u16 width)
{
    return UiChip_CenterXInSpan(leftEdge, rightEdge, width);
}

void TypeIcon_WidthSetFixed(struct TypeIconWidth *cfg, u16 px, bool8 tileAlign)
{
    UiChip_WidthSetFixed((struct UiChipWidth *)cfg, px, tileAlign);
}

void TypeIcon_WidthSetFit(struct TypeIconWidth *cfg, u8 padX, u16 min, u16 max, bool8 tileAlign)
{
    UiChip_WidthSetFit((struct UiChipWidth *)cfg, padX, min, max, tileAlign);
}

u16 TypeIcon_ResolveWidth(const struct TypeIconWidth *cfg, const u8 *label)
{
    return UiChip_ResolveWidth((const struct UiChipWidth *)cfg, label, TYPE_ICON_BLANK_WIDTH);
}

static void TypeIcon_DrawBlankSizedLabel(u8 windowId, u8 type, u16 x, u16 y, u16 width, const u8 *label)
{
    u8 top, bottom;

    TypeIcon_GetFillColors(type, &top, &bottom);
    TypeIcon_DrawBlankWithFills(windowId, top, bottom, x, y, width, TYPE_ICON_IDX_KEY);
    TypeIcon_PrintLabel(windowId, label, x, y, width);
}

void TypeIcon_DrawBlankSized(u8 windowId, u8 type, u16 x, u16 y, u16 width)
{
    if (type >= NUMBER_OF_MON_TYPES)
        return;
    TypeIcon_DrawBlankSizedLabel(windowId, type, x, y, width, gTypeNames[type]);
}

void TypeIcon_DrawBlank(u8 windowId, u8 type, u16 x, u16 y)
{
    TypeIcon_DrawBlankSized(windowId, type, x, y, TYPE_ICON_BLANK_WIDTH);
}

void TypeIcon_DrawBlankEx(u8 windowId, u8 type, u16 x, u16 y, const struct TypeIconWidth *widthCfg, const u8 *prefix)
{
    u8 label[TYPE_ICON_LABEL_BUF_SIZE];
    u16 width;

    TypeIcon_FormatTypeLabel(label, type, prefix);
    width = TypeIcon_ResolveWidth(widthCfg, label);
    TypeIcon_DrawBlankSizedLabel(windowId, type, x, y, width, label);
}

static u16 TypeIcon_GetThemeFillColor(u8 windowId)
{
    u8 winPal = GetWindowAttribute(windowId, WINDOW_PALETTE_NUM);

    return gPlttBufferUnfaded[BG_PLTT_ID(winPal) + UI_THEME_IDX_FILL];
}

bool8 TypeIcon_IsUnsafeSurroundColor(u16 color)
{
    return UiChip_IsUnsafeSurroundColor(color);
}

u16 TypeIcon_SanitizeSurroundColor(u16 color, u16 fallback)
{
    return UiChip_SanitizeSurroundColor(color, fallback);
}

u16 TypeIcon_SurroundFromBgPal(u8 bgPalNum)
{
    return UiChip_SurroundFromBgPal(bgPalNum);
}

static u32 TypeIcon_PackTypeBits(const u16 ivs[6])
{
    return ((ivs[0] & 1) << 0)
         | ((ivs[1] & 1) << 1)
         | ((ivs[2] & 1) << 2)
         | ((ivs[3] & 1) << 3)
         | ((ivs[4] & 1) << 4)
         | ((ivs[5] & 1) << 5);
}

static u32 TypeIcon_PackPowerBits(const u16 ivs[6])
{
    return ((ivs[0] & 2) >> 1)
         | ((ivs[1] & 2) << 0)
         | ((ivs[2] & 2) << 1)
         | ((ivs[3] & 2) << 2)
         | ((ivs[4] & 2) << 3)
         | ((ivs[5] & 2) << 4);
}

u8 TypeIcon_CalcHiddenPowerType(const u16 ivs[6])
{
    u32 typeBits = TypeIcon_PackTypeBits(ivs);
    u32 type = ((NUMBER_OF_MON_TYPES - 3) * typeBits) / 63 + 1;

    if (type >= TYPE_MYSTERY)
        type++;
    return type;
}

u8 TypeIcon_CalcHiddenPowerPower(const u16 ivs[6])
{
    u32 powerBits = TypeIcon_PackPowerBits(ivs);

    return (40 * powerBits) / 63 + 30;
}

u8 TypeIcon_CalcHiddenPowerTypeFromMon(struct Pokemon *mon)
{
    u16 ivs[6];

    ivs[0] = GetMonData(mon, MON_DATA_HP_IV, NULL);
    ivs[1] = GetMonData(mon, MON_DATA_ATK_IV, NULL);
    ivs[2] = GetMonData(mon, MON_DATA_DEF_IV, NULL);
    ivs[3] = GetMonData(mon, MON_DATA_SPEED_IV, NULL);
    ivs[4] = GetMonData(mon, MON_DATA_SPATK_IV, NULL);
    ivs[5] = GetMonData(mon, MON_DATA_SPDEF_IV, NULL);
    return TypeIcon_CalcHiddenPowerType(ivs);
}

void TypeIcon_Draw(u8 windowId, u8 type, u16 x, u16 y)
{
    if (type < NUMBER_OF_MON_TYPES)
        BlitMenuInfoIcon(windowId, type + 1, x, y);
}

u16 TypeIcon_AlignX(u16 x)
{
    return UiChip_AlignX(x);
}

void TypeIcon_BlitWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 surroundColor)
{
    u16 ax = x;
    u16 ay = y;

    UiChip_AlignPos(&ax, &ay);
    surroundColor = UiChip_SanitizeSurroundColor(surroundColor, TYPE_ICON_FALLBACK_SURROUND);
    UiChip_ApplySurroundColor(bgPalNum, surroundColor);

    if (type >= NUMBER_OF_MON_TYPES)
        return;

    FillWindowPixelRect(windowId, TYPE_ICON_SURROUND_IDX, ax, ay,
                        TYPE_ICON_TILE_W * TYPE_ICON_TILE_PX,
                        TYPE_ICON_TILE_H * TYPE_ICON_TILE_PX);
    BlitMenuInfoIcon(windowId, type + 1, ax, ay);
}

void TypeIcon_Blit(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y)
{
    TypeIcon_BlitWithSurround(windowId, bgPalNum, type, x, y,
                              TypeIcon_GetThemeFillColor(windowId));
}

void TypeIcon_BlitBlankSizedWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 width, u16 surroundColor)
{
    u8 label[TYPE_ICON_LABEL_BUF_SIZE];

    if (type >= NUMBER_OF_MON_TYPES)
        return;
    TypeIcon_FormatTypeLabel(label, type, NULL);
    TypeIcon_BlitBlankSizedLabelWithSurround(windowId, bgPalNum, type, x, y, width, label, surroundColor);
}

static void TypeIcon_BlitBlankSizedLabelWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 width, const u8 *label, u16 surroundColor)
{
    u8 top, bottom;

    if (type >= NUMBER_OF_MON_TYPES)
        return;

    TypeIcon_GetFillColors(type, &top, &bottom);
    UiChip_BlitLabeled(windowId, bgPalNum, top, bottom, x, y, width, TYPE_ICON_HEIGHT,
                       label, &sTypeLabelStyle, surroundColor);
}

u16 TypeIcon_BlitBlankEx(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y,
                         const struct TypeIconWidth *widthCfg, const u8 *prefix, u16 surroundColor)
{
    u8 label[TYPE_ICON_LABEL_BUF_SIZE];
    u16 width;
    struct TypeIconWidth local;

    if (widthCfg == NULL)
    {
        TypeIcon_WidthSetFixed(&local, TYPE_ICON_BLANK_WIDTH, TRUE);
        widthCfg = &local;
    }

    TypeIcon_FormatTypeLabel(label, type, prefix);
    width = TypeIcon_ResolveWidth(widthCfg, label);
    TypeIcon_BlitBlankSizedLabelWithSurround(windowId, bgPalNum, type, x, y, width, label, surroundColor);
    return width;
}

void TypeIcon_BlitBlankWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 surroundColor)
{
    TypeIcon_BlitBlankSizedWithSurround(windowId, bgPalNum, type, x, y, TYPE_ICON_BLANK_WIDTH, surroundColor);
}

void TypeIcon_BlitBlank(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y)
{
    TypeIcon_BlitBlankWithSurround(windowId, bgPalNum, type, x, y,
                                   TypeIcon_GetThemeFillColor(windowId));
}

void TypeIcon_ApplyPaletteOverride(u8 windowId, u8 bgPalNum, u16 x, u16 y)
{
    TypeIcon_ApplyPaletteOverrideSized(windowId, bgPalNum, x, y, TYPE_ICON_WIDTH);
}

void TypeIcon_ApplyPaletteOverrideSized(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width)
{
    UiChip_ApplyPaletteOverride(windowId, bgPalNum, x, y, width, TYPE_ICON_HEIGHT);
}

void TypeIcon_Commit(u8 windowId, u8 bgPalNum, u16 x, u16 y, u8 copyMode)
{
    TypeIcon_CommitSized(windowId, bgPalNum, x, y, TYPE_ICON_WIDTH, copyMode);
}

void TypeIcon_CommitSized(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width, u8 copyMode)
{
    UiChip_Commit(windowId, bgPalNum, x, y, width, TYPE_ICON_HEIGHT, copyMode);
}

u16 TypeIcon_HiddenPowerWidth(void)
{
    return TYPE_ICON_WIDTH;
}

u16 TypeIcon_DrawHiddenPowerBlockWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y, u16 surroundColor)
{
    u16 iconX = TypeIcon_AlignX(startX);

    TypeIcon_BlitWithSurround(windowId, bgPalNum, type, iconX, y, surroundColor);
    return iconX;
}

u16 TypeIcon_DrawHiddenPowerBlock(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y)
{
    return TypeIcon_DrawHiddenPowerBlockWithSurround(windowId, bgPalNum, type, startX, y,
                                                     TypeIcon_GetThemeFillColor(windowId));
}

u16 TypeIcon_DrawHiddenPowerBlankBlockSizedWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y, u16 width, u16 surroundColor)
{
    u16 iconX = TypeIcon_AlignX(startX);

    TypeIcon_BlitBlankSizedWithSurround(windowId, bgPalNum, type, iconX, y, width, surroundColor);
    return iconX;
}

u16 TypeIcon_DrawHiddenPowerBlankBlockSized(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y, u16 width)
{
    return TypeIcon_DrawHiddenPowerBlankBlockSizedWithSurround(windowId, bgPalNum, type, startX, y, width,
                                                              TypeIcon_GetThemeFillColor(windowId));
}

u16 TypeIcon_DrawHiddenPowerBlankBlock(u8 windowId, u8 bgPalNum, u8 type, u16 startX, u16 y)
{
    return TypeIcon_DrawHiddenPowerBlankBlockSized(windowId, bgPalNum, type, startX, y, TYPE_ICON_BLANK_WIDTH);
}

u16 TypeIcon_DrawHiddenPowerBlankExWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 rightX, u16 y,
                                                const struct TypeIconWidth *widthCfg, const u8 *prefix,
                                                u16 surroundColor, u16 *outWidth)
{
    u8 label[TYPE_ICON_LABEL_BUF_SIZE];
    u16 width;
    u16 iconX;
    struct TypeIconWidth local;

    if (widthCfg == NULL)
    {
        TypeIcon_WidthSetFixed(&local, TYPE_ICON_BLANK_WIDTH, TRUE);
        widthCfg = &local;
    }

    TypeIcon_FormatTypeLabel(label, type, prefix);
    width = TypeIcon_ResolveWidth(widthCfg, label);
    if (rightX < width)
        iconX = 0;
    else
        iconX = TypeIcon_AlignXFloor(rightX - width);

    TypeIcon_BlitBlankSizedLabelWithSurround(windowId, bgPalNum, type, iconX, y, width, label, surroundColor);
    if (outWidth != NULL)
        *outWidth = width;
    return iconX;
}

u16 TypeIcon_DrawHiddenPowerBlankEx(u8 windowId, u8 bgPalNum, u8 type, u16 rightX, u16 y,
                                    const struct TypeIconWidth *widthCfg, const u8 *prefix, u16 *outWidth)
{
    return TypeIcon_DrawHiddenPowerBlankExWithSurround(windowId, bgPalNum, type, rightX, y,
                                                       widthCfg, prefix,
                                                       TypeIcon_GetThemeFillColor(windowId), outWidth);
}
