# GalaxyLib: STC Integration Guide

This guide outlines the integration of the **Sector Traffic Controller (STC)** system into the GalaxyLib game engine. It is designed for the UI and Visual teams to facilitate the display of flight paths, HUD advisories, and traffic status.

---

## 1. Overview
The STC (Sector Traffic Controller) is a system-level service owned by each `StarSystem`. It manages ship traffic, generates multi-phase flight plans, provides predictive collision avoidance, and monitors flight path deviations.

**Key Technical Facts:**
- **Frequency:** The STC operates on a **1Hz (1 second) heartbeat**.
- **Logic:** Uses **Cross-Track Error** (perpendicular distance from path) and **Predictive Intercepts** (20-second look-ahead).
- **Orientation Deadzone:** Heading and pitch advisories are suppressed if the error is **< 5 degrees**.
- **Climb Angle:** Surface port departures use a gradual **15-20 degree** climb towards the destination.

---

## 2. Accessing the STC
Each `StarSystem` instance owns one `STC`. You access it via the system object:

```cpp
#include "galaxylib/star_system.hpp"
#include "galaxylib/stc.hpp"

// Assuming you have a pointer to the current StarSystem
auto& stc = current_system->stc();
```

---

## 3. Workflow Phase 1: Ship Registration
Before a ship receives guidance, it must register its "Manifesto." This triggers the STC to calculate a procedural flight plan (Departure -> EnRoute -> Approach -> Final).

**UI Action:** Call this when the player sets a destination or requests takeoff.

```cpp
galaxy::ShipManifesto manifesto;
manifesto.ship_id = "SHIP-001";
manifesto.call_sign = "Vanguard";
manifesto.destination_id = "Station-Alpha-01"; // Body or Port ID
manifesto.pos_x = ...; // Initial system coordinates
manifesto.dir_x = ...; // Current orientation (normalized)
manifesto.is_ifr = true; // Instrument Flight Rules (standard)

stc.receive_manifesto(manifesto);
```

---

## 4. Workflow Phase 2: The Radar Ping (1Hz)
The game world (or the ship's local controller) must feed the STC "Radar Pings." The STC uses these to check for deviations and collisions.

**Integration Note:** Feed this once per second. If the ship is in the Reference Frame (RF) of a station or planet, include the relative coordinates for precise docking logic.

```cpp
galaxy::ShipUpdatePing ping;
ping.ship_id = "SHIP-001";
ping.pos_x = ship_pos.x;
ping.vel_x = ship_vel.x;
ping.dir_x = ship_forward_vector.x; // CRITICAL for orientation checks

// Optional: For relative movement (docking/landing)
ping.parent_object_id = "Mercury"; 
ping.rel_pos_x = relative_pos.x;

stc.process_radar_ping(ping);
stc.tick(1.0); // Drive the 1Hz update loop
```

---

## 5. Workflow Phase 3: Consuming Advisories (HUD/UI)
The STC generates `STCOrder` objects. The UI team should pull these and display them on the HUD or use them to drive the Autopilot.

```cpp
std::vector<galaxy::STCOrder> orders = stc.get_pending_orders("SHIP-001");

for (const auto& order : orders) {
    // order.type (e.g., TurnLeftRight, PitchUpDown, AccelerateTo)
    // order.value (e.g., 90.0 degrees or 500.0 km/h)
    // order.description (e.g., "ROUTE DEVIATION: Correct heading to 90.0")
    
    HUD::DisplayMessage(order.description);
    HUD::UpdateNavPointer(order.type, order.value);
}
```

---

## 6. Visual & HUD Recommendations

### A. Navigation Pointers
- **Heading/Pitch Orders:** Display a ghost-vector or a "Flight Path Marker" on the HUD when a `TurnLeftRight` or `PitchUpDown` order is issued.
- **Speed Bars:** Use `AccelerateTo` / `DecelerateTo` to show a "Target Speed" notch on the throttle UI. Values are provided in **km/h**.

### B. Collision Warnings
- **Predictive Intercepts:** If an order description contains `"COLLISION ADVISORY"`, it means a collision is predicted within **20 seconds**. These should be rendered in **Flashing Red**.
- **Break Turns:** The `order.value` for a collision advisory is the recommended "Safe Heading" to break the intercept.

### C. Route Deviations
- **Cross-Track Error:** If a `"ROUTE DEVIATION"` is issued, the ship has drifted too far laterally from its flight path. 
- **Guidance vs. Deviation:** 
    - `Turn to heading...` is **Guidance** (normal navigation).
    - `ROUTE DEVIATION: Correct heading to...` is an **Alert** (player is off-course).

### D. Flight Phases for UI Labels
The STC handles the logic for 4 core phases:
- `DepartureClimb`: Gradual 15-20° ascent.
- `EnRouteCoast`: High-speed cruise between bodies.
- `Approach`: Entering the destination's 10km safety zone.
- `CloseFinal`: Low-speed docking/landing maneuver.

---

## 7. Data Structure Reference

| Struct | Purpose |
| :--- | :--- |
| `STCOrder` | The instruction to show the user (Type, Value, Text). |
| `STCOrderType` | Enum identifying if the value is a Heading, Pitch, or Speed. |
| `FlightPhase` | Current high-level status (Departure, EnRoute, etc.). |
| `ShipUpdatePing` | The data packet the UI/World must send to STC every second. |

---

**Contact:** For logic changes or specialized advisory types, please contact the Core Systems team. Integration tests can be found in `apps/stc_test/main.cpp`.
