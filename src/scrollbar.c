#include "global.h"
#include "gflib.h"
#include "scrollbar.h"
#include "task.h"
#include "window.h"
#include "list_menu.h"
#include "malloc.h"

#define tStatePtr 0

#define SCROLLBAR_DEFAULT_MIN_THUMB  12
#define SCROLLBAR_DEFAULT_PRIORITY   3
#define SCROLLBAR_DEFAULT_HIGHLIGHT  1

struct Scrollbar
{
    u16 *scrollOffset;
    u16 totalItems;
    u16 visibleItems;
    u8 windowId;
    u8 trackColor;
    u8 thumbColor;
    u8 borderColor;
    u8 highlightColor;
    u8 minThumbPx;
    u8 padTop;
    u8 padBottom;
    u8 padLeft;
    u8 padRight;
};

static void Task_Scrollbar(u8 taskId);
static void Scrollbar_Draw(struct Scrollbar *bar);
static struct Scrollbar *Scrollbar_Get(u8 taskId);

static struct Scrollbar *Scrollbar_Get(u8 taskId)
{
    if (taskId == SCROLLBAR_NONE || !gTasks[taskId].isActive)
        return NULL;
    return (struct Scrollbar *)GetWordTaskArg(taskId, tStatePtr);
}

static void Scrollbar_Draw(struct Scrollbar *bar)
{
    u16 winW = GetWindowAttribute(bar->windowId, WINDOW_WIDTH) * 8;
    u16 winH = GetWindowAttribute(bar->windowId, WINDOW_HEIGHT) * 8;
    u16 trackX = bar->padLeft;
    u16 trackY = bar->padTop;
    u16 trackW;
    u16 trackH;
    u16 maxScroll;
    u16 thumbH;
    u16 travel;
    u16 thumbY;
    u16 offset;
    u16 innerX, innerY, innerW, innerH;

    if (bar->totalItems <= bar->visibleItems || bar->visibleItems == 0)
        return;

    if (winW <= bar->padLeft + bar->padRight || winH <= bar->padTop + bar->padBottom)
        return;

    trackW = winW - bar->padLeft - bar->padRight;
    trackH = winH - bar->padTop - bar->padBottom;
    if (trackW < 4 || trackH < 4)
        return;

    maxScroll = bar->totalItems - bar->visibleItems;

    // Outer border
    FillWindowPixelRect(bar->windowId, PIXEL_FILL(bar->borderColor),
                        trackX, trackY, trackW, trackH);
    // Recessed track (1px inset)
    innerX = trackX + 1;
    innerY = trackY + 1;
    innerW = trackW - 2;
    innerH = trackH - 2;
    FillWindowPixelRect(bar->windowId, PIXEL_FILL(bar->trackColor),
                        innerX, innerY, innerW, innerH);

    thumbH = (innerH * bar->visibleItems) / bar->totalItems;
    if (thumbH < bar->minThumbPx)
        thumbH = bar->minThumbPx;
    if (thumbH > innerH)
        thumbH = innerH;

    travel = innerH - thumbH;
    offset = *bar->scrollOffset;
    if (offset > maxScroll)
        offset = maxScroll;

    if (maxScroll == 0)
        thumbY = innerY;
    else
        thumbY = innerY + (offset * travel) / maxScroll;

    // Thumb face
    FillWindowPixelRect(bar->windowId, PIXEL_FILL(bar->thumbColor),
                        innerX, thumbY, innerW, thumbH);

    // Bevel: light top + left, dark bottom + right (needs >= 3px thumb)
    if (thumbH >= 3 && innerW >= 3)
    {
        FillWindowPixelRect(bar->windowId, PIXEL_FILL(bar->highlightColor),
                            innerX, thumbY, innerW - 1, 1);
        FillWindowPixelRect(bar->windowId, PIXEL_FILL(bar->highlightColor),
                            innerX, thumbY, 1, thumbH - 1);
        // Full bottom edge (includes bottom-left; left highlight stops one px short)
        FillWindowPixelRect(bar->windowId, PIXEL_FILL(bar->borderColor),
                            innerX, thumbY + thumbH - 1, innerW, 1);
        FillWindowPixelRect(bar->windowId, PIXEL_FILL(bar->borderColor),
                            innerX + innerW - 1, thumbY, 1, thumbH - 1);
    }

    CopyWindowToVram(bar->windowId, COPYWIN_GFX);
}

static void Task_Scrollbar(u8 taskId)
{
    struct Scrollbar *bar = Scrollbar_Get(taskId);

    if (bar == NULL || bar->scrollOffset == NULL)
        return;

    // Always redraw: ListMenu may have repainted over the track this frame.
    Scrollbar_Draw(bar);
}

u8 Scrollbar_Create(const struct ScrollbarConfig *config,
                    u16 *scrollOffset, u16 totalItems, u16 visibleItems)
{
    struct Scrollbar *bar;
    u8 taskId;
    u8 priority;

    if (config == NULL || scrollOffset == NULL)
        return SCROLLBAR_NONE;

    if (totalItems <= visibleItems || visibleItems == 0)
        return SCROLLBAR_NONE;

    bar = AllocZeroed(sizeof(*bar));
    if (bar == NULL)
        return SCROLLBAR_NONE;

    priority = (config->taskPriority != 0) ? config->taskPriority : SCROLLBAR_DEFAULT_PRIORITY;
    taskId = CreateTask(Task_Scrollbar, priority);
    if (taskId == TASK_NONE)
    {
        Free(bar);
        return SCROLLBAR_NONE;
    }

    SetWordTaskArg(taskId, tStatePtr, (uintptr_t)bar);

    bar->scrollOffset = scrollOffset;
    bar->totalItems = totalItems;
    bar->visibleItems = visibleItems;
    bar->windowId = config->windowId;
    bar->trackColor = config->trackColor;
    bar->thumbColor = config->thumbColor;
    bar->borderColor = config->borderColor;
    bar->highlightColor = (config->highlightColor != 0) ? config->highlightColor : SCROLLBAR_DEFAULT_HIGHLIGHT;
    bar->minThumbPx = (config->minThumbPx != 0) ? config->minThumbPx : SCROLLBAR_DEFAULT_MIN_THUMB;
    bar->padTop = config->padTop;
    bar->padBottom = config->padBottom;
    bar->padLeft = config->padLeft;
    bar->padRight = config->padRight;

    return taskId;
}

void Scrollbar_SetRange(u8 taskId, u16 totalItems, u16 visibleItems)
{
    struct Scrollbar *bar = Scrollbar_Get(taskId);

    if (bar == NULL)
        return;

    bar->totalItems = totalItems;
    bar->visibleItems = visibleItems;
}

void Scrollbar_Destroy(u8 taskId)
{
    struct Scrollbar *bar = Scrollbar_Get(taskId);

    if (taskId == SCROLLBAR_NONE)
        return;

    // No paint on destroy — caller redraws / unmaps the window if needed.
    if (bar != NULL)
        Free(bar);
    DestroyTask(taskId);
}

void Scrollbar_SyncFromListMenu(u8 scrollbarTaskId, u8 listTaskId)
{
    struct Scrollbar *bar = Scrollbar_Get(scrollbarTaskId);
    u16 scrollOffset;

    if (bar == NULL || bar->scrollOffset == NULL)
        return;

    // ListMenu naming is inverted vs intuition: cursorPos == items scrolled off the
    // top; itemsAbove == cursor row within the visible window (0 .. maxShowed-1).
    ListMenuGetScrollAndRow(listTaskId, &scrollOffset, NULL);
    *bar->scrollOffset = scrollOffset;
}
