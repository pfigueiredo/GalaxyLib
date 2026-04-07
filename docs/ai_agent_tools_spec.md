# Specification: Agent Tool Manifest (ADK Bridge)

This document defines the schema for the tools available to LLM Agents (NPCs and System Authorities) to interact with the GalaxyLib state and the player.

## 1. Perception Tools (The "Senses")
These tools allow the agent to read the current procedural state.

| Tool Name | Parameters | Description |
| :--- | :--- | :--- |
| `sense_nearby_traffic` | `radius_km` | Returns a list of `ShipID`, `Velocity`, and `Heading` within the STC range. |
| `query_body_resources` | `body_id` | Returns the `Resource` density and current `MarketPrices` for a planet/station. |
| `check_ship_status` | `ship_id` | Returns damage levels, fuel, and `IFR` compliance of a target ship. |
| `read_system_clock` | `none` | Returns the current `StarDate` and local `CalendarDate`. |

## 2. Navigation & Action Tools (The "Body")
How agents move and act within the C++ simulation.

| Tool Name | Parameters | Description |
| :--- | :--- | :--- |
| `request_stc_path` | `dest_id`, `priority` | Interacts with `stc.cpp` to generate a 4-phase flight plan. |
| `adjust_vector` | `heading`, `pitch`, `speed` | Manually overrides the current trajectory (used by Rogue/Deceptive agents). |
| `execute_trade` | `item_id`, `qty`, `target_id` | Transfers resources between the agent's inventory and a port/ship. |
| `transmit_hail` | `target_id`, `msg_text` | Sends a natural language message to be displayed on the player's HUD. |

## 3. Dynamic Quest Weaver Tools (The "Will")
The core tools for creating the *Frontier/Elite* style emergent gameplay.

| Tool Name | Parameters | Description |
| :--- | :--- | :--- |
| `forge_mission` | `type`, `target`, `reward` | Creates a persistent `MissionObject` in the External DB. |
| `sign_relay_contract` | `mission_id`, `next_npc_id` | Chains a mission to another NPC, adding a "Handoff" logic. |
| `validate_objective` | `mission_id` | Queries GalaxyLib state to see if the player actually delivered the goods or killed the target. |
| `leak_intel` | `target_ship_id` | Broadcasts a ship's current `STC` waypoints to "Bounty Hunter" agents. |

## 4. System Authority Tools (The "Law")
Exclusive high-privilege tools for the system-wide "Overmind" agent.

| Tool Name | Parameters | Description |
| :--- | :--- | :--- |
| `set_system_policy` | `safety_margin`, `tax_rate` | Modifies the global `STC` collision buffers and trade tariffs. |
| `issue_emergency_order` | `text_msg`, `max_speed` | Broadcasts a mandatory speed limit to all ships in the system. |
| `scramble_patrol` | `target_ship_id` | Commands 3-5 NPC agents to intercept and interdict a specific ID. |
| `revoke_docking_rights` | `ship_id`, `station_id` | Hard-locks the `SpacePort` interface for the player. |

## 5. Implementation Logic (The ADK Bridge)
When an LLM calls `forge_mission(type="cargo", target="Sol_d_Port_1", reward=5000)`:
1. **ADK Validation:** Validates that `Sol_d_Port_1` exists in the `StarSystem`.
2. **C++ Side:** The ADK Server triggers `galaxy::stc::register_mission_trigger()` if applicable.
3. **Database Side:** A new row is inserted into the `active_quests` table with the LLM-generated narrative "backstory."
4. **Interaction:** The NPC sends a `transmit_hail` to the player with the backstory and the "Accept/Decline" prompt.
