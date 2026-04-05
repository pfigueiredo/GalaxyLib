#pragma once

#include "galaxylib/coords.hpp"
#include "galaxylib/star_system.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace galaxy {

/// Matches `GalaxyE.Galaxy.Sector` (partial port — system count, params, generated systems).
class Sector {
public:
    Sector();
    Sector(std::uint32_t index_x, std::uint32_t index_y);

    std::uint32_t index_x() const { return index_x_; }
    std::uint32_t index_y() const { return index_y_; }

    /// Number of procedurally generated systems for this sector (`Generate_NumSystems`).
    int num_systems() const { return num_systems_; }

    std::string to_string_base36() const;

    /// PRNG parameters for system `sys_num` (`Sector.GetSystemParams` in C#).
    std::array<std::uint64_t, 2> get_system_params(int sys_num) const;

    /// Lazily builds the same list as C# `Sector.Systems` / `Generate_Systems`.
    const std::vector<StarSystem>& systems() const;

    /// Sector origin in local coords `(0,0,0)` (`Coords(sectorX, sectorY)` in C#).
    Coords center_coords() const { return Coords(index_x_, index_y_); }

    bool contains(const Coords& c) const {
        return c.sector_index_x() == index_x_ && c.sector_index_y() == index_y_;
    }

private:
    int generate_num_systems(std::uint32_t coordx, std::uint32_t coordy, int galaxy_scale);
    void ensure_systems() const;

    std::uint32_t index_x_{};
    std::uint32_t index_y_{};
    int num_systems_{};

    mutable std::vector<StarSystem> systems_;
    mutable bool systems_cached_{false};
};

}  // namespace galaxy
