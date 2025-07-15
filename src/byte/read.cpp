#include "read.h"
#include "length.h"

#include <bit_manip/bit_manip.h>

#include <cassert>
#include <format>
#include <functional>
#include <ranges>
#include <unordered_map>

namespace secs2::byte::r {

namespace err {

Error MakeIncompleteDataError() noexcept {
    return {std::make_error_code(std::errc::message_size), "Incomplete data"};
}

Error MakeUnknownTypeError(const Type type) noexcept {
    return {std::make_error_code(std::errc::argument_out_of_domain),
            std::format("Unknown format type: 0x{:02X}",
                        static_cast<std::uint8_t>(type))};
}

Error MakeUnalignedLengthError(const std::size_t len, const Type type,
                               const std::size_t align) noexcept {
    return {std::make_error_code(std::errc::message_size),
            std::format("Length {} is not aligned to {} size {}", len,
                        to_string(type), align)};
}

Error MakeInvalidLengthByteCountError(const std::size_t count) noexcept {
    return {std::make_error_code(std::errc::argument_out_of_domain),
            std::format("Invalid number of length bytes: {}", count)};
}

}  // namespace err

std::expected<Loaded, Error> LoadBoolValBytes(
    const std::span<const std::byte> bytes, const std::size_t len) noexcept {
    if (bytes.size() < len) [[unlikely]] {
        return std::unexpected {err::MakeIncompleteDataError()};
    }

    Boolean vals;
    const auto count {len};
    std::ranges::for_each(std::views::iota(static_cast<std::size_t>(0), count),
                          [&vals, &bytes](const auto i) noexcept {
                              vals.push_back(static_cast<bool>(bytes[i]));
                          });
    return std::pair {vals, len};
}

std::expected<Loaded, Error> LoadListValBytes(
    const std::span<const std::byte> bytes, const std::size_t len) noexcept {
    List list;
    const auto count {len};
    list.reserve(count);

    std::size_t byte_size {0};
    for (std::size_t i {0}; i != count; ++i) {
        if (auto loaded {LoadMsgBytes(bytes.subspan(byte_size))};
            loaded.has_value()) [[likely]] {
            byte_size += loaded->second;
            list.push_back(std::move(loaded)->first);
        } else {
            return std::unexpected {loaded.error()};
        }
    }

    return std::pair {list, byte_size};
}

std::expected<Loaded, Error> LoadMsgBytes(
    const std::span<const std::byte> bytes) noexcept {
    using Loader = std::function<std::expected<Loaded, Error>(
        std::span<const std::byte>, std::size_t)>;
    static const std::unordered_map<Type, Loader> loaders {
        {Type::List, LoadValBytes<List>},
        {Type::Boolean, LoadValBytes<Boolean>},
        {Type::ASCII, LoadValBytes<ASCII>},
        {Type::Binary, LoadValBytes<Binary>},
        {Type::I1, LoadValBytes<I1>},
        {Type::I2, LoadValBytes<I2>},
        {Type::I4, LoadValBytes<I4>},
        {Type::I8, LoadValBytes<I8>},
        {Type::U1, LoadValBytes<U1>},
        {Type::U2, LoadValBytes<U2>},
        {Type::U4, LoadValBytes<U4>},
        {Type::U8, LoadValBytes<U8>},
        {Type::F4, LoadValBytes<F4>},
        {Type::F8, LoadValBytes<F8>}};

    if (bytes.empty()) [[unlikely]] {
        return std::unexpected {err::MakeIncompleteDataError()};
    }

    const auto [type, len_byte_count] {ReadFormatByte(bytes.front())};
    if (IsExceedLengthByteCountRange(len_byte_count)) [[unlikely]] {
        return std::unexpected {
            err::MakeInvalidLengthByteCountError(len_byte_count)};
    }

    const auto loader {loaders.find(type)};
    if (loader == loaders.cend()) [[unlikely]] {
        return std::unexpected {err::MakeUnknownTypeError(type)};
    }

    const auto len_bytes {bytes.subspan(1)};
    const auto len {ReadLength(len_bytes, len_byte_count)};
    if (len.has_value()) [[likely]] {
        const auto val_bytes {len_bytes.subspan(len_byte_count)};
        return loader->second(val_bytes, *len)
            .transform([len_byte_count](const auto& val) noexcept {
                return std::pair {
                    val.first, sizeof(std::byte) + len_byte_count + val.second};
            });
    } else {
        return std::unexpected {err::MakeIncompleteDataError()};
    }
}

template <>
std::expected<Loaded, Error> LoadItemValBytes<Boolean>(
    const std::span<const std::byte> bytes, const std::size_t len) noexcept {
    return LoadBoolValBytes(bytes, len);
}

}  // namespace secs2::byte::r