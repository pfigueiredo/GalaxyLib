#include "galaxylib/system_body.hpp"

#include "galaxylib/galaxy_utils.hpp"
#include "galaxylib/star_system.hpp"
#include "galaxylib/galaxy_curves.hpp"
#include "space_port_tables.hpp"

#include <cmath>
#include <cstdint>
#include <random>
#include <vector>
#include <algorithm>

namespace galaxy {
namespace {

/// AU in metres (matches C# `SystemLocation` scaling).
constexpr double k_au_m = 149598000.0;
/// Default elapsed days (2012-01-01 → 2024-01-01) used when constructing bodies, matching old fixed C# behavior.
constexpr double k_default_elapsed_days_since_reference = 4383.0;

// `PlanetBaseDistance` / `PlanetBaseTemperature` in `SystemBody.cs`.
constexpr float k_planet_base_distance[11][2] = {
    {0.3F, 8.F},     // Asteroid
    {15.F, 60.F},    // RockyPlanetoid
    {1.5F, 6.F},     // RockyWorld
    {0.5F, 0.9F},    // Venuzian
    {0.01F, 0.7F},   // Inferno
    {1.8F, 5.F},     // IceWorld
    {0.5F, 1.4F},    // WaterWorld
    {0.6F, 1.5F},    // Terrestrial
    {9.F, 200.F},    // SubGasGiant
    {8.F, 20.F},     // RingedGasGiant
    {4.F, 80.F},     // GasGiant
};

constexpr int k_planet_base_temperature[11][2] = {
    {400, -800},   // Asteroid
    {100, -500},   // RockyPlanetoid
    {-50, -400},   // RockyWorld
    {450, 350},    // Venuzian
    {350, 100},    // Inferno
    {-50, -100},   // IceWorld
    {50, 15},      // WaterWorld
    {50, -5},      // Terrestrial
    {-350, -250},  // SubGasGiant
    {-150, -190},  // RingedGasGiant
    {-100, -150},  // GasGiant
};

}  // namespace

SystemBody::SystemBody(const StarSystem& star_system, int system_body_id) 
    : system_body_id_(system_body_id), star_system_(star_system) {
    name_ = star_system.name();
    name_ += ' ';
    name_ += static_cast<char>(static_cast<int>('b') + system_body_id);

    generate_body(star_system);
    calculate_habitability(star_system);

    if (!star_system.is_generated() && star_system.name() == "Sol") {
        generate_sol_bodies(system_body_id);
    } else if (star_system.is_colonized()) {
        const std::uint32_t pop_seed = static_cast<std::uint32_t>(star_system.system_id() ^ system_body_id_);
        std::mt19937 pop_gen(pop_seed);
        std::uniform_real_distribution<float> pop_dist(0.8f, 1.2f);

        // 1. Base Scale from Distance (Bubble vs Frontier)
        const Coords sol_coords(4111, 6303, 0, 0, 0);
        const float dist_ly = static_cast<float>(star_system.coordinates().distance_to(sol_coords));
        
        // Use smooth Exponential Decay for population density
        // Core: High millions/billions. Dropoff rate of 120 LY.
        float scale_millions = GalaxyCurves::exponential_decay(dist_ly, 6000.0f, 120.0f);
        // Ensure even in deep space there's a baseline for colonized systems
        if (scale_millions < 1.0f) scale_millions = 1.0f;

        // 2. Economy Multiplier
        float economy_mult = 1.0f;
        switch (star_system.economy_type()) {
            case SystemEconomyType::HighTech:   economy_mult = 1.5f; break;
            case SystemEconomyType::Industrial: economy_mult = 1.3f; break;
            case SystemEconomyType::Consumer:   economy_mult = 1.2f; break;
            case SystemEconomyType::Extraction:
            case SystemEconomyType::Mining:     economy_mult = 0.5f; break; // Sparsely populated
            default: break;
        }

        population_millions_ = habitability_score_ * scale_millions * economy_mult * pop_dist(pop_gen);
        
        // Small chance for mining outposts on non-habitable bodies.
        if (population_millions_ < 0.1f && (pop_seed % 100) < 15) {
            std::uniform_real_distribution<float> outpost_dist(0.001f, 0.05f);
            population_millions_ = outpost_dist(pop_gen);
        }
    }
}

void SystemBody::calculate_habitability(const StarSystem& star) {
    habitability_score_ = 0.0f;

    // Body type factor.
    switch (body_type_) {
        case SystemBodyType::Terrestrial:   habitability_score_ = 1.0f; break;
        case SystemBodyType::WaterWorld:    habitability_score_ = 0.8f; break;
        case SystemBodyType::RockyWorld:    habitability_score_ = 0.3f; break;
        case SystemBodyType::IceWorld:      habitability_score_ = 0.1f; break;
        case SystemBodyType::RockyPlanetoid: habitability_score_ = 0.05f; break;
        case SystemBodyType::Asteroid:      habitability_score_ = 0.01f; break;
        default: habitability_score_ = 0.0f; break; // Gas giants have no surface population.
    }

    // Temperature factor (ideal around 20C).
    const float temp_diff = std::abs(static_cast<float>(temperature_c_) - 20.0f);
    float temp_factor = 1.0f - (temp_diff / 100.0f);
    if (temp_factor < 0.0f) temp_factor = 0.0f;
    
    habitability_score_ *= temp_factor;

    // Radius factor (ideal around Earth size = 1.0).
    const float rad_diff = std::abs(static_cast<float>(radius_) - 1.0f);
    float rad_factor = 1.0f - (rad_diff / 2.0f);
    if (rad_factor < 0.1f) rad_factor = 0.1f;

    habitability_score_ *= rad_factor;
}

void SystemBody::generate_body(const StarSystem& star) {
    const std::uint64_t sid = star.system_id();
    const int bid = system_body_id_ & 0xFF;

    std::uint64_t param1 = (sid << 16) | (sid >> (12 + bid));
    std::uint64_t param2 =
        ((static_cast<std::uint64_t>(system_body_id_) << 16) | (sid >> 12)) + (sid & 0xFFULL);

    rotate_system_params(param1, param2);

    const std::uint32_t seed_u = static_cast<std::uint32_t>(param1 + param2);
    std::mt19937 gen(seed_u);
    std::uniform_real_distribution<double> uni01(0.0, 1.0);

    const float hz = star.habitable_zone_au();
    float min_dist = hz * 0.2F;
    float max_dist = hz * 60.0F;

    if (max_dist > 5000.F) {
        max_dist = 5000.F;
    }
    if (min_dist > 600.F) {
        min_dist = 600.F;
    }

    const int total_bodies = star.num_bodies();
    const double p_num = (static_cast<double>(system_body_id_) + 1.0) * 0.9;
    const double denom = std::pow(2.0, static_cast<double>(total_bodies - 1));
    const double p_min =
        (std::pow(2.0, p_num - 1.0) * static_cast<double>(max_dist - min_dist) / denom) +
        static_cast<double>(min_dist);
    const double p_max =
        (std::pow(2.0, p_num) * static_cast<double>(max_dist - min_dist) / denom) +
        static_cast<double>(min_dist);

    const double spread_hint = p_max - p_min;
    const double base_distance = uni01(gen) * (spread_hint / 2.0) + p_min;

    std::vector<SystemBodyType> eligible;
    eligible.reserve(8);
    for (int i = 0; i < 11; ++i) {
        const double mind = static_cast<double>(k_planet_base_distance[i][0]) * static_cast<double>(hz);
        const double maxd = static_cast<double>(k_planet_base_distance[i][1]) * static_cast<double>(hz);
        if (base_distance >= mind && base_distance <= maxd) {
            eligible.push_back(static_cast<SystemBodyType>(i));
        }
    }

    if (eligible.empty()) {
        eligible.push_back(SystemBodyType::RockyPlanetoid);
        eligible.push_back(SystemBodyType::Asteroid);
    }

    std::uniform_int_distribution<std::size_t> pick_type(0, eligible.size() - 1);
    body_type_ = eligible[pick_type(gen)];

    const int ti = static_cast<int>(body_type_);
    const int base_temp_a = k_planet_base_temperature[ti][0];
    const int base_temp_b = k_planet_base_temperature[ti][1];
    const int base_temp_r = base_temp_a - base_temp_b;

    const double base_dist_a = static_cast<double>(k_planet_base_distance[ti][0]) * static_cast<double>(hz);
    const double base_dist_b = static_cast<double>(k_planet_base_distance[ti][1]) * static_cast<double>(hz);
    const double base_dist_r = base_dist_b - base_dist_a;

    if (base_dist_r > 0.0) {
        temperature_c_ = static_cast<int>(
            static_cast<double>(base_temp_a) -
            ((std::pow(base_distance, 0.2) - std::pow(base_dist_a, 0.2)) / std::pow(base_dist_r, 0.2) *
             static_cast<double>(base_temp_r)));
    } else {
        temperature_c_ = base_temp_a;
    }

    orbital_radius_au_ = static_cast<float>(base_distance);

    const double period_raw = std::pow(static_cast<double>(orbital_radius_au_), 1.5) * 1000.0;
    const double period_rounded = std::round(period_raw * 100000.0) / 100000.0;
    orbital_period_years_ = static_cast<float>(period_rounded / 1000.0);

    std::uniform_int_distribution<int> rad_dist(20, 999);
    radius_ = rad_dist(gen) / 100;

    std::uniform_int_distribution<int> ang_dist(0, 359);
    base_angle_deg_ = ang_dist(gen);

    generate_zones(star);

    update_system_location(k_default_elapsed_days_since_reference);
}

void SystemBody::generate_zones(const StarSystem& star) {
    zones_.clear();
    ring_zones_.clear();

    const std::uint32_t zone_seed = static_cast<std::uint32_t>(star.system_id() ^ system_body_id_ ^ 0x98765432u);
    std::mt19937 gen(zone_seed);
    std::uniform_real_distribution<float> lat_dist(-90.0f, 90.0f);
    std::uniform_real_distribution<float> lon_dist(-180.0f, 180.0f);
    std::uniform_real_distribution<float> inf_dist(0.3f, 1.0f);

    auto add_zone = [&](const std::string& name, BiomeType biome, float lat, float lon, float influence) {
        zones_.push_back({name, biome, lat, lon, influence});
    };

    auto get_procedural_name = [&](BiomeType biome, int index) {
        std::uniform_int_distribution<int> cat_dist(0, 4);
        int category = cat_dist(gen);
        const char** table = nullptr;
        int size = 0;
        switch (category) {
            case 0: table = tables::k_mathematicians; size = 20; break;
            case 1: table = tables::k_astronauts; size = 20; break;
            case 2: table = tables::k_actors; size = 20; break;
            case 3: table = tables::k_scientists; size = 20; break;
            case 4: table = tables::k_gods; size = 27; break;
        }
        std::uniform_int_distribution<int> name_dist(0, size - 1);
        std::string n = table[name_dist(gen)];
        n += " ";
        
        const char* suffix = "Region";
        switch (biome) {
            case BiomeType::Ocean: {
                const char* s[] = {"Sea", "Ocean", "Expanse", "Abyss"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Plains: {
                const char* s[] = {"Plains", "Fields", "Flatlands", "Plateau"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Mountains: {
                const char* s[] = {"Range", "Peaks", "Heights", "Massif"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Desert: {
                const char* s[] = {"Desert", "Wastes", "Barrens", "Dunes"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Rocky: {
                const char* s[] = {"Ridge", "Basin", "Crater", "Highlands"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Forest: {
                const char* s[] = {"Forest", "Wilds", "Woods", "Grove"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Tundra: {
                const char* s[] = {"Tundra", "Frostlands", "Permafrost", "Step"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Volcanic: {
                const char* s[] = {"Volcano", "Inferno", "Lava Lake", "Caldera"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Ice: {
                const char* s[] = {"Ice Cap", "Glacier", "Ice Sheet", "Frozen Waste"};
                suffix = s[index % 4];
                break;
            }
            case BiomeType::Gas: {
                const char* s[] = {"Cloud Layer", "Storm", "Vortex", "Band"};
                suffix = s[index % 4];
                break;
            }
            default: break;
        }
        n += suffix;
        return n;
    };

    // Gas Giants: 100% Gas
    if (body_type_ == SystemBodyType::GasGiant || body_type_ == SystemBodyType::SubGasGiant || body_type_ == SystemBodyType::RingedGasGiant) {
        add_zone(get_procedural_name(BiomeType::Gas, 0), BiomeType::Gas, 0.0f, 0.0f, 1.0f);
        
        // Add rings if applicable
        if (body_type_ == SystemBodyType::RingedGasGiant) {
            float planet_radius_km = 6371.0f * static_cast<float>(radius_);
            ring_zones_.push_back({BiomeType::RingMetalRich, planet_radius_km * 1.1f, planet_radius_km * 1.5f});
            ring_zones_.push_back({BiomeType::RingRock, planet_radius_km * 1.5f, planet_radius_km * 2.3f});
            ring_zones_.push_back({BiomeType::RingIce, planet_radius_km * 2.3f, planet_radius_km * 2.5f});
        }
        return;
    }

    // Determine biome probabilities based on temperature and sun proximity
    bool is_hot = (temperature_c_ > 100) || (orbital_radius_au_ < 0.7f);
    bool is_frozen = (temperature_c_ < -50);

    if (body_type_ == SystemBodyType::Inferno || body_type_ == SystemBodyType::Venuzian) {
        // Extreme heat
        add_zone(get_procedural_name(BiomeType::Volcanic, 0), BiomeType::Volcanic, 0.0f, lon_dist(gen), 1.0f);
        add_zone(get_procedural_name(BiomeType::Desert, 1), BiomeType::Desert, lat_dist(gen), lon_dist(gen), inf_dist(gen));
        add_zone(get_procedural_name(BiomeType::Rocky, 2), BiomeType::Rocky, lat_dist(gen), lon_dist(gen), inf_dist(gen));
    } else if (body_type_ == SystemBodyType::IceWorld || is_frozen) {
        // Extreme cold
        add_zone(get_procedural_name(BiomeType::Ice, 0), BiomeType::Ice, 0.0f, lon_dist(gen), 1.0f);
        add_zone(get_procedural_name(BiomeType::Tundra, 1), BiomeType::Tundra, lat_dist(gen), lon_dist(gen), inf_dist(gen));
        add_zone(get_procedural_name(BiomeType::Mountains, 2), BiomeType::Mountains, lat_dist(gen), lon_dist(gen), inf_dist(gen));
    } else if (body_type_ == SystemBodyType::WaterWorld) {
        add_zone(get_procedural_name(BiomeType::Ocean, 0), BiomeType::Ocean, 0.0f, 0.0f, 1.0f);
        add_zone(get_procedural_name(BiomeType::Plains, 1), BiomeType::Plains, lat_dist(gen), lon_dist(gen), 0.2f);
        add_zone(get_procedural_name(BiomeType::Rocky, 2), BiomeType::Rocky, lat_dist(gen), lon_dist(gen), 0.15f);
    } else if (body_type_ == SystemBodyType::Terrestrial) {
        // Balanced Earth-like
        add_zone("North Polar Cap", BiomeType::Ice, 90.0f, 0.0f, 0.4f);
        add_zone("South Polar Cap", BiomeType::Ice, -90.0f, 0.0f, 0.4f);
        add_zone(get_procedural_name(BiomeType::Ocean, 0), BiomeType::Ocean, lat_dist(gen), lon_dist(gen), 0.8f);
        add_zone(get_procedural_name(BiomeType::Plains, 1), BiomeType::Plains, lat_dist(gen), lon_dist(gen), 0.7f);
        add_zone(get_procedural_name(BiomeType::Mountains, 2), BiomeType::Mountains, lat_dist(gen), lon_dist(gen), 0.5f);
        add_zone(get_procedural_name(BiomeType::Forest, 3), BiomeType::Forest, lat_dist(gen), lon_dist(gen), 0.6f);
        
        if (is_hot) {
            add_zone(get_procedural_name(BiomeType::Desert, 4), BiomeType::Desert, 0.0f, lon_dist(gen), 0.6f);
        }
    } else {
        // Rocky World / Asteroid / Default
        add_zone(get_procedural_name(BiomeType::Rocky, 0), BiomeType::Rocky, lat_dist(gen), lon_dist(gen), 1.0f);
        add_zone(get_procedural_name(BiomeType::Desert, 1), BiomeType::Desert, lat_dist(gen), lon_dist(gen), 0.4f);
    }
}

BiomeType SystemBody::get_biome_at(float lat, float lon) const {
    if (zones_.empty()) return BiomeType::Rocky;

    auto degree_to_rad = [](float deg) { return deg * 0.0174532925f; };
    
    float lat1 = degree_to_rad(lat);
    float lon1 = degree_to_rad(lon);

    float max_influence = -1.0f;
    BiomeType best_biome = zones_[0].biome;

    for (const auto& zone : zones_) {
        float lat2 = degree_to_rad(zone.center_lat);
        float lon2 = degree_to_rad(zone.center_lon);

        // Great-circle distance (Haversine-ish simplified for influence)
        float dlat = lat2 - lat1;
        float dlon = lon2 - lon1;
        float a = std::sin(dlat / 2) * std::sin(dlat / 2) +
                  std::cos(lat1) * std::cos(lat2) * std::sin(dlon / 2) * std::sin(dlon / 2);
        float c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        
        // Influence decays with distance. c is 0 at center, pi at antipode.
        float influence = zone.influence * (1.0f / (1.0f + c * 2.0f));
        
        if (influence > max_influence) {
            max_influence = influence;
            best_biome = zone.biome;
        }
    }

    return best_biome;
}

BiomeType SystemBody::get_ring_biome_at(float distance_km) const {
    for (const auto& r : ring_zones_) {
        if (distance_km >= r.inner_radius_km && distance_km <= r.outer_radius_km) {
            return r.biome;
        }
    }
    return BiomeType::Gas; // Fallback for gas giant rings
}

const std::vector<ResourceDefinition>& SystemBody::get_resource_definitions() {
    return resource::get_definitions();
}

std::vector<ResourceDeposit> SystemBody::get_resources_at(float lat, float lon) const {
    const BiomeType biome = get_biome_at(lat, lon);
    const std::uint32_t loc_seed = static_cast<std::uint32_t>(
        system_body_id_ ^ 
        (static_cast<int>(lat * 100) << 16) ^ 
        static_cast<int>(lon * 100) ^ 
        0x55AA55AAu);
    
    // Final resource availability multiplier
    float multiplier = 1.0f;

    // 1. Distance factor: Boost availability further from Sol using Growth Curve
    // Min richness 0.2 at Sol, Max 2.5 in deep space. Growth rate of 300 LY.
    const Coords sol_coords(4111, 6303, 0, 0, 0);
    const float dist_ly = static_cast<float>(star_system_.coordinates().distance_to(sol_coords));
    multiplier = GalaxyCurves::inverse_exponential(dist_ly, 0.2f, 2.5f, 300.0f);

    // 2. Economy factor: Depletion in hubs, richness in extraction zones
    switch (star_system_.economy_type()) {
        case SystemEconomyType::HighTech:
        case SystemEconomyType::Industrial:
            multiplier *= 0.2f; // 80% depleted
            break;
        case SystemEconomyType::Extraction:
        case SystemEconomyType::Mining:
            multiplier *= 1.5f; // Rich deposits
            break;
        default:
            break;
    }
    
    return resource::generate_surface_deposits(biome, loc_seed, body_type_, multiplier);
}

std::vector<ResourceDeposit> SystemBody::get_ring_resources_at(float distance_km) const {
    const BiomeType biome = get_ring_biome_at(distance_km);
    const std::uint32_t dist_seed = static_cast<std::uint32_t>(
        system_body_id_ ^ 
        (static_cast<int>(distance_km)) ^ 
        0xAA55AA55u);

    // Same multiplier logic for rings
    const Coords sol_coords(4111, 6303, 0, 0, 0);
    const float dist_ly = static_cast<float>(star_system_.coordinates().distance_to(sol_coords));
    float multiplier = GalaxyCurves::inverse_exponential(dist_ly, 0.2f, 2.5f, 300.0f);

    switch (star_system_.economy_type()) {
        case SystemEconomyType::HighTech:
        case SystemEconomyType::Industrial:
            multiplier *= 0.2f;
            break;
        case SystemEconomyType::Extraction:
        case SystemEconomyType::Mining:
            multiplier *= 1.5f;
            break;
        default:
            break;
    }

    return resource::generate_ring_deposits(biome, dist_seed, multiplier);
}


void SystemBody::generate_sol_bodies(int id) {
    zones_.clear();
    auto add_zone = [&](const std::string& name, BiomeType biome, float lat, float lon, float influence) {
        zones_.push_back({name, biome, lat, lon, influence});
    };

    switch (id) {
        case 0:
            name_ = "Mercury";
            body_type_ = SystemBodyType::Inferno;
            orbital_radius_au_ = 0.41F;
            orbital_period_years_ = 0.24F;
            temperature_c_ = 396;
            radius_ = 0; // Procedural index will be around 0.38
            population_millions_ = 0.01f; // Tiny outpost
            add_zone("Caloris Basin", BiomeType::Volcanic, 31.0f, 189.0f, 1.0f);
            add_zone("Northern Wastes", BiomeType::Desert, 70.0f, 0.0f, 0.8f);
            break;
        case 1:
            name_ = "Venus";
            body_type_ = SystemBodyType::Venuzian;
            orbital_radius_au_ = 0.72F;
            orbital_period_years_ = 0.65F;
            temperature_c_ = 480;
            radius_ = 1; // Approx 1.0
            population_millions_ = 0.0f;
            add_zone("Aphrodite Terra", BiomeType::Volcanic, -10.0f, 100.0f, 1.0f);
            add_zone("Ishtar Terra", BiomeType::Rocky, 70.0f, 30.0f, 0.9f);
            break;
        case 2:
            name_ = "Earth";
            body_type_ = SystemBodyType::Terrestrial;
            orbital_radius_au_ = 1.F;
            orbital_period_years_ = 1.F;
            temperature_c_ = 22;
            radius_ = 1; // Exactly 1.0
            population_millions_ = 8000.0f;
            add_zone("Pacific Ocean", BiomeType::Ocean, 0.0f, -160.0f, 1.0f);
            add_zone("Atlantic Ocean", BiomeType::Ocean, 0.0f, -30.0f, 0.8f);
            add_zone("Eurasian Plains", BiomeType::Plains, 45.0f, 90.0f, 0.9f);
            add_zone("Sahara Desert", BiomeType::Desert, 23.0f, 12.0f, 0.7f);
            add_zone("Himalayas", BiomeType::Mountains, 28.0f, 86.0f, 0.5f);
            add_zone("Amazon Rainforest", BiomeType::Forest, -3.0f, -60.0f, 0.6f);
            add_zone("Antarctica", BiomeType::Ice, -90.0f, 0.0f, 0.8f);
            add_zone("Arctic", BiomeType::Ice, 90.0f, 0.0f, 0.8f);
            break;
        case 3:
            name_ = "Mars";
            body_type_ = SystemBodyType::RockyWorld;
            orbital_radius_au_ = 1.45F;
            orbital_period_years_ = 1.88F;
            temperature_c_ = -25;
            radius_ = 0; // Approx 0.53
            population_millions_ = 1.2f;
            add_zone("Olympus Mons", BiomeType::Mountains, 18.0f, -133.0f, 0.6f);
            add_zone("Valles Marineris", BiomeType::Rocky, -13.0f, -59.0f, 0.8f);
            add_zone("Planum Boreum", BiomeType::Ice, 90.0f, 0.0f, 0.5f);
            add_zone("Utopia Planitia", BiomeType::Plains, 46.0f, 119.0f, 0.9f);
            break;
        case 4:
            name_ = "Jupiter";
            body_type_ = SystemBodyType::GasGiant;
            orbital_radius_au_ = 5.42F;
            orbital_period_years_ = 11.8F;
            temperature_c_ = -161;
            radius_ = 11; // Approx 11.2
            population_millions_ = 0.5f; // Scientific outposts in orbit
            add_zone("Great Red Spot", BiomeType::Gas, -22.0f, 0.0f, 1.0f);
            break;
        case 5:
            name_ = "Saturn";
            body_type_ = SystemBodyType::RingedGasGiant;
            orbital_radius_au_ = 10.11F;
            orbital_period_years_ = 29.45F;
            temperature_c_ = -190;
            radius_ = 9; // Approx 9.45
            population_millions_ = 0.2f; // Mining stations in rings
            add_zone("Cloud Bands", BiomeType::Gas, 0.0f, 0.0f, 1.0f);
            {
                float planet_radius_km = 6371.0f * static_cast<float>(radius_);
                ring_zones_.push_back({BiomeType::RingRock, planet_radius_km * 1.1f, planet_radius_km * 1.5f});
                ring_zones_.push_back({BiomeType::RingIce, planet_radius_km * 1.5f, planet_radius_km * 2.3f});
                ring_zones_.push_back({BiomeType::RingMetalRich, planet_radius_km * 2.3f, planet_radius_km * 2.5f});
            }
            break;
        case 6:
            name_ = "Uranus";
            body_type_ = SystemBodyType::SubGasGiant;
            orbital_radius_au_ = 20.08F;
            orbital_period_years_ = 84.32F;
            temperature_c_ = -224;
            radius_ = 4; // Approx 4.0
            population_millions_ = 0.01f;
            add_zone("Upper Atmosphere", BiomeType::Gas, 0.0f, 0.0f, 1.0f);
            break;
        case 7:
            name_ = "Neptune";
            body_type_ = SystemBodyType::SubGasGiant;
            orbital_radius_au_ = 30.44F;
            orbital_period_years_ = 164.79F;
            temperature_c_ = -218;
            radius_ = 4; // Approx 3.88
            population_millions_ = 0.01f;
            add_zone("Stormy Skies", BiomeType::Gas, 0.0f, 0.0f, 1.0f);
            break;
        case 8:
            name_ = "Pluto";
            body_type_ = SystemBodyType::RockyPlanetoid;
            orbital_radius_au_ = 33.45F;
            orbital_period_years_ = 248.09F;
            temperature_c_ = -260;
            radius_ = 0; // Approx 0.18
            population_millions_ = 0.005f;
            add_zone("Tombaugh Regio", BiomeType::Ice, 0.0f, 180.0f, 0.8f);
            break;
        default:
            break;
    }
    update_system_location(k_default_elapsed_days_since_reference);
}

void SystemBody::update_system_location(double elapsed_days_since_reference) {
    double angle = static_cast<double>(base_angle_deg_);
    const double radius_au = static_cast<double>(orbital_radius_au_);
    const double period = static_cast<double>(orbital_period_years_);

    angle += (elapsed_days_since_reference / 365.25) * period * 360.0;

    constexpr double rad = 0.0174532925;
    const double r_m = radius_au * k_au_m;
    system_x_m_ = r_m * std::cos(angle * rad);
    system_y_m_ = r_m * std::sin(angle * rad);
    system_z_m_ = 0.0;

    // Update stations if ports are cached.
    if (ports_cached_) {
        const float r_km = 6371.0f * static_cast<float>(radius_);
        for (auto& port : ports_) {
            if (port.type == SpacePortType::OrbitalStation && port.orbital_period_hours <= 0.0f) {
                // Fail-safe for manual/Sol stations: calculate period if missing
                float r_orbit = r_km + port.altitude_km;
                port.orbital_period_hours = 1.5f * std::pow(r_orbit / (6371.0f + 400.0f), 1.5f);
            }
            port.update(elapsed_days_since_reference, r_km);
        }
    }
}

const std::vector<SpacePort>& SystemBody::ports() const {
    if (!ports_cached_) {
        generate_ports();
    }
    return ports_;
}

void SystemBody::generate_ports() const {
    ports_.clear();
    ports_cached_ = true;

    if (population_millions_ <= 0.0f) {
        return;
    }

    // Determine number of ports based on population.
    // 1 station for small outposts, multiple for billions.
    int num_stations = 1;
    if (population_millions_ > 10.0f) num_stations = 2;
    if (population_millions_ > 100.0f) num_stations = 3;
    if (population_millions_ > 1000.0f) num_stations = 5;

    int num_surface = 0;
    // Only solid surfaces get surface ports.
    if (body_type_ != SystemBodyType::GasGiant && 
        body_type_ != SystemBodyType::SubGasGiant && 
        body_type_ != SystemBodyType::RingedGasGiant) {
        if (population_millions_ > 1.0f) num_surface = 1;
        if (population_millions_ > 50.0f) num_surface = 2;
        if (population_millions_ > 500.0f) num_surface = 3;
        if (population_millions_ > 2000.0f) num_surface = 5;
    }

    std::uint32_t port_seed = static_cast<std::uint32_t>(system_body_id_ ^ 0x12345678u);
    std::mt19937 gen(port_seed);
    
    auto get_themed_name = [&](SpacePortType type, int index) {
        std::uniform_int_distribution<int> cat_dist(0, 4);
        int category = cat_dist(gen);
        const char** table = nullptr;
        int size = 0;
        switch (category) {
            case 0: table = tables::k_mathematicians; size = 20; break;
            case 1: table = tables::k_astronauts; size = 20; break;
            case 2: table = tables::k_actors; size = 20; break;
            case 3: table = tables::k_scientists; size = 20; break;
            case 4: table = tables::k_gods; size = 27; break;
        }
        std::uniform_int_distribution<int> name_dist(0, size - 1);
        std::uniform_int_distribution<int> suffix_dist(0, 9);
        
        std::string n = table[name_dist(gen)];
        n += " ";
        n += tables::k_port_suffixes[suffix_dist(gen)];
        return n;
    };

    // Generate Orbital Stations
    for (int i = 0; i < num_stations; ++i) {
        SpacePort p;
        p.id = "S" + std::to_string(i);
        p.name = get_themed_name(SpacePortType::OrbitalStation, i);
        p.type = SpacePortType::OrbitalStation;
        
        std::uniform_real_distribution<float> alt_dist(300.0f, 5000.0f);
        std::uniform_real_distribution<float> inc_dist(0.0f, 180.0f);
        std::uniform_real_distribution<float> phase_dist(0.0f, 360.0f);
        
        p.altitude_km = alt_dist(gen);
        p.inclination_deg = inc_dist(gen);
        p.phase_deg = phase_dist(gen);

        // Approximate orbital period using Kepler's Third Law: T = 2pi * sqrt(r^3 / GM)
        // Assume GM proportional to radius_ (Earth radius as reference).
        // For Earth: altitude 400km -> ~92 mins (1.5 hours).
        float earth_radius_km = 6371.0f;
        float r_km = earth_radius_km * static_cast<float>(radius_) + p.altitude_km;
        // Simplified period model: period_hours = 1.5 * (r / (earth_radius + 400))^1.5
        p.orbital_period_hours = 1.5f * std::pow(r_km / (earth_radius_km + 400.0f), 1.5f);
        
        ports_.push_back(p);
    }

    // Generate Surface Ports
    for (int i = 0; i < num_surface; ++i) {
        SpacePort p;
        p.id = "P" + std::to_string(i);
        p.name = get_themed_name(SpacePortType::SurfacePort, i);
        p.type = SpacePortType::SurfacePort;
        
        std::uniform_real_distribution<float> lat_dist(-90.0f, 90.0f);
        std::uniform_real_distribution<float> lon_dist(-180.0f, 180.0f);
        
        float lat = lat_dist(gen);
        float lon = lon_dist(gen);

        // Biome validation and nudging
        auto is_suitable = [](BiomeType b) {
            return b == BiomeType::Plains || b == BiomeType::Desert || 
                   b == BiomeType::Rocky || b == BiomeType::Tundra || 
                   b == BiomeType::Forest;
        };

        if (!is_suitable(get_biome_at(lat, lon))) {
            // Try 10 random nearby spots
            bool found = false;
            for (int attempt = 0; attempt < 10; ++attempt) {
                std::uniform_real_distribution<float> nudge(-10.0f, 10.0f);
                float n_lat = std::clamp(lat + nudge(gen), -90.0f, 90.0f);
                float n_lon = lon + nudge(gen);
                if (n_lon > 180.0f) n_lon -= 360.0f;
                if (n_lon < -180.0f) n_lon += 360.0f;

                if (is_suitable(get_biome_at(n_lat, n_lon))) {
                    lat = n_lat;
                    lon = n_lon;
                    found = true;
                    break;
                }
            }

            // If still not found, snap to a suitable zone center
            if (!found) {
                for (const auto& zone : zones_) {
                    if (is_suitable(zone.biome)) {
                        lat = zone.center_lat;
                        lon = zone.center_lon;
                        break;
                    }
                }
            }
        }
        
        p.latitude = lat;
        p.longitude = lon;
        
        ports_.push_back(p);
    }
}

void SystemBody::update_system_location(CalendarDate date) {
    update_system_location(days_since_orbit_reference(date));
}

std::string_view SystemBody::body_class_ext() const {
    switch (body_type_) {
        case SystemBodyType::Asteroid:
            return "Asteroid";
        case SystemBodyType::RockyPlanetoid:
            return "Rocky Planetoid";
        case SystemBodyType::RockyWorld:
            return "Rocky World";
        case SystemBodyType::Venuzian:
            return "Venuzian";
        case SystemBodyType::Inferno:
            return "Inferno (Mercury like)";
        case SystemBodyType::IceWorld:
            return "Ice world";
        case SystemBodyType::WaterWorld:
            return "Water World";
        case SystemBodyType::Terrestrial:
            return "Terrestrial";
        case SystemBodyType::SubGasGiant:
            return "Sub Gas Giant";
        case SystemBodyType::RingedGasGiant:
            return "Gas Giant (Saturn like)";
        case SystemBodyType::GasGiant:
            return "Gas Giant";
        default:
            return "Unknown";
    }
}

}  // namespace galaxy
