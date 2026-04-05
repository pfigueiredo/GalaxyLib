# Using GalaxyLib from another project

This document is for **game or tool code** that links against **GalaxyLib** (not for developing the library itself). It covers CMake wiring, assets, initialization, and a recommended usage pattern.

## What you need

- **C++17** or newer.
- **GalaxyLib** built as a static library (`galaxylib`) or your project compiling the library sources (see below).
- **`Galaxy.bin`** at **runtime** — raw RGB triplets (same format as the legacy C# loader). Generate it with **`python tools/build_galaxy_bin.py`** from the GalaxyLib tree (writes `assets/Galaxy.bin`), or ship your own file and pass its path to `initialize`. If the file is missing or empty, initialization fails and **`Sector` construction can throw** if the map was left empty due to a bug—always ensure a non-empty valid `Galaxy.bin` after a successful load.

**MSVC:** GalaxyLib is built with `/utf-8` for string tables. Your executable should use the same if you include GalaxyLib headers that pull in narrow string literals from the library’s tables, or you may see mojibake. Easiest: `target_compile_options(your_target PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)`.

## CMake: link the library

### Option A — `add_subdirectory` (monorepo / vendor copy)

If GalaxyLib lives next to your game (e.g. `third_party/GalaxyLib`):

```cmake
add_subdirectory(third_party/GalaxyLib EXCLUDE_FROM_ALL)
target_link_libraries(your_game PRIVATE galaxylib)
```

`galaxylib` exposes includes via `target_include_directories(... PUBLIC ...)`, so `target_link_libraries` is enough for `#include "galaxylib/..."`.

### Option B — Prebuilt `galaxylib.lib` + headers

Point at the **include** tree (`GalaxyLib/include`) and the **import library** from your build (`build/Release/galaxylib.lib` on Windows). Add the include directory and link the `.lib` to your target. You must use the **same** runtime (e.g. MSVC `/MD` vs `/MDd`) as GalaxyLib.

### Option C — `FetchContent` / package manager

Same idea as A: produce a target named `galaxylib` (or wrap the library in an `INTERFACE` target that sets include dirs + imported library).

There is **no** `install()` rules in the stock `CMakeLists.txt`; if you add install rules in your fork, consumers can use `find_package` patterns later.

## Runtime asset: `Galaxy.bin`

1. Copy `Galaxy.bin` into your game’s data directory (or keep a known path relative to the executable).
2. At startup, pass the **absolute or relative path** to the loader — **not** a compile-time path unless you deliberately embed it.

Example pattern:

```cpp
const std::string galaxy_bin = (std::filesystem::path(exe_dir) / "data" / "Galaxy.bin").string();
if (!galaxy::api::initialize({galaxy_bin})) {
    // handle error: file missing, corrupt, wrong format
}
```

`galaxy::api::initialize` loads the file into the global galaxy map used by `Sector` (see [architecture.md](architecture.md)).

## Startup sequence

1. Call **`galaxy::api::initialize`** once, before any **`galaxy::Sector`** use.
2. Optionally check **`galaxy::api::galaxy_ready()`** (true if the map loaded and is non-empty).
3. Read **`galaxy::api::library_version()`** if you want to log or gate features.

```cpp
#include "galaxylib/galaxy_api.hpp"
#include "galaxylib/sector.hpp"

// ...
if (!galaxy::api::initialize({path_to_galaxy_bin})) {
    return false;
}
```

## Recommended game flow (performance)

| Phase | What to use | What to avoid |
|--------|-------------|----------------|
| **Regional / sector map** | `galaxy::Sector`, `num_systems()`, `galaxy::api::list_system_summaries(sector)` or `sector.systems()` for `StarSystem` refs | Calling **`StarSystem::bodies()`** for every system — that generates all planets at once. |
| **Selected system** | `StarSystem::bodies()` when the player enters or selects that system | Regenerating sectors every frame — **cache** `Sector` / systems if possible. |

- **`num_bodies()`** is cheap (count only).
- **`bodies()`** is expensive (builds `SystemBody` list once, then caches on that `StarSystem` instance).

## Headers you will touch most

| Header | Purpose |
|--------|---------|
| [`galaxylib/galaxy_api.hpp`](../include/galaxylib/galaxy_api.hpp) | `galaxy::api::initialize`, `library_version`, `SystemSummary`, `list_system_summaries` |
| [`galaxylib/sector.hpp`](../include/galaxylib/sector.hpp) | `galaxy::Sector` — indices, `systems()`, `center_coords`, `contains` |
| [`galaxylib/star_system.hpp`](../include/galaxylib/star_system.hpp) | `StarSystem` — name, type, coords, `num_bodies()`, `bodies()` |
| [`galaxylib/system_body.hpp`](../include/galaxylib/system_body.hpp) | `SystemBody` — orbit, `update_system_location`, etc. |
| [`galaxylib/coords.hpp`](../include/galaxylib/coords.hpp) | `Coords`, base36 strings, IDs |
| [`galaxylib/star_date.hpp`](../include/galaxylib/star_date.hpp) | In-universe calendar (`StarDate`) |
| [`galaxylib/calendar_date.hpp`](../include/galaxylib/calendar_date.hpp) | Gregorian `CalendarDate`, days vs orbit reference |

Lower-level or specialized: `base36.hpp`, `galaxy_constants.hpp`, `known_space.hpp` (catalogue behavior is wired inside `Sector`).

## Small code sketch

```cpp
#include "galaxylib/galaxy_api.hpp"
#include "galaxylib/sector.hpp"

void example(std::uint32_t sector_x, std::uint32_t sector_y) {
    galaxy::Sector sector(sector_x, sector_y);

    // Map UI: summaries only (no planets generated)
    for (const auto& row : galaxy::api::list_system_summaries(sector)) {
        (void)row.name;
        (void)row.num_bodies;
    }

    // Player picks one system
    const auto& systems = sector.systems();
    if (!systems.empty()) {
        const galaxy::StarSystem& sys = systems[0];
        const auto& bodies = sys.bodies();  // planets generated here
        (void)bodies.size();
    }
}
```

## Threading and lifetime

- After a successful **`initialize`**, treat galaxy map data as **read-only** from multiple threads if you do not call **`initialize`** again.
- **`Sector`** and **`StarSystem`** own generated data; keep them **alive** while you hold references to `StarSystem` or `SystemBody` (e.g. don’t destroy a `Sector` while storing pointers into `sector.systems()`).

## Reference project

The **`galaxy_test`** app ([`apps/galaxy_test/main.cpp`](../apps/galaxy_test/main.cpp)) loads assets, builds sectors, lists systems, and prints bodies for Sol — use it as a minimal integration test.

## Further reading

- [architecture.md](architecture.md) — data flow, modules, assets  
- [todo_roadmap.md](todo_roadmap.md) — planned features (zones, resources, etc.)  
- [rich-galaxy-extensions.md](rich-galaxy-extensions.md) — design notes for a richer game layer  
