# Rich galaxy — sensible extensions for a future game

**Actionable checklist:** [todo_roadmap.md](todo_roadmap.md)

This note lists **additions that would make sense** for GalaxyLib (and companion game code) if the goal is a **rich, explorable galaxy** comparable in depth to the legacy **GalaxyE** reference. It is not a commitment or full design spec; it prioritizes work and keeps rendering, UI, and networking in the game layer.

## What you already have

- **Macro scale:** sector grid, `Galaxy.bin` density, procedural `StarSystem` generation, `KnownSpace` catalogue, `Coords` / base36 strings.
- **Meso scale:** per-system bodies with types, AU orbits, simplified in-system positions (`SystemBody`).
- **Determinism:** stable IDs and table-driven generation suitable for simulation and saves keyed by coordinates.

The gaps below are the main levers for *richness*: navigation, time, places to land, resources, and content hooks.

---

## 1. Simulation time and moving orbits

**Why:** Bodies are currently advanced with a **fixed** reference epoch (matching the old C# workaround). A game needs a **current** galactic date so orbits, events, and UI clocks stay coherent.

**Sensible additions**

- **`StarDate`** — implemented as `galaxy::StarDate` (`star_date.hpp`): `from_earth`, `from_earth_utc`, `s_date`, `to_string` (UTC calendar math, +1204 years like C#).
- **`SystemLocation` / orbit API:** expose `get_updated_location(body, star_date)` (or equivalent) so the game does not duplicate `Cos`/`Sin` and AU constants.
- Optional: **hyperspace vs normal space** flags (`Location.cs` / `Coords.cs` patterns) as small structs the engine can branch on.

**Keep in the game:** animation tick rate, interpolation, time dilation for gameplay.

---

## 2. Spatial queries and “where am I?”

**Why:** GalaxyE resolves **sector → system → body** from coordinates and distances (`GameLocation`, `UpdateSystemBody`). A rich galaxy needs fast, predictable queries.

**Sensible additions**

- **`ResolveLocation(coords)`** (or split API): given `Coords`, return sector index, optional nearest system within a light-year or sector-local radius, optional nearest body using in-system distance (C# used moon-distance–scale thresholds).
- **Distance helpers:** same coordinate system as C# (`GalacticLocation` / ly conversions already in `coords`); add helpers for “distance between systems” or “to sector center” if the game needs mission ranges.
- **Stable ordering** when two bodies are equidistant (document tie-break).

**Keep in the game:** physics engine, collision meshes, floating origin.

---

## 3. Celestial hierarchy beyond “major bodies”

**Why:** One list of major planets is enough for a map; **gameplay** often wants moons, asteroid belts, ring systems, and stations.

**Sensible additions**

- **Optional children** under a `SystemBody` or a small `OrbitalNode` tree (name, type, orbital elements or parent-relative offset).
- **Belt / cluster** generators (thin wrappers around the same PRNG discipline as `SystemBody`) so asteroid fields do not look like single planets.
- **Named overrides** via data (extension files or JSON) for hand-authored systems, mirroring C# `ObjectExtender` / `sectors.txt` ideas without pulling in that whole pipeline on day one.

---

## 4. Surfaces, zones, and ports (gameplay density)

**Why:** GalaxyE’s **`Zones/`** layer (`Zone.cs`, `SpacePort.cs`, `Generator.cs`) turns a planet into **places**: landing zones, ports, POIs. That is where “rich” usually shows up for players.

**Sensible additions (phased)**

- **Minimal:** port list per body (id, name, type) — enough for missions and trade UI; can match C# `SpacePort.GeneratePorts` behavior later.
- **Medium:** **zone count** and coarse zone IDs (`Zone.CalculateNumZones`–style) without simulating every tile.
- **Full port:** planet **sector grid** (`SystemBody.PlanetSector` in C#) — only if the game needs tactical/surface maps; it is the largest chunk of logic and data.

**Keep in the game:** 3D terrain, combat arenas, detailed UI.

---

## 5. Resources and economy hooks

**Why:** `SystemBody` in C# carries **natural resource concentrations** (`NaturalResource` in `SystemBody.cs`). Trading, mining, and missions lean on this.

**Sensible additions**

- **Table-driven concentrations** (already defined in C#): per `SystemBodyType`, min/max per resource id; roll at generation time with the **same seed discipline** as other body properties if you need reproducibility.
- **Opaque resource id** enum or small struct so the game can map to icons, prices, and factions without GalaxyLib depending on a full economy sim.

**Keep in the game:** market prices, NPC factions, inventory, crafting.

---

## 6. Content pipeline and live data

**Why:** A “living” galaxy benefits from **updates** without recompiling.

**Sensible additions**

- External **`KnownSpace`**-style tables (CSV/JSON) built by scripts, same as today’s Python emitters.
- Optional **per-system / per-body overrides** (name, description, orbit hints) loaded from game data directories.
- Versioning: document **schema version** in emitted catalogues so saves can migrate.

**Keep in the game:** localization, voice, narrative scripting.

---

## 7. Persistence, networking, and determinism

**Why:** Multiplayer or save/load needs **stable IDs** and **reproducible generation**.

**Sensible additions**

- Serialize **IDs only** (`SectorId`, `SystemId`, body index, optional seed) and regenerate on load; avoid duplicating full star tables in saves unless needed for mods.
- Document **RNG parity** choices (e.g. `mt19937` vs .NET `Random`) wherever bit-identical output is required.
- Thread-safety contract: **const** generation after `init_galaxy_data`, or explicit instance API for `GalaxyMap` (see `architecture.md`).

**Keep in the game:** replication, anti-cheat, cloud saves.

---

## 8. API shape for engine integration

**Why:** A C++ game needs clear boundaries.

**Sensible additions**

- Thin **facade** (single include or namespace) listing: init map, load sector, query systems/bodies, resolve location, advance time.
- Optional **C ABI** for non-C++ engines if you need it later (`architecture.md` already mentions this).

---

## Suggested priority tiers

| Tier | Focus | Outcome |
|------|--------|--------|
| **A — Navigation & time** | `StarDate`, updatable orbital positions, `ResolveLocation`-style queries | Ships move in-system; missions can target bodies |
| **B — Places** | Spaceports + coarse zones | Land, dock, trade UI |
| **C — Resources** | Natural resource rolls | Mining / economy hooks |
| **D — Surface detail** | Full planet sector grid | Tactical / detailed planetary gameplay |
| **E — Content & tooling** | External tables, overrides, versioning | Live ops and modding |

---

## Text-mode (MUD / telnet) legacy — remove vs adapt

GalaxyE was a **multi-user text adventure** (telnet, ANSI, fixed-width `TextCanvas`). The repo splits roughly into **simulation** (`GalaxyE/Galaxy/*.cs`) and **presentation** (`TelnetServer/Game/io*.cs`, `TextCanvas`, `ANSI`). When building a graphical game, treat these as follows.

### Drop entirely (replace in your game / UI)

- **Telnet stack:** `TelnetServer/*`, negotiation, line-based `Send` / `SendLine`, prompts, `Message.ReadDefinedMessage` banners.
- **Text rendering:** `TextCanvas`, grid drawing, FANSI/ANSI color as the primary view model.
- **“IO” facades:** `ioStarSystemInfo`, `ioPlanetInfo`, `ioHUD`, `ioShipComputer`, etc. — they encode *how* to print stars and jumps, not new galaxy facts. Rebuild as UI views over the same data.
- **BinaryFormatter / heavy `ISerializable` graphs** (`SectorObject`, DB hooks like `SqlClient` in old files): do **not** reproduce for saves; use explicit save schemas or IDs + regeneration.

### Adapt (keep the idea, change the shape)

- **String location codes** (`GameLocation` parsing `tokens` from `"sectorX.sectorY...."`, `PlanetSector` like `"A3"` or `"p12"`): replace with **typed structs** (sector indices, system index, body id, optional grid `(x,y)` or surface id). Strings stay a **serialization / debug** format only.
- **Planet sector grid as characters:** `SystemBody.PlanetSector` filled from letters in `sectors.txt` — for 3D games, map the same logical grid to **tile indices** or **UV**; keep authoring in text only if you want a MUD-style editor, otherwise use images or tools.
- **Content extensions:** `ObjectExtender`, per-object `desc.txt` / `ports.txt` — replace with **data files** (JSON, binary blobs, or your ECS prefabs) loaded by the game; GalaxyLib can stay unaware or expose a small “override” hook.
- **`StarDate` display** (`"0.0000 EGMT …"`): keep the **numeric model** if useful; **formatting** belongs in UI/localization.
- **Fixed simulation date** in `SystemLocation.UpdateLocation` (hard-coded 2024 vs 2012): was a stability hack for a text world; a real game should drive orbits from **your** simulation clock (see §1 above).

### Keep as-is conceptually (often already in GalaxyLib)

- **Coords, sectors, system params, star generation, bodies, AU-based positions** — core simulation, not UI.
- **Enums** (`SystemBodyType`, spectral classes): use in code; let the game map them to icons, shaders, and localized names instead of relying on `GetClass()` / `GetClass_Ext()` strings in logic.

---

## Non-goals for GalaxyLib

To stay reusable and testable, keep **out of the core library**:

- Rendering, audio, input
- Ship combat, AI, factions
- Full quest engine and dialogue
- Netcode

Those belong in the game; GalaxyLib should stay **geometry + generation + identifiers + optional resource/placement data**.

---

## Reference map (GalaxyE → concept)

| Concept | Primary C# reference |
|--------|----------------------|
| Galactic time | `StarDate.cs` |
| Where you are | `Location.cs` (`GameLocation`, `SystemLocation`) |
| Orbital motion | `Coords.cs` (`GetUpdatedLocation`, `UpdateLocation`) |
| Bodies | `SystemBody.cs` |
| Ports / zones | `Galaxy/Zones/*.cs` |
| Utilities | `Utils.cs`, `StarSystemUtils.cs` |

For port status, see [porting-roadmap.md](porting-roadmap.md) and [architecture.md](architecture.md).
