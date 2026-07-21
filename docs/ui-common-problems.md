# UI common problems

Central pitfalls for romhack UI: **shared window painters** (MAKEOVER, debug menu, summary) and **mid-battle overlays** (catch trainer Pokémon). Pair with [ui-components.md](ui-components.md), [ui-themes.md](ui-themes.md), and [debug_menu.md](debug_menu.md).

| Section | When to read |
|---------|----------------|
| [Window painters / TypeIcon](#window-painters--typeicon) | Fringe, wrong colors, gray/magenta pads, stripes vs solid surround, stretch, label centering, draw order |
| [Mid-battle overlays](#mid-battle-overlays) | Dex / nickname mid-fight, reshow, sprites, OT give |

---

# Window painters / TypeIcon

Lessons from shared window painters. When a new painter looks “almost right,” walk this list before inventing a second draw path. For how the next component should be shaped (ownership, happy path, draw protocol): [design-principles/reusable-ui-components.md](design-principles/reusable-ui-components.md).

## Mental model (windows)

| Layer | What it is | Gotcha |
|-------|------------|--------|
| Window pixel buffer | 4bpp indices 0–15 in RAM | Indices are **not** RGB; meaning depends on the tile’s palette bank |
| WindowTemplate.`paletteNum` | Default bank for that window’s tiles | Theme text/fill live here (MAKEOVER: 14/15) |
| Tilemap palette override | Per **8×8 tile**, can point a rect at another BG bank | Cannot override half a tile; leftover pixels in the rect use the override bank |
| BG palette index **0** | Hardware **transparent** on BG layers | Shows whatever is behind the BG — not “color 0 of this pal” |

Summary-screen type pills dodge mixed-palette pain when the **whole window** already uses `pokemon_types` (`paletteNum` 6) — Moves type column and Skills detail HP pill. MAKEOVER footer does **not** (shared themed window → mixed-palette path).

## Checklist (mixed-palette gfx)

1. Tile-align X/Y to 8px before any `PutWindowRectTilemapOverridePalette` (blit also snaps Y down silently).
2. Fill the **full** override rect (W×H in tile multiples) with a **non-zero** index you control — **or** prefer a same-pal type window so idx 0 can punch through to stripes.
3. Color-key blit stock art (skip 0) so rounded corners keep that surround index (opaque path).
4. `PutWindowTilemap` for the window **before** palette override (override last).
5. `CopyWindowToVram(…, COPYWIN_FULL)` when both gfx and map changed.
6. Never stretch baked letter art; never assume index 0 is a visible “match bg” color on mixed-palette BGs.

## Problems we hit (type badge / Hidden Power)

### 1. Badge uses theme colors (wrong pink / muddy text)

**Symptom:** Pill visible but colors match the UI theme, not summary type colors.

**Why:** `PutWindowTilemap` rewrote the badge tiles back to the window’s `paletteNum`. Palette override ran too early and was wiped.

**Fix:** Draw order must be:

```text
Fill / blit badge pixels
→ other painters (radar, text)
→ PutWindowTilemap(window)
→ PutWindowRectTilemapOverridePalette(badge tiles, typePal)
→ CopyWindowToVram(COPYWIN_FULL)
```

Reference: `TypeIcon_Blit` then caller `TypeIcon_ApplyPaletteOverride` after tilemap (`src/type_icon.c`, MAKEOVER footer).

### 2. Gray / orange fringe at rounded corners

**Symptom:** Ear-like or corner blocks; EV text next to the pill looks corrupted.

**Why:** Override is **per 8×8 tile**. If the badge isn’t tile-aligned, theme fill/text shares those tiles and is remapped through `pokemon_types` (e.g. fill `1` → fighting red, shadow `3` → muddy gray).

**Fix:** `TypeIcon_AlignX` / align Y down to a tile edge. Clear the entire override rect before blitting so no theme pixels remain in those tiles.

### 3. Gray strip (or “ears”) under a correct pink pill

**Symptom:** After corner fixes, a gray bar the width of the pill sits under it (extra height). Earlier: same area was solid pink.

**Why (two layers):**

1. Badge art is **12px** tall; override must cover **16px** (2 tile rows). The bottom 4px must be *some* index under the type bank.
2. Filling that pad with **index 0** (or leaving color-key holes as 0) does **not** show a patched `type-pal[0]`. On GBA BGs, **index 0 is transparent** → you see chrome behind the window (gray). Filling with the **body pink** instead makes a tall pink rectangle.

**Fix:** Use an index **unused by badge art** (type badges never use **10**). Patch `type-pal[10]` to the host window **fill** color (`UI_THEME_IDX_FILL` on UiTheme windows, or an explicit surround RGB via `*WithSurround`). Fill the override rect with `10`, then color-key blit the stock badge **or** paint blank-pill corners with surround idx 10.

Do **not** “fix” this by painting body color into the pad — that brings back the rectangle.

### 3b. Magenta / hot-pink pad on a non-theme window (same geometry as #3)

**Symptom:** MAKEOVER footer looks fine, but another screen (e.g. summary Moves proof) shows a **solid magenta / darker-pink** rectangle under the badge — thin vertical edges + a thick bottom bar. No scanlines in the pad (opaque, not “see-through”).

**Why:** Surround RGB was sampled from **BG pal index 0**. On summary chrome (`bg.gbapal`) and many tilesets, idx 0 is `RGB_MAGENTA` (chroma key / transparent intent), not the visible panel lavender. Patching type-pal[10] with that key paints an opaque magenta pad. Host pal index 1 is also unsafe to assume (summary memo pal 5 uses it for radar blue).

**Fix:**

- Prefer `TypeIcon_SurroundFromBgPal(bgPal)` (skips idx 0, prefers panel slots like idx 3).
- Or pass a known panel RGB15; never `gPlttBufferUnfaded[BG_PLTT_ID(n)]` alone.
- `TypeIcon_BlitWithSurround` calls `TypeIcon_SanitizeSurroundColor` so a magenta sample falls back to a lavender default — defense in depth, not a substitute for picking the right surround.

**Rebuild note:** If the pad “won’t die,” confirm `type_icon.o` / the ROM are newer than the source. A stale `.gba` keeps the old surround forever.

### 3c. Solid surround looks wrong on striped chrome (Skills detail)

**Symptom:** Pill colors are correct, but a flat lavender/gray rectangle under/around the badge fights the striped summary panel — “half the time” it mismatches a stripe.

**Why:** Opaque surround is **one RGB**. Striped page chrome is **multiple colors**. A solid pad can never match both. Leaving pad as idx 0 on a **mixed-palette override** still remaps through the type bank / transparency rules and often shows the wrong layer (classic gray bar, §3).

**Fix (preferred):** Don’t put the badge on the foreign canvas. Give it a **dedicated type-palette window** (Moves pattern): `paletteNum` = type bank, `FillWindowPixelBuffer(0)`, `TypeIcon_Draw` / `BlitMenuInfoIcon`, no surround/`Commit`. Empty pixels punch through to the real stripes.

**When mixed-palette is unavoidable** (MAKEOVER flat footer): keep opaque UiTheme fill surround — flat hosts don’t need stripe punch-through.

Reference: Skills detail `PokeSum_DrawSkillsDetailHpTypeIcon`; Moves `PokeSum_DrawMoveTypeIcons`.

### 4. Stretching / ad-hoc rebuilding the pill (ghost text, wrong fonts)

**Symptom:** Smeared letters above `PSYCHC`; custom “HP PSYCHIC” text wrong color; solid rectangle instead of rounded pill.

**Why:**

- Horizontally stretching `menu_info` badges **resamples baked type names** → garbage mid-pill.
- Drawing a solid fill + `AddTextPrinter` is a **second entity**: text colors are indices into whichever bank the tiles use; easy to pick theme indices while tiles are on the type bank (or the reverse).
- Stock badges already encode condensed names (`PSYCHC`, `ELECTR`, …). That is data, not corruption.

**Fix:** Use `TypeIcon_Draw` / `Blit` for the bake, or the deliberate procedural path (`DrawBlank*` / `BlitBlank*`) — silhouette + `gTypeNames` + white-ink center. Don’t stretch lettered tiles or invent a one-off printer on the type bank.

### 4b. Procedural label 1px low in-game (preview looked fine)

**Symptom:** MAKEOVER blank pill matches [`tools/preview_type_pills.py`](../tools/preview_type_pills.py) horizontally but sits **1px too low** vs stock / the PNG. Hard reload didn’t help.

**Why:** Centering used `(pillCY - inkCY) / 2`. Glyph cells are taller than the 12px pill, so the half-delta is **negative**. agbcc truncates toward zero (`-3/2 → -1`); Python `//` floors (`→ -2`). Preview matched stock; ROM didn’t.

**Fix:** Center with leftover pads only (non-negative divides):  
`origin = pill + (size - inkSize) / 2 - inkMin`, measuring **white** fg (idx 15) only so the SE shadow can hang. Keep preview on the same formula. See `TypeIcon_GetLabelOrigin`.

### 5. “HP” prefix fights the EV line / badge

**Symptom:** `EV 510/HP`, overlapping theme “HP” on the pill, or no visible “HP” change after rebuild.

**Why:** Layout treated “HP ” as theme text beside a fixed 32px badge without a reserved, non-overlapping slot; later attempts drew on top of baked letters. Footer left strip is only ~132px and already holds name + EV + radar.

**Fix for checkpoint:** Show the type pill alone (right-aligned in the left strip). Custom “HP {type}” belongs in a future TypeIcon config, not a one-off footer printer.
### 6. Radar overwrites the badge

**Symptom:** Pill clipped or missing after EV adjust.

**Why:** Same window; draw order. Radar after badge stomps pixels (and can leave theme indices in override tiles).

**Fix:** Draw badge **after** radar (MAKEOVER), or give the badge an exclusive rect the radar never touches.

### 7. Live refresh flashes the std frame

**Symptom:** Footer bezel blinks every IV/EV tick.

**Why:** `FramedPanel_Reset` redraws the frame every call.

**Fix:** Content-only refresh: `FillWindowPixelBuffer` interior → reprint → `COPYWIN_GFX` / `FULL`. Reserve `FramedPanel_Reset` for open / page change. (Already in [ui-components.md](ui-components.md) FramedPanel section.)

### 8. 4bpp `PIXEL_FILL` fringe on multi-color plots

**Symptom:** Green edge pixels on radar / plots.

**Why:** `FillBitmapRect4Bit` ORs a full `PIXEL_FILL` byte into one nibble; mixed neighbors become wrong indices.

**Fix:** Pass **raw** indices 0–15 into multi-color painters. See gotcha #1 in [ui-components.md](ui-components.md).

## RhLog / `_("…")` in call args

**Symptom:** `syntax error before '{'` compiling `RhLogf(_("…"), …)`.

**Why:** Game strings need `static const u8 sFmt[] = _("…");` so the string preprocessor emits a proper array. Inline `_("…")` as a call argument is not the same.

**Fix:** Static format strings (see debug menu / `rh_log` call sites).

## When adding TypeIcon (or any type badge)

| Do | Don’t |
|----|--------|
| Prefer a **same-pal type window** on striped / foreign chrome | Force mixed-palette + solid surround onto stripes |
| Load `pokemon_types` into a free BG bank (mixed path) | Assume window `paletteNum` is already the type pal when it isn’t |
| Tile-align; fill override rect with surround idx 10 (mixed) | Leave theme pixels in the override rect |
| Color-key stock badge **or** blank corners with surround idx | Opaque-blit index-0 corners expecting “invisible” on mixed BGs |
| Override palette **after** `PutWindowTilemap`; re-Commit after remount | Override then `PutWindowTilemap` |
| Pass `*WithSurround` / `SurroundFromBgPal` outside UiTheme | Blind `TypeIcon_Blit` when host idx 1 isn’t fill |
| Procedural: white-ink AABB + leftover-pad origin; check with `preview_type_pills.py` | Stretch / OCR badge art; center with signed `(cx-cy)/2` on agbcc; sample surround from BG **idx 0** |
| Rebuild after surround/palette/centering tweaks | Assume mGBA auto-picked up a stale `.gba` |

Call-site recipe: [ui-components.md](ui-components.md) TypeIcon section.

---

# Mid-battle overlays

Lessons from wiring **Catch Trainer Pokémon** (dex page + nickname mid-fight, then return to a living battle). Vanilla FireRed almost never does this: wild catch tears the battle down and goes to the overworld. Trainer steals must **rebuild** the battle instead.

Primary code: [`src/battle_script_commands.c`](../src/battle_script_commands.c) (`displaydexinfo`, `trygivecaughtmonnick`, `givecaughtmon`, `reshowbattlescreen`), [`src/reshow_battle_screen.c`](../src/reshow_battle_screen.c), [`data/battle_scripts_2.s`](../data/battle_scripts_2.s), [`src/trainer_pokemon_sprites.c`](../src/trainer_pokemon_sprites.c).

## Mental model (battle callbacks)

| Piece | Role |
|-------|------|
| `BattleMainCB1` | Every frame: `gBattleMainFunc()` + **battler controllers** |
| `BattleMainCB2` | Every frame: sprites, tasks, text, fades |
| Controllers | Often wait on sprite callbacks (`CompleteOnBattlerSprite*`) |
| `ReshowBattleScreenAfterMenu` | Bag-style full rebuild of battle gfx (use this to return from menus) |

If you change CB2 (dex, naming, party, bag) but leave CB1 running, controllers keep ticking against whatever sprite state you left behind.

## Do not `ResetSpriteData()` while controllers are live

`ResetSpriteData()` sets every sprite callback to `SpriteCallbackDummy`.

Any controller still in `CompleteOnBattlerSprite*` then thinks the anim finished and may `FreeTrainerFrontPic` / `DestroySprite` with **stale IDs** → hard crash.

That can happen on nickname **No** as well as **Yes** — you never need the naming keyboard.

**Safer after dex / before catch summary:**

- Hide in-use sprites (`invisible = TRUE`)
- Wipe VRAM
- Restore `VBlankCB_Battle`
- Free OBJ tiles / pals only when you are about to `CreateMonPic` (not in the same breath as “dex just closed” if you can avoid it)

**If you must `ResetSpriteData` (e.g. naming screen does):** freeze battle logic for that whole screen:

```c
savedCb1 = gMain.callback1;
SetMainCallback1(NULL);
DoNamingScreen(..., returnCbThatRestoresCb1ThenReshow);
```

## `FreeAndDestroyMonPicSprite` and inactive pic slots

`sSpritePics[]` empty slots are **zeroed**: `spriteId = 0`, `paletteTag = 0`.

`TAG_NONE` is `0xFFFF`, so `paletteTag == 0` is **not** “no palette”.

This is unsafe:

```c
// BAD — FreeAndDestroy(0) matches the first zeroed slot and may
// FreeSpritePaletteByTag() on garbage → crash right after closing the dex
for (i = 0; i < MAX_SPRITES; i++)
    FreeAndDestroyMonPicSprite(i);
```

Use `FreeAllPicSprites()` (only `.active` entries) or destroy a known sprite id. `FreeAndDestroyPicSpriteInternal` must require `.active` when matching.

Also: `ResetSpriteData()` does **not** free `sSpritePics` Alloc buffers (~8KB per pic). Always `FreeAndDestroyMonPicSprite` (or `FreeAllPicSprites`) before a large `Alloc` (naming screen is ~7.5KB).

## Dex mid-battle is already violent

`DexScreen_LoadResources()` does `ResetSpriteData()`, `ResetTasks()`, its own BGs/windows/VBlank.

Vanilla gets away with it because catch ends the battle. On a trainer steal:

- Battle tasks are gone (battle logic is mostly CB1, so it may still limp along)
- Dex often leaves mon-pic slots allocated
- Closing the dex (`DoClosePokedex`) frees windows/tilemaps but does not fully restore battle gfx

After dex, prefer a **vanilla-like** `displaydexinfo` case 2 (VRAM wipe + battle VBlank), then reclaim pics/tiles when creating the catch-summary mon pic. Full battle rebuild belongs in `reshowbattlescreen` / bag reshow.

## Naming screen enter / exit

**Enter (trainer steal):**

1. Destroy catch mon pic (free `sSpritePics` heap)
2. `SetVBlankCallback(NULL)` / clear HBlank / `ScanlineEffect_Clear()`
3. `FreeAllWindowBuffers()`
4. Freeze CB1
5. `DoNamingScreen(..., CB2_ReturnFromStolenMonNaming)`

**Exit:** restore CB1, then `ReshowBattleScreenAfterMenu()` — **not** bare `BattleMainCB2`. Catch UI is not a real battle screen; returning straight to CB2 with torn-down gfx crashes.

Wild catch can keep `BattleMainCB2` as the return callback because the fight is over.

If `Alloc` for naming fails, `DoNamingScreen` immediately `SetMainCallback2(returnCallback)` — that return path must still be safe (reshow + restore CB1).

## Give mon vs reshow order (HP)

Steal prep keeps **catch HP on the enemy party slot** while `gBattleMons[].hp == 0` (for exp / faint flags).

`ReshowBattleScreenAfterMenu` decides “create real mon sprite vs invisible placeholder” from **party HP**.

| Order | Result |
|-------|--------|
| Reshow **before** `givecaughtmon` | Party HP still > 0 → reloads the stolen mon as if it were still out |
| `givecaughtmon` **then** reshow | Party HP set to 0 → placeholder; send-out can proceed |

`givecaughtmon` must zero the field slot’s HP before any bag-style reshow.

## Healthboxes and battler sprites after reshow

- Do **not** destroy/recreate healthboxes in a post-reshow cleanup “just to be safe” — that stacks duplicate boxes.
- After reshow, only hide the fainted side’s healthbox/shadow and replace the battler sprite with an invisible placeholder if needed (`CleanupFaintedOpponentAfterStealReshow`).
- After dex `ResetSpriteData` (if you ever use it), `gHealthboxSpriteIds[]` are stale — guard with `inUse` before `SetHealthboxSpriteInvisible`.
- Opponent send-out: destroy any leftover placeholder in `StartSendOutAnim` before creating the new mon sprite.

## Shared `gBattleCommunication` fields

```c
#define MULTIUSE_STATE   0
#define CURSOR_POSITION  1
#define TASK_ID          1  // same byte as cursor!
```

Do not stash a long-lived value in `TASK_ID` across the nickname Yes/No UI. Catch-summary mon pic id lives in `gBattleStruct->field_78` (`0xFF` = none). `CreateMonPic` returns `u16` — clamp before storing in a `u8`.

## OT / give helpers

`GiveMonToPlayer()` rewrites OT id and can turn stolen `OT_ID_RANDOM_NO_SHINY` mons into **Bad Eggs**. Trainer steals use `GiveMonToPlayerPreserveOT()` after stamping the opponent’s **display** name (rival via `GetExpandedPlaceholder(PLACEHOLDER_ID_RIVAL)`, not the data-table `"TERRY"`).

## Quick checklist (new mid-battle menu)

1. Will CB1 / controllers still run? If you `ResetSpriteData`, freeze CB1.
2. Are you freeing mon pics by id `0..63`? Use active-only free.
3. Does return need `ReshowBattleScreenAfterMenu` instead of `BattleMainCB2`?
4. If the fight continues: give/faint party HP **before** reshow.
5. Avoid recreate-all-healthboxes; hide + placeholder only.
6. Don’t overload `gBattleCommunication[TASK_ID]` across Yes/No UI.

## Mid-battle related

- Cheat behavior summary: [debug_menu.md](debug_menu.md) (Catch Trainer Pokémon)
- Steal script: `BattleScript_BallThrowSteal` in [`data/battle_scripts_2.s`](../data/battle_scripts_2.s)
