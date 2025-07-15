#include "write.h"
#include "length.h"
#include "traits.h"

#include <bit_manip/bit_manip.h>

#include <iterator>
#include <ranges>
#include <span>
#include <type_traits>

namespace secs2::byte::w {

std::size_t CopyBoolValBytes(const Boolean& vals,
                             std::vector<std::byte>& buf) noexcept {
    const auto init_buf_size {buf.size()};
    std::ranges::transform(
        vals, std::back_inserter(buf),
        [](const auto val) noexcept { return static_cast<std::byte>(val); });
    return buf.size() - init_buf_size;
}

std::size_t CopyValBytes(const Item& item,
                         std::vector<std::byte>& buf) noexcept {
    const auto type {GetType(item)};
    assert(type != Type::Unknown && type != Type::List);
    const auto init_buf_size {buf.size()};
    const Overload visitor {
        [&buf](const Boolean& raw) noexcept { CopyBoolValBytes(raw, buf); },
        [&buf](const auto& raw) noexcept {
            using Value =
                std::ranges::range_value_t<std::decay_t<decltype(raw)>>;
            if constexpr (sizeof(Value) <= sizeof(std::byte)) {
                std::ranges::copy(std::as_bytes(std::span {raw}),
                                  std::back_inserter(buf));
            } else {
                std::ranges::for_each(raw, [&buf](const auto val) noexcept {
                    Value big_endian_val {};
                    const auto big_endian_buf {
                        std::as_writable_bytes(std::span {&big_endian_val, 1})};
                    bit::WriteBytes(val, big_endian_buf, std::endian::big);
                    std::ranges::copy(std::as_bytes(big_endian_buf),
                                      std::back_inserter(buf));
                });
            }
        }};
    std::visit(visitor, item);
    return buf.size() - init_buf_size;
}

std::size_t BuildHeaderBytes(const Type type, const std::size_t len,
                             std::vector<std::byte>& buf) noexcept {
    assert(IsNotExceedMaxLength(len));
    const auto init_buf_size {buf.size()};
    const auto format_byte {BuildFormatByte(type, len)};
    buf.push_back(format_byte);

    const LengthBytes len_bytes {len};
    assert(len_bytes.valid_count <= len_bytes.reserved.size());
    for (std::size_t i {0}; i != len_bytes.valid_count; ++i) {
        buf.push_back(static_cast<std::byte>(len_bytes.reserved[i]));
    }

    return buf.size() - init_buf_size;
}

std::optional<std::size_t> BuildMsgBytes(const Item& item,
                                         std::vector<std::byte>& buf) noexcept {
    if (const auto len {CalcLength(item)}; IsNotExceedMaxLength(len))
        [[likely]] {
        const auto init_buf_size {buf.size()};
        BuildHeaderBytes(GetType(item), len, buf);
        CopyValBytes(item, buf);
        return buf.size() - init_buf_size;
    } else {
        return std::nullopt;
    }
}

std::optional<std::size_t> BuildMsgBytes(const List& list,
                                         std::vector<std::byte>& buf) noexcept {
    if (const auto len {CalcLength(list)}; IsNotExceedMaxLength(len))
        [[likely]] {
        const auto init_buf_size {buf.size()};
        BuildHeaderBytes(Type::List, len, buf);
        for (const auto& val : list) {
            if (!std::visit(
                     [&buf](const auto& raw) noexcept {
                         return BuildMsgBytes(raw, buf);
                     },
                     val)
                     .has_value()) [[unlikely]] {
                buf.resize(init_buf_size);
                buf.shrink_to_fit();
                return std::nullopt;
            }
        }

        return buf.size() - init_buf_size;
    } else {
        return std::nullopt;
    }
}

std::optional<std::size_t> BuildMsgBytes(const Message::Value& val,
                                         std::vector<std::byte>& buf) noexcept {
    return std::visit(
        [&buf](const auto& raw) noexcept { return BuildMsgBytes(raw, buf); },
        val);
}

}  // namespace secs2::byte::w