#include "galaxylib/base36.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <stdexcept>

namespace galaxy {

namespace {

constexpr const char* k_char_list = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

}  // namespace

std::string base36_encode(std::int64_t input) {
    bool neg = false;
    if (input < 0) {
        neg = true;
        input = -input;
    }
    std::string rev;
    while (input != 0) {
        rev.push_back(k_char_list[input % 36]);
        input /= 36;
    }
    if (rev.empty()) {
        rev = "0";
    } else {
        std::reverse(rev.begin(), rev.end());
    }
    return (neg ? "-" : "") + rev;
}

std::int64_t base36_decode(std::string_view data) {
    while (!data.empty() && (data.front() == ' ' || data.front() == '\t')) {
        data.remove_prefix(1);
    }
    while (!data.empty() && (data.back() == ' ' || data.back() == '\t')) {
        data.remove_suffix(1);
    }
    bool neg = !data.empty() && data.front() == '-';
    if (neg) {
        data.remove_prefix(1);
    }
    // Trim leading zeros (after uppercasing in C#: TrimStart('0'))
    std::string upper;
    upper.reserve(data.size());
    for (char c : data) {
        upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    size_t start = 0;
    while (start < upper.size() && upper[start] == '0') {
        ++start;
    }
    std::int64_t result = 0;
    int pos = 0;
    for (size_t i = upper.size(); i > start; --i) {
        char c = upper[i - 1];
        const char* p = std::strchr(k_char_list, c);
        if (!p) {
            throw std::invalid_argument("base36_decode: invalid character");
        }
        int digit = static_cast<int>(p - k_char_list);
        std::int64_t pow36 = 1;
        for (int k = 0; k < pos; ++k) {
            pow36 *= 36;
        }
        result += digit * pow36;
        ++pos;
    }
    return neg ? -result : result;
}

}  // namespace galaxy
