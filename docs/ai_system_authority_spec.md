# Specification: LLM System Authority (SA)

## 1. Concept
Every star system in GalaxyLib is governed by a unique **System Authority (SA)**. The SA is a local LLM agent instance that manages the star system's "Local Policy," economic status, and security posture.

## 2. Personality & Ethos
Upon a player's first entry into a system, the SA is instantiated with a personality derived from the `StarSystem` spectral type and population data.
- **High-Sec (Sol):** Bureaucratic, polite, strict adherence to STC flight plans.
- **Lawless (Outer Rim):** Cryptic, aggressive, or purely transactional.
- **Scientific (Nebulae):** Curiosity-driven, granting access based on data sharing.

## 3. Gameplay Influence (The "Tool Box")
The SA has access to the following ADK tools:
- `adjust_stc_safety_margin(float meters)`: Increases/decreases the 1000m buffer based on trust.
- `issue_custom_stc_order(string text, float value)`: Sends direct messages to players via the STC HUD.
- `blacklist_ship(string ship_id)`: Orders local NPCs to intercept or denies docking at `SystemBody` ports.
- `set_market_tax(float percent)`: Influences the `Resource` costs in the system.

## 4. Decision Engine
The SA monitors the `STC` logs. If a ship repeatedly triggers `ROUTE DEVIATION`, the SA's LLM prompt is updated with "Agitation" tokens, eventually leading to a security intercept order.
