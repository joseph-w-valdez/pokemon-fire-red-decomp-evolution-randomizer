#ifndef GUARD_VALUE_SLIDER_H
#define GUARD_VALUE_SLIDER_H

#include "global.h"

typedef u16 (*ValueSliderGetMaxFunc)(void *ctx);

struct ValueSliderConfig
{
    u8 windowId;
    const u8 *label;
    u16 min;
    u16 displayMax;
    u16 *value;
    ValueSliderGetMaxFunc getMax;
    void *ctx;
    u8 trackColor;
    u8 thumbColor;
    u8 borderColor;
    u8 highlightColor;
};

// Defaults: min 0, displayMax 100, std-window-ish colors (track 3 / thumb 1 /
// border 13 / highlight 14). Caller still sets windowId, label, value, getMax.
void ValueSlider_SetDefaults(struct ValueSliderConfig *config);

u16 ValueSlider_GetEffectiveMax(const struct ValueSliderConfig *config);

// Compact one-row slider: label | track | value at the given row Y.
void ValueSlider_DrawInline(const struct ValueSliderConfig *config, u8 y,
                            bool8 active, const u8 *textColors);

bool8 ValueSlider_ProcessInput(const struct ValueSliderConfig *config, u8 *heldFrames, u8 *lastDir);

#endif // GUARD_VALUE_SLIDER_H
