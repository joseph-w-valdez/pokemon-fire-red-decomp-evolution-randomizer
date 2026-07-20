# Changelog

All notable **version bumps** for this repo are recorded here.  
Feature details and planned work live in [CHANGES.md](CHANGES.md).

Version string comes from `HACK_VERSION` in [`config.mk`](config.mk). Format follows [Keep a Changelog](https://keepachangelog.com/).

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
