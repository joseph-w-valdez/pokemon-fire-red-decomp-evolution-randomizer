#include "global.h"
#include "type_icon.h"
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

// Summary type pills: window palette == type palette, clear to 0, color-key blit.
// Index 0 looks fine there because the pane behind is also "0".
//
// On a mixed-palette BG window, hardware treats BG palette index 0 as transparent,
// so a 0-filled pad under the 12px badge shows whatever is behind the BG (gray).
// Use unused index 10 as surround, patched to an explicit RGB15 (theme fill or caller).

#define TYPE_ICON_SURROUND_IDX 10 // unused by any menu_info type badge

// Summary chrome panel lavender (bg.gbapal bank 0 idx 3). Used when a caller
// accidentally passes RGB_MAGENTA (asset chroma key / BG idx 0).
#define TYPE_ICON_FALLBACK_SURROUND RGB(28, 25, 31)

static u16 TypeIcon_TileWForWidth(u16 width);
static void TypeIcon_BlitBlankSizedLabelWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 width, const u8 *label, u16 surroundColor);

const u8 gText_TypeIconHpPrefix[] = _("HP ");

// Body indices per TYPE_* in pokemon_types (from menu_info.4bpp non-letter pixels).
// Solid types: top == bottom. Two-tone: horizontal split at TYPE_ICON_HEIGHT / 2.
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
    u16 half = TYPE_ICON_HEIGHT / 2;

    if (width < 2)
        width = 2;

    // Top / bottom halves (stock two-tone is a clean mid split; solid = same index twice).
    FillWindowPixelRect(windowId, fillTop, x, y, width, half);
    FillWindowPixelRect(windowId, fillBottom, x, y + half, width, TYPE_ICON_HEIGHT - half);
    // Stock BlitMenuInfoIcon color-keys 0 (skips writing corners). Same-pal: cornerIdx 0
    // punches through to under-BGs. Mixed: pass surround idx so corners stay opaque.
    FillWindowPixelRect(windowId, cornerIdx, x, y, 1, 1);
    FillWindowPixelRect(windowId, cornerIdx, x + width - 1, y, 1, 1);
    FillWindowPixelRect(windowId, cornerIdx, x, y + TYPE_ICON_HEIGHT - 1, 1, 1);
    FillWindowPixelRect(windowId, cornerIdx, x + width - 1, y + TYPE_ICON_HEIGHT - 1, 1, 1);
}

// gTypeNames (6-char caps) with stock pill palette roles: white + SE shadow.
// bg=0 so glyph empty cells skip-write and leave the fill (GLYPH_COPY ignores 0).

// Expand ink AABB for the current gGlyphInfo at pen X (FONT_SMALL layout).
// Count fg (TYPE_ICON_IDX_TEXT) only — stock menu_info centers the white
// letters; SE shadow (idx 14) hangs outside that box. Including shadow pulls
// long labels (ELECTR / PSYCHC) 1px left of the sheet.
static void TypeIcon_AccumulateGlyphInk(s16 penX, s16 *minX, s16 *minY, s16 *maxX, s16 *maxY, bool8 *hasInk)
{
    s16 y;
    s16 x;
    u8 *src;
    u32 pixrow;
    s16 gx;
    s16 gy;
    u8 pix;

    for (y = 0; y < (s16)gGlyphInfo.height; y++)
    {
        if (y < 8)
            src = gGlyphInfo.pixels + y * 4;
        else
            src = gGlyphInfo.pixels + 0x40 + (y - 8) * 4;

        pixrow = *(u32 *)src;
        for (x = 0; x < (s16)gGlyphInfo.width; x++)
        {
            pix = (pixrow >> (x * 4)) & 0xF;
            if (pix != TYPE_ICON_IDX_TEXT)
                continue;

            gx = penX + x;
            gy = y;
            if (!*hasInk || gx < *minX)
                *minX = gx;
            if (!*hasInk || gy < *minY)
                *minY = gy;
            if (!*hasInk || gx > *maxX)
                *maxX = gx;
            if (!*hasInk || gy > *maxY)
                *maxY = gy;
            *hasInk = TRUE;
        }
    }
}

// Ink bounding box of str in pen space (origin = first glyph's top-left cell).
static bool8 TypeIcon_MeasureLabelInk(u8 fontId, const u8 *str, s16 *minX, s16 *minY, s16 *maxX, s16 *maxY)
{
    s16 penX = 0;
    bool8 hasInk = FALSE;
    u8 i;

    *minX = *minY = 0;
    *maxX = *maxY = -1;

    // Decompress uses the half-row LUT — must be non-transparent fg/shadow to see ink.
    GenerateFontHalfRowLookupTable(TYPE_ICON_IDX_TEXT, TYPE_ICON_IDX_KEY, TYPE_ICON_IDX_OUTLINE);

    for (i = 0; str[i] != EOS; i++)
    {
        if (fontId != FONT_SMALL)
            return FALSE;

        DecompressGlyph_Small(str[i], FALSE);
        TypeIcon_AccumulateGlyphInk(penX, minX, minY, maxX, maxY, &hasInk);
        penX += gGlyphInfo.width;
    }

    return hasInk;
}

void TypeIcon_GetLabelOrigin(u8 fontId, const u8 *str, u16 pillX, u16 pillY, u16 pillW, u16 pillH, s16 *outX, s16 *outY)
{
    s16 minX, minY, maxX, maxY;

    if (!TypeIcon_MeasureLabelInk(fontId, str, &minX, &minY, &maxX, &maxY))
    {
        // Fallback: advance-width center if ink measure fails.
        s32 textW = GetStringWidth(fontId, str, 0);
        *outX = (s16)((s32)pillX + ((s32)pillW - textW) / 2 + TYPE_ICON_LABEL_NUDGE_X);
        *outY = (s16)((s32)pillY + TYPE_ICON_LABEL_NUDGE_Y);
        return;
    }

    // Center white ink in the pill via leftover pads. Prefer this over
    // (pillCX - inkCX) / 2: agbcc truncates toward zero, so a negative
    // half-delta (tall FONT_SMALL cell vs 12px pill) sat labels 1px low vs
    // stock / the Python preview (which floors).
    {
        s32 inkW = (s32)maxX - (s32)minX + 1;
        s32 inkH = (s32)maxY - (s32)minY + 1;

        *outX = (s16)((s32)pillX + ((s32)pillW - inkW) / 2 - (s32)minX + TYPE_ICON_LABEL_NUDGE_X);
        *outY = (s16)((s32)pillY + ((s32)pillH - inkH) / 2 - (s32)minY + TYPE_ICON_LABEL_NUDGE_Y);
    }
}

static void TypeIcon_BlitGlyphAt(u8 windowId, s16 left, s16 top)
{
    u8 *tilesDest = gWindows[windowId].tileData;
    u16 winW = 8 * gWindows[windowId].window.width;
    u16 winH = 8 * gWindows[windowId].window.height;
    u16 sizeX = (winW + (winW & 7)) >> 3;
    s16 glyphW = gGlyphInfo.width;
    s16 glyphH = gGlyphInfo.height;
    s16 xAdd;
    s16 yAdd;
    s16 xpos;
    s16 ypos;
    u8 *src;
    u8 *dst;
    u32 pixrow;
    int toOrr;
    int bits;

    // Same layout as text_printer GLYPH_COPY, but allows a signed dest (clips off-window).
    src = gGlyphInfo.pixels;
    for (yAdd = 0, ypos = top; yAdd < glyphH; yAdd++, ypos++)
    {
        if (yAdd == 8)
            src = gGlyphInfo.pixels + 0x40;

        if (ypos < 0 || ypos >= (s16)winH)
        {
            src += 4;
            continue;
        }

        pixrow = *(u32 *)src;
        for (xAdd = 0, xpos = left; xAdd < glyphW; xAdd++, xpos++)
        {
            if (xpos < 0 || xpos >= (s16)winW)
                continue;

            toOrr = (pixrow >> (xAdd * 4)) & 0xF;
            if (toOrr != 0)
            {
                dst = tilesDest + ((xpos >> 1) & 3) + ((xpos >> 3) << 5)
                    + (((ypos >> 3) * sizeX) << 5) + ((u32)(ypos << 29) >> 27);
                bits = (xpos & 1) * 4;
                *dst = (toOrr << bits) | (*dst & (0xF0 >> bits));
            }
        }
        src += 4;
    }
}

void TypeIcon_PrintLabel(u8 windowId, const u8 *str, u16 pillX, u16 pillY, u16 width)
{
    s16 x;
    s16 y;
    u8 i;

    if (str == NULL)
        return;

    TypeIcon_GetLabelOrigin(FONT_SMALL, str, pillX, pillY, width, TYPE_ICON_HEIGHT, &x, &y);
    // Param order is (fg, bg, shadow); LUT stores {bg, fg, shadow}.
    GenerateFontHalfRowLookupTable(TYPE_ICON_IDX_TEXT, TYPE_ICON_IDX_KEY, TYPE_ICON_IDX_OUTLINE);

    for (i = 0; str[i] != EOS; i++)
    {
        DecompressGlyph_Small(str[i], FALSE);
        TypeIcon_BlitGlyphAt(windowId, x, y);
        x += gGlyphInfo.width;
    }
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
    return x & ~(TYPE_ICON_TILE_PX - 1);
}

u16 TypeIcon_AlignXNearest(u16 x)
{
    return ((x + TYPE_ICON_TILE_PX / 2) / TYPE_ICON_TILE_PX) * TYPE_ICON_TILE_PX;
}

u16 TypeIcon_CenterXInSpan(u16 leftEdge, u16 rightEdge, u16 width)
{
    u16 span;

    if (rightEdge <= leftEdge)
        return leftEdge;
    span = rightEdge - leftEdge;
    if (width >= span)
        return leftEdge;
    return leftEdge + (span - width) / 2;
}

void TypeIcon_WidthSetFixed(struct TypeIconWidth *cfg, u16 px, bool8 tileAlign)
{
    if (cfg == NULL)
        return;
    cfg->mode = TYPE_ICON_WIDTH_FIXED;
    cfg->fixed = px;
    cfg->min = 0;
    cfg->max = 0;
    cfg->padX = 0;
    cfg->tileAlign = tileAlign;
}

void TypeIcon_WidthSetFit(struct TypeIconWidth *cfg, u8 padX, u16 min, u16 max, bool8 tileAlign)
{
    if (cfg == NULL)
        return;
    cfg->mode = TYPE_ICON_WIDTH_FIT;
    cfg->fixed = 0;
    cfg->min = min;
    cfg->max = max;
    cfg->padX = padX;
    cfg->tileAlign = tileAlign;
}

u16 TypeIcon_ResolveWidth(const struct TypeIconWidth *cfg, const u8 *label)
{
    u16 w;
    struct TypeIconWidth stock;

    if (cfg == NULL)
    {
        TypeIcon_WidthSetFixed(&stock, TYPE_ICON_BLANK_WIDTH, FALSE);
        cfg = &stock;
    }

    if (cfg->mode == TYPE_ICON_WIDTH_FIT)
    {
        s32 labelW = 0;

        if (label != NULL)
            labelW = GetStringWidth(FONT_SMALL, label, 0);
        w = (u16)(labelW + (s32)cfg->padX * 2);
        if (cfg->min != 0 && w < cfg->min)
            w = cfg->min;
        if (cfg->max != 0 && w > cfg->max)
            w = cfg->max;
    }
    else
    {
        w = cfg->fixed;
    }

    if (cfg->tileAlign)
        w = TypeIcon_TileWForWidth(w) * TYPE_ICON_TILE_PX;

    if (w < 2)
        w = 2;
    return w;
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

static void TypeIcon_ApplySurroundColor(u8 typePalNum, u16 surroundColor)
{
    LoadPalette(&surroundColor, BG_PLTT_ID(typePalNum) + TYPE_ICON_SURROUND_IDX, PLTT_SIZEOF(1));
}

bool8 TypeIcon_IsUnsafeSurroundColor(u16 color)
{
    // Asset / tileset chroma keys — opaque pad in these colors is always wrong.
    return color == RGB_MAGENTA;
}

u16 TypeIcon_SanitizeSurroundColor(u16 color, u16 fallback)
{
    if (TypeIcon_IsUnsafeSurroundColor(color))
        return fallback;
    return color;
}

u16 TypeIcon_SurroundFromBgPal(u8 bgPalNum)
{
    const u16 *pal = &gPlttBufferUnfaded[BG_PLTT_ID(bgPalNum)];
    // Prefer summary-style panel slots; never idx 0 (key / transparent intent).
    static const u8 sPrefer[] = { 3, 2, 8, 10, 6, 7, 1, 4, 5, 9, 11, 12, 13, 14, 15 };
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sPrefer); i++)
    {
        if (!TypeIcon_IsUnsafeSurroundColor(pal[sPrefer[i]]))
            return pal[sPrefer[i]];
    }

    return TYPE_ICON_FALLBACK_SURROUND;
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
    return (x + (TYPE_ICON_TILE_PX - 1)) & ~(TYPE_ICON_TILE_PX - 1);
}

static void TypeIcon_AlignPos(u16 *x, u16 *y)
{
    *x = TypeIcon_AlignX(*x);
    *y = (*y / TYPE_ICON_TILE_PX) * TYPE_ICON_TILE_PX;
}

void TypeIcon_BlitWithSurround(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y, u16 surroundColor)
{
    TypeIcon_AlignPos(&x, &y);
    surroundColor = TypeIcon_SanitizeSurroundColor(surroundColor, TYPE_ICON_FALLBACK_SURROUND);
    // Caller loaded the type bank at screen init; only patch surround here.
    TypeIcon_ApplySurroundColor(bgPalNum, surroundColor);

    if (type >= NUMBER_OF_MON_TYPES)
        return;

    // Non-zero surround: BG index 0 is transparent and would show gray through.
    FillWindowPixelRect(windowId, TYPE_ICON_SURROUND_IDX, x, y,
                        TYPE_ICON_TILE_W * TYPE_ICON_TILE_PX,
                        TYPE_ICON_TILE_H * TYPE_ICON_TILE_PX);
    BlitMenuInfoIcon(windowId, type + 1, x, y);
}

void TypeIcon_Blit(u8 windowId, u8 bgPalNum, u8 type, u16 x, u16 y)
{
    TypeIcon_BlitWithSurround(windowId, bgPalNum, type, x, y,
                              TypeIcon_GetThemeFillColor(windowId));
}

static u16 TypeIcon_TileWForWidth(u16 width)
{
    if (width < 1)
        width = 1;
    return (width + TYPE_ICON_TILE_PX - 1) / TYPE_ICON_TILE_PX;
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
    u16 tileW;
    u8 top, bottom;

    TypeIcon_AlignPos(&x, &y);
    surroundColor = TypeIcon_SanitizeSurroundColor(surroundColor, TYPE_ICON_FALLBACK_SURROUND);
    TypeIcon_ApplySurroundColor(bgPalNum, surroundColor);

    if (type >= NUMBER_OF_MON_TYPES)
        return;

    if (width < 2)
        width = 2;
    tileW = TypeIcon_TileWForWidth(width);
    FillWindowPixelRect(windowId, TYPE_ICON_SURROUND_IDX, x, y,
                        tileW * TYPE_ICON_TILE_PX,
                        TYPE_ICON_TILE_H * TYPE_ICON_TILE_PX);
    TypeIcon_GetFillColors(type, &top, &bottom);
    TypeIcon_DrawBlankWithFills(windowId, top, bottom, x, y, width, TYPE_ICON_SURROUND_IDX);
    TypeIcon_PrintLabel(windowId, label, x, y, width);
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
    TypeIcon_AlignPos(&x, &y);
    PutWindowRectTilemapOverridePalette(windowId, x / TYPE_ICON_TILE_PX, y / TYPE_ICON_TILE_PX,
                                        TypeIcon_TileWForWidth(width), TYPE_ICON_TILE_H, bgPalNum);
}

void TypeIcon_Commit(u8 windowId, u8 bgPalNum, u16 x, u16 y, u8 copyMode)
{
    TypeIcon_CommitSized(windowId, bgPalNum, x, y, TYPE_ICON_WIDTH, copyMode);
}

void TypeIcon_CommitSized(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width, u8 copyMode)
{
    TypeIcon_ApplyPaletteOverrideSized(windowId, bgPalNum, x, y, width);
    if (copyMode != COPYWIN_NONE)
        CopyWindowToVram(windowId, copyMode);
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
