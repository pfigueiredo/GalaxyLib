# Specification: Engaging Gameplay Scenarios (AI & Engine Synergy)

This document provides detailed scenarios illustrating how the **GalaxyLib** physics, **STC** guidance, and **LLM Agent** intelligence create emergent gameplay loops.

---

## Scenario 1: The Bureaucratic Siege (Sol System)
**Key Systems:** System Authority (SA), STC Social Engineering, Deception.

### 1. The Setup
The player is carrying time-sensitive medical supplies to *Einstein Base* (Mercury). The Sol **System Authority (SA)** is procedurally seeded with the "Bureaucratic" and "Strict" traits.

### 2. The Interaction
As the player enters the 10km `Approach` phase, the STC issues a `DecelerateTo` order of 100 km/h (unusually slow).
- **SA Hail:** "Vanguard-01, traffic density is high. Maintain current spacing or face fines."
- **The Reality:** The player senses (via `sense_nearby_traffic`) that the "High Traffic" is actually just one luxury yacht belonging to a high-ranking official.

### 3. Player Options
- **Obey:** Arrive late, fail the mission, and lose trust with the mission giver.
- **Negotiate (ADK Hail):** "This is an emergency medical transport. I request priority docking."
- **AI Decision:** The SA LLM calls `check_ship_status`. If the player actually has medical cargo, it may grant a `CloseFinal` priority. If the player is bluffing, the SA calls `issue_emergency_order` to lock the player's engines.

---

## Scenario 2: The Trojan Relay (Inter-System Intrigue)
**Key Systems:** Dynamic Quest Weaver (DQW), Chaining, Unreliable Narrator.

### 1. The Setup
In a low-sec system, NPC-A ("The Deserter") hires the player to deliver "Agricultural Scanners" to NPC-B in a distant sector.

### 2. The Twist
Mid-flight, the player receives a `ROUTE DEVIATION` alert from the STC, even though they are on-vector. 
- **LLM Context:** A "Bounty Hunter" agent has hacked the STC to force the player to stop for "inspection."
- **The Scan:** If the player uses a ship tool to scan their own cargo, they find the "Scanners" are actually encrypted SA data logs.

### 3. The Chain
Upon arrival at NPC-B, the LLM reads the player's history.
- **NPC-B Interaction:** "I see the STC flagged you for deviation. Did you look inside the box, Pilot?"
- **Branching:** If the player lies and the LLM detects "Low Sentiment," NPC-B tool-calls `forge_mission` to hire a nearby "Assassin NPC" to tail the player after they leave the station.

---

## Scenario 3: The Ghost in the Density Map (Exploration)
**Key Systems:** Galaxy.bin, Procedural Lore, Inter-Agent Social Fabric.

### 1. The Setup
An NPC "System Scout" hails the player in a bar. He claims to have found a "Starship Graveyard" at specific coordinates not shown on the standard map.

### 2. The Simulation
The "Graveyard" is actually a procedural "noise spike" in the `Galaxy.bin` texture that the LLM has interpreted as a narrative event.
- **Action:** The player follows the STC coordinates provided by the Scout.
- **The Conflict:** A "Scavenger Cartel" (Group Agent) is already there. They share a **Joint Vector Memory**.

### 3. Emergent Loop
The Cartel doesn't just attack. The lead Scavenger hails: "The Scout who sold you these coordinates owes us credits. Pay his debt, and you can have the salvage."
- **Persistence:** If the player pays, the Cartel adds the player to their "Protected" list across all sectors they control.

---

## Scenario 4: The Vengeance Vector (Narrative Persistence)
**Key Systems:** External Database, Cross-System Impact, Vengeance Loop.

### 1. The Setup
Five hours ago, in the Centauri system, the player accidentally collided with a small miner ship while ignoring a `COLLISION ADVISORY`. The miner died.

### 2. The Consequence
The player is now in the Sol system, 4 light-years away. 
- **The Trigger:** The Sol SA fetches the "Galaxy Log" from the DB. 
- **SA Hail:** "Vanguard-01, your flight record shows a fatal negligence event in Centauri. Sol System policy requires a safety bond of 10,000 credits to proceed to Einstein Base."

### 3. The Climax
While the player is negotiating with the SA, the dead miner's "Brother" (an NPC agent) appears on the radar.
- **Agent Action:** He uses the `leak_intel` tool to broadcast the player's `Approach` vector to local pirates, creating a multi-ship ambush during the player's most vulnerable flight phase.

---

## 5. Summary of Gameplay Verbs
| Player Action | AI Response | Engine Impact |
| :--- | :--- | :--- |
| **Hailing Authority** | LLM Sentiment Analysis | STC priority levels change. |
| **Scanning Cargo** | Logic Discrepancy Detection | "Unreliable Narrator" reveal. |
| **Ignoring STC** | "Agitation" Token increase | Safety margins shrink; Patrols scramble. |
| **Fulfilling Needs** | Quest Weaver Chaining | New NPCs enter the player's social web. |
