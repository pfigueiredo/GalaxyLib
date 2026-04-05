#include "galaxylib/galaxy_map.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>

namespace galaxy {

bool GalaxyMap::load_from_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return false;
    }
    rgb_.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    if (rgb_.empty() || rgb_.size() % 3 != 0) {
        rgb_.clear();
        return false;
    }
    return true;
}

void GalaxyMap::rgb_at(std::size_t pixel_index, std::uint8_t& r, std::uint8_t& g, std::uint8_t& b) const {
    if (pixel_index >= pixel_count()) {
        throw std::out_of_range("GalaxyMap::rgb_at: pixel index out of range");
    }
    const std::size_t o = pixel_index * 3;
    r = rgb_[o];
    g = rgb_[o + 1];
    b = rgb_[o + 2];
}

GalaxyMap& galaxy_map_instance() {
    static GalaxyMap map;
    return map;
}

bool init_galaxy_data(const std::string& galaxy_bin_path) {
    return galaxy_map_instance().load_from_file(galaxy_bin_path);
}

}  // namespace galaxy
