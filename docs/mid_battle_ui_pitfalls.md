# Mid-battle UI pitfalls

Lessons from wiring **Catch Trainer Pokémon** (dex page + nickname mid-fight, then return to a living battle). Vanilla FireRed almost never does this: wild catch tears the battle down and goes to the overworld. Trainer steals must **rebuild** the battle instead.

Primary code: [`src/battle_script_commands.c`](../src/battle_script_commands.c) (`displaydexinfo`, `trygivecaughtmonnick`, `givecaughtmon`, `reshowbattlescreen`), [`src/reshow_battle_screen.c`](../src/reshow_battle_screen.c), [`data/battle_scripts_2.s`](../data/battle_scripts_2.s), [`src/trainer_pokemon_sprites.c`](../src/trainer_pokemon_sprites.c).

---

## Mental model

| Piece | Role |
|-------|------|
| `BattleMainCB1` | Every frame: `gBattleMainFunc()` + **battler controllers** |
| `BattleMainCB2` | Every frame: sprites, tasks, text, fades |
| Controllers | Often wait on sprite callbacks (`CompleteOnBattlerSprite*`) |
| `ReshowBattleScreenAfterMenu` | Bag-style full rebuild of battle gfx (use this to return from menus) |

If you change CB2 (dex, naming, party, bag) but leave CB1 running, controllers keep ticking against whatever sprite state you left behind.

---

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

---

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

---

## Dex mid-battle is already violent

`DexScreen_LoadResources()` does `ResetSpriteData()`, `ResetTasks()`, its own BGs/windows/VBlank.

Vanilla gets away with it because catch ends the battle. On a trainer steal:

- Battle tasks are gone (battle logic is mostly CB1, so it may still limp along)
- Dex often leaves mon-pic slots allocated
- Closing the dex (`DoClosePokedex`) frees windows/tilemaps but does not fully restore battle gfx

After dex, prefer a **vanilla-like** `displaydexinfo` case 2 (VRAM wipe + battle VBlank), then reclaim pics/tiles when creating the catch-summary mon pic. Full battle rebuild belongs in `reshowbattlescreen` / bag reshow.

---

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

---

## Give mon vs reshow order (HP)

Steal prep keeps **catch HP on the enemy party slot** while `gBattleMons[].hp == 0` (for exp / faint flags).

`ReshowBattleScreenAfterMenu` decides “create real mon sprite vs invisible placeholder” from **party HP**.

| Order | Result |
|-------|--------|
| Reshow **before** `givecaughtmon` | Party HP still > 0 → reloads the stolen mon as if it were still out |
| `givecaughtmon` **then** reshow | Party HP set to 0 → placeholder; send-out can proceed |

`givecaughtmon` must zero the field slot’s HP before any bag-style reshow.

---

## Healthboxes and battler sprites after reshow

- Do **not** destroy/recreate healthboxes in a post-reshow cleanup “just to be safe” — that stacks duplicate boxes.
- After reshow, only hide the fainted side’s healthbox/shadow and replace the battler sprite with an invisible placeholder if needed (`CleanupFaintedOpponentAfterStealReshow`).
- After dex `ResetSpriteData` (if you ever use it), `gHealthboxSpriteIds[]` are stale — guard with `inUse` before `SetHealthboxSpriteInvisible`.
- Opponent send-out: destroy any leftover placeholder in `StartSendOutAnim` before creating the new mon sprite.

---

## Shared `gBattleCommunication` fields

```c
#define MULTIUSE_STATE   0
#define CURSOR_POSITION  1
#define TASK_ID          1  // same byte as cursor!
```

Do not stash a long-lived value in `TASK_ID` across the nickname Yes/No UI. Catch-summary mon pic id lives in `gBattleStruct->field_78` (`0xFF` = none). `CreateMonPic` returns `u16` — clamp before storing in a `u8`.

---

## OT / give helpers

`GiveMonToPlayer()` rewrites OT id and can turn stolen `OT_ID_RANDOM_NO_SHINY` mons into **Bad Eggs**. Trainer steals use `GiveMonToPlayerPreserveOT()` after stamping the opponent’s **display** name (rival via `GetExpandedPlaceholder(PLACEHOLDER_ID_RIVAL)`, not the data-table `"TERRY"`).

---

## Quick checklist (new mid-battle menu)

1. Will CB1 / controllers still run? If you `ResetSpriteData`, freeze CB1.
2. Are you freeing mon pics by id `0..63`? Use active-only free.
3. Does return need `ReshowBattleScreenAfterMenu` instead of `BattleMainCB2`?
4. If the fight continues: give/faint party HP **before** reshow.
5. Avoid recreate-all-healthboxes; hide + placeholder only.
6. Don’t overload `gBattleCommunication[TASK_ID]` across Yes/No UI.

---

## Related

- Cheat behavior summary: [`docs/debug_menu.md`](debug_menu.md) (Catch Trainer Pokémon)
- Steal script: `BattleScript_BallThrowSteal` in [`data/battle_scripts_2.s`](../data/battle_scripts_2.s)
