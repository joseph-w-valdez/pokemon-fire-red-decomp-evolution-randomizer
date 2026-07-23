# Custom CB2 screen bootstrap

Minimal pattern for a full-screen overlay like MAKEOVER ([`src/rh_stat_editor.c`](../src/rh_stat_editor.c)) or the debug menu. Most of those files are this boilerplate; feature code sits on top.

## Ownership

- **You own:** BG templates, `WindowTemplate`s, palette loads, DISPCNT, VBlank, tasks, fade in/out, freeing windows/sprites on exit.
- **Components own:** painting into windows you already created ([ui-components.md](ui-components.md)).

## Skeleton (order matters)

```text
CB2_Open…
  save gMain.savedCallback if needed
  → StatEditor_InitGfx / equivalent

InitGfx:
  SetVBlankCallback(NULL)
  clear VRAM / OAM / PLTT
  ResetBgsAndClearDma3BusyFlags
  InitBgsFromTemplates(…)
  InitWindows(…)
  ResetPaletteFade / ResetTasks / ResetSpriteData / FreeAllSpritePalettes
  LoadStdWindowGfx(firstWindow, 0x1C0, BG_PLTT_ID(14))   // frame tiles + base pal
  UiTheme_ApplyStdWindow()                               // shared themes → pals 14+15 + gap
  Fill + CopyBgTilemapBufferToVram
  ...
  // In HANDLE_INPUT: SELECT → UiTheme_Cycle(TRUE)  (optional; MAKEOVER + Debug do this)
  ShowBg(0)
  DISPCNT: MODE_0 | OBJ_1D_MAP | OBJ_ON | BGx_ON
  SetVBlankCallback(VBlankCB_…)   // TransferPlttBuffer, ProcessSpriteCopyRequests, …
  CreateTask(Task_…, 0) → FADE_IN
  build UI (FramedPanel / ListMenu / components)
  SetMainCallback2(CB2_…)         // RunTasks, AnimateSprites, BuildOamBuffer, UpdatePaletteFade

Task states:
  FADE_IN → WAIT_FADE_IN → HANDLE_INPUT → (fade out) → EXIT
EXIT:
  destroy sprites/components, FreeAllWindowBuffers, DestroyTask
  SetMainCallback2(gMain.savedCallback)
```

## Party-item entry

From bag → party → your screen:

```c
// ItemUseCB
RhThing_SetPending(gPartyMenu.slotId, gSpecialVar_ItemId);
sPartyMenuInternal->exitCallback = CB2_OpenThing;
Task_ClosePartyMenu(taskId);
```

See [new-item.md](new-item.md).

## Windows

- Sizes are in **tiles** (×8 = pixels). Preview fit with `python3 tools/ui_layout.py window --tiles W H`.
- `.paletteNum` must match the BG pal bank you loaded for that window’s art/text.
- Std frame tile base `0x1C0` + pal 14 is the usual framed-panel contract.
- **BG0–BG3 only** (GBA hardware). Overlapping std-framed windows need **different BGs** or a ≥1-tile gap — see [ui-common-problems.md](ui-common-problems.md) § BG layers. MAKEOVER: editor BG0, HP pill BG1, YesNo BG2, confirm MSG BG3.

## Don’ts

- Don’t `FramedPanel_Reset` every input tick if only row contents change — see paint refresh in [ui-components.md](ui-components.md).
- Don’t leave VBlank pointing at a destroyed screen’s callback.
- Don’t forget `ld_script.ld` when adding a new `src/*.c`.
- Don’t put two overlapping `DrawStdFrame*` windows on the **same** BG (last map write deletes the earlier bezel).
- Don’t paper over frame transparency with a full-rect opaque matte if you want list/footer visible around rounded corners.

Related: [design-principles](design-principles/README.md), [ui-themes.md](ui-themes.md), [ui-components.md](ui-components.md), [ui-common-problems.md](ui-common-problems.md) (window painters, BG layers, mid-battle overlays).
