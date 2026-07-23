#include "global.h"
#include "gflib.h"
#include "bg.h"
#include "gpu_regs.h"
#include "list_menu.h"
#include "menu.h"
#include "new_menu_helpers.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_summary_screen.h"
#include "scanline_effect.h"
#include "sound.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "item.h"
#include "overworld.h"
#include "party_menu.h"
#include "text.h"
#include "rh_stat_editor.h"
#include "scrollbar.h"
#include "framed_panel.h"
#include "value_slider.h"
#include "mon_portrait.h"
#include "stat_radar.h"
#include "footer_strip.h"
#include "type_icon.h"
#include "ui_theme.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/rgb.h"
#include "constants/songs.h"

enum
{
    WIN_LIST,
    WIN_SCROLL,
    WIN_FOOTER,
    WIN_HP_PILL, // BG1 same-pal type host (idx 0 punches through to BG0 cream)
    WIN_MATTE, // reserved VRAM; confirm dialogs no longer map a cream plate
    WIN_MSG,
};

enum
{
    STATE_FADE_IN,
    STATE_WAIT_FADE_IN,
    STATE_HANDLE_INPUT,
    STATE_CONFIRM_CANCEL,
    STATE_CONFIRM_SAVE,
    STATE_WAIT_FADE_OUT,
    STATE_EXIT,
};

enum
{
    PAGE_MAIN,
    PAGE_NATURE_PICKER,
};

#define tState         data[0]
#define tListTaskId    data[1]
#define tPage          data[2]
#define tNatureRow     data[3]
#define tNatureCol     data[4]
#define tNatureScroll  data[5]

#define LIST_NONE 0xFF

#define STAT_ROW_COUNT 6
#define STAT_EDITOR_ROW_COUNT (3 + 1 + STAT_ROW_COUNT + STAT_ROW_COUNT)
#define STAT_EDITOR_VISIBLE_ROWS 6

#define STAT_EDITOR_LABEL_X  12
#define STAT_EDITOR_VALUE_X  168
#define STAT_EDITOR_NATURE_VALUE_X  96
#define STAT_EDITOR_HEADER_X  8

#define ROW_NATURE 1
#define ROW_EV_START 3
#define ROW_IV_START (ROW_EV_START + STAT_ROW_COUNT + 1)

#define NATURE_PICKER_COLS 3
#define NATURE_PICKER_ROW_COUNT ((NUM_NATURES + NATURE_PICKER_COLS - 1) / NATURE_PICKER_COLS)
#define NATURE_PICKER_VISIBLE_ROWS 4
#define NATURE_PICKER_ROW_HEIGHT 22
#define NATURE_PICKER_GRID_Y 4
#define NATURE_PICKER_INSET_X 8
#define NATURE_PICKER_COL_WIDTH 60
#define NATURE_PICKER_COL_GAP 10
#define NATURE_PICKER_CURSOR_INDENT 8

// Manual pixel nudge after window-relative placement (right / up).
#define STAT_EDITOR_PORTRAIT_NUDGE_X  10
#define STAT_EDITOR_PORTRAIT_NUDGE_Y  (-3)

// FooterStrip_Split3: text | EV radar | party icon (px inside pad).
// Chosen so mid.cx ≈ former full-window center + nudgeX 46.
#define STAT_EDITOR_FOOTER_PAD        2
#define STAT_EDITOR_FOOTER_LEFT_W     132
#define STAT_EDITOR_FOOTER_RIGHT_W    40
#define STAT_EDITOR_FOOTER_TEXT_X     4
#define STAT_EDITOR_HP_TYPE_PAL       13
#define STAT_EDITOR_FOOTER_TILE_LEFT  1
#define STAT_EDITOR_FOOTER_TILE_TOP   15
// Same-pal HP pill canvas (Skills detail pattern). Room for longest "HP "+type FIT.
#define STAT_EDITOR_HP_PILL_TILE_W    7
#define STAT_EDITOR_HP_PILL_TILE_H    2
#define STAT_EDITOR_HP_PILL_BASE      387 // after footer tiles 309–386; before std frame 0x1C0
// Radar place rect nudge — keep in sync with StatEditor_PlaceEvRadar.
#define STAT_EDITOR_RADAR_RECT_NUDGE_X 10
// Match FramedPanel / LoadStdWindowGfx in StatEditor_InitGfx (not stock 0x214).
#define STAT_EDITOR_FRAME_TILE  0x1C0
#define STAT_EDITOR_FRAME_PAL   14
// Std frame gfx is 9 tiles at 0x1C0 (448–456). Dialog windows start after that.
// Layer stack (GBA has BG0–BG3 only):
//   BG0 pri2 editor | BG1 pri1 HP pill | BG3 pri0 MSG | BG2 pri0 YesNo (on top)
// Equal pri0: lower BG# wins, so YesNo(BG2) stacks over MSG(BG3).
// Confirm never touches BG1 — pill stays mapped. No cream matte.
// WIN_MATTE kept in the window table (VRAM reserved) but is not mapped.
#define STAT_EDITOR_MATTE_BASE  457
// Reserved plate size (unused at runtime); keeps MSG/YesNo baseBlocks stable.
#define STAT_EDITOR_MATTE_W     28
#define STAT_EDITOR_MATTE_H     7
#define STAT_EDITOR_MSG_BASE    (STAT_EDITOR_MATTE_BASE + STAT_EDITOR_MATTE_W * STAT_EDITOR_MATTE_H) // 653
#define STAT_EDITOR_YESNO_BASE  (STAT_EDITOR_MSG_BASE + 18 * 5) // 743
// Pixel nudge for dialog duo (BG2 YesNo + BG3 MSG) via HOFS/VOFS.
#define STAT_EDITOR_DIALOG_NUDGE_X  4
#define STAT_EDITOR_DIALOG_NUDGE_Y  5

struct StatEditorDraft
{
    u16 ev[STAT_ROW_COUNT];
    u16 iv[STAT_ROW_COUNT];
    u8 nature;
};

static EWRAM_DATA u8 sPartySlot = 0;
static EWRAM_DATA u16 sItemId = ITEM_NONE;
static EWRAM_DATA struct StatEditorDraft sDraft;
static EWRAM_DATA u8 sScrollbarTaskId = SCROLLBAR_NONE;
static EWRAM_DATA u16 sScrollbarScroll = 0;
static EWRAM_DATA u32 sSelectedRowId = ROW_NATURE;
static EWRAM_DATA u8 sHeldFrames = 0;
static EWRAM_DATA u8 sLastDir = 0;

static const u8 sTextColors[3] = { 1, 2, 3 };

static const u8 sText_HdrNature[] = _("Nature");
static const u8 sText_HdrEv[] = _("Effort Values");
static const u8 sText_HdrIv[] = _("Individual Values");
static const u8 sText_NatureLabel[] = _("Nature");
static const u8 sText_FooterEvMid[] = _("  EV ");
static const u8 sText_FooterEvEnd[] = _("/510");
static const u8 sText_FooterNaturePicker[] = _("{A_BUTTON} Choose   {B_BUTTON} Back");
static const u8 sText_FooterHintNature[] = _("{A_BUTTON} Nature  {B_BUTTON} Quit");
static const u8 sText_FooterHintSlider[] = _("{DPAD_LEFTRIGHT} adjust  {L_BUTTON}=x4  {R_BUTTON}=x16");
static const u8 sText_ExitWithoutSaving[] = _("Exit without saving\nchanges?");
static const u8 sText_SaveChanges[] = _("Save changes?");

static const u8 sRowDummyLabel[] = _("");

static const u8 sText_StatHp[] = _("HP");
static const u8 sText_StatAtk[] = _("ATK");
static const u8 sText_StatDef[] = _("DEF");
static const u8 sText_StatSpe[] = _("SPE");
static const u8 sText_StatSpA[] = _("SPA");
static const u8 sText_StatSpD[] = _("SPD");

static const u8 *const sStatLabels[STAT_ROW_COUNT] =
{
    sText_StatHp,
    sText_StatAtk,
    sText_StatDef,
    sText_StatSpe,
    sText_StatSpA,
    sText_StatSpD,
};

static const u8 sEvMonDataIds[STAT_ROW_COUNT] =
{
    MON_DATA_HP_EV,
    MON_DATA_ATK_EV,
    MON_DATA_DEF_EV,
    MON_DATA_SPEED_EV,
    MON_DATA_SPATK_EV,
    MON_DATA_SPDEF_EV,
};

static const u8 sIvMonDataIds[STAT_ROW_COUNT] =
{
    MON_DATA_HP_IV,
    MON_DATA_ATK_IV,
    MON_DATA_DEF_IV,
    MON_DATA_SPEED_IV,
    MON_DATA_SPATK_IV,
    MON_DATA_SPDEF_IV,
};

static const struct ListMenuItem sRowItems[STAT_EDITOR_ROW_COUNT] =
{
    { sText_HdrNature, LIST_HEADER },
    { sRowDummyLabel, ROW_NATURE },
    { sText_HdrEv, LIST_HEADER },
    { sRowDummyLabel, ROW_EV_START + 0 },
    { sRowDummyLabel, ROW_EV_START + 1 },
    { sRowDummyLabel, ROW_EV_START + 2 },
    { sRowDummyLabel, ROW_EV_START + 3 },
    { sRowDummyLabel, ROW_EV_START + 4 },
    { sRowDummyLabel, ROW_EV_START + 5 },
    { sText_HdrIv, LIST_HEADER },
    { sRowDummyLabel, ROW_IV_START + 0 },
    { sRowDummyLabel, ROW_IV_START + 1 },
    { sRowDummyLabel, ROW_IV_START + 2 },
    { sRowDummyLabel, ROW_IV_START + 3 },
    { sRowDummyLabel, ROW_IV_START + 4 },
    { sRowDummyLabel, ROW_IV_START + 5 },
};

static const struct ListMenuTemplate sListTemplate =
{
    .items = sRowItems,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .itemPrintFunc = NULL,
    .totalItems = STAT_EDITOR_ROW_COUNT,
    .maxShowed = STAT_EDITOR_VISIBLE_ROWS,
    .windowId = WIN_LIST,
    .header_X = STAT_EDITOR_HEADER_X,
    .item_X = 200,
    .cursor_X = 0,
    .upText_Y = 2,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 0,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = FONT_NORMAL,
    .cursorKind = 0,
};

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2, // editor chrome
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1, // HP pill (stays up during confirm)
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 0,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0, // confirm YesNo (on top of MSG)
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0, // confirm MSG (under YesNo; over pill)
        .baseTile = 0
    }
};

static const struct WindowTemplate sWinTemplates[] =
{
    [WIN_LIST] =
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 2,
        .width = 26,
        .height = 11,
        .paletteNum = 15,
        .baseBlock = 1
    },
    [WIN_SCROLL] =
    {
        .bg = 0,
        .tilemapLeft = 28,
        .tilemapTop = 2,
        .width = 2,
        .height = 11,
        .paletteNum = 15,
        .baseBlock = 287
    },
    [WIN_FOOTER] =
    {
        .bg = 0,
        .tilemapLeft = STAT_EDITOR_FOOTER_TILE_LEFT,
        .tilemapTop = STAT_EDITOR_FOOTER_TILE_TOP,
        .width = 26,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 309
    },
    // Same-pal type window over footer gap; tilemapLeft updated at draw time.
    [WIN_HP_PILL] =
    {
        .bg = 1,
        .tilemapLeft = 10,
        .tilemapTop = STAT_EDITOR_FOOTER_TILE_TOP,
        .width = STAT_EDITOR_HP_PILL_TILE_W,
        .height = STAT_EDITOR_HP_PILL_TILE_H,
        .paletteNum = STAT_EDITOR_HP_TYPE_PAL,
        .baseBlock = STAT_EDITOR_HP_PILL_BASE
    },
    // Reserved opaque plate (not mapped). VRAM still reserved so MSG/YesNo bases stay put.
    [WIN_MATTE] =
    {
        .bg = 1,
        .tilemapLeft = 1,
        .tilemapTop = 7,
        .width = STAT_EDITOR_MATTE_W,
        .height = STAT_EDITOR_MATTE_H,
        .paletteNum = 15,
        .baseBlock = STAT_EDITOR_MATTE_BASE
    },
    [WIN_MSG] =
    {
        .bg = 3, // under YesNo (BG2); leaves BG1 free for HP pill
        .tilemapLeft = 2,
        .tilemapTop = 8,
        .width = 18,
        .height = 5,
        .paletteNum = 15,
        .baseBlock = STAT_EDITOR_MSG_BASE
    },
    DUMMY_WIN_TEMPLATE
};

// CreateYesNoMenu — BG2 on top of WIN_MSG (BG3); left=21 overlaps MSG's right
// frame without deleting MSG tiles (different BG tilemaps).
static const struct WindowTemplate sYesNoWindowTemplate =
{
    .bg = 2,
    .tilemapLeft = 21,
    .tilemapTop = 9,
    .width = 6,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = STAT_EDITOR_YESNO_BASE
};

static const struct ScrollbarConfig sScrollbarConfig =
{
    .windowId = WIN_SCROLL,
    .trackColor = 3,
    .thumbColor = 14,
    .borderColor = 13,
    .highlightColor = 5,
    .minThumbPx = 12,
    .taskPriority = 0,
    .padTop = 2,
    .padBottom = 2,
    .padLeft = 4,
    .padRight = 2,
};

static void VBlankCB_StatEditor(void);
static void CB2_StatEditor(void);
static void Task_StatEditor(u8 taskId);
static void StatEditor_InitGfx(void);
static void StatEditor_BeginClose(u8 taskId);
static void StatEditor_ShowConfirmDialog(u8 taskId, const u8 *str, u8 nextState);
static void StatEditor_ShowCancelConfirm(u8 taskId);
static void StatEditor_ShowSaveConfirm(u8 taskId);
static void StatEditor_DismissConfirm(u8 taskId);
static void StatEditor_HandleCancelConfirm(u8 taskId);
static void StatEditor_HandleSaveConfirm(u8 taskId);
static void StatEditor_HideDialogBg(u8 taskId);
static void StatEditor_HideHpTypeIcon(void);
static void StatEditor_DrawFooterHpTypeIcon(u16 leftEdge);
static void StatEditor_DestroyScrollbar(void);
static void StatEditor_HideScrollStrip(void);
static void StatEditor_DestroyList(u8 taskId);
static void StatEditor_CreateScrollbar(u8 taskId);
static void StatEditor_CreateNatureScrollbar(u8 taskId);
static void StatEditor_UpdateScrollbar(u8 taskId);
static void StatEditor_UpdateNatureScrollbar(u8 taskId);
static void StatEditor_LoadDraftFromMon(void);
static void StatEditor_ApplyDraft(void);
static u16 StatEditor_GetEvTotal(void);
static u16 StatEditor_GetMaxEvForStat(u8 statIndex);
static void StatEditor_PrintRow(u8 windowId, u32 row, u8 y);
static void StatEditor_MoveCursor(s32 itemIndex, bool8 onInit, struct ListMenu *list);
static void StatEditor_ShowMain(u8 taskId);
static void StatEditor_PrintFooter(u8 taskId);
static void StatEditor_InitFooter(u8 taskId);
static void StatEditor_DrawEvRadar(void);
static void StatEditor_RedrawFocusedSliderRow(u8 taskId);
static void StatEditor_HandleMainInput(u8 taskId);
static bool8 StatEditor_IsSliderRow(u32 row);
static u8 StatEditor_GetStatIndexFromRow(u32 row);
static bool8 StatEditor_IsEvRow(u32 row);
static u32 StatEditor_GetSelectedRow(u8 taskId);
static u16 StatEditor_SliderGetEvMax(void *ctx);
static void StatEditor_BuildSliderConfigForRow(u32 row, struct ValueSliderConfig *config);
static bool8 StatEditor_NaturePickerPosValid(u8 row, u8 col);
static u8 StatEditor_NaturePickerPosToIndex(u8 row, u8 col);
static u8 StatEditor_NaturePickerColX(u8 col);
static void StatEditor_NaturePickerScrollToCursor(u8 taskId);
static void StatEditor_DrawNaturePicker(u8 taskId);
static void StatEditor_ShowNaturePicker(u8 taskId);
static void StatEditor_CloseNaturePicker(u8 taskId);
static void StatEditor_HandleNaturePickerInput(u8 taskId);
static struct Pokemon *StatEditor_GetMon(void);

void RhStatEditor_SetPending(u8 partySlot, u16 itemId)
{
    sPartySlot = partySlot;
    sItemId = itemId;
}

static struct Pokemon *StatEditor_GetMon(void)
{
    return &gPlayerParty[sPartySlot];
}

static void StatEditor_LoadDraftFromMon(void)
{
    struct Pokemon *mon = StatEditor_GetMon();
    u8 i;

    for (i = 0; i < STAT_ROW_COUNT; i++)
    {
        sDraft.ev[i] = GetMonData(mon, sEvMonDataIds[i], NULL);
        sDraft.iv[i] = GetMonData(mon, sIvMonDataIds[i], NULL);
    }
    sDraft.nature = GetNature(mon);
}

static u16 StatEditor_GetEvTotal(void)
{
    u16 total = 0;
    u8 i;

    for (i = 0; i < STAT_ROW_COUNT; i++)
        total += sDraft.ev[i];
    return total;
}

static u16 StatEditor_GetMaxEvForStat(u8 statIndex)
{
    u16 otherTotal = StatEditor_GetEvTotal() - sDraft.ev[statIndex];
    u16 maxForStat = MAX_TOTAL_EVS - otherTotal;

    if (maxForStat > 252)
        maxForStat = 252;
    return maxForStat;
}

static void StatEditor_ApplyDraft(void)
{
    struct Pokemon *mon = StatEditor_GetMon();
    u8 i;
    u8 byteValue;
    u16 itemId = sItemId;

    for (i = 0; i < STAT_ROW_COUNT; i++)
    {
        byteValue = (u8)sDraft.ev[i];
        SetMonData(mon, sEvMonDataIds[i], &byteValue);
        byteValue = (u8)sDraft.iv[i];
        SetMonData(mon, sIvMonDataIds[i], &byteValue);
    }
    SetMonNature(mon, sDraft.nature);
    CalculateMonStats(mon);

    // Editor is only opened via FULL MAKEOVER; don't rely solely on sItemId in case
    // it was never latched (RemoveBagItem(ITEM_NONE) is a no-op).
    if (itemId == ITEM_NONE)
        itemId = ITEM_FULL_MAKEOVER;
    RemoveBagItem(itemId, 1);
    sItemId = ITEM_NONE;
}

static void StatEditor_PrintRow(u8 windowId, u32 row, u8 y)
{
    struct ValueSliderConfig config;

    if (row == (u32)LIST_HEADER)
        return;

    if (row == ROW_NATURE)
    {
        AddTextPrinterParameterized4(windowId, FONT_NORMAL, STAT_EDITOR_LABEL_X, y, 0, 0, sTextColors,
                                     TEXT_SPEED_INSTANT, sText_NatureLabel);
        AddTextPrinterParameterized4(windowId, FONT_NORMAL, STAT_EDITOR_NATURE_VALUE_X, y, 0, 0, sTextColors,
                                     TEXT_SPEED_INSTANT, gNatureNamePointers[sDraft.nature]);
        return;
    }

    if (!StatEditor_IsSliderRow(row))
        return;

    StatEditor_BuildSliderConfigForRow(row, &config);
    config.windowId = windowId;
    ValueSlider_DrawInline(&config, y, row == sSelectedRowId, sTextColors);
}

static bool8 StatEditor_IsEvRow(u32 row)
{
    return row >= ROW_EV_START && row < ROW_IV_START;
}

static bool8 StatEditor_IsSliderRow(u32 row)
{
    return StatEditor_IsEvRow(row) || (row >= ROW_IV_START && row < STAT_EDITOR_ROW_COUNT);
}

static u8 StatEditor_GetStatIndexFromRow(u32 row)
{
    if (StatEditor_IsEvRow(row))
        return (u8)(row - ROW_EV_START);
    return (u8)(row - ROW_IV_START);
}

static u32 StatEditor_GetSelectedRow(u8 taskId)
{
    u16 cursorPos;
    u16 itemsAbove;

    if (gTasks[taskId].tListTaskId == LIST_NONE)
        return ROW_NATURE;

    ListMenuGetScrollAndRow(gTasks[taskId].tListTaskId, &cursorPos, &itemsAbove);
    return sRowItems[cursorPos + itemsAbove].index;
}

static u16 StatEditor_SliderGetEvMax(void *ctx)
{
    return StatEditor_GetMaxEvForStat((u8)(uintptr_t)ctx);
}

static void StatEditor_BuildSliderConfigForRow(u32 row, struct ValueSliderConfig *config)
{
    u8 statIndex = StatEditor_GetStatIndexFromRow(row);

    // Recipe: docs/ui-components.md § ValueSlider
    ValueSlider_SetDefaults(config);
    config->windowId = WIN_LIST;
    config->label = sStatLabels[statIndex];
    config->value = StatEditor_IsEvRow(row) ? &sDraft.ev[statIndex] : &sDraft.iv[statIndex];
    config->ctx = (void *)(uintptr_t)statIndex;

    if (StatEditor_IsEvRow(row))
    {
        config->displayMax = 252;
        config->getMax = StatEditor_SliderGetEvMax;
    }
    else
    {
        config->displayMax = 31;
        config->getMax = NULL;
    }
}

static void StatEditor_PlaceEvRadar(struct StatRadarConfig *config, struct FooterStripLayout *strip)
{
    FooterStrip_Split3(WIN_FOOTER, STAT_EDITOR_FOOTER_PAD,
                       STAT_EDITOR_FOOTER_LEFT_W, STAT_EDITOR_FOOTER_RIGHT_W, strip);

    StatRadar_SetDefaults(config);
    StatRadar_ApplySizePreset(config, STAT_RADAR_SIZE_PREVIEW);
    config->axisCount = STAT_ROW_COUNT;
    config->values = sDraft.ev;
    config->valueMax = 252;
    config->gridColor = 3;
    config->fillColor = 14;
    config->outlineColor = 13;
    StatRadar_PlaceInRect(config, WIN_FOOTER,
                          strip->mid.x + STAT_EDITOR_RADAR_RECT_NUDGE_X, strip->mid.y,
                          strip->mid.w, strip->mid.h,
                          StatRadar_PadForSize(STAT_RADAR_SIZE_PREVIEW));
}

static void StatEditor_DrawEvRadar(void)
{
    // Recipe: docs/ui-components.md § StatRadar (PREVIEW) + FooterStrip mid.
    struct StatRadarConfig config;
    struct FooterStripLayout strip;

    StatEditor_PlaceEvRadar(&config, &strip);
    StatRadar_Draw(&config);
}

static EWRAM_DATA bool8 sHpPillMapped = FALSE;

static void StatEditor_HideHpTypeIcon(void)
{
    if (!sHpPillMapped)
        return;
    ClearWindowTilemap(WIN_HP_PILL);
    CopyWindowToVram(WIN_HP_PILL, COPYWIN_MAP);
    sHpPillMapped = FALSE;
}

// Same-pal type window on BG1: pill pixels only; idx 0 punches through to BG0 cream.
// No surround fill / Commit (avoids tile-pad stomping EV text + radar).
static void StatEditor_DrawFooterHpTypeIcon(u16 leftEdge)
{
    u8 type = TypeIcon_CalcHiddenPowerType(sDraft.iv);
    struct TypeIconWidth widthCfg;
    struct StatRadarConfig radar;
    struct FooterStripLayout strip;
    u8 label[TYPE_ICON_LABEL_BUF_SIZE];
    u16 width;
    u16 rightEdge;
    u16 pillX;
    u16 screenX;
    u16 tileLeft;
    u16 drawX;
    u16 canvasW = STAT_EDITOR_HP_PILL_TILE_W * 8;

    StatEditor_PlaceEvRadar(&radar, &strip);
    rightEdge = (u16)(radar.cx + radar.nudgeX - (s16)radar.radius);

    TypeIcon_WidthSetFit(&widthCfg, 1, 0, 0, FALSE);
    TypeIcon_FormatTypeLabel(label, type, gText_TypeIconHpPrefix);
    width = TypeIcon_ResolveWidth(&widthCfg, label);
    if (width > canvasW)
        width = canvasW;

    pillX = TypeIcon_CenterXInSpan(leftEdge, rightEdge, width);
    screenX = STAT_EDITOR_FOOTER_TILE_LEFT * 8 + pillX;
    tileLeft = screenX / 8;
    drawX = screenX - tileLeft * 8;
    if (drawX + width > canvasW)
    {
        // Keep the pill inside the 7-tile canvas.
        u16 overflow = (drawX + width) - canvasW;
        if (tileLeft >= (overflow + 7) / 8)
            tileLeft -= (overflow + 7) / 8;
        else
            tileLeft = 0;
        screenX = STAT_EDITOR_FOOTER_TILE_LEFT * 8 + pillX;
        drawX = screenX - tileLeft * 8;
        if (drawX + width > canvasW)
            drawX = canvasW - width;
    }

    SetWindowAttribute(WIN_HP_PILL, WINDOW_TILEMAP_LEFT, tileLeft);
    SetWindowAttribute(WIN_HP_PILL, WINDOW_TILEMAP_TOP, STAT_EDITOR_FOOTER_TILE_TOP);
    FillWindowPixelBuffer(WIN_HP_PILL, PIXEL_FILL(0));
    TypeIcon_DrawBlankEx(WIN_HP_PILL, type, drawX, 0, &widthCfg, gText_TypeIconHpPrefix);
    PutWindowTilemap(WIN_HP_PILL);
    CopyWindowToVram(WIN_HP_PILL, COPYWIN_FULL);
    sHpPillMapped = TRUE;
}

static void StatEditor_PrintFooter(u8 taskId)
{
    const u8 *hint;
    u32 row = StatEditor_GetSelectedRow(taskId);

    if (gTasks[taskId].tPage == PAGE_NATURE_PICKER)
    {
        StatEditor_HideHpTypeIcon();
        FramedPanel_ShowText(WIN_FOOTER, sText_FooterNaturePicker, sTextColors, NULL);
        return;
    }

    // Content-only refresh so the std frame isn't rebuilt every adjust (avoids flash).
    FillWindowPixelBuffer(WIN_FOOTER, PIXEL_FILL(1));

    GetMonNickname(StatEditor_GetMon(), gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, StatEditor_GetEvTotal(), STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringCopy(gStringVar3, gStringVar1);
    StringAppend(gStringVar3, sText_FooterEvMid);
    StringAppend(gStringVar3, gStringVar2);
    StringAppend(gStringVar3, sText_FooterEvEnd);
    AddTextPrinterParameterized3(WIN_FOOTER, FONT_SMALL, STAT_EDITOR_FOOTER_TEXT_X, 0,
                                 sTextColors, TEXT_SKIP_DRAW, gStringVar3);

    if (StatEditor_IsSliderRow(row))
        hint = sText_FooterHintSlider;
    else
        hint = sText_FooterHintNature;

    AddTextPrinterParameterized3(WIN_FOOTER, FONT_SMALL, STAT_EDITOR_FOOTER_TEXT_X, 11,
                                 sTextColors, TEXT_SKIP_DRAW, hint);
    StatEditor_DrawEvRadar();
    PutWindowTilemap(WIN_FOOTER);
    CopyWindowToVram(WIN_FOOTER, COPYWIN_FULL);
    StatEditor_DrawFooterHpTypeIcon(
        STAT_EDITOR_FOOTER_TEXT_X + GetStringWidth(FONT_SMALL, gStringVar3, 0));
}

static void StatEditor_InitFooter(u8 taskId)
{
    FramedPanel_Reset(WIN_FOOTER, NULL);
    StatEditor_PrintFooter(taskId);
    CopyWindowToVram(WIN_FOOTER, COPYWIN_FULL);
}

static void StatEditor_RedrawFocusedSliderRow(u8 taskId)
{
    u8 listTaskId = gTasks[taskId].tListTaskId;
    u8 y;
    u16 winW;
    struct ValueSliderConfig config;

    if (listTaskId == LIST_NONE || !StatEditor_IsSliderRow(sSelectedRowId))
        return;

    y = ListMenuGetYCoordForPrintingArrowCursor(listTaskId);
    winW = GetWindowAttribute(WIN_LIST, WINDOW_WIDTH) * 8;

    // Only repaint this row — full RedrawListMenu was flashing the whole panel.
    FillWindowPixelRect(WIN_LIST, PIXEL_FILL(1), 0, y, winW, 16);
    AddTextPrinterParameterized4(WIN_LIST, FONT_NORMAL, 0, y, 0, 0, sTextColors,
                                 TEXT_SPEED_INSTANT, gText_SelectorArrow2);
    StatEditor_BuildSliderConfigForRow(sSelectedRowId, &config);
    config.windowId = WIN_LIST;
    ValueSlider_DrawInline(&config, y, TRUE, sTextColors);
    CopyWindowToVram(WIN_LIST, COPYWIN_GFX);
}

static void StatEditor_DestroyScrollbar(void)
{
    if (sScrollbarTaskId != SCROLLBAR_NONE)
    {
        Scrollbar_Destroy(sScrollbarTaskId);
        sScrollbarTaskId = SCROLLBAR_NONE;
    }
    StatEditor_HideScrollStrip();
}

static void StatEditor_HideScrollStrip(void)
{
    ClearWindowTilemap(WIN_SCROLL);
    CopyWindowToVram(WIN_SCROLL, COPYWIN_MAP);
}

static void StatEditor_DestroyList(u8 taskId)
{
    if (gTasks[taskId].tListTaskId != LIST_NONE)
    {
        DestroyListMenuTask(gTasks[taskId].tListTaskId, NULL, NULL);
        gTasks[taskId].tListTaskId = LIST_NONE;
    }
    StatEditor_DestroyScrollbar();
}

static void StatEditor_CreateScrollbar(u8 taskId)
{
    StatEditor_DestroyScrollbar();
    sScrollbarScroll = 0;

    if (STAT_EDITOR_ROW_COUNT <= STAT_EDITOR_VISIBLE_ROWS)
        return;

    sScrollbarTaskId = Scrollbar_Create(&sScrollbarConfig,
                                          &sScrollbarScroll,
                                          STAT_EDITOR_ROW_COUNT,
                                          STAT_EDITOR_VISIBLE_ROWS);
    if (sScrollbarTaskId == SCROLLBAR_NONE)
        return;

    FillWindowPixelBuffer(WIN_SCROLL, PIXEL_FILL(0));
    PutWindowTilemap(WIN_SCROLL);
    CopyWindowToVram(WIN_SCROLL, COPYWIN_FULL);
    StatEditor_UpdateScrollbar(taskId);
}

static void StatEditor_UpdateScrollbar(u8 taskId)
{
    if (sScrollbarTaskId != SCROLLBAR_NONE && gTasks[taskId].tListTaskId != LIST_NONE)
        Scrollbar_SyncFromListMenu(sScrollbarTaskId, gTasks[taskId].tListTaskId);
}

static void StatEditor_CreateNatureScrollbar(u8 taskId)
{
    StatEditor_DestroyScrollbar();
    sScrollbarScroll = gTasks[taskId].tNatureScroll;

    if (NATURE_PICKER_ROW_COUNT <= NATURE_PICKER_VISIBLE_ROWS)
        return;

    sScrollbarTaskId = Scrollbar_Create(&sScrollbarConfig,
                                          &sScrollbarScroll,
                                          NATURE_PICKER_ROW_COUNT,
                                          NATURE_PICKER_VISIBLE_ROWS);
    if (sScrollbarTaskId == SCROLLBAR_NONE)
        return;

    FillWindowPixelBuffer(WIN_SCROLL, PIXEL_FILL(0));
    PutWindowTilemap(WIN_SCROLL);
    CopyWindowToVram(WIN_SCROLL, COPYWIN_FULL);
    StatEditor_UpdateNatureScrollbar(taskId);
}

static void StatEditor_UpdateNatureScrollbar(u8 taskId)
{
    if (sScrollbarTaskId != SCROLLBAR_NONE)
    {
        sScrollbarScroll = gTasks[taskId].tNatureScroll;
    }
}

static void StatEditor_MoveCursor(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    if (sSelectedRowId != (u32)itemIndex)
    {
        sSelectedRowId = itemIndex;
        sHeldFrames = 0;
        sLastDir = 0;
    }
    ListMenuDefaultCursorMoveFunc(itemIndex, onInit, list);
}

static void StatEditor_ShowMain(u8 taskId)
{
    struct ListMenuTemplate template = sListTemplate;

    gTasks[taskId].tPage = PAGE_MAIN;
    StatEditor_DestroyList(taskId);
    FramedPanel_ShowEmpty(WIN_LIST, NULL);

    template.moveCursorFunc = StatEditor_MoveCursor;
    template.itemPrintFunc = StatEditor_PrintRow;
    gTasks[taskId].tListTaskId = ListMenuInit(&template, 0, 0);
    sSelectedRowId = StatEditor_GetSelectedRow(taskId);
    sHeldFrames = 0;
    sLastDir = 0;
    StatEditor_CreateScrollbar(taskId);
    StatEditor_InitFooter(taskId);
}

static bool8 StatEditor_NaturePickerPosValid(u8 row, u8 col)
{
    if (row >= NATURE_PICKER_ROW_COUNT || col >= NATURE_PICKER_COLS)
        return FALSE;
    return StatEditor_NaturePickerPosToIndex(row, col) < NUM_NATURES;
}

static u8 StatEditor_NaturePickerPosToIndex(u8 row, u8 col)
{
    return row * NATURE_PICKER_COLS + col;
}

static u8 StatEditor_NaturePickerColX(u8 col)
{
    return NATURE_PICKER_INSET_X + col * (NATURE_PICKER_COL_WIDTH + NATURE_PICKER_COL_GAP) + NATURE_PICKER_CURSOR_INDENT;
}

static void StatEditor_NaturePickerScrollToCursor(u8 taskId)
{
    u8 row = gTasks[taskId].tNatureRow;
    u8 scroll = gTasks[taskId].tNatureScroll;

    if (row < scroll)
        gTasks[taskId].tNatureScroll = row;
    else if (row >= scroll + NATURE_PICKER_VISIBLE_ROWS)
        gTasks[taskId].tNatureScroll = row - (NATURE_PICKER_VISIBLE_ROWS - 1);
}

static void StatEditor_DrawNaturePicker(u8 taskId)
{
    u8 displayRow;
    u8 row;
    u8 col;
    u8 nature;
    u8 x;
    u8 y;
    u8 scroll = gTasks[taskId].tNatureScroll;
    u8 selRow = gTasks[taskId].tNatureRow;
    u8 selCol = gTasks[taskId].tNatureCol;

    FramedPanel_Reset(WIN_LIST, NULL);

    for (displayRow = 0; displayRow < NATURE_PICKER_VISIBLE_ROWS; displayRow++)
    {
        row = scroll + displayRow;
        if (row >= NATURE_PICKER_ROW_COUNT)
            break;

        y = NATURE_PICKER_GRID_Y + displayRow * NATURE_PICKER_ROW_HEIGHT;
        for (col = 0; col < NATURE_PICKER_COLS; col++)
        {
            if (!StatEditor_NaturePickerPosValid(row, col))
                continue;

            nature = StatEditor_NaturePickerPosToIndex(row, col);
            x = StatEditor_NaturePickerColX(col);

            if (row == selRow && col == selCol)
            {
                gStringVar1[0] = CHAR_RIGHT_ARROW;
                gStringVar1[1] = CHAR_SPACE;
                StringCopy(&gStringVar1[2], gNatureNamePointers[nature]);
                AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, x - NATURE_PICKER_CURSOR_INDENT, y,
                                             sTextColors, TEXT_SKIP_DRAW, gStringVar1);
            }
            else
            {
                AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, x, y, sTextColors, TEXT_SKIP_DRAW,
                                             gNatureNamePointers[nature]);
            }
        }
    }

    FramedPanel_Flush(WIN_LIST);
}

static void StatEditor_ShowNaturePicker(u8 taskId)
{
    gTasks[taskId].tPage = PAGE_NATURE_PICKER;
    gTasks[taskId].tNatureRow = sDraft.nature / NATURE_PICKER_COLS;
    gTasks[taskId].tNatureCol = sDraft.nature % NATURE_PICKER_COLS;
    gTasks[taskId].tNatureScroll = 0;

    StatEditor_DestroyList(taskId);
    StatEditor_NaturePickerScrollToCursor(taskId);
    StatEditor_DrawNaturePicker(taskId);
    StatEditor_CreateNatureScrollbar(taskId);
    StatEditor_HideHpTypeIcon();
    FramedPanel_ShowText(WIN_FOOTER, sText_FooterNaturePicker, sTextColors, NULL);
}

static void StatEditor_CloseNaturePicker(u8 taskId)
{
    StatEditor_ShowMain(taskId);
}

static void StatEditor_HandleNaturePickerInput(u8 taskId)
{
    u8 row = gTasks[taskId].tNatureRow;
    u8 col = gTasks[taskId].tNatureCol;
    bool8 moved = FALSE;

    StatEditor_UpdateNatureScrollbar(taskId);

    if (JOY_NEW(SELECT_BUTTON))
    {
        UiTheme_Cycle(TRUE);
        return;
    }

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        StatEditor_CloseNaturePicker(taskId);
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sDraft.nature = StatEditor_NaturePickerPosToIndex(row, col);
        StatEditor_CloseNaturePicker(taskId);
        return;
    }

    if (JOY_HELD(DPAD_RIGHT))
    {
        do
        {
            if (col < NATURE_PICKER_COLS - 1)
                col++;
            else if (row < NATURE_PICKER_ROW_COUNT - 1)
            {
                row++;
                col = 0;
            }
            else
                break;
        } while (!StatEditor_NaturePickerPosValid(row, col));
        moved = TRUE;
    }
    else if (JOY_HELD(DPAD_LEFT))
    {
        do
        {
            if (col > 0)
                col--;
            else if (row > 0)
            {
                row--;
                col = NATURE_PICKER_COLS - 1;
            }
            else
                break;
        } while (!StatEditor_NaturePickerPosValid(row, col));
        moved = TRUE;
    }
    else if (JOY_HELD(DPAD_DOWN))
    {
        if (row < NATURE_PICKER_ROW_COUNT - 1)
        {
            row++;
            while (!StatEditor_NaturePickerPosValid(row, col) && col > 0)
                col--;
            if (StatEditor_NaturePickerPosValid(row, col))
                moved = TRUE;
            else
                row--;
        }
    }
    else if (JOY_HELD(DPAD_UP))
    {
        if (row > 0)
        {
            row--;
            while (!StatEditor_NaturePickerPosValid(row, col) && col > 0)
                col--;
            if (StatEditor_NaturePickerPosValid(row, col))
                moved = TRUE;
            else
                row++;
        }
    }

    if (!moved)
        return;

    PlaySE(SE_SELECT);
    gTasks[taskId].tNatureRow = row;
    gTasks[taskId].tNatureCol = col;
    StatEditor_NaturePickerScrollToCursor(taskId);
    StatEditor_DrawNaturePicker(taskId);
    StatEditor_UpdateNatureScrollbar(taskId);
}

static void StatEditor_HandleMainInput(u8 taskId)
{
    u8 listTaskId = gTasks[taskId].tListTaskId;
    s32 input;
    u32 selectedRow;
    u32 prevRow;
    struct ValueSliderConfig config;
    bool8 changed;

    prevRow = sSelectedRowId;
    input = ListMenu_ProcessInput(listTaskId);
    StatEditor_UpdateScrollbar(taskId);
    selectedRow = StatEditor_GetSelectedRow(taskId);
    sSelectedRowId = selectedRow;

    if (selectedRow != prevRow)
        StatEditor_PrintFooter(taskId);

    if (JOY_NEW(SELECT_BUTTON))
    {
        UiTheme_Cycle(TRUE);
        return;
    }

    if (JOY_NEW(START_BUTTON))
    {
        StatEditor_ShowSaveConfirm(taskId);
        return;
    }

    if (input == LIST_CANCEL)
    {
        StatEditor_ShowCancelConfirm(taskId);
        return;
    }

    if (input != LIST_NOTHING_CHOSEN)
    {
        if (selectedRow == ROW_NATURE)
        {
            PlaySE(SE_SELECT);
            StatEditor_ShowNaturePicker(taskId);
        }
        return;
    }

    if (StatEditor_IsSliderRow(selectedRow))
    {
        StatEditor_BuildSliderConfigForRow(selectedRow, &config);
        changed = ValueSlider_ProcessInput(&config, &sHeldFrames, &sLastDir);
        if (changed)
        {
            StatEditor_RedrawFocusedSliderRow(taskId);
            StatEditor_PrintFooter(taskId);
        }
    }
}

static void VBlankCB_StatEditor(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_StatEditor(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void StatEditor_BeginClose(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].tState = STATE_WAIT_FADE_OUT;
}

static void StatEditor_ShowConfirmDialog(u8 taskId, const u8 *str, u8 nextState)
{
    struct FramedPanelConfig msgCfg;

    PlaySE(SE_SELECT);
    // MSG on BG3, YesNo on BG2 (on top). BG1 HP pill stays put. Overlap is OK —
    // separate tilemaps, so frames do not delete each other. No cream matte;
    // transparent frame corners punch to MSG fill / pill / BG0 underneath.
    FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(3, 0, 0, 0, 32, 32);
    // HOFS/VOFS: +x moves content left, +y moves content up (sub-tile nudge).
    ChangeBgX(2, STAT_EDITOR_DIALOG_NUDGE_X << 8, BG_COORD_SET);
    ChangeBgY(2, STAT_EDITOR_DIALOG_NUDGE_Y << 8, BG_COORD_SET);
    ChangeBgX(3, STAT_EDITOR_DIALOG_NUDGE_X << 8, BG_COORD_SET);
    ChangeBgY(3, STAT_EDITOR_DIALOG_NUDGE_Y << 8, BG_COORD_SET);
    ShowBg(2);
    ShowBg(3);

    msgCfg.fillColor = FRAMED_PANEL_DEFAULT_FILL;
    msgCfg.frameTile = STAT_EDITOR_FRAME_TILE;
    msgCfg.framePalette = STAT_EDITOR_FRAME_PAL;
    msgCfg.fontId = FONT_NORMAL;
    msgCfg.textX = 8;
    msgCfg.textY = 8;
    FramedPanel_ShowText(WIN_MSG, str, sTextColors, &msgCfg);
    CreateYesNoMenu(&sYesNoWindowTemplate, FONT_NORMAL, 0, 2,
                    STAT_EDITOR_FRAME_TILE, STAT_EDITOR_FRAME_PAL, 0);
    gTasks[taskId].tState = nextState;
}

static void StatEditor_ShowCancelConfirm(u8 taskId)
{
    StatEditor_ShowConfirmDialog(taskId, sText_ExitWithoutSaving, STATE_CONFIRM_CANCEL);
}

static void StatEditor_ShowSaveConfirm(u8 taskId)
{
    StatEditor_ShowConfirmDialog(taskId, sText_SaveChanges, STATE_CONFIRM_SAVE);
}

static void StatEditor_HideDialogBg(u8 taskId)
{
    ClearStdWindowAndFrameToTransparent(WIN_MSG, FALSE);
    ClearWindowTilemap(WIN_MSG);
    FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(3, 0, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(2);
    CopyBgTilemapBufferToVram(3);
    ChangeBgX(2, 0, BG_COORD_SET);
    ChangeBgY(2, 0, BG_COORD_SET);
    ChangeBgX(3, 0, BG_COORD_SET);
    ChangeBgY(3, 0, BG_COORD_SET);
    HideBg(2);
    HideBg(3);
}

static void StatEditor_DismissConfirm(u8 taskId)
{
    // YesNo already destroyed by Menu_ProcessInputNoWrapClearOnChoose.
    StatEditor_HideDialogBg(taskId);
    gTasks[taskId].tState = STATE_HANDLE_INPUT;
}

static void StatEditor_HandleCancelConfirm(u8 taskId)
{
    s8 input = Menu_ProcessInputNoWrapClearOnChoose();

    switch (input)
    {
    case 0: // YES — exit without saving (no ApplyDraft / no item consume)
        PlaySE(SE_SELECT);
        StatEditor_HideDialogBg(taskId);
        StatEditor_BeginClose(taskId);
        break;
    case 1: // NO
    case MENU_B_PRESSED:
        PlaySE(SE_SELECT);
        StatEditor_DismissConfirm(taskId);
        break;
    }
}

static void StatEditor_HandleSaveConfirm(u8 taskId)
{
    s8 input = Menu_ProcessInputNoWrapClearOnChoose();

    switch (input)
    {
    case 0: // YES — apply draft, consume item, exit
        PlaySE(SE_USE_ITEM);
        StatEditor_HideDialogBg(taskId);
        StatEditor_ApplyDraft();
        StatEditor_BeginClose(taskId);
        break;
    case 1: // NO
    case MENU_B_PRESSED:
        PlaySE(SE_SELECT);
        StatEditor_DismissConfirm(taskId);
        break;
    }
}

static void Task_StatEditor(u8 taskId)
{
    switch (gTasks[taskId].tState)
    {
    case STATE_FADE_IN:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gTasks[taskId].tState = STATE_WAIT_FADE_IN;
        break;
    case STATE_WAIT_FADE_IN:
        if (!gPaletteFade.active)
            gTasks[taskId].tState = STATE_HANDLE_INPUT;
        break;
    case STATE_HANDLE_INPUT:
        if (gTasks[taskId].tPage == PAGE_NATURE_PICKER)
            StatEditor_HandleNaturePickerInput(taskId);
        else
            StatEditor_HandleMainInput(taskId);
        break;
    case STATE_CONFIRM_CANCEL:
        StatEditor_HandleCancelConfirm(taskId);
        break;
    case STATE_CONFIRM_SAVE:
        StatEditor_HandleSaveConfirm(taskId);
        break;
    case STATE_WAIT_FADE_OUT:
        if (!gPaletteFade.active)
            gTasks[taskId].tState = STATE_EXIT;
        break;
    case STATE_EXIT:
        MonPortrait_Destroy();
        FreeAllWindowBuffers();
        DestroyTask(taskId);
        SetMainCallback2(gMain.savedCallback);
        break;
    }
}

static void StatEditor_InitGfx(void)
{
    u8 taskId;

    SetVBlankCallback(NULL);
    SetHBlankCallback(NULL);

    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaClear32(3, (void *)OAM, OAM_SIZE);
    DmaClear16(3, (void *)PLTT, PLTT_SIZE);

    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    ChangeBgX(1, 0, BG_COORD_SET);
    ChangeBgY(1, 0, BG_COORD_SET);
    ChangeBgX(2, 0, BG_COORD_SET);
    ChangeBgY(2, 0, BG_COORD_SET);
    ChangeBgX(3, 0, BG_COORD_SET);
    ChangeBgY(3, 0, BG_COORD_SET);
    InitWindows(sWinTemplates);
    DeactivateAllTextPrinters();

    ResetPaletteFade();
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ScanlineEffect_Stop();

    LoadStdWindowGfx(WIN_LIST, 0x1C0, BG_PLTT_ID(14));
    UiTheme_ApplyStdWindow();
    TypeIcon_LoadPalette(STAT_EDITOR_HP_TYPE_PAL);
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(3, 0, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(0);
    CopyBgTilemapBufferToVram(1);
    CopyBgTilemapBufferToVram(2);
    CopyBgTilemapBufferToVram(3);

    ShowBg(0);
    ShowBg(1); // HP pill punches through to BG0
    HideBg(2);
    HideBg(3);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON
             | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_BG2_ON | DISPCNT_BG3_ON);
    SetVBlankCallback(VBlankCB_StatEditor);

    StatEditor_LoadDraftFromMon();
    taskId = CreateTask(Task_StatEditor, 0);
    gTasks[taskId].tState = STATE_FADE_IN;
    gTasks[taskId].tListTaskId = LIST_NONE;
    gTasks[taskId].tPage = PAGE_MAIN;
    sSelectedRowId = ROW_NATURE;
    sHeldFrames = 0;
    sLastDir = 0;
    sScrollbarTaskId = SCROLLBAR_NONE;
    StatEditor_ShowMain(taskId);
    {
        // Portrait sits in FooterStrip right region (same split as EV radar).
        struct FooterStripLayout strip;
        struct MonPortraitConfig portrait;
        s16 x;
        s16 y;

        FooterStrip_Split3(WIN_FOOTER, STAT_EDITOR_FOOTER_PAD,
                           STAT_EDITOR_FOOTER_LEFT_W, STAT_EDITOR_FOOTER_RIGHT_W, &strip);
        FooterStrip_RegionScreenXY(WIN_FOOTER, &strip.right, &x, &y);
        MonPortrait_SetDefaults(&portrait);
        portrait.usePartySlot = TRUE;
        portrait.partySlot = sPartySlot;
        portrait.align = MON_PORTRAIT_ALIGN_ABS;
        portrait.x = x;
        portrait.y = y;
        portrait.nudgeX = STAT_EDITOR_PORTRAIT_NUDGE_X;
        portrait.nudgeY = STAT_EDITOR_PORTRAIT_NUDGE_Y;
        MonPortrait_Show(&portrait);
    }
    SetMainCallback2(CB2_StatEditor);
}

void CB2_OpenStatEditor(void)
{
    if (gMain.savedCallback == NULL)
        gMain.savedCallback = CB2_ReturnToField;
    StatEditor_InitGfx();
}
