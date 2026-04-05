#include "galaxylib/resource.hpp"
#include "galaxylib/system_body.hpp"

#include <random>

namespace galaxy {
namespace resource {

const std::vector<ResourceDefinition>& get_definitions() {
    static const std::vector<ResourceDefinition> k_resources = {
        // ID, Name, Category, Rarity, Value
        // Gases
        {0, "Hydrogen", ResourceCategory::Gas, ResourceRarity::VeryCommon, 1.0f},
        {1, "Helium-3", ResourceCategory::Gas, ResourceRarity::Common, 5.0f},
        {2, "Oxygen", ResourceCategory::Gas, ResourceRarity::VeryCommon, 1.2f},
        {3, "Xenon", ResourceCategory::Gas, ResourceRarity::Rare, 15.0f},
        {4, "Methane", ResourceCategory::Gas, ResourceRarity::Common, 2.0f},
        // Liquids
        {10, "Water", ResourceCategory::Liquid, ResourceRarity::VeryCommon, 1.5f},
        {11, "Ammonia", ResourceCategory::Liquid, ResourceRarity::Common, 2.5f},
        {12, "Deuterium", ResourceCategory::Liquid, ResourceRarity::Standard, 10.0f},
        // Industrial Metals
        {20, "Iron", ResourceCategory::Metal, ResourceRarity::VeryCommon, 0.8f},
        {21, "Copper", ResourceCategory::Metal, ResourceRarity::VeryCommon, 1.0f},
        {22, "Aluminum", ResourceCategory::Metal, ResourceRarity::VeryCommon, 1.2f},
        {23, "Titanium", ResourceCategory::Metal, ResourceRarity::Common, 4.0f},
        {24, "Lead", ResourceCategory::Metal, ResourceRarity::Common, 0.9f},
        // Precious Metals
        {30, "Gold", ResourceCategory::Metal, ResourceRarity::Standard, 50.0f},
        {31, "Platinum", ResourceCategory::Metal, ResourceRarity::Rare, 150.0f},
        {32, "Silver", ResourceCategory::Metal, ResourceRarity::Standard, 25.0f},
        {33, "Iridium", ResourceCategory::Metal, ResourceRarity::Rare, 200.0f},
        {34, "Osmium", ResourceCategory::Metal, ResourceRarity::Rare, 180.0f},
        // Minerals
        {40, "Silicates", ResourceCategory::Mineral, ResourceRarity::VeryCommon, 0.5f},
        {41, "Sulfur", ResourceCategory::Mineral, ResourceRarity::Common, 1.8f},
        {42, "Monazite", ResourceCategory::Mineral, ResourceRarity::Rare, 80.0f},
        {43, "Void Opals", ResourceCategory::Mineral, ResourceRarity::Exotic, 500.0f},
        {44, "Painite", ResourceCategory::Mineral, ResourceRarity::Rare, 120.0f},
        // Exotics (Legacy)
        {50, "Octerium", ResourceCategory::Exotic, ResourceRarity::Exotic, 1000.0f},
        {51, "Criptium", ResourceCategory::Exotic, ResourceRarity::Exotic, 1200.0f},
        {52, "Diritium", ResourceCategory::Exotic, ResourceRarity::Exotic, 1500.0f},
        {53, "Hallum", ResourceCategory::Exotic, ResourceRarity::Exotic, 2000.0f}
    };
    return k_resources;
}

std::vector<ResourceDeposit> generate_surface_deposits(BiomeType biome, std::uint32_t seed, SystemBodyType body_type, float availability_multiplier) {
    std::vector<ResourceDeposit> deposits;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    auto add_if = [&](int id, float base_chance, float max_concentration) {
        if (dist01(gen) < base_chance) {
            float conc = dist01(gen) * max_concentration * availability_multiplier;
            if (conc > 1.0f) conc = 1.0f;
            if (conc > 0.001f) {
                deposits.push_back({id, conc});
            }
        }
    };

    switch (biome) {
        case BiomeType::Ocean:
            add_if(10, 1.0f, 1.0f);   // Water
            add_if(12, 0.2f, 0.1f);   // Deuterium
            add_if(0, 0.1f, 0.05f);   // Hydrogen
            break;
        case BiomeType::Plains:
            add_if(20, 0.8f, 0.5f);   // Iron
            add_if(21, 0.4f, 0.3f);   // Copper
            add_if(40, 1.0f, 0.8f);   // Silicates
            break;
        case BiomeType::Mountains:
            add_if(20, 0.6f, 0.4f);   // Iron
            add_if(23, 0.3f, 0.5f);   // Titanium
            add_if(30, 0.1f, 0.2f);   // Gold
            add_if(31, 0.05f, 0.1f);  // Platinum
            break;
        case BiomeType::Desert:
            add_if(40, 1.0f, 1.0f);   // Silicates
            add_if(30, 0.2f, 0.3f);   // Gold
            add_if(21, 0.1f, 0.2f);   // Copper
            break;
        case BiomeType::Rocky:
            add_if(20, 0.7f, 0.6f);   // Iron
            add_if(22, 0.5f, 0.4f);   // Aluminum
            add_if(40, 0.9f, 0.7f);   // Silicates
            add_if(33, 0.02f, 0.15f); // Iridium
            break;
        case BiomeType::Forest:
            add_if(10, 0.5f, 0.3f);   // Water
            add_if(40, 0.8f, 0.5f);   // Silicates
            add_if(2, 0.4f, 0.2f);    // Oxygen
            break;
        case BiomeType::Tundra:
            add_if(10, 0.8f, 0.9f);   // Water (Ice)
            add_if(4, 0.3f, 0.4f);    // Methane
            break;
        case BiomeType::Volcanic:
            add_if(41, 0.9f, 0.8f);   // Sulfur
            add_if(20, 0.7f, 0.6f);   // Iron
            add_if(23, 0.4f, 0.4f);   // Titanium
            add_if(33, 0.1f, 0.3f);   // Iridium
            add_if(50, 0.01f, 0.1f);  // Octerium
            break;
        case BiomeType::Ice:
            add_if(10, 1.0f, 1.0f);   // Water
            add_if(11, 0.4f, 0.5f);   // Ammonia
            add_if(4, 0.5f, 0.6f);    // Methane
            add_if(43, 0.05f, 0.2f);  // Void Opals
            break;
        case BiomeType::Gas:
            add_if(0, 1.0f, 1.0f);    // Hydrogen
            add_if(1, 0.7f, 0.8f);    // Helium-3
            add_if(4, 0.4f, 0.3f);    // Methane
            add_if(3, 0.1f, 0.2f);    // Xenon
            break;
        default:
            break;
    }

    // Exotic modifier for specific planet types
    if (body_type == SystemBodyType::Inferno || body_type == SystemBodyType::Venuzian) {
        add_if(51, 0.02f, 0.2f); // Criptium
    } else if (body_type == SystemBodyType::IceWorld) {
        add_if(52, 0.02f, 0.2f); // Diritium
    } else if (body_type == SystemBodyType::Asteroid) {
        add_if(53, 0.05f, 0.3f); // Hallum
    }

    return deposits;
}

std::vector<ResourceDeposit> generate_ring_deposits(BiomeType biome, std::uint32_t seed, float availability_multiplier) {
    std::vector<ResourceDeposit> deposits;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    auto add_if = [&](int id, float base_chance, float max_concentration) {
        if (dist01(gen) < base_chance) {
            float conc = dist01(gen) * max_concentration * availability_multiplier;
            if (conc > 1.0f) conc = 1.0f;
            if (conc > 0.001f) {
                deposits.push_back({id, conc});
            }
        }
    };

    switch (biome) {
        case BiomeType::RingMetalRich:
            add_if(31, 0.5f, 0.4f); // Platinum
            add_if(34, 0.4f, 0.3f); // Osmium
            add_if(20, 0.8f, 0.7f); // Iron
            add_if(44, 0.2f, 0.3f); // Painite
            break;
        case BiomeType::RingRock:
            add_if(40, 0.9f, 0.8f); // Silicates
            add_if(42, 0.3f, 0.4f); // Monazite
            add_if(20, 0.5f, 0.4f); // Iron
            break;
        case BiomeType::RingIce:
            add_if(10, 1.0f, 1.0f); // Water
            add_if(4, 0.6f, 0.5f);  // Methane
            add_if(43, 0.1f, 0.4f); // Void Opals
            break;
        default:
            break;
    }

    return deposits;
}

} // namespace resource
} // namespace galaxy
