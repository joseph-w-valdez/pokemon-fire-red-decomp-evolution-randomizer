# SetMonNature pitfalls

`bool8 SetMonNature(struct Pokemon *mon, u8 nature)` in [`src/pokemon.c`](../src/pokemon.c) (declared in [`include/pokemon.h`](../include/pokemon.h)).

Nature is **not** a free field on the mon — it is derived from `personality % NUM_NATURES`. Changing nature means finding a **new personality** and rewriting encrypted substructs.

## What it preserves

While searching for `newPersonality`, the function keeps:

- **Nature** = requested value  
- **Gender** for that species (from old personality)  
- **Shiny** state vs OT id (from old personality)

Substruct payloads (EVs, IVs, moves, etc.) are copied under the new personality key, checksummed, re-encrypted; then `CalculateMonStats`.

## Failure mode

It tries up to **60 000** random personalities. If none match nature + gender + shiny, it restores the old personality and returns **`FALSE`**. Callers should not assume success (MAKEOVER currently ignores the return — fine for UX if rare, but worth knowing when debugging “nature didn’t stick”).

## Do / don’t

- **Do** call after writing EVs/IVs if you also change nature in the same apply path (MAKEOVER `StatEditor_ApplyDraft` does both).
- **Do** expect ability from personality / hidden ability rules to follow the new PID where applicable.
- **Don’t** poke `box.personality` yourself without the substruct dance — checksum/encryption will corrupt the mon.
- **Don’t** use this for “display only” — it mutates save data.

## Why this exists

Gen 3 has no dedicated nature byte in the struct layout used here; PID is the source of truth. Any editor UI that offers natures needs this (or an equivalent PID search).
