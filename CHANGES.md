# Changes

Living overview of what this romhack does relative to pret FireRed / LeafGreen.  
For versioned release notes, see [CHANGELOG.md](CHANGELOG.md).

**Build:** `make` → `pokemon-random-evolution-v{version}.gba`  
**Version:** set `HACK_VERSION` in [`config.mk`](config.mk).

**Feature toggles & tunables:** [`include/config.h`](include/config.h)  
- `RH_*` / `OW_FOLLOWERS_ENABLED` — on/off gates  
- `RH_WALK_PX_PER_FRAME`, `RH_BIKE_PX_PER_FRAME`, text delays, battle speed multipliers, etc. — tweak values in one place  

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
- Cheat battle/field hooks: God Mode, Infinite PP, accuracy / catch / shiny overrides, catch trainer Pokémon (mid-battle UI — [docs/mid_battle_ui_pitfalls.md](docs/mid_battle_ui_pitfalls.md)), free marts, always obey, max IVs on create, no wild encounters, instant egg hatch, walk through walls

### Story / map fixes
- `RH_SKIP_CATCH_TUTORIAL` (default TRUE): Viridian roadblock old man is present before Oak’s Parcel and is removed after delivery; Teachy TV is granted at new game. Set FALSE for vanilla catch tutorial + no auto Teachy TV.
- Pewter aide spawn fixed so ALL EXP SHARE guy appears correctly after Brock

### Followers (code present, off by default)
- HGSS-style party-lead follower engine is in the tree (`src/follow_me.c`, hooks in overworld / movement / warps)
- **Disabled** via `OW_FOLLOWERS_ENABLED` in [`include/config.h`](include/config.h) (`FALSE`)
- Set that define to `TRUE` to turn it on after sprites are ready

## Planned

### Reusable UI components
Same “caller owns the window / config paints” split as the shared scrollbar (`src/scrollbar.c`). Likely order: panel + footer first, then toggle row + confirm.

- **Std framed panel** — `Fill` + `DrawStdFrame` + map/unmap helper
- **Footer / control bar** — framed prompt row (`{A} Open`, `{B} Back`, `{L} Close`, etc.)
- **Value / toggle row** — label + `FALSE ▶ TRUE` or cycled enums (cheat-style strips)
- **Confirm modal** — YES/NO overlay with default-cursor and callback (Reset All pattern)
- **Scrollable custom list** — cursor/scroll owner + row painter (unify ListMenu pages vs hand-drawn cheat list)
- **Quantity picker** — wrap 0–99 (or configurable range) like Give qty
- Later / optional: toast banner, progress bar, tab strip

### Pokémon following (HGSS-style)
- Goal: lead Pokémon walks behind you like Gold/Silver / HeartGold/SoulSilver
- Engine work is largely done and gated behind `OW_FOLLOWERS_ENABLED`
- **Limitation:** FireRed only includes overworld sprites for a small set of species. Enabling followers without a fuller OW sprite pack means most Pokémon appear as a Pikachu placeholder (or similar fallback)
- **Before enabling:** add / port overworld sprites (and palettes) for the species you care about — ideally Gen 1–3 coverage — then set `OW_FOLLOWERS_ENABLED` to `TRUE` and rebuild
- Optional polish after sprites: bobbing idle, Poké Ball exit anim, richer talk lines / emotes

### EV / IV manipulation items
- Add romhack items to adjust EVs and IVs (exact set / UX TBD)
- Sell them at the Celadon vitamin NPC at a scaled price (premium vs normal vitamins)

## Notes
- Existing saves may need map re-entry after some flag/scene fixes (e.g. Viridian old man, Pewter aide)
- Follower save data occupies space formerly in `SaveBlock1` padding; keep `OW_FOLLOWERS_ENABLED` off until you want the feature live
