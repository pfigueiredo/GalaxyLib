# Design: Faction-Authority Dynamics

## 1. The "Biased" System Authority
The System Authority (SA) is the administrative LLM of the system. However, its "Prompt Context" is dictated by the Ruling Faction.
- **Corporate Rule:** The SA becomes hyper-strict about `ROUTE DEVIATION` but ignores `COLLISION ADVISORY` near corporate refineries.
- **Rebel Rule:** The SA might "accidentally" leak STC coordinates of corporate convoys to the player.

## 2. System States (Emergent Gameplay)
Factions trigger "System States" based on their interactions:
- **Civil War:** Two factions have high `Aggression` tokens. The STC enters "Combat Mode," issuing waypoints that avoid active battle zones.
- **Economic Boom:** Trade factions have >70% Influence. `MarketPrices` for luxuries skyrocket.
- **Lockdown:** The SA (under Syndicate influence) revokes docking rights for all non-allied ships.

## 3. Innovative Tool: `lobby_authority`
NPC Faction Leaders can call a special ADK tool to change system laws:
- `lobby_authority(rule_change, bribe_amount)`
- **Example:** The *Syndicate* bribes the SA to reduce the `safety_margin` to 200m, allowing "stealth" ships to hide in traffic noise.
