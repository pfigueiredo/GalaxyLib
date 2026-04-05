#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace galaxy {

/// Matches `GalaxyE.Galaxy.Base36`.
std::string base36_encode(std::int64_t input);
std::int64_t base36_decode(std::string_view data);

}  // namespace galaxy
