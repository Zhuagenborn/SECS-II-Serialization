/**
 * @file write.h
 * @brief Byte serialization for SECS-II messages.
 *
 * @par GitHub
 * https://github.com/Zhuagenborn
 *
 * @date 2025-04-18
 */

#pragma once

#include "length.h"
#include "secs2.h"

#include <bit_manip/bit_manip.h>

#include <cassert>
#include <cstdint>
#include <optional>
#include <vector>

namespace secs2::byte::w {

//! Copy the bytes of an item to a buffer.
std::size_t CopyValBytes(const Item& item,
                         std::vector<std::byte>& buf) noexcept;

//! Build and write the bytes of a message header to a buffer.
std::size_t BuildHeaderBytes(Type type, std::size_t len,
                             std::vector<std::byte>& buf) noexcept;

//! @overload
std::optional<std::size_t> BuildMsgBytes(const Item& item,
                                         std::vector<std::byte>& buf) noexcept;

//! @overload
std::optional<std::size_t> BuildMsgBytes(const List& list,
                                         std::vector<std::byte>& buf) noexcept;

/**
 * @brief Build and write the bytes of a message to a buffer.
 *
 * @return
 * The number of bytes written to the buffer if the message does not exceed the maximum allowed length.
 * Otherwise @p std::nullopt and no byte is written to the buffer.
 */
std::optional<std::size_t> BuildMsgBytes(const Message::Value& val,
                                         std::vector<std::byte>& buf) noexcept;

//! Build the format byte (the first byte) for a given type and length.
constexpr std::byte BuildFormatByte(const Type type,
                                    const std::size_t len) noexcept {
    assert(IsNotExceedMaxLength(len));
    [[assume(IsNotExceedMaxLength(len))]];
    const auto len_byte_count {CalcLengthByteCount(len).value_or(0)};
    assert(len_byte_count > 0);
    [[assume(len_byte_count > 0)]];
    std::byte byte {0};
    bit::SetBits(byte, static_cast<std::uint8_t>(len_byte_count), len_bit_begin,
                 len_bit_count);
    bit::SetBits(byte, static_cast<std::uint8_t>(type), type_bit_begin,
                 type_bit_count);
    return byte;
}

}  // namespace secs2::byte::w