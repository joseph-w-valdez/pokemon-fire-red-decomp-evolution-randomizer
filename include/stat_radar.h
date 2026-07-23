#ifndef GUARD_STAT_RADAR_H
#define GUARD_STAT_RADAR_H

#include "global.h"

#define STAT_RADAR_MAX_AXES 8
#define STAT_RADAR_ANGLE_UP 192

// Size presets for ApplySizePreset / PadForSize. See docs/ui-components.md.
enum
{
    STAT_RADAR_SIZE_PREVIEW = 0, // unlabeled mini (MAKEOVER / Stat Editor footer)
    STAT_RADAR_SIZE_SMALL,       // unlabeled auto-fit @ 80%
    STAT_RADAR_SIZE_MEDIUM,      // labeled chart (Skills detail EV/IV; tune scale/labelRadius)
    STAT_RADAR_SIZE_LARGE,       // labeled full auto-fit @ 100%
};

struct StatRadarConfig
{
    u8 windowId;
    s16 cx;
    s16 cy;
    u8 radius;          // drawn cage/fill; 0 = auto-size when using PlaceInWindow
    u8 labelRadius;     // label/value anchors; 0 = same as radius (shrink cage, keep labels out)
    u8 scalePercent;    // applied to radius by PlaceInWindow; 0 or 100 = no change
    s16 nudgeX;         // added at draw time (and usable after PlaceInWindow)
    s16 nudgeY;
    u8 axisCount;       // 3–STAT_RADAR_MAX_AXES
    const u16 *values;
    u16 valueMax;
    u8 gridColor;
    u8 fillColor;
    u8 outlineColor;
    u8 startAngle;      // Sin/Cos units; tip-up = STAT_RADAR_ANGLE_UP
    bool8 drawCage;
    bool8 drawFill;
    bool8 drawOutline;
    bool8 drawCenterDot;
    bool8 drawLabels;
    bool8 drawValues;   // numeric value near each tip
    bool8 drawDebugGuides; // tip crosses + center (uses debugColor)
    u8 debugColor;      // raw 0–15 index for guides; 0 = use gridColor
    u8 fontId;
    const u8 *textColors; // AddTextPrinterParameterized3 colors; may be NULL
    s8 labelOffset;     // pixels outward from tip before auto text clearance
    s8 labelPad;        // extra gap (shadow/clearance) added to auto placement
    s8 labelNudgeX[STAT_RADAR_MAX_AXES]; // optional fine-tune after auto place
    s8 labelNudgeY[STAT_RADAR_MAX_AXES];
    const u8 *const *labels;
};

// Fill sensible defaults (cage+fill+outline, tip-up, FONT_SMALL, scale 100%).
void StatRadar_SetDefaults(struct StatRadarConfig *config);

// Apply radius/scale/label flags (+ medium nudgeX). Still call PlaceInWindow
// with StatRadar_PadForSize(size), then optional NudgeTipUpToMidline.
void StatRadar_ApplySizePreset(struct StatRadarConfig *config, u8 size);

// Recommended PlaceInWindow pad for a size preset.
u8 StatRadar_PadForSize(u8 size);

// Center in window; if radius is 0, pick a fit (shrinks when labels/values on),
// then apply scalePercent. Does not apply nudge (Draw does).
void StatRadar_PlaceInWindow(struct StatRadarConfig *config, u8 windowId, u8 pad);

// Same as PlaceInWindow but within a window-local rect (FooterStrip mid, etc.).
void StatRadar_PlaceInRect(struct StatRadarConfig *config, u8 windowId,
                           s16 x, s16 y, s16 w, s16 h, u8 pad);

// After PlaceInWindow: set nudgeY so the top tip sits on the window midline
// plus biasY (tip-up charts with labels). Summary medium uses biasY = 1.
// No-op if not tip-up.
void StatRadar_NudgeTipUpToMidline(struct StatRadarConfig *config, s16 biasY);

// Paints into an existing window (does not map/copy). Uses cx+nudgeX, cy+nudgeY.
// Labels anchor on labelRadius when set, else radius — so the cage can shrink
// while text stays on a larger ring. Labels are centered with GetStringWidth and
// pushed outward along each axis. Near-vertical tips (HP/SPE) skip width-based
// extra push. Pass raw palette indices 0–15 for colors (never PIXEL_FILL).
void StatRadar_Draw(const struct StatRadarConfig *config);

#endif // GUARD_STAT_RADAR_H
