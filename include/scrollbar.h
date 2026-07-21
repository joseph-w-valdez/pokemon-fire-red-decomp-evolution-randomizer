#ifndef GUARD_SCROLLBAR_H
#define GUARD_SCROLLBAR_H

#include "global.h"
#include "task.h"

#define SCROLLBAR_NONE TASK_NONE

// Proportional scrollbar painted into an existing window.
//
// Caller owns:
//   - WindowTemplate size/position (tilemapLeft/Top/width/height)
//   - Palette load for the color indices below
//   - PutWindowTilemap / ClearWindowTilemap / FillWindowPixelBuffer
//
// This module owns: track/thumb math, bevel draw, optional ListMenu sync.

struct ScrollbarConfig
{
    u8 windowId;
    u8 trackColor;      // recessed track fill
    u8 thumbColor;      // thumb face
    u8 borderColor;     // track outline + thumb shadow (bottom/right)
    u8 highlightColor;  // thumb highlight (top/left); 0 = use 1 (typical white)
    u8 minThumbPx;      // 0 = default 12
    u8 taskPriority;    // 0 = default 3 (paint after ListMenu)
    u8 padTop;          // inset inside window (px)
    u8 padBottom;
    u8 padLeft;
    u8 padRight;
};

// Returns task id, or SCROLLBAR_NONE if scrolling is not needed / on failure.
// Does not map or clear the window; does not draw when totalItems <= visibleItems.
u8 Scrollbar_Create(const struct ScrollbarConfig *config,
                    u16 *scrollOffset, u16 totalItems, u16 visibleItems);
void Scrollbar_SetRange(u8 taskId, u16 totalItems, u16 visibleItems);
void Scrollbar_Destroy(u8 taskId); // stops the task; does not paint or unmap

// ListMenu: cursorPos is the scroll offset (items off the top).
void Scrollbar_SyncFromListMenu(u8 scrollbarTaskId, u8 listTaskId);

#endif // GUARD_SCROLLBAR_H
