#include "secs2.h"
#include "byte/read.h"
#include "byte/write.h"
#include "sml.h"
#include "traits.h"

#include <sstream>
#include <unordered_map>

namespace secs2 {

Message::Message(Value val) noexcept : val_ {std::move(val)} {}

std::size_t Message::GetSize() const noexcept {
    const Overload visitor {
        [](const List& list) noexcept { return list.size(); },
        [](const Item& item) noexcept {
            return std::visit(
                [](const auto& raw) noexcept { return raw.size(); }, item);
        },
    };
    return std::visit(visitor, val_);
}

Type Message::GetType() const noexcept {
    return secs2::GetType(val_);
}

const Message::Value& Message::GetValue() const noexcept {
    return val_;
}

std::optional<std::vector<std::byte>> Message::ToBytes() const noexcept {
    std::vector<std::byte> buf;
    buf.reserve(GetSize());
    if (byte::w::BuildMsgBytes(val_, buf).has_value()) [[likely]] {
        return buf;
    } else {
        return std::nullopt;
    }
}

std::string Message::ToSml(const std::size_t indent_width) const noexcept {
    std::ostringstream os;
    sml::BuildSml(os, val_, 0, indent_width);
    return os.str();
}

std::expected<DeserializedMessage, Error> Message::BuildFromBytes(
    const std::span<const std::byte> bytes) noexcept {
    return BuildMsgFromBytes(bytes);
}

std::expected<DeserializedMessage, Error> BuildMsgFromBytes(
    const std::span<const std::byte> bytes) noexcept {
    return byte::r::LoadMsgBytes(bytes).transform([](const auto& val) noexcept {
        return std::pair {Message {val.first}, val.second};
    });
}

std::ostream& operator<<(std::ostream& os, const Message& msg) noexcept {
    os << msg.ToSml();
    return os;
}

std::ostream& operator<<(std::ostream& os, const Type type) noexcept {
    os << to_string(type);
    return os;
}

std::string to_string(const Message& msg) noexcept {
    return msg.ToSml();
}

std::string to_string(const Type type) noexcept {
    static const std::unordered_map<Type, std::string_view> types {
        {Type::List, "List"},       {Type::Binary, "Binary"},
        {Type::Boolean, "Boolean"}, {Type::ASCII, "ASCII"},
        {Type::F4, "F4"},           {Type::F8, "F8"},
        {Type::I1, "I1"},           {Type::I2, "I2"},
        {Type::I4, "I4"},           {Type::I8, "I8"},
        {Type::U1, "U1"},           {Type::U2, "U2"},
        {Type::U4, "U4"},           {Type::U8, "U8"},
        {Type::Unknown, "Unknown"}};
    assert(types.contains(type));
    [[assume(types.contains(type))]];
    return types.contains(type) ? types.at(type).data()
                                : types.at(Type::Unknown).data();
}

void Message::swap(Message& msg) noexcept {
    std::ranges::swap(val_, msg.val_);
}

void swap(Message& lhs, Message& rhs) noexcept {
    lhs.swap(rhs);
}

}  // namespace secs2

auto std::formatter<secs2::Message>::format(
    const secs2::Message& msg, std::format_context& ctx) const noexcept {
    return std::formatter<std::string>::format(
        indent_width_.has_value() ? msg.ToSml(*indent_width_) : msg.ToSml(),
        ctx);
}

auto std::formatter<secs2::Type>::format(
    const secs2::Type type, std::format_context& ctx) const noexcept {
    return std::formatter<std::string>::format(secs2::to_string(type), ctx);
}