/**
 * @file read.h
 * @brief Byte deserialization for SECS-II messages.
 *
 * @par GitHub
 * https://github.com/Zhuagenborn
 *
 * @date 2025-04-18
 */

#pragma once

#include "length.h"
#include "secs2.h"
#include "traits.h"

#include <bit_manip/bit_manip.h>

#include <algorithm>
#include <cassert>
#include <expected>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

namespace secs2::byte::r {

namespace err {

//! Make an error indicating the provided data are incomplete.
Error MakeIncompleteDataError() noexcept;

//! Make an error indicating that the specified type is unknown or unsupported.
Error MakeUnknownTypeError(Type type) noexcept;

//! Make an error indicating that the number of length bytes is invalid.
Error MakeInvalidLengthByteCountError(std::size_t count) noexcept;

//! Make an error indicating that the length is not properly aligned.
Error MakeUnalignedLengthError(std::size_t len, Type type,
                               std::size_t align) noexcept;
}  // namespace err

//! A deserialized message and its size in bytes.
using Loaded = std::pair<Message::Value, std::size_t>;

//! Same as @ref Message::BuildFromBytes.
std::expected<Loaded, Error> LoadMsgBytes(
    std::span<const std::byte> bytes) noexcept;

/**
 * @brief Read the bytes of a boolean from a buffer.
 *
 * @param bytes A buffer.
 * @param len The size of the boolean in bytes.
 */
std::expected<Loaded, Error> LoadBoolValBytes(std::span<const std::byte> bytes,
                                              std::size_t len) noexcept;

/**
 * @brief Read the bytes of a list from a buffer.
 *
 * @param bytes A buffer.
 * @param len The number of direct elements in the list.
 */
std::expected<Loaded, Error> LoadListValBytes(std::span<const std::byte> bytes,
                                              std::size_t len) noexcept;

//! Read a format byte (the first byte) from a buffer.
constexpr std::pair<Type, std::size_t> ReadFormatByte(
    const std::byte byte) noexcept {
    const auto type {
        static_cast<Type>(bit::GetBits(byte, type_bit_begin, type_bit_count))};
    const auto len_byte_count {
        bit::GetBits(byte, len_bit_begin, len_bit_count)};
    return {type, len_byte_count};
}

//! Read the length from a buffer.
constexpr std::optional<std::size_t> ReadLength(
    const std::span<const std::byte> bytes, const std::size_t count) noexcept {
    assert(IsNotExceedLengthByteCountRange(count));
    if (bytes.size() >= count) [[likely]] {
        std::size_t size {0};
        bit::ReadBytes(bytes.subspan(0, count), size, std::endian::big);
        return size;
    } else {
        return std::nullopt;
    }
}

/**
 * @brief Read the bytes of an item from a buffer.
 *
 * @param bytes A buffer.
 * @param len The size of the item in bytes.
 */
template <typename T>
    requires(!std::same_as<T, List>)
std::expected<Loaded, Error> LoadItemValBytes(
    const std::span<const std::byte> bytes, const std::size_t len) noexcept {
    using Value = std::ranges::range_value_t<T>;
    if (bytes.size() < len) [[unlikely]] {
        return std::unexpected {err::MakeIncompleteDataError()};
    }

    T vals;
    if (len % sizeof(Value) != 0) [[unlikely]] {
        return std::unexpected {
            err::MakeUnalignedLengthError(len, format_code<T>, sizeof(Value))};
    }

    const auto count {len / sizeof(Value)};
    if constexpr (std::ranges::contiguous_range<T>) {
        vals.reserve(count);
    }

    if constexpr (sizeof(Value) <= sizeof(std::byte)) {
        std::ranges::transform(
            bytes.subspan(0, count), std::back_inserter(vals),
            [](const auto byte) noexcept { return static_cast<Value>(byte); });
    } else {
        std::ranges::for_each(
            std::views::iota(static_cast<std::size_t>(0), count),
            [bytes, &vals](const auto i) noexcept {
                Value val;
                bit::ReadBytes(bytes.subspan(i * sizeof(Value)), val,
                               std::endian::big);
                vals.push_back(val);
            });
    }
    return std::pair {vals, len};
}

//! Read the value of an item or list from a buffer.
template <typename T>
std::expected<Loaded, Error> LoadValBytes(
    const std::span<const std::byte> bytes, const std::size_t len) noexcept {
    if constexpr (std::same_as<T, List>) {
        return LoadListValBytes(bytes, len);
    } else {
        return LoadItemValBytes<T>(bytes, len);
    }
}

template <>
std::expected<Loaded, Error> LoadItemValBytes<Boolean>(
    std::span<const std::byte> bytes, std::size_t len) noexcept;

}  // namespace secs2::byte::r