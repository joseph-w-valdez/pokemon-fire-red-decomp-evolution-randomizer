# Debug Menu

Romhack development overlay opened from the **DEBUG MENU** key item. Used to give items, warp, toggle cheats, and view an in-game log while testing.

## Enable / disable

In [`include/config.h`](../include/config.h):

```c
#define RH_DEBUG_MENU              TRUE   // or FALSE
```

Rebuild after changing it (`make`).

| Value | Effect |
|-------|--------|
| `TRUE` | Key item is kept in the bag; Use opens the overlay |
| `FALSE` | Key item is removed if present; Use does nothing useful (Oak “stops you”) |

The item definition itself stays in the ROM either way (`ITEM_DEBUG_MENU` in items data). Only sync + Use behavior are gated.

## When the bag is synced

`RhDebugMenu_SyncKeyItem()` in [`src/rh_debug_menu.c`](../src/rh_debug_menu.c) runs on:

1. **New game** — [`src/new_game.c`](../src/new_game.c) after the bag is cleared  
2. **Continue** — [`src/overworld.c`](../src/overworld.c) `CB2_ContinueSavedGame` after the save is loaded  

So flipping `RH_DEBUG_MENU` and rebuilding updates **existing saves on load**, not only fresh games.

- `TRUE` → `AddBagItem(ITEM_DEBUG_MENU, 1)` if missing  
- `FALSE` → remove it if present, and clear it if it was the registered key item  

## How to use (in-game)

1. Open the bag → **Key Items** → **DEBUG MENU** → Use  
2. Root options: Log, Give, Warp, Cheats, Events, Close  
3. **L** closes the whole menu from any page (footer reminds you)  
4. **B** / **BACK** goes up one level  

### Give

Folders → (optional subfolders) → item name list → quantity:

| Folder | Contents |
|--------|----------|
| Poké Balls | All balls |
| Medicine | Potions/Drinks, Status Heals, PP Restore, Revives |
| Battle / Field | X items, dolls, ropes, repels, flutes |
| Vitamins | Vitamins, PP Up/Max, Rare Candy |
| Evolution | Stones + trade/hold evo items |
| Hold Items | Battle, Type Boosters, Species, Other |
| Berries | Status, Heal, EV, Growth/Catch, Pinch Stats |
| Mail | All mail |
| Treasures | Sellables, shards, fossils, orbs |
| TMs | TM01–25, TM26–50 |
| HMs | HM01–08 |
| Key Items | Story, Tools, Passes/Tickets, Other/Unused |
| Romhack | Custom key items + Debug Menu |

- **A** on an item → quantity page (Up/Down wraps `0`–`99`, starts at `1`)  
- **A** on quantity → `AddBagItem`  
- **B** / **BACK** goes up one level  

Catalog tables live in [`src/data/rh_debug_give.h`](../src/data/rh_debug_give.h).

### Warp

Folders → destination (same landing spots as Town Map **Fly** / heal locations):

| Folder | Destinations |
|--------|----------------|
| Towns / Cities | Pallet → Indigo (incl. Cinnabar, Saffron) |
| Routes | Nested: 1–10, 11–20, 21+ (21 North/South through 25) |
| Special | Articuno, Zapdos, Moltres, Mewtwo, Deoxys, Lugia, Ho-Oh, Snorlax (R12/R16) — stand in front of the encounter |
| Sevii Islands | One → Seven Island |

Selecting a destination fades out and warps (does not return to the bag).

### Cheats

Root → **CHEATS** → category folder → cheat list.

| Folder | Cheats |
|--------|--------|
| Player | God Mode, 100% Player Acc, Infinite PP, Exp Mult |
| Enemy | 0% Enemy Acc |
| Catching | 100% Catch Rate, Catch Trainer Pokémon, Max IVs |
| Misc | 100% Shiny, Walk Thru Walls, No Wild Encounters, Instant Egg Hatch, Free Poké Marts, Always Obey, Max Money (A apply), Unlock National Dex (A apply) |
| Reset All | Clears every cheat flag and sets Exp Mult back to 1x (asks **YES / NO** first; cursor starts on **NO**) |

Bool cheats show **FALSE** / **TRUE** on the line under the label; **EXP MULT** shows **1x 2x 5x 10x 15x** the same way.

- **Up / Down** — select a row (or BACK)  
- **Left / Right** (or **A**) — toggle bool / cycle exp mult; a `▶` marks the active value  
- **B** / BACK — return to the previous level (cheat list → folders → root)  
- **Reset All** → confirm **YES** to clear all, or **NO** / **B** to cancel  

| Cheat | Effect |
|-------|--------|
| God Mode | Player Pokémon take no HP damage in battle |
| 100% Shiny | All Pokémon count as shiny (generation + display) |
| 100% Player Acc | Player moves never miss; also hits through Protect / Fly / Dig / Dive (type immunities still apply) |
| Infinite PP | Player moves do not consume PP in battle |
| 0% Enemy Acc | Opponent moves always miss the accuracy roll |
| 100% Catch Rate | Any ball always catches (like a Master Ball roll) |
| Catch Trainer Pokémon | Skip the trainer ball-block. On a successful catch: faint+exp → dex page (if new) + nickname → give with **opponent as OT** → bag-style battle reshow → fight continues. See [`mid_battle_ui_pitfalls.md`](mid_battle_ui_pitfalls.md) if you change this flow. |
| Walk Thru Walls | Noclip on the overworld |
| No Wild Encounters | Skip random grass / water / surf wild battles (fishing & Rock Smash still work) |
| Instant Egg Hatch | Party eggs hatch on the next step check (very fast) |
| Max IVs | Newly generated Pokémon get 31 in every IV (wild, gifts, trainers, etc.) |
| Free Poké Marts | Mart buy prices are ¥0 (selling still uses half price) |
| Always Obey | Party Pokémon never disobey (ignores level / OT / badge limits) |
| Max Money | One-shot: set cash to 999999 (**A: APPLY**) |
| Unlock National Dex | One-shot: enable National Dex mode (**A: APPLY**) |
| Exp Mult | Battle EXP ×1 / ×2 / ×5 / ×10 / ×15 (`VAR_CHEAT_EXP_MULT`) |

Bools use `FLAG_SYS_CHEAT_*` (persist in the save).

### Log

In-game ring-buffer console (session-only).

| Control | Action |
|---------|--------|
| Up / Down | Scroll (**newest at top**; down = older) |
| **A** | Clear all lines and **free** the heap buffer |
| **B** | Back to root |

**Memory:** nothing reserved until the first `RhLog` / `RhLogf`. Then ~24×36 chars are allocated from the heap; Clear frees them so other features can reuse that RAM.

**API** (`include/rh_log.h`, `src/rh_log.c`): usable from **any** `.c` file (battles, field, new features, etc.). The debug menu Log page is only the viewer — logging does not require the menu to be open.

```c
#include "rh_log.h"

static const u8 sMsg[] = _("my feature hit");
static const u8 sFmt[] = _("value=%d name=%S");

RhLog(sMsg);
RhLogf(sFmt, someInt, someGameString);
```

Prefer **file-scope** `_("…")` strings for formats (inline `_()` in expressions can break agbcc/preproc). Specs: `%d`, `%x`/`%X`, `%s`/`%S`.

**Already hooked:** debug menu open, give item success/fail, cheat ON/OFF (by name), event flag ON/OFF (by name), exp mult label, max money / nat dex apply, reset all cheats, warps.

### Events

Root → **EVENTS** → folder → (optional subfolder) → flag toggles.

Catalog: [`src/data/rh_debug_events.h`](../src/data/rh_debug_events.h) (generated — prefer editing [`tools/gen_rh_debug_events.py`](../tools/gen_rh_debug_events.py)). ~368 story/progress flags.

**Out of this menu (on purpose):** TEMP flags, trainer-defeated bits, item-ball / hidden-item loot, Wonder Card unused slots, `FLAG_SYS_CHEAT_*` (use **Cheats**), special `0x4000+` EWRAM flags.

| Top folder | Nested? | Contents |
|------------|---------|----------|
| Early Game | Flat | Starter/lab SYS + hides, Viridian block, R22, Pewter guides |
| Kanto Story | Subs | Mt Moon, Bill, Anne, Rockets, Fuji, Snorlax, Silph/doors, Seafoam, Cinnabar, … |
| Badges / League | Flat | Badges, gym/E4/Champ defeats, game clear, postgame hides |
| HMs / Key Items | Flat | HMs, bike, rods, tools, Eevee/Dojo, flute, tea |
| Side Content | Subs | Trades, tutors, gym TMs, Oak aides, Fan Club, Day Care, misc |
| Legendaries | Flat | Birds, Mewtwo, Lugia, Ho-Oh, Deoxys |
| Sevii / Postgame | Subs | Nat Dex / maps / ships, tickets, Celio, Lostelle, warehouse, Selphy, … |
| World Map | Subs | Kanto towns, Kanto dungeons, Sevii maps |
| SYS Misc | Flat | Non-cheat permanent SYS extras |

#### How to use (in-game)

Same toggle chrome as Cheats: label on one line, **FALSE** / **TRUE** on the next. A short description for the **highlighted** row is drawn at the bottom of the list window.

| Control | Action |
|---------|--------|
| Up / Down | Move cursor (includes **BACK**) |
| Left / Right or **A** | Toggle the flag (`FlagSet` / `FlagClear`) and append to **Log** (`NAME ON` / `NAME OFF`) |
| **B** or BACK | Previous level (list → subfolder or folders → root) |
| **L** | Close the whole debug menu |

**Hide flags:** `TRUE` usually means the overworld object is **hidden**. You may need to leave and re-enter the map to see the change.

**Softlock note:** one bit alone (especially a hide without the matching GOT/story flag, or a badge without related script state) can desync scripts. Prefer flipping related flags together when jumping progress.

#### Flag ranges (reference)

| Range | Role | In Events? |
|-------|------|------------|
| `0x000`–`0x01F` | TEMP (cleared on map load) | No |
| `0x028`–`0x0AE` | Story hide/show | Selected |
| `0x154`–`0x1FE` | Item-ball hides | No |
| `0x230`–`0x2FF` | Story / GOT / FOUGHT | Selected |
| `0x3E8`–`0x4A6` | Hidden items | No |
| `0x4B0`–`0x4BC` | Boss clears | Yes (Badges / League) |
| `0x500`–`0x7FF` | Trainers | No |
| `0x800`–`0x8FF` | SYS | Progress only; no cheats |
| `0x4000+` | Special EWRAM | No |

#### Adding / maintaining events

**Source of truth:** [`tools/gen_rh_debug_events.py`](../tools/gen_rh_debug_events.py). Do **not** hand-edit `rh_debug_events.h` long-term — regenerate after catalog changes:

```bash
python tools/gen_rh_debug_events.py
make -j$(nproc)
```

Each row is `E("FLAG_*", "MENU NAME", "desc_key")` where `desc_key` is one of the shared keys in `DESCS` (`badge`, `defeat`, `hide`, `got`, `sys`, `fought`, `flew`, `door`, `boulder`, `current`, `map`, `tutor`, `trade`, `quiz`, `shop`). To add a new shared blurb, extend `DESCS` then use that key.

**Add a flag that already exists in** [`include/constants/flags.h`](../include/constants/flags.h)

1. Pick the target list in `EVENTS[...]` (e.g. `EVENTS["early"]`, `EVENTS["k_silph"]`, `EVENTS["v_celio"]`).  
2. Append `E("FLAG_YOUR_FLAG", "SHORT LABEL", "got"),` (or the right desc key).  
3. **Do not** reuse a flag already listed elsewhere — the generator rejects duplicates.  
4. Regenerate + rebuild.

**Example — expose a custom romhack flag under Early Game:**

```c
// include/constants/flags.h  (pick an unused save slot in the story/SYS range)
#define FLAG_RH_MY_QUEST_DONE  0x2E5
```

```python
# tools/gen_rh_debug_events.py  — inside EVENTS["early"]
E("FLAG_RH_MY_QUEST_DONE", "MY QUEST DONE", "got"),
```

Then regenerate. Scripts / C use `FlagGet` / `FlagSet` as usual; the Events menu only toggles the bit.

**Add a row to an existing nested subfolder**  
Same as above, but edit the array named in `KANTO_SUBS` / `SIDE_SUBS` / `SEVII_SUBS` / `WORLD_SUBS` (third field is the `EVENTS` key, e.g. `"k_anne"` → `EVENTS["k_anne"]`).

**Add a new subfolder under Kanto / Side / Sevii / World Map**

1. Create `EVENTS["k_myarea"] = [ E(...), ... ]`.  
2. Append `( "MYAREA", "MY AREA", "k_myarea" )` to the matching `*_SUBS` list.  
3. Regenerate — enums, labels, ListMenu rows, and lookup tables are emitted automatically.  
4. No `rh_debug_menu.c` change needed for new subs under those four parents.

**Add a new top-level folder**

1. Add `EVENTS["myfolder"] = [ ... ]` (flat) **or** a new `*_SUBS` list + arrays (nested).  
2. Append a row to `FOLDER_ORDER`: `( "MYFOLDER", "MY FOLDER", "myfolder", None )` for flat, or `( "MYFOLDER", "MY FOLDER", None, "mysubs" )` for nested.  
3. If nested, teach `RhDebugMenu_GetEventSubList` / table switches in [`src/rh_debug_menu.c`](../src/rh_debug_menu.c) the same way as Kanto/Side/Sevii/World Map. Flat folders need **no** C change beyond regenerate.  
4. Regenerate + rebuild.

**Rename / reorder / delete**  
Edit labels or list order in the Python catalogs; delete the `E(...)` line to remove. Regenerate. Leave unused flags in `flags.h` if scripts still need them.

**Quick hand-edit (prototyping only)**  
You can append `{ FLAG_*, name, desc }` to a `sEvents_*` array in `rh_debug_events.h` and rebuild, but the next generator run will overwrite it — move the change into the Python file when done.

## Source map

| Piece | Where |
|-------|--------|
| Feature flag | `include/config.h` → `RH_DEBUG_MENU` |
| Overlay UI + sync | `src/rh_debug_menu.c`, `include/rh_debug_menu.h` |
| Log ring buffer | `src/rh_log.c`, `include/rh_log.h` |
| Scrollbar (shared) | `src/scrollbar.c`, `include/scrollbar.h` |
| Give item catalogs | `src/data/rh_debug_give.h` |
| Events catalogs | `src/data/rh_debug_events.h` (from `tools/gen_rh_debug_events.py`) |
| Bag Use | `src/item_use.c` → `FieldUseFunc_DebugMenu` |
| Item id | `include/constants/items.h` → `ITEM_DEBUG_MENU` |
| Item data / name | `src/data/items.json` |
| Item icon | `src/data/item_icon_table.h` |
| Linker | `ld_script.ld` (`rh_debug_menu.o`) |

## How to update / maintain

Almost all menu content lives in [`src/rh_debug_menu.c`](../src/rh_debug_menu.c). After any edit: rebuild with `make`, then test in-game.

There are **two warp destination kinds**:

| Kind | Used by | List `index` means | Applied via |
|------|---------|----------------------|-------------|
| Heal / Fly | Towns, Sevii | `HEAL_LOCATION_*` | `SetWarpDestinationToHealLocation` |
| Map coords | **Routes**, **Special** | Index into a `struct RhDebugMapWarp` table | `SetWarpDestination(MAP_GROUP, MAP_NUM, WARP_ID_NONE, x, y)` |

`RhDebugMenu_ApplyWarp` uses `sRouteWarps` for `WARP_FOLDER_ROUTES`, `sSpecialWarps` for `WARP_FOLDER_SPECIAL`, otherwise heal.

Keep a trailing `{sText_Back, LIST_CANCEL}` on every destination list so B / BACK still work.

---

### Heal-based warps (Towns / Sevii)

**Symbols:** `sWarpTownItems`, `sWarpSeviiItems`  
**IDs:** [`include/constants/heal_locations.h`](../include/constants/heal_locations.h) (from [`src/data/heal_locations.json`](../src/data/heal_locations.json))

**Add**
1. Confirm the spot exists as `HEAL_LOCATION_*` (or add it in `heal_locations.json` and rebuild so the header regenerates).  
2. Add a label: `static const u8 sText_WarpFoo[] = _("FOO");`  
3. Append before BACK: `{sText_WarpFoo, HEAL_LOCATION_FOO},`

**Tweak / rename**
- Label only → edit the `_("…")` string.  
- Different fly spot → change the `HEAL_LOCATION_*` in the list entry.  
- Landing tile for a heal spot → edit `x`/`y` in `heal_locations.json` (affects Fly/respawn too — usually leave alone).

**Delete / reorder**
- Remove or move the `{label, HEAL_LOCATION_*}` line; leave BACK last.  
- Unused `sText_*` strings can be deleted to avoid clutter.

**Example — add Pewter under Towns (already present; pattern only):**
```c
{sText_WarpPewter, HEAL_LOCATION_PEWTER_CITY},
```

---

### Route warps (map + x, y)

**Symbols:** `ROUTE_WARP_…`, `sRouteWarps[]`, `sWarpRouteGroupItems`, `sWarpRouteItems_1_10` / `_11_20` / `_21Plus`  
Same pattern as Special. Groups: **1–10**, **11–20**, **21+**. Landing spots are outdoor tiles (near signs; R4/R10 use the Center fly tiles).

**Add / tweak / delete** — same steps as Special, but edit the Route symbols/table/list (and put the menu row in the right decade group).

---

### Special warps (map + x, y)

**Symbols:**
- Labels: `sText_Special…`
- Enum indices: `SPECIAL_WARP_…` (must match table slots)
- Coords: `sSpecialWarps[]`
- Menu rows: `sWarpSpecialItems[]`

Landing convention: **one tile in front of** a `FACE_DOWN` object → usually object `(x, y + 1)`. Check `data/maps/<Map>/map.json` object events for exact mon tiles.

**Add a Special destination**
1. Find `MAP_*` in [`include/constants/map_groups.h`](../include/constants/map_groups.h) and target `(x, y)`.  
2. Add label string.  
3. Append a new `SPECIAL_WARP_…` at the **end of the enum** (before any future values; keep contiguous).  
4. Add matching row in `sSpecialWarps`:
   ```c
   [SPECIAL_WARP_FOO] = {MAP_SOME_MAP, x, y},
   ```
5. Add menu row in `sWarpSpecialItems` **before** BACK:
   ```c
   {sText_SpecialFoo, SPECIAL_WARP_FOO},
   ```
6. Rebuild and smoke-test (walk into the object / press A).

**Tweak coordinates**
- Edit only `sSpecialWarps[…].x` / `.y` (or `.map` if the mon moved).  
- No menu/enum change needed if the index stays the same.

**Rename**
- Edit the `_("…")` label string only.

**Delete**
1. Remove the row from `sWarpSpecialItems`.  
2. Remove the `[SPECIAL_WARP_…]` entry from `sSpecialWarps`.  
3. Remove the enum value.  
4. If you remove a value from the **middle** of the enum, **renumber / rebuild the table** so enum indices still match `sSpecialWarps` slots (designated initializers help, but gaps are confusing — prefer deleting from the end or rewriting both enum + table cleanly).  
5. Remove the unused `sText_Special…` string.

**Reorder menu only**
- Reorder lines in `sWarpSpecialItems` only. Do **not** reorder the enum unless you also keep `sSpecialWarps` in sync.

**Example — current Zapdos entry:**
```c
[SPECIAL_WARP_ZAPDOS] = {MAP_POWER_PLANT, 5, 12},  // mon at (5,11), stand south
// …
{sText_SpecialZapdos, SPECIAL_WARP_ZAPDOS},
```

---

### Warp folders

**Symbols:** `WARP_FOLDER_*` enum, `sWarpFolderItems`, `RhDebugMenu_ShowWarpDest`, `RhDebugMenu_ApplyWarp`

**Add a folder**
1. Add `WARP_FOLDER_FOO` to the folder enum.  
2. Add folder label + `{sText_FolderFoo, WARP_FOLDER_FOO}` in `sWarpFolderItems` (before BACK).  
3. Add a dest list (`sWarpFooItems`) ending with `{sText_Back, LIST_CANCEL}`.  
4. In `RhDebugMenu_ShowWarpDest`, add a `case WARP_FOLDER_FOO:` that selects that list.  
5. If destinations are **map coords** (like Special), teach `RhDebugMenu_ApplyWarp` to handle `WARP_FOLDER_FOO` the same way as `WARP_FOLDER_SPECIAL` (or share one map-warp table). If they are **heal IDs**, the default `else` branch is enough.

**Rename / reorder / delete a folder**
- Rename → folder label string.  
- Reorder → reorder `sWarpFolderItems`.  
- Delete → remove folder enum value, folder list row, dest list, and the `case` in `ShowWarpDest` / `ApplyWarp`.

**Scroll**
- Lists longer than `WARP_LIST_MAX_SHOWED` (currently `5`) scroll automatically. Raise that `#define` to show more rows at once.

---

### Give menu

**Symbols:** pages `PAGE_GIVE_*`, catalogs in [`src/data/rh_debug_give.h`](../src/data/rh_debug_give.h), builders in `rh_debug_menu.c`.

Flow: folders → optional subfolders → item `ListMenu` (names from `ItemId_GetName`) → qty page → `AddBagItem`.

**Add an item to an existing list**
1. Append the `ITEM_*` id to the right `sGiveItems_…[]` array in `rh_debug_give.h`.  
2. Rebuild.

**Add a new leaf folder**
1. Add `GIVE_CAT_…` enum value.  
2. Add label + row in `sGiveFolderItems` (before BACK).  
3. Add `sGiveItems_…[]` and a `case` in `RhDebugMenu_GetGiveItemIds`.  

**Add a nested subfolder under an existing category**
1. Add sub enum + label row in the matching `sGive…SubItems` list.  
2. Add item id array.  
3. Extend the nested `switch` in `RhDebugMenu_GetGiveItemIds`.  
4. Ensure `RhDebugMenu_GiveFolderHasSubs` / `RhDebugMenu_GetGiveSubList` already cover that parent (or add them).

**Qty**
- Wrap `0`–`99` → `RhDebugMenu_AdjustGiveQty`.  
- Grant → `RhDebugMenu_TryGiveItem`.

**Scroll**
- Lists longer than `GIVE_LIST_MAX_SHOWED` (currently `5`) scroll. Raise that `#define` to show more rows.

---

### Root menu entries

1. Add `sText_…` + `DEBUG_MENU_…` and a row in `sRootItems`.  
2. Handle in `RhDebugMenu_HandleRootInput` (open a page or run an action).  
3. Rebuild.

To remove an entry, delete its `sRootItems` row and the switch arm (and any dead page code).

---

### Footers / chrome

Footer strings: `sText_FooterRoot`, `sText_FooterGiveFolders`, `sText_FooterGiveItems`, `sText_FooterGiveQty`, `sText_FooterWarpFolders`, `sText_FooterWarpDest`, `sText_FooterCheatFolders`, `sText_FooterCheats`, `sText_FooterEventFolders`, `sText_FooterEvents`, `sText_FooterLog`.  
Window layout: `sWinTemplates` (`WIN_LIST`, `WIN_SCROLL`, `WIN_FOOTER`).

Scrollable lists show a proportional scrollbar in `WIN_SCROLL` (one tile past the std-frame right border, col 29) via the shared [`scrollbar`](../include/scrollbar.h) module. If the list fits on screen, that window stays unmapped so it never covers the frame.

The scrollbar paints into a caller-owned window: size/position come from the `WindowTemplate`, colors/pads/`highlightColor`/`taskPriority` from `ScrollbarConfig`. Map/unmap and palette load stay in the menu.

---

### Disable for a release build

Set `RH_DEBUG_MENU` to `FALSE`, rebuild, then load each save once (or start new) so the key item is stripped. Or leave `TRUE` for internal test ROMs only.
