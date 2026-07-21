#include "global.h"
#include "gflib.h"
#include "framed_panel.h"
#include "menu.h"
#include "text.h"
#include "window.h"

void FramedPanel_Reset(u8 windowId, const struct FramedPanelConfig *config)
{
    u8 fill = FRAMED_PANEL_DEFAULT_FILL;
    u16 frameTile = FRAMED_PANEL_DEFAULT_TILE;
    u8 framePalette = FRAMED_PANEL_DEFAULT_PALETTE;

    if (config != NULL)
    {
        fill = config->fillColor;
        if (config->frameTile != 0)
            frameTile = config->frameTile;
        if (config->framePalette != 0)
            framePalette = config->framePalette;
    }

    FillWindowPixelBuffer(windowId, PIXEL_FILL(fill));
    DrawStdFrameWithCustomTileAndPalette(windowId, FALSE, frameTile, framePalette);
}

void FramedPanel_Flush(u8 windowId)
{
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

void FramedPanel_ShowEmpty(u8 windowId, const struct FramedPanelConfig *config)
{
    FramedPanel_Reset(windowId, config);
    FramedPanel_Flush(windowId);
}

void FramedPanel_ShowText(u8 windowId, const u8 *str, const u8 *textColors,
                          const struct FramedPanelConfig *config)
{
    u8 fontId = FONT_SMALL;
    u8 textX = FRAMED_PANEL_DEFAULT_TEXT_X;
    u8 textY = FRAMED_PANEL_DEFAULT_TEXT_Y;

    if (config != NULL)
    {
        if (config->fontId != 0)
            fontId = config->fontId;
        textX = config->textX;
        textY = config->textY;
    }

    FramedPanel_Reset(windowId, config);
    AddTextPrinterParameterized3(windowId, fontId, textX, textY, textColors, TEXT_SKIP_DRAW, str);
    FramedPanel_Flush(windowId);
}
