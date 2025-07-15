#include "length.h"

#include <ranges>

namespace secs2 {

std::size_t CalcLength(const Item& item) noexcept {
    return std::visit(
        [](const auto& raw) noexcept {
            using Value =
                std::ranges::range_value_t<std::decay_t<decltype(raw)>>;
            return static_cast<std::size_t>(raw.size()) * sizeof(Value);
        },
        item);
}

std::size_t CalcLength(const List& list) noexcept {
    return list.size();
}

std::size_t CalcLength(const Message::Value& val) noexcept {
    return std::visit([](const auto& raw) noexcept { return CalcLength(raw); },
                      val);
}

}  // namespace secs2