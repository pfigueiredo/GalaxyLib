In the folder `GalaxyE-master` there is a legacy C# project (`GalaxyE`) with classes that generate a procedural galaxy.

This repository hosts the **C++ port (CMake)**: a linkable library for use from a C++ game, plus a **console tester**.

## Documentation

- [docs/integration.md](docs/integration.md) — **how to link and use the library from another CMake / C++ project**
- [include/galaxylib/galaxy_api.hpp](include/galaxylib/galaxy_api.hpp) — **`galaxy::api`** — init, version, map-friendly `SystemSummary`
- [include/galaxylib/star_date.hpp](include/galaxylib/star_date.hpp) — **`galaxy::StarDate`** — matches C# `StarDate.cs`
- [docs/architecture.md](docs/architecture.md) — modules, data flow, assets
- [docs/clean-code.md](docs/clean-code.md) — porting rules and style
- [docs/porting-roadmap.md](docs/porting-roadmap.md) — what is done vs next
- [docs/rich-galaxy-extensions.md](docs/rich-galaxy-extensions.md) — ideas for a richer game-facing galaxy (time, places, resources, priorities)
- [docs/todo_roadmap.md](docs/todo_roadmap.md) — consolidated actionable checklist (tiers A–E + game layer)

## Build

From the repo root:

```text
python tools/build_galaxy_bin.py
cmake -B build
cmake --build build --config Release
```

`tools/build_galaxy_bin.py` writes `assets/Galaxy.bin` (RGB data derived from `TheMilkyWay2` in `GalaxyE` `Sector.cs`). **Without this file, `galaxy_test` and any app calling `galaxy::api::initialize` will fail** until you generate or supply a `Galaxy.bin`.

Run `build/Release/galaxy_test.exe` (Windows); it loads `assets/Galaxy.bin` via the CMake `GALAXYLIB_ASSETS_DIR` path.
