# Design: The Dynamic Quest Weaver (DQW)

## 1. The Philosophical Shift
In GalaxyLib, missions are not "spawned" by a quest manager. They are **Manifested Needs** of LLM Agents. An NPC "needs" a resource or a rival "removed" because their internal state (Greed, Revenge, survival) has been triggered by the procedural state of the galaxy.

## 2. The Desire Engine (The "Why")
Every NPC agent runs a background process that evaluates their current "Drives":
- **Material Drive:** Needs specific `Resources` (e.g., fuel, luxury goods).
- **Political Drive:** Needs to influence a `System Authority` (SA) or undermine a rival faction.
- **Social Drive:** Needs to find a lost "Contact" or deliver a message.

## 3. Mission Synthesis (Tool: `forge_mission`)
The NPC uses an ADK tool to generate a gameplay contract.
- **Inputs:** Current Galaxy State (Prices, Traffic, Faction Allegiance) + NPC Goal.
- **Output:** A structured `MissionObject` containing:
    - `ObjectiveType`: (Cargo, Combat, Data, Escort, Sabotage).
    - `TargetID`: (A specific Ship, SystemBody, or SpacePort).
    - `NarrativeContext`: The "Lie" or "Truth" the NPC tells the player.

## 4. The "Relay" (Chaining NPCs)
A mission can contain a `HandoffCondition`.
- **Example:** NPC-A (The Broker) hires you to deliver a "Locked Case" to System X.
- **The Chain:** Upon arrival, NPC-B (The Contact) is notified via the Database. NPC-B's LLM reads the history of your flight (Did you hit any `ROUTE DEVIATION`? Did you take too long?).
- **Branching:** NPC-B might then tool-call `mutate_mission` to add a new leg: "The Broker betrayed us. Take this bomb back to him."
