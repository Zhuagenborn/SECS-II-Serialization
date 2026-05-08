#include "sml.h"
#include "traits.h"

#include <algorithm>
#include <array>
#include <format>
#include <limits>
#include <ranges>
#include <string_view>

namespace secs2::sml {

inline constexpr std::size_t hex_string_length {sizeof("0xXX") - 1};

using HexString = std::array<char, hex_string_length>;

consteval auto CreateHexStringTable() noexcept {
    constexpr std::string_view digits {"0123456789ABCDEF"};
    constexpr auto val_count {std::numeric_limits<std::uint8_t>::max() + 1};
    std::array<HexString, val_count> table {};
    for (auto i {0}; i != table.size(); ++i) {
        table[i] = {'0', 'x', digits[i >> 4], digits[i & 0x0F]};
    }
    return table;
}

constexpr std::string_view FastFormatHex(const std::byte val) noexcept {
    static constexpr auto hex_table {CreateHexStringTable()};
    const auto& str {hex_table[std::to_integer<std::size_t>(val)]};
    return {str.data(), str.size()};
}

std::ostream& BuildSml(std::ostream& os, const ASCII& vals,
                       const std::size_t indent_lvl,
                       const std::size_t indent_width) noexcept {
    const auto spaces {BuildIndent(indent_lvl, indent_width)};
    const auto tag {format_tag<ASCII>};
    if (!vals.empty()) [[likely]] {
        os << std::format("{}<{} [{}] \"{}\">", spaces, tag, vals.size(), vals);
    } else {
        os << std::format("{}<{} [{}]>", spaces, tag, 0);
    }
    return os;
}

std::ostream& BuildSml(std::ostream& os, const Binary& vals,
                       const std::size_t indent_lvl,
                       const std::size_t indent_width) noexcept {
    return BuildSml<std::string_view>(
        os, vals, [](const auto val) noexcept { return FastFormatHex(val); },
        indent_lvl, indent_width);
}

std::ostream& BuildSml(std::ostream& os, const Boolean& vals,
                       const std::size_t indent_lvl,
                       const std::size_t indent_width) noexcept {
    return BuildSml<std::string_view>(
        os, vals,
        [](const auto val) noexcept -> std::string_view {
            return val ? "true" : "false";
        },
        indent_lvl, indent_width);
}

std::ostream& BuildSml(std::ostream& os, const Message::Value& val,
                       const std::size_t indent_lvl,
                       const std::size_t indent_width) noexcept {
    std::visit(
        [&](const auto& raw) noexcept {
            BuildSml(os, raw, indent_lvl, indent_width);
        },
        val);
    return os;
}

std::ostream& BuildSml(std::ostream& os, const Item& item,
                       const std::size_t indent_lvl,
                       const std::size_t indent_width) noexcept {
    std::visit(
        [&](const auto& raw) noexcept {
            BuildSml(os, raw, indent_lvl, indent_width);
        },
        item);
    return os;
}

std::ostream& BuildSml(std::ostream& os, const List& list,
                       const std::size_t indent_lvl,
                       const std::size_t indent_width) noexcept {
    const auto spaces {BuildIndent(indent_lvl, indent_width)};
    os << std::format("{}<L [{}]\n", spaces, list.size());
    std::ranges::for_each(list, [&](const auto& val) noexcept {
        std::visit(
            [&](const auto& raw) noexcept {
                BuildSml(os, raw, indent_lvl + 1, indent_width);
                os << '\n';
            },
            val);
    });
    os << std::format("{}>", spaces);
    return os;
}

}  // namespace secs2::sml
