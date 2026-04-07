# Specification: AI Level-of-Detail (LOD)

## LOD 0: "Sentient" (LLM Active)
- **Criteria:** NPCs within 50km; System Authority of current system.
- **Processing:** Full access to `Agent Tool Manifest`. Persistent Vector Memory enabled.
- **Cost:** High (Inference tokens used).

## LOD 1: "Simulated" (State Machine)
- **Criteria:** NPCs in the same StarSystem but >50km away.
- **Processing:** Follows basic "Drives" (Trade, Patrol) using C++ logic. 
- **LLM Role:** The LLM is "off," but it has left a "Directive" (e.g., "Stay aggressive toward miners").

## LOD 2: "Statistical" (Database Record)
- **Criteria:** All other 400 billion systems.
- **Processing:** No active code. Only `SystemId` and `FactionInfluence` percentages exist in the DB.
- **LLM Role:** "Wakes up" only when a player jumps into the system, reading the DB to "reconstruct" the system's history.
