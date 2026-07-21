#include "global.h"
#include "footer_strip.h"
#include "window.h"

static void FooterStrip_FillRegion(struct FooterStripRegion *r, s16 x, s16 y, s16 w, s16 h)
{
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    r->cx = x + w / 2;
    r->cy = y + h / 2;
}

void FooterStrip_Split3(u8 windowId, u8 pad, u16 leftW, u16 rightW,
                        struct FooterStripLayout *out)
{
    s16 winW;
    s16 winH;
    s16 innerW;
    s16 innerH;
    s16 midW;
    s16 y;
    u16 left;
    u16 right;

    if (out == NULL)
        return;

    winW = GetWindowAttribute(windowId, WINDOW_WIDTH) * 8;
    winH = GetWindowAttribute(windowId, WINDOW_HEIGHT) * 8;
    out->windowId = windowId;
    out->winW = winW;
    out->winH = winH;

    if (pad * 2 >= winW || pad * 2 >= winH)
    {
        FooterStrip_FillRegion(&out->left, 0, 0, winW, winH);
        FooterStrip_FillRegion(&out->mid, 0, 0, 0, 0);
        FooterStrip_FillRegion(&out->right, 0, 0, 0, 0);
        return;
    }

    innerW = winW - pad * 2;
    innerH = winH - pad * 2;
    y = pad;
    left = leftW;
    right = rightW;

    if (left + right > (u16)innerW)
    {
        // Prefer keeping right (icon) if both don't fit.
        if (right > (u16)innerW)
            right = innerW;
        left = innerW - right;
    }

    midW = innerW - left - right;

    FooterStrip_FillRegion(&out->left, pad, y, left, innerH);
    FooterStrip_FillRegion(&out->mid, pad + left, y, midW, innerH);
    FooterStrip_FillRegion(&out->right, pad + left + midW, y, right, innerH);
}

void FooterStrip_RegionScreenXY(u8 windowId, const struct FooterStripRegion *region,
                                s16 *outX, s16 *outY)
{
    s16 left;
    s16 top;

    if (region == NULL || outX == NULL || outY == NULL)
        return;

    left = GetWindowAttribute(windowId, WINDOW_TILEMAP_LEFT) * 8;
    top = GetWindowAttribute(windowId, WINDOW_TILEMAP_TOP) * 8;
    *outX = left + region->cx;
    *outY = top + region->cy;
}
