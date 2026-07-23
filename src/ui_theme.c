#include "global.h"
#include "ui_theme.h"
#include "palette.h"
#include "sound.h"
#include "constants/rgb.h"
#include "constants/songs.h"

static EWRAM_DATA u8 sUiThemeIndex = UI_THEME_PORCELAIN;

static const u8 sNamePorcelain[] = _("Porcelain");
static const u8 sNameCarbon[] = _("Carbon");
static const u8 sNameSteel[] = _("Steel");
static const u8 sNameInk[] = _("Ink");
static const u8 sNameSnow[] = _("Snow");
static const u8 sNameBlush[] = _("Blush");
static const u8 sNameCrimson[] = _("Crimson");
static const u8 sNameHoney[] = _("Honey");
static const u8 sNameUnknown[] = _("?");

static const u8 *const sUiThemeNames[UI_THEME_COUNT] =
{
    [UI_THEME_PORCELAIN] = sNamePorcelain,
    [UI_THEME_CARBON] = sNameCarbon,
    [UI_THEME_STEEL] = sNameSteel,
    [UI_THEME_INK] = sNameInk,
    [UI_THEME_SNOW] = sNameSnow,
    [UI_THEME_BLUSH] = sNameBlush,
    [UI_THEME_CRIMSON] = sNameCrimson,
    [UI_THEME_HONEY] = sNameHoney,
};

// Index roles match stdpal_3: 0=gap/BG, 1=fill, 2=text, 3=shadow/track,
// 11-15=frame, 14=accent (slider/scrollbar).
static const u16 sUiThemes[UI_THEME_COUNT][16] =
{
    [UI_THEME_PORCELAIN] = {
        RGB(15, 16, 17),
        RGB(31, 31, 30),
        RGB( 3,  3,  4),
        RGB(20, 20, 21),
        RGB(28,  4,  4),
        RGB(28, 20,  8),
        RGB( 4, 18,  8),
        RGB(14, 26, 16),
        RGB( 5, 14, 26),
        RGB(16, 22, 28),
        RGB(24, 12, 16),
        RGB(28, 28, 28),
        RGB(14, 15, 16),
        RGB( 7,  8,  9),
        RGB( 5, 14, 26),
        RGB(10, 11, 12),
    },
    [UI_THEME_CARBON] = {
        RGB( 2,  2,  3),
        RGB( 5,  5,  7),
        RGB(28, 28, 29),
        RGB(10, 10, 12),
        RGB(28,  4,  4),
        RGB(28, 20,  8),
        RGB( 4, 18,  8),
        RGB(14, 26, 16),
        RGB( 8, 18, 28),
        RGB(14, 22, 28),
        RGB(24, 12, 16),
        RGB(14, 14, 16),
        RGB( 8,  8, 10),
        RGB( 3,  3,  5),
        RGB( 7, 24, 28),
        RGB( 6,  6,  8),
    },
    [UI_THEME_STEEL] = {
        RGB( 5,  7, 10),
        RGB(27, 28, 29),
        RGB( 3,  4,  6),
        RGB(17, 19, 21),
        RGB(28,  4,  4),
        RGB(28, 20,  8),
        RGB( 4, 18,  8),
        RGB(14, 26, 16),
        RGB( 8, 16, 26),
        RGB(16, 22, 28),
        RGB(24, 12, 16),
        RGB(24, 26, 28),
        RGB(10, 13, 16),
        RGB( 4,  6,  8),
        RGB(10, 20, 27),
        RGB( 7,  9, 12),
    },
    [UI_THEME_INK] = {
        RGB( 1,  1,  2),
        RGB(31, 31, 31),
        RGB( 2,  2,  3),
        RGB(18, 18, 19),
        RGB(28,  4,  4),
        RGB(28, 20,  8),
        RGB( 4, 18,  8),
        RGB(14, 26, 16),
        RGB( 6, 14, 28),
        RGB(16, 22, 28),
        RGB(24, 12, 16),
        RGB(28, 28, 28),
        RGB(12, 12, 14),
        RGB( 4,  4,  5),
        RGB( 5, 13, 28),
        RGB( 8,  8, 10),
    },
    [UI_THEME_SNOW] = {
        RGB(26, 27, 28),
        RGB(31, 31, 31),
        RGB( 4,  5,  6),
        RGB(22, 23, 24),
        RGB(28,  4,  4),
        RGB(28, 20,  8),
        RGB( 4, 18,  8),
        RGB(14, 26, 16),
        RGB( 8, 16, 26),
        RGB(18, 24, 28),
        RGB(24, 14, 18),
        RGB(30, 30, 30),
        RGB(18, 19, 20),
        RGB(10, 11, 12),
        RGB( 8, 16, 26),
        RGB(14, 15, 16),
    },
    [UI_THEME_BLUSH] = {
        RGB(28, 20, 24),
        RGB(31, 28, 30),
        RGB(10,  4,  6),
        RGB(26, 20, 22),
        RGB(28,  4,  4),
        RGB(28, 18, 10),
        RGB( 4, 18,  8),
        RGB(14, 26, 16),
        RGB(18, 10, 20),
        RGB(26, 18, 24),
        RGB(28, 12, 18),
        RGB(31, 26, 28),
        RGB(24, 14, 18),
        RGB(16,  8, 12),
        RGB(28, 10, 18),
        RGB(20, 12, 16),
    },
    [UI_THEME_CRIMSON] = {
        RGB(14,  4,  5),
        RGB(30, 26, 24),
        RGB( 8,  2,  2),
        RGB(22, 16, 14),
        RGB(28,  4,  4),
        RGB(28, 18,  6),
        RGB( 4, 18,  8),
        RGB(14, 26, 16),
        RGB(20,  6,  6),
        RGB(28, 16, 14),
        RGB(28, 10, 10),
        RGB(28, 22, 20),
        RGB(18,  8,  8),
        RGB(10,  3,  3),
        RGB(28,  8,  6),
        RGB(14,  6,  6),
    },
    [UI_THEME_HONEY] = {
        RGB(24, 20,  6),
        RGB(31, 30, 24),
        RGB( 8,  6,  2),
        RGB(24, 22, 14),
        RGB(28,  4,  4),
        RGB(28, 22,  4),
        RGB( 4, 18,  8),
        RGB(14, 26, 16),
        RGB(20, 14,  4),
        RGB(28, 26, 12),
        RGB(28, 18,  8),
        RGB(31, 30, 22),
        RGB(22, 18,  8),
        RGB(12,  9,  3),
        RGB(28, 22,  4),
        RGB(16, 13,  5),
    },
};

u8 UiTheme_GetCount(void)
{
    return UI_THEME_COUNT;
}

u8 UiTheme_GetIndex(void)
{
    return sUiThemeIndex;
}

void UiTheme_SetIndex(u8 index)
{
    if (index >= UI_THEME_COUNT)
        index = 0;
    sUiThemeIndex = index;
}

const u16 *UiTheme_GetPalette(u8 index)
{
    if (index >= UI_THEME_COUNT)
        return NULL;
    return sUiThemes[index];
}

const u8 *UiTheme_GetName(u8 index)
{
    if (index >= UI_THEME_COUNT)
        return sNameUnknown;
    return sUiThemeNames[index];
}

void UiTheme_ApplyStdWindow(void)
{
    const u16 *pal = UiTheme_GetPalette(sUiThemeIndex);

    if (pal == NULL)
        return;

    LoadPalette(pal, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    LoadPalette(pal, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadPalette(pal, BG_PLTT_ID(0), PLTT_SIZEOF(1));
}

u8 UiTheme_Cycle(bool8 playSe)
{
    sUiThemeIndex++;
    if (sUiThemeIndex >= UI_THEME_COUNT)
        sUiThemeIndex = 0;
    UiTheme_ApplyStdWindow();
    if (playSe)
        PlaySE(SE_SELECT);
    return sUiThemeIndex;
}
