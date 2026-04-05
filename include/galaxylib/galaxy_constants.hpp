#pragma once

namespace galaxy {

/// Matches `GalaxyConstants` in `Coords.cs`.
struct GalaxyConstants {
    static constexpr double galaxy_width = 100000.0;          // Ly
    static constexpr double number_sectors = 8191.0;
    static constexpr double sector_width = galaxy_width / number_sectors;
    static constexpr double sector_height = sector_width;
    static constexpr double sector_depth = sector_width * 2.0;
    static constexpr double local_sector_width = 128.0;
    static constexpr double local_sector_height = 128.0;
    static constexpr double local_sector_depth = local_sector_width * 2.0;
};

}  // namespace galaxy
