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

### QoL / systems
- Text speed options: Fast (8) and Faster (2) only (Instant removed)
- Skip “Previously on your quest…” Quest Log intro
- VS Seeker: no charge wait, 100% rematch chance, usable anywhere, all rematch tiers; Vermilion Pokémon Center dialogue updated
- ALL EXP SHARE given by Pewter Oak’s Aide after Brock (not via new-game shoes)
- Cerulean Pokémon Center: Gentleman infects the lead with Pokérus when talked to; Lass gives the Shiny Charm (3× shiny odds)
- Oak National Dex completion text redirects players to Cerulean for those rewards

### Story / map fixes
- Viridian roadblock old man is present before Oak’s Parcel and is removed after delivery (catch tutorial disabled — he does not stay for the Teachy TV demo)
- Pewter aide spawn fixed so ALL EXP SHARE guy appears correctly after Brock

### Followers (code present, off by default)
- HGSS-style party-lead follower engine is in the tree (`src/follow_me.c`, hooks in overworld / movement / warps)
- **Disabled** via `OW_FOLLOWERS_ENABLED` in [`include/config.h`](include/config.h) (`FALSE`)
- Set that define to `TRUE` to turn it on after sprites are ready

## Planned

### Pokémon following (HGSS-style)
- Goal: lead Pokémon walks behind you like Gold/Silver / HeartGold/SoulSilver
- Engine work is largely done and gated behind `OW_FOLLOWERS_ENABLED`
- **Limitation:** FireRed only includes overworld sprites for a small set of species. Enabling followers without a fuller OW sprite pack means most Pokémon appear as a Pikachu placeholder (or similar fallback)
- **Before enabling:** add / port overworld sprites (and palettes) for the species you care about — ideally Gen 1–3 coverage — then set `OW_FOLLOWERS_ENABLED` to `TRUE` and rebuild
- Optional polish after sprites: bobbing idle, Poké Ball exit anim, richer talk lines / emotes

## Notes
- Existing saves may need map re-entry after some flag/scene fixes (e.g. Viridian old man, Pewter aide)
- Follower save data occupies space formerly in `SaveBlock1` padding; keep `OW_FOLLOWERS_ENABLED` off until you want the feature live
