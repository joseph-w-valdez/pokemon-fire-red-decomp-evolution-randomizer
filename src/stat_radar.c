#include "global.h"
#include "gflib.h"
#include "stat_radar.h"
#include "trig.h"
#include "window.h"
#include "text.h"
#include "menu.h"
#include "string_util.h"

static const u8 sText_Space[] = _(" ");

static u8 StatRadar_AxisAngle(u8 startAngle, u8 axisCount, u8 axis)
{
    return (startAngle + ((u16)axis * 256) / axisCount) & 0xFF;
}

static void StatRadar_Plot(u8 windowId, s16 x, s16 y, u8 color, s16 winW, s16 winH)
{
    // Pass the raw 0–15 index (not PIXEL_FILL). FillBitmapRect4Bit ORs the full
    // byte into one nibble; PIXEL_FILL(a) next to color b yields (a|b) — e.g. 1|4=5
    // (type-palette green) on the sibling pixel.
    if (x >= 0 && y >= 0 && x < winW && y < winH)
        FillWindowPixelRect(windowId, color, x, y, 1, 1);
}

static void StatRadar_DrawLine(u8 windowId, s16 x0, s16 y0, s16 x1, s16 y1, u8 color, s16 winW, s16 winH)
{
    s16 dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    s16 sx = (x0 < x1) ? 1 : -1;
    s16 dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);
    s16 sy = (y0 < y1) ? 1 : -1;
    s16 err = dx + dy;
    s16 e2;

    for (;;)
    {
        StatRadar_Plot(windowId, x0, y0, color, winW, winH);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = err * 2;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

static void StatRadar_SortNodes(s16 *nodes, u8 count)
{
    u8 i, j;
    s16 tmp;

    for (i = 1; i < count; i++)
    {
        tmp = nodes[i];
        j = i;
        while (j > 0 && nodes[j - 1] > tmp)
        {
            nodes[j] = nodes[j - 1];
            j--;
        }
        nodes[j] = tmp;
    }
}

static void StatRadar_FillPolygon(u8 windowId, const s16 *xs, const s16 *ys, u8 n, u8 color, s16 winW, s16 winH)
{
    s16 minY;
    s16 maxY;
    s16 y;
    s16 i;
    s16 j;
    s16 nodes[STAT_RADAR_MAX_AXES];
    u8 nodeCount;
    u8 k;

    if (n < 3)
        return;

    minY = ys[0];
    maxY = ys[0];
    for (i = 1; i < n; i++)
    {
        if (ys[i] < minY)
            minY = ys[i];
        if (ys[i] > maxY)
            maxY = ys[i];
    }

    if (minY < 0)
        minY = 0;
    if (maxY >= winH)
        maxY = winH - 1;

    for (y = minY; y <= maxY; y++)
    {
        nodeCount = 0;
        j = n - 1;
        for (i = 0; i < n; i++)
        {
            if ((ys[i] < y && ys[j] >= y) || (ys[j] < y && ys[i] >= y))
            {
                nodes[nodeCount] = xs[i] + (s16)(((s32)(y - ys[i]) * (xs[j] - xs[i])) / (ys[j] - ys[i]));
                nodeCount++;
                if (nodeCount >= STAT_RADAR_MAX_AXES)
                    break;
            }
            j = i;
        }

        if (nodeCount < 2)
            continue;

        StatRadar_SortNodes(nodes, nodeCount);
        for (k = 0; k + 1 < nodeCount; k += 2)
        {
            s16 x0 = nodes[k];
            s16 x1 = nodes[k + 1];
            s16 x;

            if (x0 > x1)
            {
                x = x0;
                x0 = x1;
                x1 = x;
            }
            if (x0 < 0)
                x0 = 0;
            if (x1 >= winW)
                x1 = winW - 1;
            if (x0 <= x1)
                FillWindowPixelRect(windowId, color, x0, y, (x1 - x0) + 1, 1);
        }
    }
}

static void StatRadar_ComputeRing(s16 cx, s16 cy, u8 radius, u8 startAngle, u8 axisCount, s16 *xs, s16 *ys)
{
    u8 i;
    u8 angle;

    for (i = 0; i < axisCount; i++)
    {
        angle = StatRadar_AxisAngle(startAngle, axisCount, i);
        xs[i] = cx + Cos(angle, radius);
        ys[i] = cy + Sin(angle, radius);
    }
}

static void StatRadar_StrokeRing(u8 windowId, const s16 *xs, const s16 *ys, u8 n, u8 color, s16 winW, s16 winH)
{
    u8 i;

    for (i = 0; i < n; i++)
        StatRadar_DrawLine(windowId, xs[i], ys[i], xs[(i + 1) % n], ys[(i + 1) % n], color, winW, winH);
}

// Center str on an outward anchor past the tip, sized from GetStringWidth / glyph height.
static void StatRadar_CalcLabelPos(const struct StatRadarConfig *config, s16 tipX, s16 tipY,
                                   u8 angle, const u8 *str, u8 axis, s16 winW, s16 winH,
                                   s16 *outX, s16 *outY)
{
    s16 strW;
    s16 strH;
    s16 clear;
    s16 extra;
    s16 ax;
    s16 ay;
    s16 lx;
    s16 ly;

    strW = GetStringWidth(config->fontId, str, 0);
    strH = GetFontAttribute(config->fontId, FONTATTR_MAX_LETTER_HEIGHT);
    if (strH < 8)
        strH = 8;

    // Outward past tip, then center the string on that anchor.
    // Extra push is capped so wide labels ("SPA 252") don't force a tiny chart.
    clear = config->labelOffset + config->labelPad;
    extra = strW / 4;
    if (extra > 8)
        extra = 8;
    // Near-vertical tips (HP / SPE when tip-up): skip width extra and shorten
    // radial clear so labels sit closer to the tips than diagonals do.
    if (Cos(angle, 32) >= -8 && Cos(angle, 32) <= 8)
    {
        extra = 0;
        if (clear >= 2)
            clear -= 2;
    }
    if (clear < 0)
        clear = 0;
    ax = tipX + Cos(angle, clear + extra);
    ay = tipY + Sin(angle, clear + extra);
    lx = ax - strW / 2;
    ly = ay - strH / 2;
    lx += config->labelNudgeX[axis];
    ly += config->labelNudgeY[axis];

    if (lx < 0)
        lx = 0;
    if (ly < 0)
        ly = 0;
    if (lx > winW - 1)
        lx = winW - 1;
    if (ly > winH - 1)
        ly = winH - 1;

    *outX = lx;
    *outY = ly;
}

void StatRadar_SetDefaults(struct StatRadarConfig *config)
{
    u8 i;

    if (config == NULL)
        return;

    config->windowId = 0;
    config->cx = 0;
    config->cy = 0;
    config->radius = 0;
    config->labelRadius = 0; // 0 = follow radius
    config->scalePercent = 100;
    config->nudgeX = 0;
    config->nudgeY = 0;
    config->axisCount = 6;
    config->values = NULL;
    config->valueMax = 252;
    config->gridColor = 3;
    config->fillColor = 14;
    config->outlineColor = 13;
    config->startAngle = STAT_RADAR_ANGLE_UP;
    config->drawCage = TRUE;
    config->drawFill = TRUE;
    config->drawOutline = TRUE;
    config->drawCenterDot = TRUE;
    config->drawLabels = FALSE;
    config->drawValues = FALSE;
    config->drawDebugGuides = FALSE;
    config->debugColor = 0;
    config->fontId = FONT_SMALL;
    config->textColors = NULL;
    config->labelOffset = 6;
    config->labelPad = 2;
    for (i = 0; i < STAT_RADAR_MAX_AXES; i++)
    {
        config->labelNudgeX[i] = 0;
        config->labelNudgeY[i] = 0;
    }
    config->labels = NULL;
}

void StatRadar_ApplySizePreset(struct StatRadarConfig *config, u8 size)
{
    if (config == NULL)
        return;

    switch (size)
    {
    case STAT_RADAR_SIZE_PREVIEW:
        // MAKEOVER / Stat Editor footer mini chart (portrait shares the strip).
        config->radius = 11;
        config->scalePercent = 100;
        config->drawLabels = FALSE;
        config->drawValues = FALSE;
        break;
    case STAT_RADAR_SIZE_SMALL:
        config->radius = 0;
        config->scalePercent = 80;
        config->drawLabels = FALSE;
        config->drawValues = FALSE;
        break;
    case STAT_RADAR_SIZE_MEDIUM:
        // Summary Moves memo: labeled @ 64% + slight left chrome bias.
        config->radius = 0;
        config->scalePercent = 64;
        config->nudgeX = -2;
        config->drawLabels = TRUE;
        config->drawValues = TRUE;
        break;
    case STAT_RADAR_SIZE_LARGE:
        config->radius = 0;
        config->scalePercent = 100;
        config->drawLabels = TRUE;
        config->drawValues = TRUE;
        break;
    default:
        break;
    }
}

u8 StatRadar_PadForSize(u8 size)
{
    switch (size)
    {
    case STAT_RADAR_SIZE_PREVIEW:
    case STAT_RADAR_SIZE_SMALL:
        return 2;
    case STAT_RADAR_SIZE_MEDIUM:
        return 10;
    case STAT_RADAR_SIZE_LARGE:
        return 8;
    default:
        return 4;
    }
}

void StatRadar_PlaceInRect(struct StatRadarConfig *config, u8 windowId,
                           s16 x, s16 y, s16 w, s16 h, u8 pad)
{
    s16 maxR;

    if (config == NULL)
        return;

    if (w < 1)
        w = 1;
    if (h < 1)
        h = 1;

    config->windowId = windowId;
    config->cx = x + w / 2;
    config->cy = y + h / 2;

    maxR = ((w < h) ? w : h) / 2 - pad;
    if (config->drawLabels || config->drawValues)
        maxR -= 12;
    if (maxR < 8)
        maxR = 8;
    if (config->radius == 0)
        config->radius = (u8)maxR;

    // scalePercent shrinks the drawn cage only; labelRadius is left alone so
    // callers can keep text on the pre-scale ring.
    if (config->scalePercent != 0 && config->scalePercent != 100)
    {
        config->radius = (u8)(((u16)config->radius * config->scalePercent) / 100);
        if (config->radius < 4)
            config->radius = 4;
    }
}

void StatRadar_PlaceInWindow(struct StatRadarConfig *config, u8 windowId, u8 pad)
{
    s16 w;
    s16 h;

    if (config == NULL)
        return;

    w = GetWindowAttribute(windowId, WINDOW_WIDTH) * 8;
    h = GetWindowAttribute(windowId, WINDOW_HEIGHT) * 8;
    StatRadar_PlaceInRect(config, windowId, 0, 0, w, h, pad);
}

void StatRadar_NudgeTipUpToMidline(struct StatRadarConfig *config, s16 biasY)
{
    // cy is window center from PlaceInWindow. nudgeY ≈ radius puts the top tip
    // near that midline; biasY fine-tunes (summary medium uses +1).
    if (config == NULL)
        return;
    if (config->startAngle != STAT_RADAR_ANGLE_UP)
        return;
    config->nudgeY = config->radius + biasY;
}

void StatRadar_Draw(const struct StatRadarConfig *config)
{
    s16 ringX[STAT_RADAR_MAX_AXES];
    s16 ringY[STAT_RADAR_MAX_AXES];
    s16 labelX[STAT_RADAR_MAX_AXES];
    s16 labelY[STAT_RADAR_MAX_AXES];
    s16 fillX[STAT_RADAR_MAX_AXES];
    s16 fillY[STAT_RADAR_MAX_AXES];
    s16 winW;
    s16 winH;
    s16 cx;
    s16 cy;
    u8 i;
    u8 n;
    u16 value;
    u16 maxValue;
    u8 angle;
    s16 amp;
    u8 fillRadius;
    u8 labelR;
    bool8 anyValue;
    bool8 separateLabelRing;
    u8 labelColors[3];
    const u8 *colors;
    s16 lx;
    s16 ly;
    const s16 *tipX;
    const s16 *tipY;

    if (config == NULL || config->values == NULL || config->radius == 0)
        return;

    n = config->axisCount;
    if (n < 3)
        return;
    if (n > STAT_RADAR_MAX_AXES)
        n = STAT_RADAR_MAX_AXES;

    winW = GetWindowAttribute(config->windowId, WINDOW_WIDTH) * 8;
    winH = GetWindowAttribute(config->windowId, WINDOW_HEIGHT) * 8;
    cx = config->cx + config->nudgeX;
    cy = config->cy + config->nudgeY;
    maxValue = config->valueMax;
    if (maxValue == 0)
        maxValue = 1;

    labelR = config->labelRadius;
    if (labelR == 0)
        labelR = config->radius;
    separateLabelRing = (labelR != config->radius);

    StatRadar_ComputeRing(cx, cy, config->radius, config->startAngle, n, ringX, ringY);
    if (separateLabelRing)
        StatRadar_ComputeRing(cx, cy, labelR, config->startAngle, n, labelX, labelY);

    tipX = separateLabelRing ? labelX : ringX;
    tipY = separateLabelRing ? labelY : ringY;

    if (config->drawCage)
        StatRadar_StrokeRing(config->windowId, ringX, ringY, n, config->gridColor, winW, winH);

    anyValue = FALSE;
    // Keep fill+outline inside the cage: maxed tips share the cage vertex, then the
    // outline overwrites that gray pixel and reads as a 1px overshoot (esp. SPE).
    fillRadius = config->radius;
    if (config->drawCage && fillRadius > 0)
        fillRadius--;
    for (i = 0; i < n; i++)
    {
        value = config->values[i];
        if (value > maxValue)
            value = maxValue;
        if (value != 0)
            anyValue = TRUE;
        amp = (s16)(((u32)fillRadius * value) / maxValue);
        angle = StatRadar_AxisAngle(config->startAngle, n, i);
        fillX[i] = cx + Cos(angle, amp);
        fillY[i] = cy + Sin(angle, amp);
    }

    if (anyValue)
    {
        if (config->drawFill)
            StatRadar_FillPolygon(config->windowId, fillX, fillY, n, config->fillColor, winW, winH);
        if (config->drawOutline)
            StatRadar_StrokeRing(config->windowId, fillX, fillY, n, config->outlineColor, winW, winH);
    }
    else if (config->drawCenterDot)
    {
        FillWindowPixelRect(config->windowId, config->outlineColor,
                            cx - 1, cy - 1, 3, 3);
    }

    if (config->drawDebugGuides)
    {
        u8 guide = config->debugColor;

        if (guide == 0)
            guide = config->gridColor;
        // Center cross + cage tip crosses (layout DevX; toggle off for ship).
        StatRadar_DrawLine(config->windowId, cx - 3, cy, cx + 3, cy, guide, winW, winH);
        StatRadar_DrawLine(config->windowId, cx, cy - 3, cx, cy + 3, guide, winW, winH);
        for (i = 0; i < n; i++)
        {
            StatRadar_DrawLine(config->windowId, ringX[i] - 2, ringY[i], ringX[i] + 2, ringY[i], guide, winW, winH);
            StatRadar_DrawLine(config->windowId, ringX[i], ringY[i] - 2, ringX[i], ringY[i] + 2, guide, winW, winH);
        }
    }

    if ((config->drawLabels || config->drawValues) && config->textColors != NULL)
        colors = config->textColors;
    else
    {
        labelColors[0] = 0;
        labelColors[1] = 2;
        labelColors[2] = 3;
        colors = labelColors;
    }

    // Combined "ATK 0" on one tip when both flags are set.
    if (config->drawLabels && config->drawValues && config->labels != NULL)
    {
        for (i = 0; i < n; i++)
        {
            if (config->labels[i] == NULL)
                continue;
            value = config->values[i];
            if (value > maxValue)
                value = maxValue;
            StringCopy(gStringVar1, config->labels[i]);
            StringAppend(gStringVar1, sText_Space);
            ConvertIntToDecimalStringN(gStringVar2, value, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(gStringVar1, gStringVar2);
            angle = StatRadar_AxisAngle(config->startAngle, n, i);
            StatRadar_CalcLabelPos(config, tipX[i], tipY[i], angle, gStringVar1, i, winW, winH, &lx, &ly);
            AddTextPrinterParameterized3(config->windowId, config->fontId, lx, ly,
                                         colors, TEXT_SKIP_DRAW, gStringVar1);
        }
    }
    else if (config->drawLabels && config->labels != NULL)
    {
        for (i = 0; i < n; i++)
        {
            if (config->labels[i] == NULL)
                continue;
            angle = StatRadar_AxisAngle(config->startAngle, n, i);
            StatRadar_CalcLabelPos(config, tipX[i], tipY[i], angle, config->labels[i], i, winW, winH, &lx, &ly);
            AddTextPrinterParameterized3(config->windowId, config->fontId, lx, ly,
                                         colors, TEXT_SKIP_DRAW, config->labels[i]);
        }
    }
    else if (config->drawValues)
    {
        for (i = 0; i < n; i++)
        {
            value = config->values[i];
            if (value > maxValue)
                value = maxValue;
            ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 3);
            angle = StatRadar_AxisAngle(config->startAngle, n, i);
            StatRadar_CalcLabelPos(config, tipX[i], tipY[i], angle, gStringVar1, i, winW, winH, &lx, &ly);
            AddTextPrinterParameterized3(config->windowId, config->fontId, lx, ly,
                                         colors, TEXT_SKIP_DRAW, gStringVar1);
        }
    }
}
