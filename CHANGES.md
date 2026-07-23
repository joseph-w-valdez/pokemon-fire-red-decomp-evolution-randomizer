# Changes

Living overview of what this romhack does relative to pret FireRed / LeafGreen.  
For versioned release notes, see [CHANGELOG.md](CHANGELOG.md).

**Build:** `make` → `pokemon-random-evolution-v{version}.gba`  
**Version:** set `HACK_VERSION` in [`config.mk`](config.mk).

**Feature toggles & tunables:** [`include/config.h`](include/config.h)  
- `RH_*` / `OW_FOLLOWERS_ENABLED` — on/off gates  
- `RH_WALK_PX_PER_FRAME`, `RH_BIKE_PX_PER_FRAME`, text delays, battle speed multipliers, `RH_MAKEOVER_PRICE`, etc. — tweak values in one place  

## Implemented

### Battles & Pokémon
- Random evolution on level-up (battle / Rare Candy): transforms into a random different species, then offers one random move from the new species’ learnset
- Trade evolution random overwrite is gated behind `RH_RANDOM_EVOLUTION` (vanilla trade target kept when off)
- Only **one** random evolution queued per party mon per battle (even if they level multiple times)
- Nuzlocke mode toggle at the start of a new game (Oak yes/no)

### Movement & field
- Walk speed = vanilla run; run is much faster (~4×); running allowed almost everywhere
- Start with Running Shoes
- Bike usable indoors and much faster (~8×); surfing matches bike speed
- Safari Zone uses normal wild encounters (no Safari ball mini-game restrictions as in vanilla Safari)

### Items & HMs
- TMs are reusable
- HMs are forgettable
- Field moves available as key items (e.g. WAYMO and related HM key items)
- HM08 Dive obtainable (Celadon Hotel)
- HM field-move “show mon” anim skipped when `RH_HM_KEY_ITEMS` (Cut / Flash / Strength / Rock Smash)

### QoL / systems
- Text speed options: Fast (8) and Faster (2) only (Instant removed)
- `RH_DISABLE_HELP_LR` (default TRUE): hide Help from Button Mode (LR / L=A only); new game defaults to L=A and continue migrates Help→L=A (vanilla Help path stays gated on that option)
- Skip “Previously on your quest…” Quest Log intro
- VS Seeker: no charge wait, 100% rematch chance, usable anywhere, all rematch tiers; Vermilion Pokémon Center dialogue updated
- ALL EXP SHARE given by Pewter Oak’s Aide after Brock (not via new-game shoes)
- Cerulean Pokémon Center: Gentleman infects the lead with Pokérus when talked to; Lass gives the Shiny Charm (3× shiny odds)
- Oak National Dex completion text redirects players to Cerulean for those rewards
- Fast battles (`RH_FAST_BATTLES`): anim/intro/transition speedups plus Pokéball send-out tunables (`RH_POKEBALL_BOUNCE_DELTA`, `RH_POKEBALL_ARC_FRAMES`, `RH_POKEBALL_STAGGER_FRAMES`, `RH_POKEBALL_RELEASE_FRAMES`, `RH_POKEBALL_DELAY_FRAMES`, `RH_POKEBALL_HEALTHBOX_TICKS`, `RH_POKEBALL_CRY_FRAMES_A` / `_B`)
- Debug Menu key item (`RH_DEBUG_MENU`, default TRUE): synced on new game and continue (added when TRUE, removed when FALSE). Use opens the overlay; **L** closes. Root: **Log**, Give, Warp, Cheats, **Events**, Close. Full how-to (including adding custom event flags): [docs/debug_menu.md](docs/debug_menu.md).
  - **Give** — catalogs + qty 0–99 ([`src/data/rh_debug_give.h`](src/data/rh_debug_give.h))
  - **Warp** — Towns/Cities, Routes, Special (legendaries/Snorlax), Sevii
  - **Cheats** — Player / Enemy / Catching / Misc / Reset All (bools + exp mult + one-shot money/dex); flags persist (`FLAG_SYS_CHEAT_*`, `VAR_CHEAT_EXP_MULT`)
  - **Events** — nested story/progress flag toggles (~368) with short selected-row descs ([`src/data/rh_debug_events.h`](src/data/rh_debug_events.h), from [`tools/gen_rh_debug_events.py`](tools/gen_rh_debug_events.py))
  - **Log** — heap ring-buffer console (`RhLog` / `RhLogf`, newest at top, **A** clears/frees); auto-logs menu open / give / warp / cheat / event toggles
  - Scrollable lists use the shared scrollbar module ([`src/scrollbar.c`](src/scrollbar.c))
  - Framed panels / footers use [`framed_panel`](include/framed_panel.h) (`Reset` / `Flush` / `ShowEmpty` / `ShowText`)
- Cheat battle/field hooks: God Mode, Infinite PP, accuracy / catch / shiny overrides, catch trainer Pokémon (mid-battle UI — [docs/ui-common-problems.md](docs/ui-common-problems.md#mid-battle-overlays)), free marts, always obey, max IVs on create, no wild encounters, instant egg hatch, walk through walls

### FULL MAKEOVER / stat editor
- **FULL MAKEOVER** item (`ITEM_FULL_MAKEOVER`): Celadon Dept Store 5F vitamin clerk (bottom of list), gated by `RH_STAT_MAKEOVER` (default TRUE); mart price `RH_MAKEOVER_PRICE` (default 49800)
- Use → party select → editor ([`src/rh_stat_editor.c`](src/rh_stat_editor.c)): nature grid, EV/IV sliders, footer EV total + Hidden Power blank type pill, party-icon portrait, live EV radar
- **B** → confirm exit without saving (no apply / no consume); **Start** → confirm save (apply + consume item + exit); nature-picker **B** still backs to main only
- Nature apply via `SetMonNature` (PID rewrite — [docs/set_mon_nature.md](docs/set_mon_nature.md))
- Shop buy-list prices: digit-width format + `GetStringWidth` right-align (no fixed 4-digit / `?xxx` overflow)
- **SELECT** cycles shared UI themes (same set as Debug Menu)

### Summary screen
- **Skills detail** (`A` on Skills / `B` back): blanked EXP/ability chrome, dual EV+IV `StatRadar` charts, Hidden Power blank type pill (same-pal type window)

### Story / map fixes
- `RH_SKIP_CATCH_TUTORIAL` (default TRUE): Viridian roadblock old man is present before Oak’s Parcel and is removed after delivery; Teachy TV is granted at new game. Set FALSE for vanilla catch tutorial + no auto Teachy TV.
- Pewter aide spawn fixed so ALL EXP SHARE guy appears correctly after Brock

### Followers (code present, off by default)
- HGSS-style party-lead follower engine is in the tree (`src/follow_me.c`, hooks in overworld / movement / warps)
- **Disabled** via `OW_FOLLOWERS_ENABLED` in [`include/config.h`](include/config.h) (`FALSE`)
- Set that define to `TRUE` to turn it on after sprites are ready

## Planned

### Update the Pokemon data to allow for a note setting system

### Reusable UI components
Same “caller owns the window / config paints” split as the shared scrollbar (`src/scrollbar.c`).

**Done:**
- Std framed panel + text strip — [`include/framed_panel.h`](include/framed_panel.h) / [`src/framed_panel.c`](src/framed_panel.c) (`FramedPanel_Reset` / `Flush` / `ShowEmpty` / `ShowText`). Debug menu list + footer use it.
- Inline value slider — [`include/value_slider.h`](include/value_slider.h) / [`src/value_slider.c`](src/value_slider.c) (`ValueSlider_SetDefaults` + label | track | value; hold accel; L/R step boosts). MAKEOVER EV/IV rows.
- Party-icon portrait overlay — [`include/mon_portrait.h`](include/mon_portrait.h) / [`src/mon_portrait.c`](src/mon_portrait.c) (`MonPortraitConfig`: align / nudge / pad / icon-or-front-pic / Hide). Window-relative placement.
- Stat radar (hex preview) — [`include/stat_radar.h`](include/stat_radar.h) / [`src/stat_radar.c`](src/stat_radar.c) (`SetDefaults` / `ApplySizePreset` PREVIEW·SMALL·MEDIUM·LARGE / `PadForSize` / `PlaceInWindow` / `PlaceInRect` / `NudgeTipUpToMidline(bias)` / optional `labelRadius` (0 = follow cage; set to keep labels out while `scalePercent` shrinks cage) / optional `drawDebugGuides`). Auto tip labels via `GetStringWidth`. Pass raw palette indices (not `PIXEL_FILL`). Color helpers: `RGB8` / `RGB_HEX`; CLI: [`tools/gba_color.py`](tools/gba_color.py) (convert + `--pal` dump/match); layout: [`tools/ui_layout.py`](tools/ui_layout.py). Cookbook: [docs/ui-components.md](docs/ui-components.md). MAKEOVER footer = PREVIEW in FooterStrip mid; **Skills detail** = dual EV/IV MEDIUM charts + `labelRadius` on a blanked lower canvas (`PSS_PAGE_SKILLS_DETAIL`).
- BG palette slot inject — [`include/bg_pal_slots.h`](include/bg_pal_slots.h) (`LoadBgPalSlots` / `LoadBgPalSlotRange`). Summary Skills-detail radar blues (pal 5).
- Footer strip regions — [`include/footer_strip.h`](include/footer_strip.h) (`FooterStrip_Split3` left|mid|right + screen XY). MAKEOVER text / radar / portrait.
- Shared UI themes — [`include/ui_theme.h`](include/ui_theme.h) (`UiTheme_ApplyStdWindow` / `Cycle`; 8 palettes). MAKEOVER + Debug Menu (**SELECT**); session-sticky index. [docs/ui-themes.md](docs/ui-themes.md).
- Generic UI chip painter — [`include/ui_chip.h`](include/ui_chip.h) / [`src/ui_chip.c`](src/ui_chip.c) (`UiChip_Draw` / `PrintLabel` / `BlitLabeled` / `Commit`; FIXED/FIT width; surround sanitize). TypeIcon blank path is a **preset** on top (stock bake + HP helpers stay in TypeIcon).
- Type icon — [`include/type_icon.h`](include/type_icon.h) / [`src/type_icon.c`](src/type_icon.c). Stock `menu_info` bake **and** procedural blank pills (`DrawBlank*` / `BlitBlank*`, white-ink + leftover-pad center). Offline preview: [`tools/preview_type_pills.py`](tools/preview_type_pills.py). Two host patterns:
  - **Same-pal window** (prefer when possible): window `paletteNum` = type bank → `TypeIcon_Draw` / `DrawBlank` / `BlitMenuInfoIcon`, clear to 0, stripes punch through. Summary Moves type column (stock bake); **Skills detail** Hidden Power pill (`PokeSum_DrawSkillsDetailHpTypeIcon`, blank + `HP ` + FIT, 7-tile window).
  - **Mixed-palette** (shared foreign window): `Blit` / `BlitBlank*` / `*WithSurround` / `SurroundFromBgPal` / sanitize chroma-key magenta / `Commit`/`CommitSized` after tilemap; `LoadPalette` once per screen. MAKEOVER footer = live HP type via **blank** path (canonical). Pitfalls: [docs/ui-common-problems.md](docs/ui-common-problems.md) §§3–4b. Rebuild-as-painter notes: [docs/design-principles/reusable-ui-components.md](docs/design-principles/reusable-ui-components.md).

Likely next:

- **Preview tool generalize** — `preview_type_pills.py` → e.g. `preview_ui_chip.py` (any chip config → PNG; type sheet compare = one recipe)
- **Debug component gallery** — DEBUG MENU page that shows framed panel + slider + radar presets (`PREVIEW` / `MEDIUM` / `LARGE`) + portrait (+ chip presets), ideally with live nudge/scale tweaks to cut rebuild loops
- **Value / toggle row** — label + `FALSE ▶ TRUE` or cycled enums (cheat-style strips)
- **Confirm modal** — extract MAKEOVER’s matte + Yes/No pattern (and Debug Reset All) into a shared helper
- **Scrollable custom list** — cursor/scroll owner + row painter (unify ListMenu pages vs hand-drawn cheat list)
- **Quantity picker** — wrap 0–99 (or configurable range) like Give qty
- Later / optional: toast banner, progress bar, tab strip

DevX cookbooks (from MAKEOVER branch friction): [docs/design-principles](docs/design-principles/README.md) (how to shape reusable painters; when to rebuild baked sheets), [docs/ui-components.md](docs/ui-components.md), [docs/ui-themes.md](docs/ui-themes.md), [docs/custom-screen.md](docs/custom-screen.md), [docs/new-item.md](docs/new-item.md), [docs/set_mon_nature.md](docs/set_mon_nature.md).

### Pokémon following (HGSS-style)
- Goal: lead Pokémon walks behind you like Gold/Silver / HeartGold/SoulSilver
- Engine work is largely done and gated behind `OW_FOLLOWERS_ENABLED`
- **Limitation:** FireRed only includes overworld sprites for a small set of species. Enabling followers without a fuller OW sprite pack means most Pokémon appear as a Pikachu placeholder (or similar fallback)
- **Before enabling:** add / port overworld sprites (and palettes) for the species you care about — ideally Gen 1–3 coverage — then set `OW_FOLLOWERS_ENABLED` to `TRUE` and rebuild
- Optional polish after sprites: bobbing idle, Poké Ball exit anim, richer talk lines / emotes

### FULL MAKEOVER polish (optional)
- Dedicated clerk NPC (instead of vitamin list entry)
- Extract MAKEOVER Yes/No confirm stacking (BG matte + dialog) into a reusable confirm-modal helper
- Slider / radar / footer layout tuning; footer hint text that mentions Start if desired

## Notes
- Existing saves may need map re-entry after some flag/scene fixes (e.g. Viridian old man, Pewter aide)
- Follower save data occupies space formerly in `SaveBlock1` padding; keep `OW_FOLLOWERS_ENABLED` off until you want the feature live
- **Planned bug:** Debug Menu **Log** can crash on Clear after spam-giving (mGBA `Jumped to invalid address: F7FD050A`) — repro + notes in [docs/debug_menu.md](docs/debug_menu.md#known-issues-investigate-later)
