#ifndef GUARD_FOOTER_STRIP_H
#define GUARD_FOOTER_STRIP_H

#include "global.h"

// Split a window into left / mid / right regions for mixed footer content
// (text + chart + portrait, etc.). Coords are window-local pixels.
//
// Caller owns the window + palette; this only does layout math.
// See docs/ui-components.md § FooterStrip.

struct FooterStripRegion
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
    s16 cx;
    s16 cy;
};

struct FooterStripLayout
{
    u8 windowId;
    s16 winW;
    s16 winH;
    struct FooterStripRegion left;
    struct FooterStripRegion mid;
    struct FooterStripRegion right;
};

// pad insets the strip on all sides. leftW / rightW are pixel widths inside
// the padded area; mid gets the remainder. If left+right exceed the inner
// width, mid collapses to 0 and left/right are clamped.
void FooterStrip_Split3(u8 windowId, u8 pad, u16 leftW, u16 rightW,
                        struct FooterStripLayout *out);

// Window-local region center → screen coords (for OBJ / MonPortrait ABS).
void FooterStrip_RegionScreenXY(u8 windowId, const struct FooterStripRegion *region,
                                s16 *outX, s16 *outY);

#endif // GUARD_FOOTER_STRIP_H
