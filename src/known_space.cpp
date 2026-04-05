#include "galaxylib/known_space.hpp"

#include "galaxylib/sector.hpp"
#include "galaxylib/star_system.hpp"

namespace galaxy {
namespace known_space {

bool try_fill_known_systems(const Sector& sector, std::vector<StarSystem>& out) {
    out.clear();
    bool any = false;
    for (std::size_t i = 0; i < k_catalog_size; ++i) {
        const CatalogEntry& e = k_catalog[i];
        if (e.sector_x == sector.index_x() && e.sector_y == sector.index_y()) {
            out.emplace_back(sector, e.sys_num, e.type_code, e.name, e.sx, e.sy, e.sz);
            any = true;
        }
    }
    return any;
}

}  // namespace known_space
}  // namespace galaxy
