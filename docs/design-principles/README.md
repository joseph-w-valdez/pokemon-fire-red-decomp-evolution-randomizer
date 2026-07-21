# Design principles

Forward-looking rules for how we shape shared code in this romhack. Prefer these when adding a module; use the cookbooks for APIs and incident write-ups.

| Doc | Use when |
|-----|----------|
| [reusable-ui-components.md](reusable-ui-components.md) | Designing or reviewing a window painter / overlay helper (same-pal vs mixed-palette, ownership, GBA constraints, when to rebuild baked sheets as painters) |
| [../ui-components.md](../ui-components.md) | Calling an existing component (recipes, presets, gotchas) |
| [../ui-common-problems.md](../ui-common-problems.md) | A painter looks “almost right” (symptoms → fixes) |
| [../custom-screen.md](../custom-screen.md) | Bootstrapping a full CB2 overlay |
| [../ui-themes.md](../ui-themes.md) | Std-window theme slots and cycling |

**Split:** principles = *how to design*; cookbooks = *how to use*; common-problems = *what already burned us*.
