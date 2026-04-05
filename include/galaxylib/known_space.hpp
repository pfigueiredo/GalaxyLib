#pragma once

#include "galaxylib/star_system.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace galaxy {

class Sector;

namespace known_space {

/// One catalogue star (`KnownSpace.KnownSpaceStar`) after running the C# placement math.
struct CatalogEntry {
    std::uint32_t sector_x;
    std::uint32_t sector_y;
    int sys_num;
    const char* type_code;
    const char* name;
    int sx;
    int sy;
    int sz;
};

extern const CatalogEntry k_catalog[];
extern const std::size_t k_catalog_size;

/// If this sector appears in the catalogue, appends `StarSystem` instances and returns true
/// (same as `BuildKnownSpace` + known ctor in C#). Otherwise leaves `out` unchanged and returns false.
bool try_fill_known_systems(const Sector& sector, std::vector<StarSystem>& out);

}  // namespace known_space

}  // namespace galaxy
