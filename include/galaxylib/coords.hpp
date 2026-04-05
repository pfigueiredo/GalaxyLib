#pragma once

#include "galaxylib/galaxy_constants.hpp"

#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>

namespace galaxy {

struct SectorLocation {
    int x{};
    int y{};
    int z{};

    SectorLocation() = default;
    SectorLocation(int sx, int sy, int sz) : x(sx), y(sy), z(sz) {}
};

struct ScreenLocation {
    int x{};
    int y{};
    int z{};

    ScreenLocation() = default;
    explicit ScreenLocation(const SectorLocation& s)
        : x(s.x * 8), y(s.y * 8), z(s.z + 128) {}
};

/// Galactic position in light years (`Coords.cs` / `GalacticLocation`).
struct GalacticLocation {
    double x{};
    double y{};
    double z{};

    GalacticLocation() = default;
    GalacticLocation(double gx, double gy, double gz) : x(gx), y(gy), z(gz) {}

    GalacticLocation(std::uint32_t sector_index_x, std::uint32_t sector_index_y)
        : GalacticLocation(sector_index_x, sector_index_y, SectorLocation(0, 0, 0)) {}

    GalacticLocation(std::uint32_t sector_index_x, std::uint32_t sector_index_y,
                     const SectorLocation& loc);

    SectorLocation to_sector_location() const;

    double distance_to(const GalacticLocation& o) const {
        const double dx = std::abs(static_cast<float>(o.x - x));
        const double dy = std::abs(static_cast<float>(o.y - y));
        const double dz = std::abs(static_cast<float>(o.z - z));
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

/// Packed sector id (`SectorId` in `Coords.cs`).
struct SectorId {
    std::uint64_t id{};

    explicit SectorId(std::uint64_t raw) : id(raw & 0x3FFFFFFu) {}

    SectorId(std::uint32_t sector_index_x, std::uint32_t sector_index_y)
        : id((static_cast<std::uint64_t>(sector_index_x) << 13) +
             static_cast<std::uint64_t>(sector_index_y)) {}

    std::uint32_t sector_index_x() const { return static_cast<std::uint32_t>((id >> 13) & 0x1FFFu); }
    std::uint32_t sector_index_y() const { return static_cast<std::uint32_t>(id & 0x1FFFu); }
};

/// Packed system id (`SystemId` in `Coords.cs`).
struct SystemId {
    std::uint64_t id{};

    explicit SystemId(std::uint64_t raw) : id(raw) {}

    SystemId(int sys_num, std::uint32_t sector_index_x, std::uint32_t sector_index_y)
        : id((static_cast<std::uint64_t>(sys_num) << 26) +
             (static_cast<std::uint64_t>(sector_index_x) << 13) +
             static_cast<std::uint64_t>(sector_index_y)) {}

    std::uint32_t sector_index_x() const { return static_cast<std::uint32_t>((id >> 13) & 0x1FFFu); }
    std::uint32_t sector_index_y() const { return static_cast<std::uint32_t>(id & 0x1FFFu); }
    int system_num() const { return static_cast<int>(id >> 26); }

    SectorId sector_id() const { return SectorId(id); }
};

/// Full location: sector indices + local sector coordinates (`Coords` in `Coords.cs`).
class Coords {
public:
    Coords() = default;

    Coords(std::uint32_t sector_index_x, std::uint32_t sector_index_y);

    Coords(std::uint32_t sector_index_x, std::uint32_t sector_index_y, int sx, int sy, int sz);

    Coords(std::uint32_t sector_index_x, std::uint32_t sector_index_y, SectorLocation loc);

    explicit Coords(const GalacticLocation& location);

    GalacticLocation galactic() const { return galactic_; }
    SectorLocation sector() const { return sector_; }
    ScreenLocation screen() const { return screen_; }

    std::uint32_t sector_index_x() const { return sector_index_x_; }
    std::uint32_t sector_index_y() const { return sector_index_y_; }

    void set_galactic(const GalacticLocation& g);

    double distance_to(const Coords& o) const {
        return galactic_.distance_to(o.galactic_);
    }

    bool is_in_range(const Coords& o, double range_ly) const { return distance_to(o) <= range_ly; }

    bool operator==(const Coords& o) const;
    bool operator!=(const Coords& o) const { return !(*this == o); }

    /// `base36.base36.sectorBase36:localBase36` (`Coords.ToString`).
    std::string to_string() const;

    /// `Coords.TryParse` — `is_base36` selects decode path for all numeric parts.
    static bool try_parse(std::string_view value, Coords& out, bool is_base36);

private:
    void sync_from_galactic();
    void sync_from_sector_indices_and_local();

    GalacticLocation galactic_{};
    SectorLocation sector_{};
    ScreenLocation screen_{};
    std::uint32_t sector_index_x_{};
    std::uint32_t sector_index_y_{};
};

inline GalacticLocation::GalacticLocation(std::uint32_t sector_index_x, std::uint32_t sector_index_y,
                                         const SectorLocation& sector_location) {
    const double sw = GalaxyConstants::sector_width;
    const double lw = GalaxyConstants::local_sector_width;
    const double ld = GalaxyConstants::local_sector_depth;

    x = static_cast<double>(sector_index_x) * sw +
        (static_cast<double>(sector_location.x) + lw / 2.0) * sw / lw;
    y = static_cast<double>(sector_index_y) * sw +
        (static_cast<double>(sector_location.y) + lw / 2.0) * sw / lw;
    z = (static_cast<double>(sector_location.z) + ld / 2.0) * GalaxyConstants::sector_depth / ld;
}

inline SectorLocation GalacticLocation::to_sector_location() const {
    const double sw = GalaxyConstants::sector_width;
    const double lw = GalaxyConstants::local_sector_width;
    const double ld = GalaxyConstants::local_sector_depth;

    double sx = std::fmod(x, sw) * lw / sw;
    double sy = std::fmod(y, sw) * lw / sw;
    double sz = (z * ld / GalaxyConstants::sector_depth);

    return SectorLocation(
        static_cast<int>(std::lround(sx - lw / 2.0)),
        static_cast<int>(std::lround(sy - lw / 2.0)),
        static_cast<int>(std::lround(sz - ld / 2.0)));
}

inline Coords::Coords(std::uint32_t sector_index_x, std::uint32_t sector_index_y)
    : sector_index_x_(sector_index_x), sector_index_y_(sector_index_y), sector_(0, 0, 0) {
    sync_from_sector_indices_and_local();
}

inline Coords::Coords(std::uint32_t sector_index_x, std::uint32_t sector_index_y, int sx, int sy, int sz)
    : sector_index_x_(sector_index_x), sector_index_y_(sector_index_y), sector_(sx, sy, sz) {
    sync_from_sector_indices_and_local();
}

inline Coords::Coords(std::uint32_t sector_index_x, std::uint32_t sector_index_y, SectorLocation loc)
    : sector_index_x_(sector_index_x), sector_index_y_(sector_index_y), sector_(loc) {
    sync_from_sector_indices_and_local();
}

inline Coords::Coords(const GalacticLocation& location) : galactic_(location) {
    sync_from_galactic();
}

inline void Coords::set_galactic(const GalacticLocation& g) {
    galactic_ = g;
    sync_from_galactic();
}

inline void Coords::sync_from_galactic() {
    sector_ = galactic_.to_sector_location();
    screen_ = ScreenLocation(sector_);
    const double sw = GalaxyConstants::sector_width;
    sector_index_x_ = static_cast<std::uint32_t>(galactic_.x / sw);
    sector_index_y_ = static_cast<std::uint32_t>(galactic_.y / sw);
}

inline void Coords::sync_from_sector_indices_and_local() {
    galactic_ = GalacticLocation(sector_index_x_, sector_index_y_, sector_);
    screen_ = ScreenLocation(sector_);
}

inline bool Coords::operator==(const Coords& o) const {
    return sector_index_x_ == o.sector_index_x_ && sector_index_y_ == o.sector_index_y_ &&
           sector_.x == o.sector_.x && sector_.y == o.sector_.y && sector_.z == o.sector_.z;
}

}  // namespace galaxy
