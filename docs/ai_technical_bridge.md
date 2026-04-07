# Technical Bridge: GalaxyLib <-> Google ADK

## 1. The ADK Environment
The system will run an **ADK Server** (Python/Node) that instantiates the GalaxyLib C++ library via a wrapper.

## 2. Data Flow
1. **Perception:** Every 1s (`stc::tick`), the C++ state (Ship positions, `STCOrders`, `StarDate`) is serialized to JSON.
2. **Context Injection:** The ADK Server fetches the `SystemId` and `ShipId` memories from the **External Database**.
3. **LLM Inference:** The SA or NPC Agent processes the state + memory.
4. **Action:** The Agent calls a "Tool" (e.g., `SA.black_list_ship`).
5. **Actuation:** The tool writes back to the GalaxyLib C++ instance (e.g., updating the `ships_` map in `stc.cpp`).

## 3. Persistence Schema (External DB)
- **Systems Table:** `system_id`, `personality_seed`, `influence_level`, `last_star_date`.
- **Agents Table:** `agent_id`, `history_summary`, `trust_score`, `current_inventory`.
- **Vector Store:** Embeddings of player conversations and major system events (e.g., "The great collision of 4383").

## 4. Scalability
To handle millions of systems, LLM agents are **Hibernated**. An agent only "Wakes Up" (loads context into memory) when a player is present in that `Sector`.
