# Design: Inter-Agent Social Fabric

## 1. The "Sub-Space" Bulletin (The Agent Network)
NPCs don't just wait for the player. They "Interact" in the background via the external database.
- **Tool:** `post_bounty(target_id, reward)`
- **Tool:** `negotiate_trade(agent_id, resource_id)`
- **Gameplay Effect:** When a player enters a bar at a `SpacePort`, the LLM generates the "Atmosphere" by summarizing recent background transactions. "Did you hear NPC-X lost his fleet to the SA in Sol?"

## 2. Multi-Agent Conspiracies
Multiple NPCs can form a "Group Agent" (a Cartel or a Science Team).
- They share a **Joint Vector Memory**.
- If the player aids one member, the LLM prompt for all members is updated with `TrustValue += 0.2`.
- They can coordinate "Ambush Missions" where NPC-A lures you into a deep-space sector where NPC-B and NPC-C are waiting.

## 3. The "Unreliable Narrator" Mechanic
Because agents are powered by LLMs, they can **Deceive**. 
- An agent might use the `forge_mission` tool to create a "Medical Delivery" that is actually "Contraband."
- The player must use "Scan Tools" (GalaxyLib sensors) to verify NPC claims. Discrepancies between NPC speech and Sensor data create the core gameplay tension.
