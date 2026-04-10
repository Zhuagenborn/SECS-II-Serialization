#include "sml.h"
#include "traits.h"

#include <algorithm>
#include <format>
#include <ranges>
#include <string_view>

namespace secs2::sml {
// fast formatter
static std::string_view fast_formatter(std::uint8_t val) noexcept {
    static std::array<char, 1024> hex_table{}; // 256 * "0xAB"
    static bool table_prepared = false;
    if (!table_prepared) {
        for (int i = 0; i < 256; ++i) {
            char* p = &hex_table[i * 4];
            p[0] = '0';
            p[1] = 'x';
            p[2] = "0123456789ABCDEF"[i >> 4];
            p[3] = "0123456789ABCDEF"[i & 0xF];
        }
        table_prepared = true;
    }
    return std::string_view{&hex_table[val * 4], 4}; // "0xAB"
};

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
        os, vals,
        [](const auto val) noexcept {
            return fast_formatter(static_cast<uint8_t>(val));
        },
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
