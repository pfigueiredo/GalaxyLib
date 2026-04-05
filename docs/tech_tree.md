# Tech Tree & Production Chains

This document defines the production hierarchies for the galaxy's economy. Installations (Orbital and Surface) use these formulas to transform raw resources into high-value products.

---

## Tier 0: Raw Resources (Extraction)
*Extracted directly from planet biomes and rings.*

| Category | Resources |
| :--- | :--- |
| **Metals** | Iron, Copper, Aluminum, Titanium, Gold, Silver, Platinum, Lead |
| **Minerals** | Silicates, Sulfur, Monazite, Void Opals, Painite |
| **Gases** | Hydrogen, Helium-3, Oxygen, Xenon, Methane |
| **Liquids** | Water, Ammonia, Deuterium, Hydrocarbons |
| **Exotics** | Octerium, Criptium, Diritium, Hallum |
| **Biological** | Bio-Matter, Nutrients, Natural Fibers |

---

## Tier 1: Processed Materials & Basic Components
*Produced by: Smelters, Chemical Plants, Hydroponic Farms, Textile Mills.*

| Product | Inputs | Category |
| :--- | :--- | :--- |
| **Steel Plating** | 2x Iron + 1x Silicates | Metallurgy |
| **Conductive Wiring** | 2x Copper + 1x Gold | Electronics |
| **Plastic Polymers** | 2x Methane + 1x Water | Chemical |
| **Synthetic Fabrics** | 1x Hydrocarbons + 1x Natural Fibers | Textiles |
| **Optical Glass** | 2x Silicates | Industrial |
| **Basic Circuitry** | 1x Copper + 1x Gold + 1x Silicates | Electronics |
| **Fuel Rods** | 2x Hydrogen + 1x Aluminum | Energy |
| **Fertilizer** | 1x Ammonia + 1x Bio-Matter | Agriculture |
| **Propellants** | 1x Sulfur + 1x Methane | Chemical |
| **Bulk Nutrients** | 2x Nutrients + 1x Water | Food |

---

## Tier 2: Industrial & Bio Sub-Systems
*Produced by: Component Factories, Bio-Labs, Munitions Plants.*

| Product | Inputs | Category |
| :--- | :--- | :--- |
| **Microprocessors** | 1x Basic Circuitry + 1x Silicates + 0.1x Gold | Electronics |
| **Mechanical Actuators** | 1x Steel Plating + 1x Conductive Wiring | Robotics |
| **High-Res Sensors** | 1x Optical Glass + 1x Silver + 1x Monazite | Tech |
| **Power Converters** | 2x Conductive Wiring + 1x Iron + 1x Aluminum | Energy |
| **Reinforced Frames** | 3x Steel Plating + 1x Titanium | Heavy Industry |
| **Armor Plating** | 2x Steel Plating + 1x Titanium + 1x Lead | Military |
| **Munitions** | 1x Propellants + 1x Lead + 1x Steel | Military |
| **Processed Food** | 2x Bulk Nutrients + 1x Fertilizer | Food |
| **Medical Base** | 1x Bio-Matter + 1x Water + 1x Ammonia | Medical |

---

## Tier 3: Advanced Systems & Professional Goods
*Produced by: Advanced Manufacturing, Pharmaceutical Labs, Weapon Foundries.*

| Product | Inputs | Category |
| :--- | :--- | :--- |
| **Mainframe Computers** | 2x Microprocessors + 1x Plastic + 1x Optical Glass | Computing |
| **Robotic Units** | 1x Microprocessors + 2x Mechanical Actuators + 1x Reinforced Frame | Robotics |
| **Life Support Systems** | 1x Power Converter + 1x Reinforced Frame + 2x Oxygen | Aerospace |
| **Propulsion Thrusters** | 2x Reinforced Frame + 1x Mechanical Actuators + 2x Fuel Rods | Aerospace |
| **Targeting Computers** | 1x Mainframe Computer + 1x High-Res Sensors | Military |
| **Weapon Housings** | 2x Reinforced Frame + 1x Mechanical Actuators | Military |
| **Medical Stims** | 1x Medical Base + 1x Xenon + 0.5x Gold | Medical |
| **Luxury Rations** | 2x Processed Food + 1x Bio-Matter | Food |
| **Designer Apparel** | 2x Synthetic Fabrics + 1x Silver | Consumer |

---

## Tier 4: End Products & Ship Modules
*Produced by: Shipyards, High-Tech Hubs, Medical Centers.*

| Product | Inputs | Category |
| :--- | :--- | :--- |
| **Consumer Tech** | 1x Mainframe Computer + 2x Plastic + 1x Designer Apparel | Consumer |
| **Warp Core** | 1x Octerium + 2x Mainframe Computer + 1x Power Converter | FTL Tech |
| **Shield Generator** | 1x Criptium + 1x Painite + 2x High-Res Sensors | Defense |
| **Mining Laser** | 1x Diritium + 1x Optical Glass + 1x Power Converter | Industrial |
| **Auto-Medics** | 1x Robotic Unit + 1x High-Res Sensors + 1x Medical Stims | Medical |
| **Kinetic Battery** | 2x Weapon Housings + 1x Targeting Computer + 5x Munitions | Military |
| **Plasma Cannon** | 2x Weapon Housings + 1x Targeting Computer + 1x Deuterium | Military |
| **Cybernetic Implants** | 1x Robotic Unit + 1x Void Opals + 1x Medical Stims | Medical |
| **AI Core** | 1x Mainframe Computer + 1x Hallum + 1x High-Res Sensors | Computing |
| **Luxury Furniture** | 2x Plastic Polymers + 1x Synthetic Fabrics + 1x Platinum | Consumer |

---

## Production Logic
As per `economy_ideias.txt`, the price of a product is determined by:
`Product Price = (SUM(Resource Inputs * Local Market Value) * Efficiency Multiplier) + Profit Margin`

### Installation Types
1.  **Refineries & Farms (Surface):** Extract Raw -> Produce Tier 1.
2.  **Factories (Surface/Orbital):** Consume Tier 1 -> Produce Tier 2/3.
3.  **High-Tech Hubs (Orbital):** Consume Tier 3 -> Produce Tier 4.
4.  **Shipyards (Orbital):** Consumes Tier 3/4 -> Produces Ships.
5.  **Consumer Centers (Surface/Orbital):** High demand for Tier 4, Luxury Rations, and Consumer Tech.
