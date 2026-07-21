# Changelog

All notable **version bumps** for this repo are recorded here.  
Feature details and planned work live in [CHANGES.md](CHANGES.md).

Version string comes from `HACK_VERSION` in [`config.mk`](config.mk). Format follows [Keep a Changelog](https://keepachangelog.com/).

## [0.4.0] — 2026-07-22

### Added
- **FULL MAKEOVER** item (`RH_STAT_MAKEOVER` / `RH_MAKEOVER_PRICE`): Celadon 5F vitamin clerk → party select → EV/IV/nature editor (`src/rh_stat_editor.c`) with confirm exit/save, theme cycle, live radar + Hidden Power pill
- `SetMonNature` (PID rewrite for nature changes)
- Reusable UI kit: `value_slider`, `mon_portrait`, `stat_radar` (+ `labelRadius`), `type_icon` (stock bake + procedural blank pills), `footer_strip`, `bg_pal_slots`, `ui_theme` (8 palettes; **SELECT** in MAKEOVER + Debug Menu)
- Summary **Skills detail** subpage: dual EV/IV radars + Hidden Power blank type pill
- Dev tools: `tools/gba_color.py`, `tools/ui_layout.py`, `tools/preview_type_pills.py`
- UI cookbooks: [docs/ui-components.md](docs/ui-components.md), [docs/ui-themes.md](docs/ui-themes.md), [docs/custom-screen.md](docs/custom-screen.md), [docs/new-item.md](docs/new-item.md), [docs/set_mon_nature.md](docs/set_mon_nature.md), [docs/design-principles](docs/design-principles/README.md)

### Changed
- Mart buy-list prices: variable digit width + right-align (supports 5–6 digit MAKEOVER price; no `?xxx` overflow)
- `RH_DISABLE_HELP_LR`: Help removed from Button Mode; new game defaults to L=A; continue migrates Help → L=A
- Mid-battle catch UI notes folded into [docs/ui-common-problems.md](docs/ui-common-problems.md) (replaces `docs/mid_battle_ui_pitfalls.md`)
- Debug Menu Log: known Clear crash after spam-give documented for later ([docs/debug_menu.md](docs/debug_menu.md#known-issues-investigate-later))

### Docs
- [CHANGES.md](CHANGES.md) marks MAKEOVER / Skills detail / UI kit as implemented; planned polish + Log crash note

## [0.3.1] — 2026-07-21

### Added
- Shared framed panel helpers (`src/framed_panel.c`): `FramedPanel_Reset` / `Flush` / `ShowEmpty` / `ShowText` — same caller-owns-window split as the scrollbar

### Changed
- Debug menu list and footer chrome draw through `framed_panel` instead of duplicated fill/frame/map/copy

### Docs
- [CHANGES.md](CHANGES.md) Planned reusable-UI list marks panel/footer done; [docs/debug_menu.md](docs/debug_menu.md) notes the module

## [0.3.0] — 2026-07-20

### Added
- Debug Menu key item (`RH_DEBUG_MENU`, default TRUE): synced on new game and continue; root **Log / Give / Warp / Cheats / Events / Close**. Docs: [docs/debug_menu.md](docs/debug_menu.md)
- Give catalogs + quantity (0–99); Warp Towns / Routes / Special (legendaries & Snorlax) / Sevii
- Cheats folders (Player / Enemy / Catching / Misc / Reset All) with persistent `FLAG_SYS_CHEAT_*` and `VAR_CHEAT_EXP_MULT`
- Classic AR-style cheat suite: God Mode, accuracy / catch / shiny / PP / walk-through-walls, no wild encounters, instant hatch, max IVs, free marts, always obey, exp mult, plus one-shot Max Money / National Dex
- Mid-battle catch of trainer Pokémon when that cheat is on (dex / nickname / OT from opponent; battle continues) — [docs/ui-common-problems.md](docs/ui-common-problems.md#mid-battle-overlays)
- Events: nested story/progress flag toggles (~368) with short selected-row descriptions; catalog generated from [`tools/gen_rh_debug_events.py`](tools/gen_rh_debug_events.py) → [`src/data/rh_debug_events.h`](src/data/rh_debug_events.h)
- Shared proportional menu scrollbar (`src/scrollbar.c`) for scrollable debug lists
- In-game debug Log (`RhLog` / `RhLogf`): heap ring buffer, newest-first viewer, Clear frees the buffer; auto-logs menu open / give / warps / cheats / events
- `RH_SKIP_CATCH_TUTORIAL` gate (default TRUE): Viridian tutorial skip + Teachy TV at new game; FALSE restores vanilla
- Feature gates and tunables centralized in [`include/config.h`](include/config.h) (`RH_*`, movement / text / battle / Pokéball send-out values)

### Changed
- Hack behaviors restore nearer-vanilla paths when the matching `RH_*` / `OW_FOLLOWERS_ENABLED` toggle is FALSE
- Trade evolution overwrite and HM “show mon” skip gated behind existing `RH_*` flags
- Pokéball / send-out timings moved under `RH_FAST_BATTLES` tunables

### Docs
- [CHANGES.md](CHANGES.md) documents toggles/tunables and the debug menu overview
- [docs/debug_menu.md](docs/debug_menu.md): how to use Give / Warp / Cheats / Log / Events, plus how to add custom event flags
- [docs/ui-common-problems.md](docs/ui-common-problems.md): UI pitfalls (window painters + mid-battle catch overlays)

## [0.2.0] — 2026-07-20

Initial tagged romhack release on this repository (forked from pret pokefirered).

### Added
- Random level-up evolution (one per party mon per battle) with post-evo move offer
- Optional Nuzlocke mode (Oak yes/no at new game)
- Faster walk / run / bike / surf; run almost everywhere; start with Running Shoes
- Reusable TMs; forgettable HMs; HM key items; HM08 Dive (Celadon Hotel)
- Normal wild encounters in Safari Zone
- Text speeds Fast / Faster only; skip Quest Log intro
- VS Seeker: no charge, 100% rematch, usable anywhere
- ALL EXP SHARE from Pewter aide after Brock
- Cerulean PC: Pokérus (Gentleman) and Shiny Charm (Lass)
- Versioned ROM output: `pokemon-random-evolution-v{HACK_VERSION}.gba`

### Changed
- Viridian roadblock old man: present until Oak’s Parcel, then removed (no catch tutorial)
- Pewter aide spawn so ALL EXP SHARE appears correctly after Brock
- Oak National Dex completion text points to Cerulean rewards

### Disabled / gated
- HGSS-style follower Pokémon engine present but off (`OW_FOLLOWERS_ENABLED` = `FALSE`) until OW sprites are expanded — see Planned in [CHANGES.md](CHANGES.md)
