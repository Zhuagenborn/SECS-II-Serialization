/**
 * @file traits.h
 * @brief Type inspection and access utilities.
 *
 * @par GitHub
 * https://github.com/Zhuagenborn
 *
 * @date 2025-04-18
 */

#pragma once

#include "secs2.h"

#include <type_traits>
#include <utility>
#include <variant>

namespace secs2 {

//! A function object combining multiple callable types.
template <typename... Fs>
struct Overload : Fs... {
    template <typename... Ts>
    explicit Overload(Ts&&... ts) : Fs {std::forward<Ts>(ts)}... {}

    using Fs::operator()...;
};

template <typename... Ts>
Overload(Ts&&...) -> Overload<std::remove_reference_t<Ts>...>;

//! Map types to format codes.
template <typename T>
inline constexpr Type format_code {Type::Unknown};

#define MAP_FORMAT_TYPE_TO_CODE(type) \
    template <>                       \
    inline constexpr Type format_code<type> {Type::type};

MAP_FORMAT_TYPE_TO_CODE(Binary)
MAP_FORMAT_TYPE_TO_CODE(ASCII)
MAP_FORMAT_TYPE_TO_CODE(List)
MAP_FORMAT_TYPE_TO_CODE(Boolean)
MAP_FORMAT_TYPE_TO_CODE(I1)
MAP_FORMAT_TYPE_TO_CODE(I2)
MAP_FORMAT_TYPE_TO_CODE(I4)
MAP_FORMAT_TYPE_TO_CODE(I8)
MAP_FORMAT_TYPE_TO_CODE(U1)
MAP_FORMAT_TYPE_TO_CODE(U2)
MAP_FORMAT_TYPE_TO_CODE(U4)
MAP_FORMAT_TYPE_TO_CODE(U8)
MAP_FORMAT_TYPE_TO_CODE(F4)
MAP_FORMAT_TYPE_TO_CODE(F8)

//! @overload
constexpr Type GetType(const List&) noexcept {
    return Type::List;
}

//! @overload
constexpr Type GetType(const Item& item) noexcept {
    return std::visit(
        [](const auto& raw) noexcept {
            return format_code<std::decay_t<decltype(raw)>>;
        },
        item);
}

//! Get the type of a message.
constexpr Type GetType(const Message::Value& val) noexcept {
    return std::visit([](const auto& raw) noexcept { return GetType(raw); },
                      val);
}

}  // namespace secs2