#include "sml.h"
#include "traits.h"

#include <algorithm>
#include <format>
#include <ranges>
#include <string_view>

namespace secs2::sml {

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
    return BuildSml<std::string>(
        os, vals,
        [](const auto val) noexcept {
            return std::format("0x{:02X}", static_cast<std::uint8_t>(val));
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
        [&os, indent_lvl, indent_width](const auto& raw) noexcept {
            BuildSml(os, raw, indent_lvl, indent_width);
        },
        val);
    return os;
}

std::ostream& BuildSml(std::ostream& os, const Item& item,
                       const std::size_t indent_lvl,
                       const std::size_t indent_width) noexcept {
    std::visit(
        [&os, indent_lvl, indent_width](const auto& raw) noexcept {
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
    std::ranges::for_each(
        list, [&os, indent_lvl, indent_width](const auto& val) noexcept {
            std::visit(
                [&os, indent_lvl, indent_width](const auto& raw) noexcept {
                    BuildSml(os, raw, indent_lvl + 1, indent_width);
                    os << '\n';
                },
                val);
        });
    os << std::format("{}>", spaces);
    return os;
}

}  // namespace secs2::sml