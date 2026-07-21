# New item checklist

Wiring a bag item that opens a custom screen (MAKEOVER pattern) or a simple field effect. Missing any row usually looks like “item does nothing.”

Reference implementation: **FULL MAKEOVER** (`ITEM_FULL_MAKEOVER`).

## Party-menu item → custom CB2 (MAKEOVER)

| Step | Where | What |
|------|--------|------|
| 1. ID | [`include/constants/items.h`](../include/constants/items.h) | `#define ITEM_…`, bump `ITEMS_COUNT` |
| 2. Data | [`src/data/items.json`](../src/data/items.json) | name, price, pocket, `type: ITEM_TYPE_PARTY_MENU`, `fieldUseFunc`, description |
| 3. Icon | [`src/data/item_icon_table.h`](../src/data/item_icon_table.h) | gfx + palette (reuse is fine; MAKEOVER uses Rare Candy art) |
| 4. Field use | [`src/item_use.c`](../src/item_use.c) + [`include/item_use.h`](../include/item_use.h) | `FieldUseFunc_…` → set `gItemUseCB` + `DoSetUpItemUseCallback` |
| 5. Party CB | [`src/party_menu.c`](../src/party_menu.c) + [`include/party_menu.h`](../include/party_menu.h) | `ItemUseCB_…`: stash slot/item, set `exitCallback` to your `CB2_Open…`, close party |
| 6. Screen API | your `include/rh_….h` | `SetPending(slot, itemId)`, `CB2_Open…` |
| 7. Screen impl | `src/….c` | Init gfx / task / exit; consume item on apply if needed |
| 8. Link | [`ld_script.ld`](../ld_script.ld) | `.text` + `.rodata` for new `.o` (easy to forget) |
| 9. Shop / give | map scripts / debug give tables | e.g. Celadon 5F vitamin clerk `.2byte ITEM_FULL_MAKEOVER` |

### Flow

```text
Bag Use → FieldUseFunc → party select → ItemUseCB
  → SetPending(slot, itemId) → exitCallback = CB2_Open…
  → custom screen → apply / B cancel → savedCallback (usually bag/overworld)
```

### Rebuild note

After `items.json` / icon table changes, a normal `make` regenerates item data. If the bag still shows a stale name/icon, hard-reload the ROM (not only soft reset).

## Simpler field-only key item

Same steps 1–3, then a `FieldUseFunc` that runs the effect directly (see WAYMO / HM key items in `item_use.c`). No party CB or `ld_script` entry unless you add new `.c` files.

## Debug Menu give catalog

If testers should receive it from DEBUG → Give, also add the id under [`src/data/rh_debug_give.h`](../src/data/rh_debug_give.h).

Related: [custom-screen.md](custom-screen.md), [debug_menu.md](debug_menu.md).
