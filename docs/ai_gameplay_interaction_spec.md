# Specification: Gameplay Interaction & Social Mechanics

## 1. The Reputation Loop (Sentiment to Score)
Every interaction with an NPC or System Authority (SA) is processed via the LLM to extract **Sentiment Tokens**.
- **The ADK Bridge:** Converts LLM output into a `TrustValue` (-1.0 to 1.0) stored in the external database.
- **GalaxyLib Impact:** 
    - `High Trust:` The STC provides "Optimal" flight plans (shortcuts through high-density traffic).
    - `Low Trust:` The STC issues frequent `ROUTE DEVIATION` warnings for minor errors to "harass" the player legally.

## 2. Local Authority Hierarchies
Authorities are not monolithic; they have a chain of command:
1. **System Authority (SA):** The "God" of the system. Controls the global STC logic and system-wide laws.
2. **Station Overlord (SO):** A sub-agent governing a specific `SpacePort`. Manages docking priorities and landing fees.
3. **Sector Patrol (SP):** Autonomous NPCs that enforce the SA's current "Mood."

## 3. Innovative Mechanics: "STC Social Engineering"
The player can use the ADK Hail tool to negotiate with the STC/SA:
- **The "Bribe" Prompt:** "My engines are failing, I need priority landing." 
- **AI Check:** The SA calls `ship.get_condition()` from GalaxyLib. If the ship is actually damaged, the SA might grant a `CloseFinal` priority. If the player is lying, the SA adds a "Deceptive" trait to the player's persistent DB entry.

## 4. NPC Archetypes & Dynamic Interactions
| NPC Type | Goal (LLM Directive) | Interaction Verb |
| :--- | :--- | :--- |
| **The Grumpy Miner** | Protect their "Claim" (a SystemBody). | Will hail you to stay away from a specific coordinate. |
| **The Rogue Trader** | Bypass System Authority taxes. | Offers "Illegal" STC waypoints that avoid Patrol paths. |
| **The Fanatic Monk** | Preach the "Silence of the Void." | Asks you to cut engines (`vel = 0`) for a blessing. |
| **The Bounty Hunter** | Track low-reputation players. | Uses STC data leaks to find your next waypoint. |
