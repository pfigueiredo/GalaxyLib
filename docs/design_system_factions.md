# Design: Procedural System Factions

## 1. The Faction Entity
Every `StarSystem` is populated by 3-5 local factions. These are persistent LLM-driven groups with their own balance sheets and military assets.

### Faction Archetypes:
- **Corporate Extractors:** Focused on `Resource` harvesting. Goal: High profits, low safety margins.
- **The Idealist Rebels:** Focused on independence from the `Super-Factions`. Goal: Sabotaging System Authority (SA).
- **The Worker's Union:** Focused on port maintenance. Goal: Controlling `SpacePort` landing fees.
- **The Shadow Syndicate:** Focused on contraband. Goal: Corrupting the STC safety buffers.

## 2. Faction Assets (GalaxyLib Integration)
Factions don't just exist in text; they own the "Physics":
- **Controlled Ports:** A faction can "own" a `SpacePort`, granting them the right to set taxes.
- **Fleet Strength:** A count of NPC Agents registered under the Faction's `ShipManifesto` ID.
- **Station Influence:** A percentage (0-100%) representing their control over the local System Authority.

## 3. The FIG Loop (Faction Influence & Governance)
The "Political Health" of a system is a zero-sum game.
- **Action:** If a player completes a `forge_mission` for the *Miners*, their `Influence` increases.
- **Reaction:** The *Corporate* faction's `Influence` decreases.
- **Consequence:** At >51% Influence, a faction becomes the **Ruling Faction**, effectively "installing" their own personality seed into the **System Authority (SA)**.
