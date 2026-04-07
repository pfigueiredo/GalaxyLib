# Analysis: System Integrity & Feasibility Review

This document evaluates the technical and gameplay risks of the LLM-integrated GalaxyLib and proposes architectural solutions.

## 1. Technical Bottlenecks

### Problem A: The "Inference Wall" (Latency)
*   **Risk:** LLMs (even via Google ADK) take 1–5 seconds to process a prompt. The `STC` loop runs at 1Hz. If a System Authority (SA) needs to decide on a `ROUTE DEVIATION` penalty, the player will have flown 5km past the event before the AI "speaks."
*   **Solution: Asynchronous Intent.** The C++ engine handles immediate physics and legal enforcement (fines). The LLM processes the *narrative* and *long-term posture* in the background. The AI doesn't "react" to a single tick; it reacts to "Event Batches."

### Problem B: Database Scalability (8192x8192 Sectors)
*   **Risk:** If 10,000 systems are "active" in a multiplayer or high-speed travel scenario, querying an external DB for 50,000 Factions and 100,000 NPCs will create a massive I/O bottleneck.
*   **Solution: Agent Hibernation & LOD (Level of Detail).** 
    - **LOD 0 (Active):** Full LLM Sentience (Ships within 50km of player).
    - **LOD 1 (Dormant):** State-machine only (Ships in same StarSystem).
    - **LOD 2 (Hibernated):** Compressed JSON in DB (Systems the player hasn't visited in 24 hours).

## 2. Gameplay Friction

### Problem C: "The Wall of Hails" (Information Overload)
*   **Risk:** With 5 factions and a System Authority, the player might be bombarded with constant natural language messages, making the HUD unreadable.
*   **Solution: Priority Interrupt System.** The `ADK Bridge` must implement a "Traffic Controller" for the AI itself. Only one "High Priority" hail (e.g., SA Emergency) can occupy the main HUD. Others are moved to a "Comms Log" for the player to read at leisure.

### Problem D: Soft-Locking via AI Governance
*   **Risk:** An LLM SA might procedurally decide to `revoke_docking_rights` for a player based on a hallucination or an extreme interpretation of a minor collision, effectively soft-locking the player's progress.
*   **Solution: Administrative Overrides.** Factions must have "Backdoors." If an SA locks you out, a "Black Market" NPC must be procedurally guaranteed to offer a "Hack the SA" mission to restore rights.

## 3. Improved Architecture: The "Heartbeat" Bridge

We will replace the "Direct Call" model with a **Shared Blackboard Architecture**:

1.  **C++ Engine (GalaxyLib):** Writes "Perception Events" to a fast in-memory cache (Redis).
2.  **ADK Middleware:** Polls Redis, aggregates events into a narrative summary, and sends to the LLM.
3.  **LLM Agent:** Writes "Intentions" back to the Blackboard (e.g., "I want to harass this player").
4.  **C++ Engine:** Reads the "Harass" intention and adjusts the deterministic `STC` parameters accordingly.
