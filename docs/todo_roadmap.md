# GalaxyLib — TODO roadmap

Single actionable checklist. Rationale and GalaxyE references live in [rich-galaxy-extensions.md](rich-galaxy-extensions.md); C# port history in [porting-roadmap.md](porting-roadmap.md); module layout in [architecture.md](architecture.md).

---

## Completed — library foundation

- [x] CMake: `galaxylib` + `galaxy_test`; MSVC UTF-8 where needed.
- [x] Galaxy constants, base36, `rotate_system_params` / `rotl` / `rotr`.
- [x] `Galaxy.bin` load + sector system count (`Generate_NumSystems` parity).
- [x] `Sector`: params, lazy `systems()`, `center_coords` / `contains`, KnownSpace-first fill.
- [x] `StarSystem`: procedural + catalogue ctor, tables via `extract_star_system_tables.py`.
- [x] `Coords` / `SystemId` / `SectorId`, `to_string` / `try_parse`.
- [x] KnownSpace catalogue (`build_known_space_catalog.py`) + placement parity.
- [x] `SystemBody` + `StarSystem::bodies()` (procedural + Sol).

---

## Tier A — Time, orbits, and “where am I?” (library-first)

*Unblocks moving ships, targeting bodies, and any clock in UI.*

- [x] **`StarDate`:** `galaxy::StarDate` — `from_earth`, `s_date`, `to_string` (`StarDate.cs`); UTC-based; `SystemBody` still uses `CalendarDate` / days for orbits unless you convert.
- [x] **Updatable orbital positions:** `SystemBody::update_system_location(double)` and `update_system_location(CalendarDate)`; reference epoch `k_orbit_reference_date` (2012-01-01). Default construction still uses the legacy fixed span (4383 days).
- [ ] **`SystemLocation`-style API:** meters in-system from AU + angle + time; single place for AU→m constant (match C# or document drift).
- [ ] **`ResolveLocation` / queries:** from `Coords` → sector (given map), nearest system within range, optional nearest body (document distance threshold vs C# moon-scale check).
- [ ] **Distance helpers:** ly between points / to sector center if missions need ranges.
- [ ] **Optional:** hyperspace vs normal-space flags as small types (see `Location.cs` patterns)—only if the game design needs them in the lib.

---

## Tier B — Ports and coarse zones

*Land, dock, basic missions without full planetary grid.*

- [ ] **Spaceport list per body:** id, name, type—enough for hooks; align with `SpacePort` / generator in C# when ready.
- [ ] **Zone count + coarse zone ids** (`Zone.CalculateNumZones`–style) without simulating every tile.

---

## Tier C — Resources

*Mining / economy hooks; not market simulation.*

- [ ] **Natural resource rolls** on generation: table min/max per `SystemBodyType` (`NaturalResource` in `SystemBody.cs`).
- [ ] **Stable resource ids** (enum or ints) for the game to map to UI and prices.

---

## Tier D — Richer celestial tree (optional)

*Only when the game needs more than one layer of “things in orbit”.*

- [ ] Moons / child orbiters or `OrbitalNode`-style tree.
- [ ] Belt / cluster generators (same RNG discipline as bodies).

---

## Tier E — Surface detail (heavy)

*Defer until you need tactical / map-scale planetary play.*

- [ ] Full **planet sector grid** (`PlanetSector` in C#)—large port; pair with game terrain or 2D map.

---

## Content pipeline & persistence

*Cross-cutting; can start early for mods.*

- [ ] Document **schema version** for emitted catalogues / overrides.
- [ ] Optional **JSON (or similar) overrides** for known systems / bodies (replace ad-hoc `ObjectExtender` / `.txt` sidecars).
- [ ] **Save/load contract:** serialize IDs + regen vs full snapshot—document RNG parity (`mt19937` vs .NET).
- [ ] **Threading contract:** immutable galaxy map after init; or instance-owned `GalaxyMap` (see architecture).

---

## API ergonomics

- [x] **`galaxy::api`** (`galaxy_api.hpp`): `initialize`, `galaxy_ready`, `library_version`, `SystemSummary`, `make_system_summary`, `list_system_summaries` — extend this namespace for new game-facing helpers.
- [ ] Optional **C ABI** if a non-C++ engine must link.

---

## Validation & tests

- [ ] Expand **`galaxy_test`** (or unit tests): fixed sectors vs C# expectations table (names, spectral codes, counts).
- [ ] Where bit-identical RNG matters, document or add **.NET parity** test vectors.

---

## Game / engine layer (not GalaxyLib)

Do **not** port into the library; track in your game backlog:

- [ ] **Telnet / `TextCanvas` / `io*`** — replace with your UI.
- [ ] **String-only `GameLocation`** — use typed structs; keep strings for save/debug only.
- [ ] **BinaryFormatter / DB-tied serialization** — explicit save format in the game.
- [ ] **Rendering, combat, AI, factions, netcode, dialogue.**

---

## Risk reminders

- Unsigned shifts vs C# `ulong` behavior when touching packed IDs.
- Match `float`/`double` choices to C# when changing formulas shared with the reference.
