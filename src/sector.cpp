#include "galaxylib/sector.hpp"

#include "galaxylib/base36.hpp"
#include "galaxylib/galaxy_map.hpp"
#include "galaxylib/galaxy_utils.hpp"
#include "galaxylib/known_space.hpp"
#include "galaxylib/star_system.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace galaxy {

#include "system_density.inc"

namespace {

std::uint64_t ulong_as_long_bits_shift_right(std::uint64_t v, int bits) {
    std::int64_t sv;
    std::memcpy(&sv, &v, sizeof(v));
    sv >>= bits;
    std::uint64_t out;
    std::memcpy(&out, &sv, sizeof(out));
    return out;
}

}  // namespace

int Sector::generate_num_systems(std::uint32_t coordx, std::uint32_t coordy, int galaxy_scale) {
    galaxy_scale = 0;
    std::uint64_t p1 = 0;
    std::uint64_t p2 = 0;
    std::uint64_t p3 = 0;
    std::uint64_t p4 = 0;
    std::uint64_t eax = 0;
    std::uint64_t ebx = 0;
    std::uint64_t ecx = 0;
    std::uint64_t edx = 0;
    std::uint64_t esi = 0;
    std::uint64_t edi = 0;

    if (coordx > 0x1fffu || coordy > 0x1fffu) {
        return 0;
    }

    GalaxyMap& mw = galaxy_map_instance();
    if (mw.empty()) {
        throw std::logic_error("galaxy data not initialized; call init_galaxy_data() first");
    }

    const int pixelval = static_cast<int>((coordx / 64u) + 2u * (coordy & 0x1fc0u));
    if (pixelval < 0 || static_cast<std::size_t>(pixelval) >= mw.pixel_count()) {
        return 0;
    }

    std::uint8_t r0 = 0;
    std::uint8_t g0 = 0;
    std::uint8_t b0 = 0;
    mw.rgb_at(static_cast<std::size_t>(pixelval), r0, g0, b0);

    std::uint64_t value0 = (static_cast<std::uint64_t>(r0) + static_cast<std::uint64_t>(g0) +
                            static_cast<std::uint64_t>(b0)) /
                           3u;

    auto sample_avg = [&](int offset) {
        const std::size_t idx = static_cast<std::size_t>(pixelval + offset);
        if (idx >= mw.pixel_count()) {
            throw std::out_of_range("galaxy map sample index");
        }
        std::uint8_t cr = 0;
        std::uint8_t cg = 0;
        std::uint8_t cb = 0;
        mw.rgb_at(idx, cr, cg, cb);
        return (static_cast<std::uint64_t>(cr) + static_cast<std::uint64_t>(cg) +
                static_cast<std::uint64_t>(cb)) /
               3u;
    };

    std::uint64_t value1 = sample_avg(128);
    std::uint64_t value2 = sample_avg(129);
    std::uint64_t value3 = sample_avg(1);

    p1 = value0;
    p3 = value1;
    p4 = value2;
    p2 = value3;

    coordx = (coordx * 512u) & 32256u;
    coordy = (coordy * 512u) & 32256u;

    ebx = (p2 - p1) * static_cast<std::uint64_t>(coordx) + (p3 - p1) * static_cast<std::uint64_t>(coordy);
    esi = (static_cast<std::uint64_t>(coordx) * static_cast<std::uint64_t>(coordy)) >> 15;
    edi = p4 - p3 - p2 + p1;
    esi *= edi;
    ebx += esi;
    ebx += ebx;
    p1 <<= 16u;
    ebx += p1;
    ecx = ebx >> 8u;

    if (galaxy_scale < 16) {
        ebx = static_cast<std::uint64_t>(coordx) + ecx;
        eax = static_cast<std::uint64_t>(coordx) * static_cast<std::uint64_t>(coordy);
        eax = ulong_as_long_bits_shift_right(eax, 15);
        ebx ^= eax;
        ebx = ulong_as_long_bits_shift_right(ebx, 5);
        ebx &= 0x7fu;
        eax = k_system_density[ebx];
        if (galaxy_scale != 0) {
            edx = static_cast<std::uint64_t>(16 - galaxy_scale);
            edx *= eax;
            edx = ulong_as_long_bits_shift_right(edx, 5);
            eax = 0xffffu - edx;
            ecx *= eax;
            ecx >>= 16u;
        } else {
            ecx *= eax;
            ecx >>= 16u;
        }
    }

    p1 = ecx;
    p1 >>= 10u;

    // Matches C#: scales sector color components (side effect only; return uses p1).
    const int r =
        (static_cast<int>(p1) * static_cast<int>(r0) / 15) & 0xFF;
    const int g =
        (static_cast<int>(p1) * static_cast<int>(g0) / 15) & 0xFF;
    const int b =
        (static_cast<int>(p1) * static_cast<int>(b0) / 15) & 0xFF;
    (void)r;
    (void)g;
    (void)b;

    return static_cast<int>(p1) >> 1;
}

Sector::Sector() : Sector(4111u, 6303u) {}

Sector::Sector(std::uint32_t index_x, std::uint32_t index_y)
    : index_x_(index_x), index_y_(index_y), num_systems_(generate_num_systems(index_x, index_y, 0)) {}

std::string Sector::to_string_base36() const {
    return base36_encode(static_cast<std::int64_t>(index_x_)) + "." +
           base36_encode(static_cast<std::int64_t>(index_y_));
}

std::array<std::uint64_t, 2> Sector::get_system_params(int sys_num) const {
    std::uint32_t coordx = index_x_;
    std::uint32_t coordy = index_y_;

    std::uint64_t system_param_0 =
        (static_cast<std::uint64_t>(coordx) << 16) + static_cast<std::uint64_t>(coordy);
    std::uint64_t system_param_1 =
        (static_cast<std::uint64_t>(coordy) << 16) + static_cast<std::uint64_t>(coordx);

    rotate_system_params(system_param_0, system_param_1);
    rotate_system_params(system_param_0, system_param_1);
    rotate_system_params(system_param_0, system_param_1);

    for (int i = 0; i <= sys_num; ++i) {
        rotate_system_params(system_param_0, system_param_1);
        if (i == sys_num) {
            return {system_param_0, system_param_1};
        }
    }

    return {0, 0};
}

void Sector::ensure_systems() const {
    if (systems_cached_) {
        return;
    }
    systems_.clear();
    if (known_space::try_fill_known_systems(*this, systems_)) {
        systems_cached_ = true;
        return;
    }
    if (num_systems_ > 0) {
        systems_.reserve(static_cast<std::size_t>(num_systems_));
    }
    for (int i = 0; i < num_systems_; ++i) {
        systems_.emplace_back(*this, i);
    }
    systems_cached_ = true;
}

const std::vector<StarSystem>& Sector::systems() const {
    ensure_systems();
    return systems_;
}

}  // namespace galaxy
