/**
 * @file sml.h
 * @brief SML (SECS Message Language) representation of SECS-II data.
 *
 * @par GitHub
 * https://github.com/Zhuagenborn
 *
 * @date 2025-04-18
 */

#pragma once

#include "secs2.h"

#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>

namespace secs2::sml {

//! @overload
std::ostream& BuildSml(std::ostream& os, const Item& item,
                       std::size_t indent_lvl,
                       std::size_t indent_width) noexcept;

//! @overload
std::ostream& BuildSml(std::ostream& os, const List& list,
                       std::size_t indent_lvl,
                       std::size_t indent_width) noexcept;

//! @overload
std::ostream& BuildSml(std::ostream& os, const ASCII& vals,
                       std::size_t indent_lvl,
                       std::size_t indent_width) noexcept;

//! @overload
std::ostream& BuildSml(std::ostream& os, const Binary& vals,
                       std::size_t indent_lvl,
                       std::size_t indent_width) noexcept;
//! @overload
std::ostream& BuildSml(std::ostream& os, const Boolean& vals,
                       std::size_t indent_lvl,
                       std::size_t indent_width) noexcept;

/**
 * @brief Format a message to a SML (SECS Message Language) string and output it to a stream.
 *
 * @param os An output stream.
 * @param val A message.
 * @param indent_lvl The indentation level.
 * @param indent_width The number of spaces per indentation level.
 */
std::ostream& BuildSml(std::ostream& os, const Message::Value& val,
                       std::size_t indent_lvl,
                       std::size_t indent_width) noexcept;

//! Map types to string tags.
template <typename T>
inline constexpr std::string_view format_tag {"Unknown"};

#define MAP_FORMAT_TYPE_TO_TAG(type, tag) \
    template <>                           \
    inline constexpr std::string_view format_tag<type> {tag};

MAP_FORMAT_TYPE_TO_TAG(Binary, "B")
MAP_FORMAT_TYPE_TO_TAG(ASCII, "A")
MAP_FORMAT_TYPE_TO_TAG(List, "L")
MAP_FORMAT_TYPE_TO_TAG(Boolean, "Boolean")
MAP_FORMAT_TYPE_TO_TAG(I1, "I1")
MAP_FORMAT_TYPE_TO_TAG(I2, "I2")
MAP_FORMAT_TYPE_TO_TAG(I4, "I4")
MAP_FORMAT_TYPE_TO_TAG(I8, "I8")
MAP_FORMAT_TYPE_TO_TAG(U1, "U1")
MAP_FORMAT_TYPE_TO_TAG(U2, "U2")
MAP_FORMAT_TYPE_TO_TAG(U4, "U4")
MAP_FORMAT_TYPE_TO_TAG(U8, "U8")
MAP_FORMAT_TYPE_TO_TAG(F4, "F4")
MAP_FORMAT_TYPE_TO_TAG(F8, "F8")

/**
 * @brief Build an indentation string.
 *
 * @param lvl The indentation level.
 * @param width The number of spaces per indentation level.
 */
constexpr std::string BuildIndent(const std::size_t lvl,
                                  const std::size_t width) noexcept {
    return std::string(lvl * width, ' ');
}

/**
 * @brief
 * Format an item to a SML (SECS Message Language) string using a custom formatter
 * and output it to a stream.
 *
 * @tparam To The return type of the formatter function (e.g., @p std::string).
 * @tparam From The type of the item.
 *
 * @param os An output stream.
 * @param vals An item
 * @param formatter A formatter that transforms each element into a formatted representation.
 * @param indent_lvl The indentation level.
 * @param indent_width The number of spaces per indentation level.
 */
template <typename To, typename From>
    requires(!std::same_as<From, ASCII> && !std::same_as<From, List>)
std::ostream& BuildSml(
    std::ostream& os, const From& vals,
    std::function<To(std::ranges::range_value_t<From>)> formatter,
    const std::size_t indent_lvl, const std::size_t indent_width) noexcept {
    const auto spaces {BuildIndent(indent_lvl, indent_width)};
    const auto tag {format_tag<From>};
    if (!vals.empty()) [[likely]] {
        const auto val_strs {vals | std::views::transform(formatter)};
        const auto str {std::ranges::to<std::string>(
            val_strs | std::views::join_with(' '))};
        os << std::format("{}<{} [{}] {}>", spaces, tag, vals.size(), str);
    } else {
        os << std::format("{}<{} [{}]>", spaces, tag, 0);
    }

    return os;
}

//! @overload
template <typename T>
    requires std::is_arithmetic_v<std::ranges::range_value_t<T>>
std::ostream& BuildSml(std::ostream& os, const T& vals,
                       const std::size_t indent_lvl,
                       const std::size_t indent_width) noexcept {
    return BuildSml<std::string>(
        os, vals,
        [](const auto val) noexcept { return std::format("{}", val); },
        indent_lvl, indent_width);
}

}  // namespace secs2::sml