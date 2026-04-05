#include "galaxylib/coords.hpp"

#include "galaxylib/base36.hpp"

#include <string>
#include <string_view>

namespace galaxy {

std::string Coords::to_string() const {
    return base36_encode(static_cast<std::int64_t>(sector_index_x_)) + "." +
           base36_encode(static_cast<std::int64_t>(sector_index_y_)) + ":" +
           base36_encode(static_cast<std::int64_t>(sector_.x)) + "." +
           base36_encode(static_cast<std::int64_t>(sector_.y)) + "." +
           base36_encode(static_cast<std::int64_t>(sector_.z));
}

bool Coords::try_parse(std::string_view value, Coords& out, bool is_base36) {
    std::uint32_t sector_index_x = 0;
    std::uint32_t sector_index_y = 0;
    int sx = 0;
    int sy = 0;
    int sz = 0;

    const auto colon = value.find(':');
    std::string_view main = value.substr(0, colon == std::string_view::npos ? value.size() : colon);
    std::string_view rest =
        (colon == std::string_view::npos) ? std::string_view{} : value.substr(colon + 1);

    if (main.empty()) {
        return false;
    }

    const auto dot1 = main.find('.');
    if (dot1 == std::string_view::npos) {
        return false;
    }
    const auto part_a = main.substr(0, dot1);
    const auto part_b = main.substr(dot1 + 1);
    if (part_b.find('.') != std::string_view::npos) {
        return false;
    }

    try {
        if (is_base36) {
            sector_index_x = static_cast<std::uint32_t>(base36_decode(part_a));
            sector_index_y = static_cast<std::uint32_t>(base36_decode(part_b));
        } else {
            sector_index_x = static_cast<std::uint32_t>(std::stoul(std::string(part_a)));
            sector_index_y = static_cast<std::uint32_t>(std::stoul(std::string(part_b)));
        }
    } catch (...) {
        return false;
    }

    if (!rest.empty()) {
        std::size_t p0 = 0;
        int parts[3] = {0, 0, 0};
        int count = 0;
        while (count < 3 && p0 < rest.size()) {
            std::size_t p1 = rest.find('.', p0);
            const auto tok =
                (p1 == std::string_view::npos) ? rest.substr(p0) : rest.substr(p0, p1 - p0);
            try {
                if (is_base36) {
                    parts[count] = static_cast<int>(base36_decode(tok));
                } else {
                    parts[count] = std::stoi(std::string(tok));
                }
            } catch (...) {
                return false;
            }
            ++count;
            if (p1 == std::string_view::npos) {
                break;
            }
            p0 = p1 + 1;
        }
        if (count >= 3) {
            sx = parts[0];
            sy = parts[1];
            sz = parts[2];
        }
    }

    out = Coords(sector_index_x, sector_index_y, sx, sy, sz);
    return true;
}

}  // namespace galaxy
