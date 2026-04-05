#pragma once

#include "galaxylib/biome.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace galaxy {

enum class SystemBodyType;

/// Resource categories for grouping and engine visualization.
enum class ResourceCategory {
    Gas,
    Liquid,
    Metal,
    Mineral,
    Exotic
};

/// Rarity tiers for resources.
enum class ResourceRarity {
    VeryCommon,
    Common,
    Standard,
    Rare,
    Exotic
};

/// Metadata for a specific natural resource.
struct ResourceDefinition {
    int id;
    std::string name;
    ResourceCategory category;
    ResourceRarity rarity;
    float base_value; // Market value multiplier
};

/// A specific concentration of a resource at a location.
struct ResourceDeposit {
    int resource_id;
    float concentration; // 0.0 to 1.0
};

namespace resource {

/// Return the master list of all available resources in the universe.
const std::vector<ResourceDefinition>& get_definitions();

/// Procedurally generate surface deposits based on biome, seed, and planet type.
std::vector<ResourceDeposit> generate_surface_deposits(BiomeType biome, std::uint32_t seed, SystemBodyType body_type, float availability_multiplier = 1.0f);

/// Procedurally generate ring deposits based on ring biome and seed.
std::vector<ResourceDeposit> generate_ring_deposits(BiomeType biome, std::uint32_t seed, float availability_multiplier = 1.0f);

} // namespace resource
} // namespace galaxy
