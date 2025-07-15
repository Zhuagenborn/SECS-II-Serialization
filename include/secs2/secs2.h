/**
 * @file secs2.h
 * @brief Serialization, deserialization and SML (SECS Message Language) representation of SECS-II data.
 *
 * @par GitHub
 * https://github.com/Zhuagenborn
 *
 * @date 2025-04-18
 *
 * @example tests/secs2_tests.cpp
 */

#pragma once

#include <cstdint>
#include <deque>
#include <expected>
#include <format>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

namespace secs2 {

//! The types and format codes of SECS-II data.
enum class Type : std::uint8_t {
    //! List of SECS-II items.
    List = 0b000000,

    //! Binary bytes.
    Binary = 0b001000,

    //! Boolean values.
    Boolean = 0b001001,

    //! An ASCII character string.
    ASCII = 0b010000,

    //! 1-byte signed integers.
    I1 = 0b011001,
    //! 2-byte signed integers.
    I2 = 0b011010,
    //! 4-byte signed integers.
    I4 = 0b011100,
    //! 8-byte signed integers.
    I8 = 0b011000,

    //! 1-byte unsigned integers.
    U1 = 0b101001,
    //! 2-byte unsigned integers.
    U2 = 0b101010,
    //! 4-byte unsigned integers.
    U4 = 0b101100,
    //! 8-byte unsigned integers.
    U8 = 0b101000,

    //! 4-byte floating points.
    F4 = 0b100100,
    //! 8-byte floating points.
    F8 = 0b100000,

    //! Unrecognized types. It is not part of the SECS-II standard.
    Unknown = 0b111111
};

//! Binary bytes.
using Binary = std::vector<std::byte>;

//! Boolean values.
using Boolean = std::deque<bool>;

//! An ASCII character string.
using ASCII = std::string;

//! 1-byte signed integers.
using I1 = std::vector<std::int8_t>;
//! 2-byte signed integers.
using I2 = std::vector<std::int16_t>;
//! 4-byte signed integers.
using I4 = std::vector<std::int32_t>;
//! 8-byte signed integers.
using I8 = std::vector<std::int64_t>;

//! 1-byte unsigned integers.
using U1 = std::vector<std::uint8_t>;
//! 2-byte unsigned integers.
using U2 = std::vector<std::uint16_t>;
//! 4-byte unsigned integers.
using U4 = std::vector<std::uint32_t>;
//! 8-byte unsigned integers.
using U8 = std::vector<std::uint64_t>;

//! 4-byte floating points.
using F4 = std::vector<float>;
//! 8-byte floating points.
using F8 = std::vector<double>;

//! A single SECS-II item, excluding lists.
using Item = std::variant<Binary, ASCII, Boolean, I1, I2, I4, I8, U1, U2, U4,
                          U8, F4, F8>;

class List;

//! An element in a SECS-II list, which can be an item or another list.
using ListElem = std::variant<Item, List>;

//! A SECS-II list that can contain nested items and lists.
class List : public std::vector<ListElem> {};

//! Get the raw value of a SECS-II item.
template <typename T>
std::optional<T> GetItemValue(const ListElem& elem) noexcept {
    if constexpr (std::same_as<T, List>) {
        if (std::holds_alternative<List>(elem)) {
            return std::get<List>(elem);
        }
    } else {
        if (std::holds_alternative<Item>(elem)) {
            if (const auto& item {std::get<Item>(elem)};
                std::holds_alternative<T>(item)) {
                return std::get<T>(item);
            }
        }
    }
    return std::nullopt;
}

//! The error with an error code and a human-readable message.
using Error = std::pair<std::error_code, std::string>;

class Message;

//! The deserialized message from bytes and the number of bytes consumed.
using DeserializedMessage = std::pair<Message, std::size_t>;

//! A SECS-II message containing a single item or a list of items.
class Message {
public:
    /**
     * @brief
     * The maximum allowed length when serializing a message to bytes.
     *
     * @details
     * - For an item, it is the maximum number of bytes.
     * - For an list, it is the maximum number of elements that is counted in terms of its direct elements only
     * and does not take into account any nested linked lists within the elements.
     */
    static constexpr std::size_t max_length {0xFFFFFF};

    //! A SECS-II value which can be a single item or a list.
    using Value = ListElem;

    /**
     * @brief Deserialize a message from a sequence of bytes.
     *
     * @return
     * A pair consisting of the deserialized message and the number of bytes consumed if successful.
     * Otherwise a pair with an error code and a descriptive error message.
     * - @p std::errc::message_size
     *   - The input buffer does not contain complete data.
     *   - The length is not aligned to the element size.
     * - @p std::errc::argument_out_of_domain
     *   - The format type is unknown.
     *   - The number of length bytes is incorrect.
     */
    static std::expected<DeserializedMessage, Error> BuildFromBytes(
        std::span<const std::byte>) noexcept;

    //! Construct a message from a given value.
    explicit Message(Value val) noexcept;

    //! Get the type of the stored value.
    Type GetType() const noexcept;

    /**
     * @brief Gets the number of elements in the stored value.
     *
     * @note
     * For an list, it is the number of elements that is counted in terms of its direct elements only
     * and does not take into account any nested linked lists within the elements.
     */
    std::size_t GetSize() const noexcept;

    //! Get the stored value.
    const Value& GetValue() const noexcept;

    //! Get the raw stored value.
    template <typename T>
    std::optional<T> GetValue() const noexcept {
        return GetItemValue<T>(val_);
    }

    /**
     * @brief Serialize the message to a sequence of bytes.
     *
     * @details
     * The format of SECS-II messages is:
     *
     * ```
     * 7  6  5  4  3  2          1           0
     * ┌──────────────┬────────────────────────┐
     * │  Format Code │ Number of Length Bytes │
     * ├──────────────┴────────────────────────┤
     * │             Length Byte 1             │ MS Byte
     * ├───────────────────────────────────────┤
     * │             Length Byte 2             │
     * ├───────────────────────────────────────┤
     * │             Length Byte 3             │ LS Byte
     * ├───────────────────────────────────────┤
     * │               Item Body               │
     * └───────────────────────────────────────┘
     * ```
     *
     * The number of length bytes is variable, from 1 to 3. The most significant byte is the first.
     *
     * The length has different meanings depending on the data type:
     * - For an item, it is the maximum number of bytes.
     * - For an list, it is the maximum number of elements that is counted in terms of its direct elements only
     * and does not take into account any nested linked lists within the elements.
     *
     * @return
     * A sequence of bytes if the length does not exceed @ref max_length,
     * otherwise @p std::nullopt.
     */
    std::optional<std::vector<std::byte>> ToBytes() const noexcept;

    /**
     * @brief Format the message to a SML (SECS Message Language) string.
     *
     * @param indent_width The number of spaces to use for indentation.
     */
    std::string ToSml(std::size_t indent_width = 4) const noexcept;

    //! Swaps this message with another.
    void swap(Message&) noexcept;

    //! Equality comparison between two messages.
    bool operator==(const Message&) const noexcept = default;

private:
    Value val_;
};

//! Same as @ref Message::swap.
void swap(Message&, Message&) noexcept;

//! Same as @ref Message::BuildFromBytes.
std::expected<DeserializedMessage, Error> BuildMsgFromBytes(
    std::span<const std::byte>) noexcept;

//! Get the raw value of a SECS-II message.
template <typename T>
std::optional<T> GetMsgValue(const Message::Value& val) noexcept {
    return GetItemValue<T>(val);
}

//! Output the SML (SECS Message Language) string of a SECS-II message to an output stream.
std::ostream& operator<<(std::ostream&, const Message&) noexcept;

//! Output the string of a SECS-II type to an output stream.
std::ostream& operator<<(std::ostream&, Type) noexcept;

//! Same as @ref Message::ToSml.
std::string to_string(const Message&) noexcept;

//! Format a SECS-II type to a string.
std::string to_string(Type) noexcept;

}  // namespace secs2

/**
 * @brief The string formatter for SECS-II messages.
 *
 * @details
 * The format specification has the form @p {indent_width}.
 *
 * @param indent_width The number of spaces to use for indentation (optional).
 */
template <>
class std::formatter<secs2::Message> : std::formatter<std::string> {
public:
    constexpr auto parse(std::format_parse_context& ctx) noexcept {
        const auto last_format {ctx.end() - sizeof('}') / sizeof(char)};

        try {
            const std::string_view arg {ctx.begin(), last_format};
            indent_width_ = std::stoul(arg.data());
        } catch (const std::exception&) {
        }

        return last_format;
    }

    auto format(const secs2::Message&, std::format_context&) const noexcept;

private:
    std::optional<std::size_t> indent_width_;
};

//! The string formatter for SECS-II types.
template <>
class std::formatter<secs2::Type> : std::formatter<std::string> {
public:
    auto format(secs2::Type, std::format_context&) const noexcept;
};