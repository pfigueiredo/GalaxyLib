#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace galaxy {

/// RGB galaxy texture; matches `Sector.TheMilkyWay` / `Galaxy.bin` layout (3 bytes per pixel).
class GalaxyMap {
public:
    /// Loads raw RGB triplets (same format as C# `BinaryReader` loop in `Sector` static ctor).
    bool load_from_file(const std::string& path);

    std::size_t pixel_count() const { return rgb_.size() / 3; }

    void rgb_at(std::size_t pixel_index, std::uint8_t& r, std::uint8_t& g, std::uint8_t& b) const;

    bool empty() const { return rgb_.empty(); }

private:
    std::vector<std::uint8_t> rgb_;
};

GalaxyMap& galaxy_map_instance();

/// Must be called before constructing sectors that need density (loads `Galaxy.bin`).
bool init_galaxy_data(const std::string& galaxy_bin_path);

}  // namespace galaxy
