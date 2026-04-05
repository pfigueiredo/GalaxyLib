# GalaxyLib — Clean code & porting rules

## Goals

- **Match the C# implementation** where it matters for gameplay continuity (coordinates, IDs, procedural output).
- **Keep the C++ codebase smaller and clearer** than a line-by-line translation: extract helpers, use named constants, but do not “improve” algorithms silently.

## Naming

- Prefer **parity names** with C# for types and functions that have a direct counterpart (`Sector`, `GalaxyConstants`, `rotate_system_params`).
- Use **snake_case** for free functions and **PascalCase** or **snake_case** for types consistently with the rest of the tree (this project uses `snake_case` for C++ files and public types like `galaxy::Sector`).

## Numbers and portability

- Reproduce C# **`uint` / `ulong` overflow and shift** semantics explicitly where the original code relies on them (e.g. masked rotations, `>>` on unsigned types).
- Use **fixed-width integers** (`std::uint32_t`, `std::uint64_t`) in ported arithmetic blocks; document when `int` vs `uint` matters.

## Dependencies

- **Standard library only** for the core library unless there is a strong reason (no Boost in the baseline).
- No hidden global mutable state beyond explicitly documented initialization (`init_galaxy_data`).

## Files and size

- Large static tables (e.g. `SystemDensity`, spectral tables) may live in **dedicated `.cpp`** files to keep compile times reasonable.
- Do not commit generated binaries without documenting how they were produced; `Galaxy.bin` here is generated from `TheMilkyWay2` in `Sector.cs`.

## Testing

- Console app checks **known inputs** (e.g. sector `368.4V5` from `TestApp`) and prints **sector system count** and coordinates.
- When porting `StarSystem`, add **golden checks** against the C# executable for the same sector and system index.

## What not to do

- Refactor the galaxy generation algorithm for “elegance” without a spec change.
- Pull in networking, UI, or engine types into `galaxylib`.
