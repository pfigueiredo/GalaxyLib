#include "galaxylib/galaxy_api.hpp"

#include "galaxylib/galaxy_map.hpp"
#include "galaxylib/sector.hpp"

namespace galaxy::api {

bool initialize(const InitConfig& config) {
    return init_galaxy_data(config.galaxy_bin_path);
}

bool galaxy_ready() {
    return !galaxy_map_instance().empty();
}

std::vector<SystemSummary> list_system_summaries(const Sector& sector) {
    const std::vector<StarSystem>& systems = sector.systems();
    std::vector<SystemSummary> out;
    out.reserve(systems.size());
    for (const StarSystem& s : systems) {
        out.push_back(make_system_summary(s));
    }
    return out;
}

}  // namespace galaxy::api
