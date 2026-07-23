# UI components (romhack)

Shared window painters used by the debug menu, MAKEOVER / Stat Editor, and summary screen.

**Ownership rule (all of these):** the caller owns the `WindowTemplate`, BG/OBJ setup, and palette load. The component only paints (and sometimes maps/copies when the helper says so).

| Component | Header | Source |
|-----------|--------|--------|
| Framed panel | [`include/framed_panel.h`](../include/framed_panel.h) | [`src/framed_panel.c`](../src/framed_panel.c) |
| Scrollbar | [`include/scrollbar.h`](../include/scrollbar.h) | [`src/scrollbar.c`](../src/scrollbar.c) |
| Value slider | [`include/value_slider.h`](../include/value_slider.h) | [`src/value_slider.c`](../src/value_slider.c) |
| Stat radar | [`include/stat_radar.h`](../include/stat_radar.h) | [`src/stat_radar.c`](../src/stat_radar.c) |
| Mon portrait | [`include/mon_portrait.h`](../include/mon_portrait.h) | [`src/mon_portrait.c`](../src/mon_portrait.c) |
| Type icon | [`include/type_icon.h`](../include/type_icon.h) | [`src/type_icon.c`](../src/type_icon.c) |

Dev tools: [`tools/gba_color.py`](../tools/gba_color.py), [`tools/ui_layout.py`](../tools/ui_layout.py).

Also: [design-principles](design-principles/README.md) · [ui-themes.md](ui-themes.md) · [ui-common-problems.md](ui-common-problems.md) · [custom-screen.md](custom-screen.md) · [new-item.md](new-item.md) · [set_mon_nature.md](set_mon_nature.md)

| Helper | Header | Source |
|--------|--------|--------|
| BG pal slot inject | [`include/bg_pal_slots.h`](../include/bg_pal_slots.h) | [`src/bg_pal_slots.c`](../src/bg_pal_slots.c) |
| Footer strip layout | [`include/footer_strip.h`](../include/footer_strip.h) | [`src/footer_strip.c`](../src/footer_strip.c) |

---

## Color workflow

GBA BG/OBJ pals are **15-bit** (`RGB(r,g,b)` with channels 0–31). Prefer writing source as:

```c
#include "constants/rgb.h"

RGB8(0, 123, 197)           // 8-bit channels
RGB_HEX(0x007BC5)           // 0xRRGGBB
```

### Convert / match

```bash
# Hex or RGB8 → GBA + C snippets
python3 tools/gba_color.py '#007BC5'
python3 tools/gba_color.py 0 123 197
python3 tools/gba_color.py --from-gba 0x61E0

# Dump a palette (JASC .pal or binary .gbapal)
python3 tools/gba_color.py --pal graphics/text_window/stdpal_0.pal

# Find closest index for a target color
python3 tools/gba_color.py --pal graphics/text_window/stdpal_0.pal --match '#007BC5'

# LoadPalette snippet for injecting one slot
python3 tools/gba_color.py --c-snippet '#007BC5' --index 1 --bg-pal 5
```

`--match` also hints at likely-free slots (pure black / magenta). Always confirm the index is unused by text/icons on that screen.

### Inject into a live BG bank (C)

```c
#include "bg_pal_slots.h"

static const u8 sIdx[] = { 1, 4 };
static const u16 sColors[] = { RGB8(0, 123, 197), RGB8(0, 74, 148) };
LoadBgPalSlots(5, sIdx, sColors, 2);          // sparse indices
// LoadBgPalSlotRange(5, 1, sColors, 2);      // contiguous from startIndex
```

Then paint with those **raw** indices (see 4bpp gotchas).

### Layout / size before rebuild

```bash
python3 tools/ui_layout.py radar --preset medium
python3 tools/ui_layout.py radar --preset preview
python3 tools/ui_layout.py radar --tiles 15 13 --pad 10 --labels --scale 64
python3 tools/ui_layout.py window --tiles 26 3
```

Presets mirror `STAT_RADAR_SIZE_*` (preview / small / medium / large).

---

## 4bpp painting gotchas

These burned real debug time; treat as checklist when adding a new painter. Deeper write-ups (especially **mixed-palette type badges**): [ui-common-problems.md](ui-common-problems.md).

1. **Radar / multi-color plots: pass raw indices 0–15, never `PIXEL_FILL(n)`.**  
   `FillBitmapRect4Bit` ORs a full byte into one nibble. `PIXEL_FILL(1)` next to color `4` becomes `1|4 = 5` (e.g. type-palette green fringe).  
   Rect fills that only touch one color (slider track, framed fill) still commonly use `PIXEL_FILL` — that’s fine when adjacent pixels share the same prepared byte pattern, but mixed plotters must use raw indices.

2. **Cage vs fill tip:** when cage + fill share a vertex at max value, inset fill radius by 1 so the outline doesn’t overwrite the cage tip (SPE overshoot). Stat radar already does this.

3. **Window palette bank ≠ color meaning:** index `1` on pal 5 is not the same RGB as index `1` on pal 14. Load blues into the bank your `WindowTemplate.paletteNum` uses.

4. **OBJ portraits:** caller must keep DISPCNT OBJ on and pick `oamPriority` / `subpriority` that sit above the BG the window uses. Prefer `MonPortrait_ShowPartySlotInWindow` for footers.

5. **Std frame tiles** share the framed-panel palette; don’t stomp indices the frame / text need unless you own that bank entirely.

6. **BG palette index 0 is transparent.** On a **same-pal type window**, clear to 0 — punch-through is the pure host. On **mixed-palette** (legacy) hosts, never “clear to 0 + patch color 0” for invisible surround — use a non-zero free index patched to the host fill (TypeIcon uses index 10).

7. **Tilemap palette override is per 8×8 tile.** Prefer a dedicated same-pal window so you never override. If stuck on a shared canvas: tile-align the blit, fill the whole override rect, color-key stock art, then override **after** `PutWindowTilemap`.

8. **Only BG0–BG3 exist** (hardware). Overlapping std-framed windows on one BG stomp bezels; split BGs or gap them. Don’t use an opaque matte rect if behind UI should show through frame corners. Full write-up: [ui-common-problems.md](ui-common-problems.md) § BG layers.

---

## FramedPanel

```c
FramedPanel_ShowText(WIN_FOOTER, str, textColors, NULL); // reset + print + flush
FramedPanel_ShowEmpty(WIN_LIST, NULL);
FramedPanel_Reset(WIN_LIST, NULL);  // paint into window, then …
FramedPanel_Flush(WIN_LIST);
```

`NULL` config → fill 1, tile `0x1C0`, palette 14, text at (4,4).

### Paint refresh (avoid frame flash)

`FramedPanel_Reset` rebuilds the **std frame** every call. Fine when opening a page; bad on every slider tick / footer EV update — the bezel visibly flashes.

**While adjusting live content** (MAKEOVER pattern):

1. `FillWindowPixelBuffer` / `FillWindowPixelRect` the interior only (usually `PIXEL_FILL(1)`).
2. Reprint text / redraw slider / radar.
3. `CopyWindowToVram(…, COPYWIN_GFX)` (or `PutWindowTilemap` + copy if needed).

Use `FramedPanel_Reset` + `Flush` when the panel is first shown or after a full page change (nature picker ↔ main list).

---

## Scrollbar

```c
taskId = Scrollbar_Create(&config, &scrollOffset, totalItems, visibleItems);
Scrollbar_SyncFromListMenu(taskId, listTaskId);
Scrollbar_Destroy(taskId); // does not unmap/clear
```

Caller maps/clears the scroll strip window.

---

## ValueSlider

```c
struct ValueSliderConfig cfg;
ValueSlider_SetDefaults(&cfg);
cfg.windowId = WIN_LIST;
cfg.label = sLabel;
cfg.value = &value;
cfg.displayMax = 252;
cfg.getMax = MyGetMax; // optional dynamic cap
ValueSlider_DrawInline(&cfg, y, active, textColors);
changed = ValueSlider_ProcessInput(&cfg, &heldFrames, &lastDir);
```

Colors are palette **indices**; load them on the list window’s bank. Defaults match MAKEOVER (track 3 / thumb 1 / border 13 / highlight 14).

**Call site:** [`src/rh_stat_editor.c`](../src/rh_stat_editor.c) `StatEditor_BuildSliderConfigForRow`.

---

## StatRadar

Happy path:

```c
struct StatRadarConfig cfg;
StatRadar_SetDefaults(&cfg);
StatRadar_ApplySizePreset(&cfg, STAT_RADAR_SIZE_MEDIUM);
cfg.values = evs;
cfg.labels = labels;           // medium/large
cfg.textColors = textColors;
cfg.gridColor = 14;
cfg.fillColor = 1;             // raw indices — see gotchas
cfg.outlineColor = 4;
StatRadar_PlaceInWindow(&cfg, windowId, StatRadar_PadForSize(STAT_RADAR_SIZE_MEDIUM));
StatRadar_NudgeTipUpToMidline(&cfg, 1); // medium summary bias
// cfg.drawDebugGuides = TRUE; // tip/center crosses while tuning
StatRadar_Draw(&cfg);
```

### Size presets

| Preset | Meaning | Typical use |
|--------|---------|-------------|
| `STAT_RADAR_SIZE_PREVIEW` | Fixed r=11, unlabeled | MAKEOVER footer |
| `STAT_RADAR_SIZE_SMALL` | Auto @ 80%, unlabeled | Compact strip |
| `STAT_RADAR_SIZE_MEDIUM` | Auto @ 64%, labels+values, nudgeX=-2 | Skills detail EV/IV (override scale/`labelRadius` as needed) |
| `STAT_RADAR_SIZE_LARGE` | Auto @ 100%, labels+values | Full pane chart |

`PadForSize`: preview/small → 2, medium → 10, large → 8.

`labelRadius`: `0` (default) = labels anchor on the cage tips. Set explicitly to keep text on a larger ring while `scalePercent` shrinks only `radius` (Skills detail EV/IV: cage @ 90%, `labelRadius` = 19).

`scalePercent` in `PlaceInRect` / `PlaceInWindow` applies to **`radius` only** — it does not scale `labelRadius`.

For footers that share space with text/portrait, prefer `FooterStrip_Split3` + `StatRadar_PlaceInRect` on the mid region (MAKEOVER) instead of full-window place + magic `nudgeX`.

### Call sites

- Dual MEDIUM + `labelRadius`: [`src/pokemon_summary_screen.c`](../src/pokemon_summary_screen.c) `PokeSum_DrawSkillsDetailRadar` (Skills detail canvas)
- Preview: [`src/rh_stat_editor.c`](../src/rh_stat_editor.c) `StatEditor_DrawEvRadar` (FooterStrip mid)

### Debug guides

Set `drawDebugGuides = TRUE` (optional `debugColor`, else `gridColor`) to paint center + tip crosses. Use while tuning nudge/scale; leave off for ship builds.

---

## FooterStrip

Split one window into **left / mid / right** pixel regions (window-local). Typical MAKEOVER footer: text | EV radar | party icon.

```c
struct FooterStripLayout strip;
FooterStrip_Split3(WIN_FOOTER, pad, leftW, rightW, &strip);

// Text: print inside strip.left (e.g. x = strip.left.x + 2)
StatRadar_PlaceInRect(&radar, WIN_FOOTER,
                      strip.mid.x, strip.mid.y, strip.mid.w, strip.mid.h, pad);
FooterStrip_RegionScreenXY(WIN_FOOTER, &strip.right, &sx, &sy); // OBJ / portrait
```

If `leftW + rightW` exceeds the padded inner width, mid collapses and left/right are clamped (right preferred).

**Call site:** [`src/rh_stat_editor.c`](../src/rh_stat_editor.c) footer radar + portrait.

---

## MonPortrait

```c
MonPortrait_ShowPartySlotInWindow(partySlot, WIN_FOOTER, nudgeX, nudgeY);
// or full config:
MonPortrait_SetDefaults(&cfg);
cfg.usePartySlot = TRUE;
cfg.partySlot = slot;
cfg.windowId = WIN_FOOTER;
cfg.align = MON_PORTRAIT_ALIGN_RIGHT_MIDDLE;
MonPortrait_Show(&cfg);
MonPortrait_Destroy();
```

Icon mode is 32×32 OBJ; front-pic mode is 64×64. Align helpers place from the window; nudge is pixel fine-tune.

---

## UiChip

Generic small chrome painter ([`include/ui_chip.h`](../include/ui_chip.h)): mid-split fill + 1px corners, optional label (white-ink leftover-pad center), FIXED/FIT width. **Host purity matters more than silhouette math:** same-pal hosts paint the pill only (`cornerIdx` 0); mixed-palette surround/`Commit` is a legacy escape hatch for rare shared canvases.

Type pills are a **preset**: existing `TypeIcon_*` blank/BlitBlank/Commit APIs wrap UiChip; stock bake and HP helpers stay in TypeIcon. New badges should call `UiChip_*` directly (custom fills/label style/height) instead of copying TypeIcon.

```c
// Pure host (preferred): dedicated window on the type bank, cleared to 0
UiChip_Draw(win, fillTop, fillBottom, x, y, w, h, 0);
UiChip_PrintLabel(win, str, x, y, w, h, &style);
PutWindowTilemap(win);
CopyWindowToVram(win, COPYWIN_FULL);

// Legacy mixed-pal (shared foreign canvas only):
UiChip_BlitLabeled(win, typePal, fillTop, fillBottom, x, y, w, h, str, &style, surround);
PutWindowTilemap(win);
UiChip_Commit(win, typePal, x, y, w, h, COPYWIN_FULL);
```

---

## TypeIcon

Stock `menu_info` type badges **and** procedural blank pills (UiChip silhouette + `gTypeNames` via `FONT_SMALL`). Hidden Power IV helpers live beside the painters.

**Host rule:** same-pal dedicated window = **pure pill** (asset pixels only; idx 0 punches through). Mixed-palette surround pad = **legacy** — keep the APIs for odd shared canvases; do **not** use for new chips when a small type window fits.

| Painter | Art | When |
|---------|-----|------|
| `TypeIcon_Draw` / `BlitMenuInfoIcon` | Baked `menu_info` (letters included) | Same-pal hosts; pixel-identical to vanilla |
| `TypeIcon_DrawBlank*` / `BlitBlank*` | Procedural fill + label | Custom width, live type, or matching stock look without stretching bake |
| `DrawHiddenPowerBlock*` / `DrawHiddenPowerBlankBlock*` | Stock or blank + tile align | Legacy shared-canvas helpers |

**Procedural label centering:** measure **white** ink only (fg idx 15; SE shadow hangs outside), then center with leftover pads — `pill + (size - ink) / 2 - min`. Do **not** use `(pillCX - inkCX) / 2` on agbcc: negative half-deltas truncate toward zero and sit labels 1px low vs stock. Nudges: `TYPE_ICON_LABEL_NUDGE_X/Y` (default 0). Offline check: [`tools/preview_type_pills.py`](../tools/preview_type_pills.py) → `build/type_pills/` (stock \| procedural + `menu_info` map sheets).

**Width (CSS-like):** `struct TypeIconWidth` + `TypeIcon_ResolveWidth` — `FIXED` (default 32) or `FIT` (`GetStringWidth(label) + 2*padX` at draw time; use `padX=0` for true fit-content), then optional `min` / `max` (`0` = none) / `tileAlign`. Helpers: `WidthSetFixed` / `WidthSetFit`. Placement: `CenterXInSpan(left, right, w)` (+ `AlignXNearest` only on mixed-pal). Labels: `FormatTypeLabel` + optional prefix (`HP `). MAKEOVER / Skills use FIT on same-pal type windows.

### Same-pal window (preferred — Moves / Skills / MAKEOVER)

Whole window `paletteNum` = type bank. Clear to 0, blit, map — no surround, no `Commit`. Index 0 punches through to the BG behind the window (place the pill window on a higher-priority BG than the cream/chrome so punch-through hits the right layer).

```c
TypeIcon_LoadPalette(TYPE_PAL); // once if this bank isn’t already loaded for the screen
FillWindowPixelBuffer(typeWin, 0);
TypeIcon_Draw(typeWin, type, 0, 0); // stock bake — or TypeIcon_DrawBlank / DrawBlankEx
PutWindowTilemap(typeWin);
CopyWindowToVram(typeWin, COPYWIN_FULL);
```

**Call sites (same-pal only):** summary Moves type column (`PokeSum_DrawMoveTypeIcons`, stock bake); Skills detail HP pill (`PokeSum_DrawSkillsDetailHpTypeIcon`, blank + `HP ` + FIT); MAKEOVER footer `WIN_HP_PILL` on BG1 (`StatEditor_DrawFooterHpTypeIcon`, blank + `HP ` + FIT; footer theme window owns text/radar only).

### Mixed-palette (legacy — shared foreign canvas)

Only when a dedicated type window is impossible. Shared window on another bank needs tile-align + surround idx 10 + override after tilemap. The pad is **not** part of the pill asset — it exists so leftover tile pixels mean something under the type bank.

**Surround color is caller-owned outside UiTheme std-windows.** `TypeIcon_Blit` / `BlitBlank` / `DrawHiddenPower*Block` patch type-pal index 10 from the host window’s `UI_THEME_IDX_FILL`. On non-theme hosts, pass explicit RGB15 via `*WithSurround` — prefer `TypeIcon_SurroundFromBgPal(bgPal)` (skips idx 0 / `RGB_MAGENTA`). Blit sanitizes unsafe colors. Call `TypeIcon_LoadPalette` **once** at screen init (blit only patches surround). Blank path paints corners with surround idx (same as stock color-key leaving surround), not transparent 0.

```c
TypeIcon_LoadPalette(TYPE_PAL); // once at screen init; free BG bank

TypeIcon_BlitBlankEx(windowId, TYPE_PAL, type, x, y, &w, _("HP "), themeFill);
PutWindowTilemap(windowId);
TypeIcon_CommitSized(windowId, TYPE_PAL, x, y, pillW, COPYWIN_FULL);

// Explicit surround on a non-theme shared canvas:
surround = TypeIcon_SurroundFromBgPal(0);
x = TypeIcon_DrawHiddenPowerBlockWithSurround(windowId, TYPE_PAL, type, startX, y, surround);
PutWindowTilemap(windowId);
TypeIcon_Commit(windowId, TYPE_PAL, x, y, COPYWIN_MAP);
```

**Note:** Override rect is always ≥ `ceil(art/8)` tiles (32×12 art → 32×16 tiles). Opaque surround cannot match striped chrome; use same-pal + idx 0 instead. Silent Y snap: blit aligns `y` down to a multiple of 8. Default blank width matches stock (`TYPE_ICON_BLANK_WIDTH` 32).

Pitfalls (fringe, gray pad, magenta pad, stripes vs solid surround, stretch, label centering): [ui-common-problems.md](ui-common-problems.md) §§3–4b.  
Design rules + “rebuild assets as painters”: [design-principles/reusable-ui-components.md](design-principles/reusable-ui-components.md).

---

## Adding a new screen (checklist)

1. Define `WindowTemplate` (tiles → px = ×8). Run `ui_layout.py` if placing a radar.
2. Load palette bank for that window; use `gba_color.py --match` / `--c-snippet` for new colors.
3. `SetDefaults` → (radar: `ApplySizePreset`) → set data/colors → `Place*` → `Draw`/`Show`.
4. If colors look wrong, green fringes, or gray pads under badges, re-read **4bpp painting gotchas** and [ui-common-problems.md](ui-common-problems.md).
5. Type pills: `python tools/preview_type_pills.py` before the emulator loop when changing blank-pill centering/fills.
6. Optional: `drawDebugGuides` or `ui_layout.py` before the next rebuild.
