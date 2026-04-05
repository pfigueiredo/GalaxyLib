#pragma once

#include "galaxylib/star_system.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace galaxy {

class Sector;

/// Stable, game-facing entry points. Use this namespace from game code; full types stay in `galaxy::`.
/// New helpers and PODs should be added here so game code does not depend on every header.
namespace api {

/// Library version (semver-style); bump when you ship breaking or feature releases.
struct LibraryVersion {
    int major{};
    int minor{};
    int patch{};
};

constexpr LibraryVersion library_version() { return {0, 1, 0}; }

/// Initialization; more fields may be added in the future — default-construct and set paths you need.
struct InitConfig {
    /// Path to `Galaxy.bin` (RGB triplets, same as C# `Sector` loader).
    std::string galaxy_bin_path;
};

/// Load galaxy density data. Call once at startup before creating `Sector` objects.
[[nodiscard]] bool initialize(const InitConfig& config);

/// True after a successful `initialize` (galaxy map non-empty).
[[nodiscard]] bool galaxy_ready();

// --- Lightweight views (no `SystemBody` construction) ----------------------------

/// Snapshot for regional maps and lists — built from `StarSystem` without calling `bodies()`.
struct SystemSummary {
    int sys_num{};
    std::uint64_t system_id{};
    std::string name;
    std::string star_type;
    bool is_generated{};
    int num_bodies{};
    bool is_colonized{};
    float population_billions{};
    std::uint32_t sector_index_x{};
    std::uint32_t sector_index_y{};
    int local_x{};
    int local_y{};
    int local_z{};
};

/// Fill a summary from an existing star (cheap).
inline SystemSummary make_system_summary(const StarSystem& s) {
    SystemSummary out;
    out.sys_num = s.sys_num();
    out.system_id = s.system_id();
    out.name = s.name();
    out.star_type = s.star_type();
    out.is_generated = s.is_generated();
    out.num_bodies = s.num_bodies();
    out.is_colonized = s.is_colonized();
    out.population_billions = s.population_billions();
    out.sector_index_x = s.sector_index_x();
    out.sector_index_y = s.sector_index_y();
    out.local_x = s.local_x();
    out.local_y = s.local_y();
    out.local_z = s.local_z();
    return out;
}

/// All systems in the sector as summaries. Internally calls `Sector::systems()` (star generation only).
[[nodiscard]] std::vector<SystemSummary> list_system_summaries(const Sector& sector);

}  // namespace api

}  // namespace galaxy
