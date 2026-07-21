#ifndef GUARD_FRAMED_PANEL_H
#define GUARD_FRAMED_PANEL_H

#include "global.h"

// Std-frame window helpers (Fill + DrawStdFrame + optional text).
//
// Caller owns:
//   - WindowTemplate size/position
//   - Palette / std-window gfx load (LoadStdWindowGfx, etc.)
//
// This module owns: fill, frame draw, optional text print, map/copy helpers.
// Same “caller owns the window” split as scrollbar.h.

#define FRAMED_PANEL_DEFAULT_FILL     1
#define FRAMED_PANEL_DEFAULT_TILE     0x1C0
#define FRAMED_PANEL_DEFAULT_PALETTE  14
#define FRAMED_PANEL_DEFAULT_TEXT_X   4
#define FRAMED_PANEL_DEFAULT_TEXT_Y   4

struct FramedPanelConfig
{
    u8 fillColor;      // PIXEL_FILL index; unused fields ignored when using helpers below
    u16 frameTile;     // std frame base tile
    u8 framePalette;   // std frame palette num
    u8 fontId;         // ShowText; 0 = FONT_SMALL
    u8 textX;          // ShowText; with NULL config defaults to 4,4
    u8 textY;
};

// Fill interior + draw std frame. Does not map or copy (paint into the window after).
// config may be NULL → fill 1, tile 0x1C0, palette 14.
void FramedPanel_Reset(u8 windowId, const struct FramedPanelConfig *config);

// PutWindowTilemap + CopyWindowToVram(FULL).
void FramedPanel_Flush(u8 windowId);

// Reset + Flush (empty framed panel).
void FramedPanel_ShowEmpty(u8 windowId, const struct FramedPanelConfig *config);

// Reset + print str + Flush. Typical footer / prompt strip.
// textColors is the usual u8[3] for AddTextPrinterParameterized3.
// config may be NULL → footer defaults (FONT_SMALL at 4,4 + std frame).
void FramedPanel_ShowText(u8 windowId, const u8 *str, const u8 *textColors,
                          const struct FramedPanelConfig *config);

#endif // GUARD_FRAMED_PANEL_H
