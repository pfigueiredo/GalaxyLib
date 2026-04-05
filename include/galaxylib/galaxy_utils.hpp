#pragma once

#include <cstdint>

namespace galaxy {

/// Matches `Utils.Rotate_SystemParams` in `GalaxyE/Galaxy/Utils.cs`.
inline void rotate_system_params(std::uint64_t& system_param_0, std::uint64_t& system_param_1) {
    std::uint64_t tmp1 = (system_param_0 << 3) | (system_param_0 >> 29);
    std::uint64_t tmp2 = system_param_0 + system_param_1;
    tmp1 += tmp2;
    system_param_0 = tmp1;
    system_param_1 = (tmp2 << 5) | (tmp2 >> 27);
}

inline std::uint32_t rotr(std::uint32_t value, int shift);

inline std::uint32_t rotl(std::uint32_t value, int shift) {
    constexpr int max_bits = 32;
    if (shift < 0) {
        return rotr(value, -shift);
    }
    if (shift > max_bits) {
        shift = shift % max_bits;
    }
    return (value << shift) | (value >> (max_bits - shift));
}

inline std::uint32_t rotr(std::uint32_t value, int shift) {
    constexpr int max_bits = 32;
    if (shift < 0) {
        return rotl(value, -shift);
    }
    if (shift > max_bits * 8) {
        shift = shift % max_bits;
    }
    return (value >> shift) | (value << (max_bits - shift));
}

}  // namespace galaxy
