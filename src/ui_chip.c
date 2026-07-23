#include "global.h"
#include "ui_chip.h"
#include "characters.h"
#include "palette.h"
#include "text.h"
#include "window.h"

static u16 UiChip_TileWForWidth(u16 width)
{
    if (width < 1)
        width = 1;
    return (width + UI_CHIP_TILE_PX - 1) / UI_CHIP_TILE_PX;
}

static u16 UiChip_TileHForHeight(u16 height)
{
    if (height < 1)
        height = 1;
    return (height + UI_CHIP_TILE_PX - 1) / UI_CHIP_TILE_PX;
}

u16 UiChip_AlignX(u16 x)
{
    return (x + (UI_CHIP_TILE_PX - 1)) & ~(UI_CHIP_TILE_PX - 1);
}

u16 UiChip_AlignXFloor(u16 x)
{
    return x & ~(UI_CHIP_TILE_PX - 1);
}

u16 UiChip_AlignXNearest(u16 x)
{
    return ((x + UI_CHIP_TILE_PX / 2) / UI_CHIP_TILE_PX) * UI_CHIP_TILE_PX;
}

u16 UiChip_CenterXInSpan(u16 leftEdge, u16 rightEdge, u16 width)
{
    u16 span;

    if (rightEdge <= leftEdge)
        return leftEdge;
    span = rightEdge - leftEdge;
    if (width >= span)
        return leftEdge;
    return leftEdge + (span - width) / 2;
}

void UiChip_AlignPos(u16 *x, u16 *y)
{
    *x = UiChip_AlignX(*x);
    *y = (*y / UI_CHIP_TILE_PX) * UI_CHIP_TILE_PX;
}

void UiChip_WidthSetFixed(struct UiChipWidth *cfg, u16 px, bool8 tileAlign)
{
    if (cfg == NULL)
        return;
    cfg->mode = UI_CHIP_WIDTH_FIXED;
    cfg->fixed = px;
    cfg->min = 0;
    cfg->max = 0;
    cfg->padX = 0;
    cfg->tileAlign = tileAlign;
}

void UiChip_WidthSetFit(struct UiChipWidth *cfg, u8 padX, u16 min, u16 max, bool8 tileAlign)
{
    if (cfg == NULL)
        return;
    cfg->mode = UI_CHIP_WIDTH_FIT;
    cfg->fixed = 0;
    cfg->min = min;
    cfg->max = max;
    cfg->padX = padX;
    cfg->tileAlign = tileAlign;
}

u16 UiChip_ResolveWidth(const struct UiChipWidth *cfg, const u8 *label, u16 defaultFixed)
{
    u16 w;
    struct UiChipWidth stock;

    if (cfg == NULL)
    {
        UiChip_WidthSetFixed(&stock, defaultFixed, FALSE);
        cfg = &stock;
    }

    if (cfg->mode == UI_CHIP_WIDTH_FIT)
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
        w = UiChip_TileWForWidth(w) * UI_CHIP_TILE_PX;

    if (w < 2)
        w = 2;
    return w;
}

void UiChip_Draw(u8 windowId, u8 fillTop, u8 fillBottom, u16 x, u16 y, u16 width, u16 height, u8 cornerIdx)
{
    u16 half;

    if (width < 2)
        width = 2;
    if (height < 2)
        height = 2;

    half = height / 2;

    // Top / bottom halves (two-tone mid split; solid = same index twice).
    FillWindowPixelRect(windowId, fillTop, x, y, width, half);
    FillWindowPixelRect(windowId, fillBottom, x, y + half, width, height - half);
    // 1px corners. Same-pal: cornerIdx 0 punches through. Mixed: surround idx.
    FillWindowPixelRect(windowId, cornerIdx, x, y, 1, 1);
    FillWindowPixelRect(windowId, cornerIdx, x + width - 1, y, 1, 1);
    FillWindowPixelRect(windowId, cornerIdx, x, y + height - 1, 1, 1);
    FillWindowPixelRect(windowId, cornerIdx, x + width - 1, y + height - 1, 1, 1);
}

// Expand ink AABB for gGlyphInfo at pen X. Count fgIdx only (shadow hangs outside).
static void UiChip_AccumulateGlyphInk(s16 penX, u8 fgIdx, s16 *minX, s16 *minY, s16 *maxX, s16 *maxY, bool8 *hasInk)
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
            if (pix != fgIdx)
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

static bool8 UiChip_MeasureLabelInk(u8 fontId, const u8 *str, u8 fgIdx, u8 bgIdx, u8 shadowIdx,
                                    s16 *minX, s16 *minY, s16 *maxX, s16 *maxY)
{
    s16 penX = 0;
    bool8 hasInk = FALSE;
    u8 i;

    *minX = *minY = 0;
    *maxX = *maxY = -1;

    GenerateFontHalfRowLookupTable(fgIdx, bgIdx, shadowIdx);

    for (i = 0; str[i] != EOS; i++)
    {
        if (fontId != FONT_SMALL)
            return FALSE;

        DecompressGlyph_Small(str[i], FALSE);
        UiChip_AccumulateGlyphInk(penX, fgIdx, minX, minY, maxX, maxY, &hasInk);
        penX += gGlyphInfo.width;
    }

    return hasInk;
}

void UiChip_GetLabelOrigin(u8 fontId, const u8 *str, u16 chipX, u16 chipY, u16 chipW, u16 chipH,
                           const struct UiChipLabelStyle *style, s16 *outX, s16 *outY)
{
    s16 minX, minY, maxX, maxY;
    s8 nudgeX = 0;
    s8 nudgeY = 0;
    u8 fgIdx = 15;
    u8 bgIdx = 0;
    u8 shadowIdx = 14;

    if (style != NULL)
    {
        nudgeX = style->nudgeX;
        nudgeY = style->nudgeY;
        fgIdx = style->fgIdx;
        bgIdx = style->bgIdx;
        shadowIdx = style->shadowIdx;
        fontId = style->fontId;
    }

    if (!UiChip_MeasureLabelInk(fontId, str, fgIdx, bgIdx, shadowIdx, &minX, &minY, &maxX, &maxY))
    {
        s32 textW = GetStringWidth(fontId, str, 0);
        *outX = (s16)((s32)chipX + ((s32)chipW - textW) / 2 + nudgeX);
        *outY = (s16)((s32)chipY + nudgeY);
        return;
    }

    // Leftover-pad center (agbcc truncates toward zero — do not half the delta).
    {
        s32 inkW = (s32)maxX - (s32)minX + 1;
        s32 inkH = (s32)maxY - (s32)minY + 1;

        *outX = (s16)((s32)chipX + ((s32)chipW - inkW) / 2 - (s32)minX + nudgeX);
        *outY = (s16)((s32)chipY + ((s32)chipH - inkH) / 2 - (s32)minY + nudgeY);
    }
}

static void UiChip_BlitGlyphAt(u8 windowId, s16 left, s16 top)
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

void UiChip_PrintLabel(u8 windowId, const u8 *str, u16 chipX, u16 chipY, u16 width, u16 height,
                       const struct UiChipLabelStyle *style)
{
    s16 x;
    s16 y;
    u8 i;
    u8 fontId = FONT_SMALL;
    u8 fgIdx = 15;
    u8 bgIdx = 0;
    u8 shadowIdx = 14;

    if (str == NULL)
        return;

    if (style != NULL)
    {
        fontId = style->fontId;
        fgIdx = style->fgIdx;
        bgIdx = style->bgIdx;
        shadowIdx = style->shadowIdx;
    }

    UiChip_GetLabelOrigin(fontId, str, chipX, chipY, width, height, style, &x, &y);
    GenerateFontHalfRowLookupTable(fgIdx, bgIdx, shadowIdx);

    for (i = 0; str[i] != EOS; i++)
    {
        if (fontId != FONT_SMALL)
            return;
        DecompressGlyph_Small(str[i], FALSE);
        UiChip_BlitGlyphAt(windowId, x, y);
        x += gGlyphInfo.width;
    }
}

bool8 UiChip_IsUnsafeSurroundColor(u16 color)
{
    return color == RGB_MAGENTA;
}

u16 UiChip_SanitizeSurroundColor(u16 color, u16 fallback)
{
    if (UiChip_IsUnsafeSurroundColor(color))
        return fallback;
    return color;
}

u16 UiChip_SurroundFromBgPal(u8 bgPalNum)
{
    const u16 *pal = &gPlttBufferUnfaded[BG_PLTT_ID(bgPalNum)];
    static const u8 sPrefer[] = { 3, 2, 8, 10, 6, 7, 1, 4, 5, 9, 11, 12, 13, 14, 15 };
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sPrefer); i++)
    {
        if (!UiChip_IsUnsafeSurroundColor(pal[sPrefer[i]]))
            return pal[sPrefer[i]];
    }

    return UI_CHIP_FALLBACK_SURROUND;
}

void UiChip_ApplySurroundColor(u8 bgPalNum, u16 surroundColor)
{
    LoadPalette(&surroundColor, BG_PLTT_ID(bgPalNum) + UI_CHIP_SURROUND_IDX, PLTT_SIZEOF(1));
}

u16 UiChip_BlitLabeled(u8 windowId, u8 bgPalNum, u8 fillTop, u8 fillBottom,
                       u16 x, u16 y, u16 width, u16 height, const u8 *label,
                       const struct UiChipLabelStyle *style, u16 surroundColor)
{
    u16 tileW;
    u16 tileH;

    UiChip_AlignPos(&x, &y);
    surroundColor = UiChip_SanitizeSurroundColor(surroundColor, UI_CHIP_FALLBACK_SURROUND);
    UiChip_ApplySurroundColor(bgPalNum, surroundColor);

    if (width < 2)
        width = 2;
    if (height < 2)
        height = 2;

    tileW = UiChip_TileWForWidth(width);
    tileH = UiChip_TileHForHeight(height);
    FillWindowPixelRect(windowId, UI_CHIP_SURROUND_IDX, x, y,
                        tileW * UI_CHIP_TILE_PX,
                        tileH * UI_CHIP_TILE_PX);
    UiChip_Draw(windowId, fillTop, fillBottom, x, y, width, height, UI_CHIP_SURROUND_IDX);
    if (label != NULL)
        UiChip_PrintLabel(windowId, label, x, y, width, height, style);
    return width;
}

void UiChip_ApplyPaletteOverride(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width, u16 height)
{
    UiChip_AlignPos(&x, &y);
    PutWindowRectTilemapOverridePalette(windowId, x / UI_CHIP_TILE_PX, y / UI_CHIP_TILE_PX,
                                        UiChip_TileWForWidth(width), UiChip_TileHForHeight(height), bgPalNum);
}

void UiChip_Commit(u8 windowId, u8 bgPalNum, u16 x, u16 y, u16 width, u16 height, u8 copyMode)
{
    UiChip_ApplyPaletteOverride(windowId, bgPalNum, x, y, width, height);
    if (copyMode != COPYWIN_NONE)
        CopyWindowToVram(windowId, copyMode);
}
