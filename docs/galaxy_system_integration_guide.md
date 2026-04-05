# GalaxyLib: Complete System Integration Guide

This guide provides a comprehensive overview of the **GalaxyLib** engine for UI, VFX, and Gameplay teams. It covers initialization, the spatial hierarchy, coordinate systems, and the Sector Traffic Controller (STC).

---

## 1. System Initialization
GalaxyLib must be initialized once at startup. It requires the `Galaxy.bin` asset, which contains the density map of the Milky Way.

```cpp
#include "galaxylib/galaxy_api.hpp"

galaxy::api::InitConfig config;
config.galaxy_bin_path = "assets/Galaxy.bin";

if (!galaxy::api::initialize(config)) {
    // Handle error: assets missing or corrupt
}
```

---

## 2. Spatial Hierarchy (The "Data Tree")
To visualize the universe, follow this hierarchy. Each level is "lazily generated" to save memory.

### 1. Sector (Regional Map)
A sector is a **100x100x100 light-year** cube.
- **UI Use:** Galactic maps, "long-range" scans.
- **Cost:** Very cheap. Generating a sector only calculates how many stars are in it.

```cpp
#include "galaxylib/sector.hpp"
galaxy::Sector sector(4111, 6303); // Sol Sector
auto systems = sector.systems();   // Triggers star generation
```

### 2. StarSystem (Local System)
A system contains a star and its orbiting bodies.
- **UI Use:** System maps, solar system views.
- **Cost:** Moderate. Accessing `bodies()` triggers the generation of all planets, moons, and stations.

```cpp
#include "galaxylib/star_system.hpp"
const galaxy::StarSystem& sol = systems[0];
std::string name = sol.name();       // "Sol"
std::string type = sol.star_type();   // "G2V"
int num_planets = sol.num_bodies();   // Cheap count
auto bodies = sol.bodies();           // Expensive generation
```

### 3. SystemBody (Planets & Moons)
Individual celestial objects.
- **UI Use:** Planet rendering, orbit lines, surface landing sites.

```cpp
#include "galaxylib/system_body.hpp"
const auto& earth = bodies[3]; // Sol d
double radius = earth.radius(); // In Earth Radii
auto ports = earth.ports();     // Cities, bases, and stations
```

---

## 3. Coordinate Systems
GalaxyLib uses three distinct coordinate frames. Understanding these is critical for VFX and UI positioning.

| Frame | Unit | Description |
| :--- | :--- | :--- |
| **Galactic** | Light Years / Sectors | Used for the Galaxy Map. (8192x8192 grid). |
| **System** | Meters | Used for the STC and local flight. Origin (0,0,0) is the System Star. |
| **Relative** | Meters | Used for landing/docking. Origin is the center of the Planet or Station. |

**VFX Note:** When rendering a planet, use its **System Coordinates** to place it in space. When the player is landing, switch to **Relative Coordinates** for high-precision jitter-free movement.

```cpp
// Get body position in the Star System (meters)
double x = earth.system_x_m();
double y = earth.system_y_m();
double z = earth.system_z_m();
```

---

## 4. Sector Traffic Controller (STC)
The STC provides real-time guidance and safety monitoring for ships within a `StarSystem`.

### 1Hz Radar Loop
The UI/Physics engine must send a "Radar Ping" to the STC every second.

```cpp
galaxy::ShipUpdatePing ping;
ping.ship_id = "Vanguard-01";
ping.pos_x = current_pos.x;
ping.dir_x = forward_vec.x; // Normalized forward vector

stc.process_radar_ping(ping);
stc.tick(1.0); // Process the second
```

### HUD Advisories
The STC issues `STCOrder` objects which the HUD should display:
- **Navigation:** `Turn to heading 90.0`, `Pitch to 18.4`.
- **Safety:** `COLLISION ADVISORY: Predicted intercept with Mercury`.
- **Alerts:** `ROUTE DEVIATION: Correct heading to 45.0`.

**Visual Rules:**
- **Deadzone:** STC will not issue guidance if the ship is within **5 degrees** of the correct path.
- **Collision:** Advisories are issued if an intercept is predicted within **20 seconds**.

---

## 5. Time & Calendar
GalaxyLib uses a procedural `StarDate` system and a Gregorian `CalendarDate`.

```cpp
#include "galaxylib/star_date.hpp"
#include "galaxylib/calendar_date.hpp"

// Convert current time to in-game StarDate
auto sd = galaxy::StarDate::from_earth_utc(2026, 4, 5, 12, 0, 0);
std::string date_str = sd.to_string(); // e.g., "4383.5"
```

---

## 6. Porting & Data IDs
- **SystemId / SectorId:** 64-bit unique identifiers for every object in the galaxy. Use these for save games or networking.
- **Base36:** Systems can be addressed via Base36 strings (e.g., `"SOL"`) for user search inputs.

---

**Integration Contact:** For architecture questions, see `docs/architecture.md`. For build issues, see `docs/integration.md`.
