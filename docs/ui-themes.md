# UI themes (std-window palette contract)

Shared module: [`include/ui_theme.h`](../include/ui_theme.h) / [`src/ui_theme.c`](../src/ui_theme.c).

Eight themes (Porcelain, Carbon, Steel, Ink, Snow, Blush, Crimson, Honey) follow vanilla **stdpal_3**-style window graphics (`LoadStdWindowGfx` + framed panels / sliders / scrollbar).

**Consumers today:** MAKEOVER / Stat Editor and Debug Menu. Both call `UiTheme_ApplyStdWindow()` after loading std window gfx; **SELECT** runs `UiTheme_Cycle(TRUE)`. The active index lives in EWRAM for the session, so cycling in one screen carries to the other until reboot.

Editing the wrong slot breaks text, the std frame, or the slider thumb without an obvious compile error. Use this map.

## API

```c
#include "ui_theme.h"

LoadStdWindowGfx(WIN_LIST, 0x1C0, BG_PLTT_ID(14));
UiTheme_ApplyStdWindow();           // pals 14 + 15 + gap color on pal 0

// In input:
if (JOY_NEW(SELECT_BUTTON))
    UiTheme_Cycle(TRUE);

UiTheme_GetName(UiTheme_GetIndex()); // "Porcelain", …
```

Any new overlay that uses the same framed-panel / list / scrollbar index roles can opt in the same way. Screens that inject one-off colors into another bank (e.g. summary EV radar on pal 5) are **not** theme tables — use `LoadBgPalSlots` for those.

## Index roles (0–15)

| Idx | Role | Used by |
|-----|------|---------|
| 0 | Screen / gap BG | `FillBgTilemapBufferRect_Palette0`, clear |
| 1 | Panel / window fill | `FramedPanel` fill, list row clear, slider thumb face |
| 2 | Primary text | `textColors[1]` style printers |
| 3 | Shadow / recessed track | Slider track, scrollbar track, text shadow |
| 4–10 | Extra accents | Optional (red/gold/green/blue/rose) |
| 11 | Frame highlight | Std window frame (top/left bevel) |
| 12 | Frame mid | Std window frame |
| 13 | Frame dark / border | Frame + slider/scrollbar border |
| 14 | Accent | Slider highlight, scrollbar thumb, radar fill |
| 15 | Frame shadow | Std window frame (deep edge); list windows often use pal 15 |

`UiTheme_ApplyStdWindow` loads the 16 colors into **BG pal 14 and 15**, and index 0 onto **pal 0** for the gap.

Named constants: `UI_THEME_IDX_FILL`, `UI_THEME_IDX_TEXT`, `UI_THEME_IDX_ACCENT`, etc. in the header.

## Authoring workflow

1. Pick hex / RGB8 in a picker.
2. Convert: `python3 tools/gba_color.py '#RRGGBB'` → paste `RGB(...)` into `sUiThemes[]` in `ui_theme.c`.
3. Keep **contrast**: text (2) vs fill (1); accent (14) vs track (3); frame 11/12/13/15 must still read as a bezel.
4. Cycle with SELECT in Debug Menu or MAKEOVER (no rebuild needed to try an already-built theme).

```bash
python3 tools/gba_color.py --pal graphics/text_window/stdpal_0.pal
```

## Checklist when changing a theme

- [ ] Idx 1 still looks like a panel (not same as 0)
- [ ] Idx 2 readable on idx 1
- [ ] Idx 3 darker/lighter than 1 for tracks
- [ ] Idx 11–15 still form a coherent frame
- [ ] Idx 14 pops as accent for slider/scrollbar
- [ ] Dark themes: bump text (2) toward white; light themes: keep (2) near black

Related: [ui-components.md](ui-components.md), [custom-screen.md](custom-screen.md), [debug_menu.md](debug_menu.md).
