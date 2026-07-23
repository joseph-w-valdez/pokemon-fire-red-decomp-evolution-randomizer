#include "global.h"
#include "gflib.h"
#include "value_slider.h"
#include "menu.h"
#include "text.h"
#include "window.h"
#include "sound.h"
#include "constants/songs.h"

#define VALUE_SLIDER_TRACK_H  3
#define VALUE_SLIDER_THUMB_W  4
#define VALUE_SLIDER_THUMB_H  7
#define VALUE_SLIDER_INLINE_LABEL_X  12
#define VALUE_SLIDER_INLINE_TRACK_X  44
#define VALUE_SLIDER_INLINE_VALUE_W  28

static u16 ValueSlider_GetStep(u8 heldFrames, bool8 lHeld, bool8 rHeld)
{
    // Shoulder buttons: fixed step while D-pad adjusts (R wins if both held).
    if (rHeld)
        return 16;
    if (lHeld)
        return 4;
    // D-pad alone: accelerate the longer you hold.
    if (heldFrames > 30)
        return 16;
    if (heldFrames > 15)
        return 8;
    if (heldFrames > 8)
        return 4;
    return 1;
}

static void ValueSlider_ClampValue(const struct ValueSliderConfig *config)
{
    u16 maxValue = ValueSlider_GetEffectiveMax(config);

    if (*config->value > maxValue)
        *config->value = maxValue;
    if (*config->value < config->min)
        *config->value = config->min;
}

static void ValueSlider_ApplyDelta(const struct ValueSliderConfig *config, s16 delta)
{
    u16 maxValue = ValueSlider_GetEffectiveMax(config);
    s32 newValue = (s32)*config->value + delta;

    if (newValue < config->min)
        newValue = config->min;
    if (newValue > maxValue)
        newValue = maxValue;
    *config->value = (u16)newValue;
}

u16 ValueSlider_GetEffectiveMax(const struct ValueSliderConfig *config)
{
    u16 maxValue;

    if (config->getMax != NULL)
        maxValue = config->getMax(config->ctx);
    else
        maxValue = config->displayMax;

    if (maxValue < config->min)
        maxValue = config->min;
    return maxValue;
}

void ValueSlider_SetDefaults(struct ValueSliderConfig *config)
{
    if (config == NULL)
        return;

    config->windowId = 0;
    config->label = NULL;
    config->min = 0;
    config->displayMax = 100;
    config->value = NULL;
    config->getMax = NULL;
    config->ctx = NULL;
    config->trackColor = 3;
    config->thumbColor = 1;
    config->borderColor = 13;
    config->highlightColor = 14;
}

void ValueSlider_DrawInline(const struct ValueSliderConfig *config, u8 y,
                            bool8 active, const u8 *textColors)
{
    u16 winW = GetWindowAttribute(config->windowId, WINDOW_WIDTH) * 8;
    u16 value = *config->value;
    u16 maxValue = ValueSlider_GetEffectiveMax(config);
    u16 range;
    u16 thumbX;
    u16 thumbTravel;
    u8 trackX = VALUE_SLIDER_INLINE_TRACK_X;
    u8 trackY = y + 4;
    u8 trackW;
    u8 valueX = winW - VALUE_SLIDER_INLINE_VALUE_W;

    if (valueX <= trackX + 8)
        valueX = trackX + 8;
    trackW = valueX - trackX - 4;

    if (config->label != NULL)
        AddTextPrinterParameterized3(config->windowId, FONT_SMALL, VALUE_SLIDER_INLINE_LABEL_X, y,
                                     textColors, TEXT_SKIP_DRAW, config->label);

    FillWindowPixelRect(config->windowId, PIXEL_FILL(config->borderColor),
                        trackX, trackY, trackW, VALUE_SLIDER_TRACK_H);
    FillWindowPixelRect(config->windowId, PIXEL_FILL(config->trackColor),
                        trackX + 1, trackY + 1, trackW - 2, VALUE_SLIDER_TRACK_H - 2);

    range = maxValue - config->min;
    thumbTravel = trackW - 2 - VALUE_SLIDER_THUMB_W;
    if (range == 0 || thumbTravel == 0)
        thumbX = trackX + 1;
    else
        thumbX = trackX + 1 + ((value - config->min) * thumbTravel) / range;

    FillWindowPixelRect(config->windowId, PIXEL_FILL(config->borderColor),
                        thumbX, trackY - 2, VALUE_SLIDER_THUMB_W, VALUE_SLIDER_THUMB_H);
    FillWindowPixelRect(config->windowId, PIXEL_FILL(config->thumbColor),
                        thumbX + 1, trackY - 1, VALUE_SLIDER_THUMB_W - 2, VALUE_SLIDER_THUMB_H - 2);
    if (active)
    {
        FillWindowPixelRect(config->windowId, PIXEL_FILL(config->highlightColor),
                            thumbX + 1, trackY - 1, VALUE_SLIDER_THUMB_W - 2, 1);
    }

    ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_RIGHT_ALIGN, 3);
    AddTextPrinterParameterized3(config->windowId, FONT_SMALL, valueX, y,
                                 textColors, TEXT_SKIP_DRAW, gStringVar1);
}

bool8 ValueSlider_ProcessInput(const struct ValueSliderConfig *config, u8 *heldFrames, u8 *lastDir)
{
    u8 dir = 0;
    u16 step;
    // Raw shoulders so L=A does not remap L away from the boost check.
    bool8 lHeld = (gMain.heldKeysRaw & L_BUTTON) != 0;
    bool8 rHeld = (gMain.heldKeysRaw & R_BUTTON) != 0;

    if (JOY_HELD(DPAD_RIGHT))
        dir = 1;
    else if (JOY_HELD(DPAD_LEFT))
        dir = 2;

    if (dir == 0)
    {
        *heldFrames = 0;
        *lastDir = 0;
        return FALSE;
    }

    // D-pad only moves the value; apply on press / key-repeat ticks (not every frame).
    if (!(gMain.newAndRepeatedKeys & (DPAD_LEFT | DPAD_RIGHT)))
        return FALSE;

    if (dir != *lastDir)
    {
        *heldFrames = 0;
        *lastDir = dir;
    }
    else
    {
        (*heldFrames)++;
    }

    // L/R are step modifiers while D-pad is held (4 / 16); D-pad alone accelerates 1→4→8→16.
    step = ValueSlider_GetStep(*heldFrames, lHeld, rHeld);
    if (dir == 1)
        ValueSlider_ApplyDelta(config, step);
    else
        ValueSlider_ApplyDelta(config, -(s16)step);

    ValueSlider_ClampValue(config);
    if (*heldFrames == 0)
        PlaySE(SE_SELECT);
    return TRUE;
}
