/**
 * @file length.h
 * @brief SECS-II length calculation utilities.
 *
 * @par GitHub
 * https://github.com/Zhuagenborn
 *
 * @date 2025-04-18
 */

#pragma once

#include "secs2.h"

#include <bit_manip/bit_manip.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <optional>

namespace secs2 {

//! The bit position where the number of length bytes begins.
inline constexpr std::size_t len_bit_begin {0};
//! The number of bits for the number of length bytes.
inline constexpr std::size_t len_bit_count {2};

//! The bit position where the format code begins.
inline constexpr std::size_t type_bit_begin {len_bit_begin + len_bit_count};
//! The number of bits for the format code.
inline constexpr std::size_t type_bit_count {CHAR_BIT - type_bit_begin};

//! The maximum number of bytes used to represent the length.
inline constexpr std::size_t max_len_byte_count {3};

//! Same as @ref Message::max_length.
inline constexpr std::size_t max_len {
    bit::GetBits(std::numeric_limits<std::uint32_t>::max(), 0,
                 max_len_byte_count* CHAR_BIT)};
static_assert(max_len == Message::max_length);

//! @overload
std::size_t CalcLength(const Item& item) noexcept;

//! @overload
std::size_t CalcLength(const List& list) noexcept;

/**
 * @brief Get the length of a message.
 *
 * @return
 * - For an item, it is the number of bytes.
 * - For an list, it is the number of elements that is counted in terms of its direct elements only
 * and does not take into account any nested linked lists within the elements.
 */
std::size_t CalcLength(const Message::Value& val) noexcept;

//! Check whether a given number of length bytes is within the valid range.
constexpr bool IsNotExceedLengthByteCountRange(
    const std::size_t count) noexcept {
    return 0 < count && count <= max_len_byte_count;
}

//! Opposite to @ref IsNotExceedLengthByteCountRange.
constexpr bool IsExceedLengthByteCountRange(const std::size_t count) noexcept {
    return !IsNotExceedLengthByteCountRange(count);
}

//! Check whether a given length exceeds the maximum allowed length.
constexpr bool IsExceedMaxLength(const std::size_t size) noexcept {
    return size > max_len;
}

//! Opposite to @ref IsExceedMaxLength.
constexpr bool IsNotExceedMaxLength(const std::size_t size) noexcept {
    return !IsExceedMaxLength(size);
}

/**
 * @brief Filter out invalid length.
 *
 * @param len A length.
 * @return The length if it does not exceed the maximum allowed length, otherwise @p std::nullopt.
 */
constexpr std::optional<std::size_t> FilterExceededLength(
    const std::size_t len) noexcept {
    return IsNotExceedMaxLength(len) ? std::make_optional<std::size_t>(len)
                                     : len;
}

//! Calculates the number of bytes required for a given length.
constexpr std::optional<std::size_t> CalcLengthByteCount(
    const std::size_t len) noexcept {
    if (len <= std::numeric_limits<std::uint8_t>::max()) [[likely]] {
        return sizeof(std::uint8_t);
    } else if (len <= std::numeric_limits<std::uint16_t>::max()) {
        return sizeof(std::uint16_t);
    } else if (IsNotExceedMaxLength(len)) {
        return sizeof(std::uint16_t) + sizeof(std::uint8_t);
    } else [[unlikely]] {
        return std::nullopt;
    }
}

//! Length bytes.
struct LengthBytes {
    //! The number of valid bytes.
    std::size_t valid_count {0};
    //! Reserved bytes.
    std::array<std::byte, max_len_byte_count> reserved {};

    //! Build length bytes for a given length.
    explicit constexpr LengthBytes(const std::size_t len) noexcept {
        assert(IsNotExceedMaxLength(len));
        [[assume(IsNotExceedMaxLength(len))]];
        valid_count = CalcLengthByteCount(len).value_or(0);
        assert(valid_count > 0);
        [[assume(valid_count > 0)]];
        bit::WriteBytes(len, std::as_writable_bytes(std::span {reserved}),
                        std::endian::big);
        if (valid_count < reserved.size()) [[likely]] {
            std::ranges::copy_n(reserved.cend() - valid_count, valid_count,
                                reserved.begin());
        }
    }
};

}  // namespace secs2