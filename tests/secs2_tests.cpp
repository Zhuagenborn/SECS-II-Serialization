#include "secs2/secs2.h"

#include <bit_manip/bit_manip.h>
#include <gtest/gtest.h>

#include <system_error>

using namespace secs2;

TEST(Secs2Message, GetType) {
    const Boolean bools {true, false};
    EXPECT_EQ(Message {bools}.GetType(), Type::Boolean);

    const ASCII str {"hello"};
    EXPECT_EQ(Message {str}.GetType(), Type::ASCII);

    List list;
    list.push_back(bools);
    list.push_back(str);
    EXPECT_EQ(Message {list}.GetType(), Type::List);
}

TEST(Secs2Message, GetValue) {
    {
        const Boolean bools {true, false};
        const Message msg {bools};
        EXPECT_EQ(msg.GetValue<List>(), std::nullopt);
        EXPECT_EQ(msg.GetValue<Boolean>(), bools);
    }
    {
        const Boolean bools {true, false};
        List list;
        list.push_back(bools);
        const Message msg {list};
        EXPECT_EQ(msg.GetValue<List>(), list);
        EXPECT_EQ(msg.GetValue<Boolean>(), std::nullopt);
        EXPECT_EQ(GetItemValue<Boolean>(list.front()), bools);
        EXPECT_EQ(GetItemValue<List>(list.front()), std::nullopt);
    }
}

TEST(Secs2Message, ToSml) {
    const I1 nums;
    EXPECT_EQ(Message {nums}.ToSml(), "<I1 [0]>");

    const Binary bins {static_cast<std::byte>(1), static_cast<std::byte>(2)};
    EXPECT_EQ(Message {bins}.ToSml(), "<B [2] 0x01 0x02>");

    const Boolean bools {true, false};
    EXPECT_EQ(Message {bools}.ToSml(), "<Boolean [2] true false>");

    const ASCII str {"hello"};
    EXPECT_EQ(Message {str}.ToSml(), "<A [5] \"hello\">");

    List list;
    EXPECT_EQ(Message {list}.ToSml(), "<L [0]\n>");

    list.push_back(nums);
    list.push_back(bins);
    EXPECT_EQ(Message {list}.ToSml(4),
              R"(<L [2]
    <I1 [0]>
    <B [2] 0x01 0x02>
>)");

    list.push_back(list);
    list.push_back(str);
    EXPECT_EQ(Message {list}.ToSml(4),
              R"(<L [4]
    <I1 [0]>
    <B [2] 0x01 0x02>
    <L [2]
        <I1 [0]>
        <B [2] 0x01 0x02>
    >
    <A [5] "hello">
>)");
}

TEST(Secs2Message, GetSize) {
    {
        const I4 nums;
        EXPECT_EQ(Message {nums}.GetSize(), nums.size());
    }
    {
        const I2 nums {1};
        EXPECT_EQ(Message {nums}.GetSize(), nums.size());
    }
    {
        const I8 nums {1, 2, 3};
        EXPECT_EQ(Message {nums}.GetSize(), nums.size());
    }
    {
        const Boolean bools {true, false};
        EXPECT_EQ(Message {bools}.GetSize(), bools.size());
    }
    {
        List list;
        EXPECT_EQ(Message {list}.GetSize(), list.size());

        list.push_back(list);
        list.push_back(I1 {1, 2, 3});
        EXPECT_EQ(Message {list}.GetSize(), list.size());
    }
}

TEST(Secs2Message, ToBytes) {
    {
        static_assert(static_cast<std::uint8_t>(Type::Binary) == 0b001000);

        const Binary bins;
        const std::vector<std::byte> target {
            static_cast<std::byte>(0b001000'01), static_cast<std::byte>(0)};
        const auto bytes {
            Message {bins}.ToBytes().value_or(std::vector<std::byte> {})};
        EXPECT_EQ(bytes, target);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::Boolean) == 0b001001);

        const Boolean bools {true, false};
        const std::vector<std::byte> target {
            static_cast<std::byte>(0b001001'01),
            static_cast<std::byte>(bools.size()
                                   * sizeof(std::ranges::range_value_t<U1>)),
            static_cast<std::byte>(true), static_cast<std::byte>(false)};
        const auto bytes {
            Message {bools}.ToBytes().value_or(std::vector<std::byte> {})};
        EXPECT_EQ(bytes, target);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U1) == 0b101001);

        const U1 nums;
        const std::vector<std::byte> target {
            static_cast<std::byte>(0b101001'01),
            static_cast<std::byte>(nums.size()
                                   * sizeof(std::ranges::range_value_t<U1>))};
        const auto bytes {
            Message {nums}.ToBytes().value_or(std::vector<std::byte> {})};
        EXPECT_EQ(bytes, target);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U1) == 0b101001);

        const U1 nums {1, 2, 3, 4};
        const std::vector<std::byte> target {
            static_cast<std::byte>(0b101001'01),
            static_cast<std::byte>(nums.size()
                                   * sizeof(std::ranges::range_value_t<U1>)),
            static_cast<std::byte>(1),
            static_cast<std::byte>(2),
            static_cast<std::byte>(3),
            static_cast<std::byte>(4)};
        const auto bytes {
            Message {nums}.ToBytes().value_or(std::vector<std::byte> {})};
        EXPECT_EQ(bytes, target);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U1) == 0b101001);

        const U1 nums(std::numeric_limits<std::uint8_t>::max() + 1, 0xFF);
        std::vector<std::byte> target {static_cast<std::byte>(0b101001'10),
                                       static_cast<std::byte>(1),
                                       static_cast<std::byte>(0)};
        target.insert(target.end(), nums.size(), static_cast<std::byte>(0xFF));
        const auto bytes {
            Message {nums}.ToBytes().value_or(std::vector<std::byte> {})};
        EXPECT_EQ(bytes, target);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U2) == 0b101010);

        const U2 nums {1, 2, 3, 4};
        const std::vector<std::byte> target {
            static_cast<std::byte>(0b101010'01),
            static_cast<std::byte>(nums.size()
                                   * sizeof(std::ranges::range_value_t<U2>)),
            static_cast<std::byte>(0),
            static_cast<std::byte>(1),
            static_cast<std::byte>(0),
            static_cast<std::byte>(2),
            static_cast<std::byte>(0),
            static_cast<std::byte>(3),
            static_cast<std::byte>(0),
            static_cast<std::byte>(4)};
        const auto bytes {
            Message {nums}.ToBytes().value_or(std::vector<std::byte> {})};
        EXPECT_EQ(bytes, target);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U1) == 0b101001);
        static_assert(static_cast<std::uint8_t>(Type::ASCII) == 0b010000);
        static_assert(static_cast<std::uint8_t>(Type::List) == 0b000000);

        const U1 nums {1, 2};
        const ASCII str {"msg"};

        List list;
        list.push_back(nums);
        list.push_back(list);
        list.push_back(str);
        const std::vector<std::byte> target {
            static_cast<std::byte>(0b000000'01),
            static_cast<std::byte>(list.size()),
            static_cast<std::byte>(0b101001'01),
            static_cast<std::byte>(nums.size()),
            static_cast<std::byte>(1),
            static_cast<std::byte>(2),
            static_cast<std::byte>(0b000000'01),
            static_cast<std::byte>(1),
            static_cast<std::byte>(0b101001'01),
            static_cast<std::byte>(nums.size()),
            static_cast<std::byte>(1),
            static_cast<std::byte>(2),
            static_cast<std::byte>(0b010000'01),
            static_cast<std::byte>(str.size()),
            static_cast<std::byte>('m'),
            static_cast<std::byte>('s'),
            static_cast<std::byte>('g')};
        const auto bytes {
            Message {list}.ToBytes().value_or(std::vector<std::byte> {})};
        EXPECT_EQ(bytes, target);
    }
    {
        U1 nums(Message::max_length + 1, 0);
        EXPECT_FALSE(Message {nums}.ToBytes().has_value());

        List list;
        list.push_back(std::move(nums));
        EXPECT_FALSE(Message {list}.ToBytes().has_value());
    }
}

TEST(Secs2Message, BuildMsgFromBytes) {
    {
        const auto loaded {BuildMsgFromBytes({})};
        EXPECT_FALSE(loaded.has_value());
        EXPECT_EQ(loaded.error().first, std::errc::message_size);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U2) == 0b101010);

        const std::vector<std::byte> bytes {static_cast<std::byte>(0b101010'01),
                                            static_cast<std::byte>(3)};
        const auto loaded {BuildMsgFromBytes(bytes)};
        EXPECT_FALSE(loaded.has_value());
        EXPECT_EQ(loaded.error().first, std::errc::message_size);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U2) == 0b101010);

        const std::vector<std::byte> bytes {
            static_cast<std::byte>(0b101010'00)};
        const auto loaded {BuildMsgFromBytes(bytes)};
        EXPECT_FALSE(loaded.has_value());
        EXPECT_EQ(loaded.error().first, std::errc::argument_out_of_domain);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U2) == 0b101010);

        const std::vector<std::byte> bytes {static_cast<std::byte>(0b101010'01),
                                            static_cast<std::byte>(4)};
        const auto loaded {BuildMsgFromBytes(bytes)};
        EXPECT_FALSE(loaded.has_value());
        EXPECT_EQ(loaded.error().first, std::errc::message_size);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::Unknown) == 0b111111);

        const std::vector<std::byte> bytes {static_cast<std::byte>(0b111111'01),
                                            static_cast<std::byte>(1)};
        auto padded_bytes {bytes};
        padded_bytes.insert(padded_bytes.end(), 10,
                            static_cast<std::byte>(0xFF));
        const auto loaded {BuildMsgFromBytes(padded_bytes)};
        EXPECT_FALSE(loaded.has_value());
        EXPECT_EQ(loaded.error().first, std::errc::argument_out_of_domain);
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::Boolean) == 0b001001);

        const Boolean target {true, true, false};
        const std::vector<std::byte> bytes {
            static_cast<std::byte>(0b001001'01), static_cast<std::byte>(3),
            static_cast<std::byte>(true), static_cast<std::byte>(0xFF),
            static_cast<std::byte>(false)};
        auto padded_bytes {bytes};
        padded_bytes.insert(padded_bytes.end(), 10,
                            static_cast<std::byte>(0xFF));
        const auto loaded {BuildMsgFromBytes(padded_bytes)};
        EXPECT_TRUE(loaded.has_value());
        EXPECT_EQ(loaded->first, Message {target});
        EXPECT_EQ(loaded->second, bytes.size());
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::List) == 0b000000);

        const List target;
        const std::vector<std::byte> bytes {static_cast<std::byte>(0b000000'01),
                                            static_cast<std::byte>(0)};
        auto padded_bytes {bytes};
        padded_bytes.insert(padded_bytes.end(), 10,
                            static_cast<std::byte>(0xFF));
        const auto loaded {BuildMsgFromBytes(padded_bytes)};
        EXPECT_TRUE(loaded.has_value());
        EXPECT_EQ(loaded->first, Message {target});
        EXPECT_EQ(loaded->second, bytes.size());
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U2) == 0b101010);

        const U2 target {1, 2, 3, 4};
        const std::vector<std::byte> bytes {
            static_cast<std::byte>(0b101010'01),
            static_cast<std::byte>(target.size()
                                   * sizeof(std::ranges::range_value_t<U2>)),
            static_cast<std::byte>(0),
            static_cast<std::byte>(1),
            static_cast<std::byte>(0),
            static_cast<std::byte>(2),
            static_cast<std::byte>(0),
            static_cast<std::byte>(3),
            static_cast<std::byte>(0),
            static_cast<std::byte>(4)};
        auto padded_bytes {bytes};
        padded_bytes.insert(padded_bytes.end(), 10,
                            static_cast<std::byte>(0xFF));
        const auto loaded {BuildMsgFromBytes(padded_bytes)};
        EXPECT_TRUE(loaded.has_value());
        EXPECT_EQ(loaded->first, Message {target});
        EXPECT_EQ(loaded->second, bytes.size());
    }
    {
        static_assert(static_cast<std::uint8_t>(Type::U1) == 0b101001);
        static_assert(static_cast<std::uint8_t>(Type::ASCII) == 0b010000);
        static_assert(static_cast<std::uint8_t>(Type::List) == 0b000000);

        const U1 nums {1, 2};
        const ASCII str {"msg"};

        List target;
        target.push_back(nums);
        target.push_back(target);
        target.push_back(str);
        target.push_back(U1 {});
        const std::vector<std::byte> bytes {
            static_cast<std::byte>(0b000000'01),
            static_cast<std::byte>(target.size()),
            static_cast<std::byte>(0b101001'01),
            static_cast<std::byte>(nums.size()),
            static_cast<std::byte>(1),
            static_cast<std::byte>(2),
            static_cast<std::byte>(0b000000'01),
            static_cast<std::byte>(1),
            static_cast<std::byte>(0b101001'01),
            static_cast<std::byte>(nums.size()),
            static_cast<std::byte>(1),
            static_cast<std::byte>(2),
            static_cast<std::byte>(0b010000'01),
            static_cast<std::byte>(str.size()),
            static_cast<std::byte>('m'),
            static_cast<std::byte>('s'),
            static_cast<std::byte>('g'),
            static_cast<std::byte>(0b101001'01),
            static_cast<std::byte>(0)};
        auto padded_bytes {bytes};
        padded_bytes.insert(padded_bytes.end(), 10,
                            static_cast<std::byte>(0xFF));
        const auto loaded {BuildMsgFromBytes(padded_bytes)};
        EXPECT_TRUE(loaded.has_value());
        EXPECT_EQ(loaded->first, Message {target});
        EXPECT_EQ(loaded->second, bytes.size());
    }
}