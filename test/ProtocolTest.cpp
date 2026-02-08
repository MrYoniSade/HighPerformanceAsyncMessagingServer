#include <gtest/gtest.h>
#include "BinaryProtocol.h"
#include "MessageSerializer.h"
#include "BitPackUtils.h"
#include "EndianUtils.h"
#include "HandlerRegistry.h"
#include "ProtocolMessages.h"

using namespace core::protocol;

// ============ BinaryProtocol Tests ============

class BinaryProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
};

TEST_F(BinaryProtocolTest, FrameHeaderValidation) {
    FrameHeader header;
    header.magic = PROTOCOL_MAGIC;
    header.version = PROTOCOL_VERSION;
    header.message_type = 0x01;
    header.flags = 0x00;
    header.payload_length = 64;
    header.reserved = 0;
    
    EXPECT_TRUE(header.is_valid());
}

TEST_F(BinaryProtocolTest, FrameFlags) {
    FrameHeader header;
    header.magic = PROTOCOL_MAGIC;
    header.version = PROTOCOL_VERSION;
    header.flags = 0;
    
    EXPECT_FALSE(header.has_flag(FrameFlags::ACK_REQUIRED));
    
    header.set_flag(FrameFlags::ACK_REQUIRED);
    EXPECT_TRUE(header.has_flag(FrameFlags::ACK_REQUIRED));
    
    header.clear_flag(FrameFlags::ACK_REQUIRED);
    EXPECT_FALSE(header.has_flag(FrameFlags::ACK_REQUIRED));
}

TEST_F(BinaryProtocolTest, CRC32Calculation) {
    uint8_t data[] = {1, 2, 3, 4, 5};
    uint32_t crc1 = crc32::calculate(data, 5);
    uint32_t crc2 = crc32::calculate(data, 5);
    
    EXPECT_EQ(crc1, crc2);
    EXPECT_NE(crc1, 0);
}

// ============ EndianUtils Tests ============

class EndianUtilsTest : public ::testing::Test {
};

TEST_F(EndianUtilsTest, ByteOrderDetection) {
    // Just verify it compiles and returns something
    auto order = EndianUtils::SYSTEM_BYTE_ORDER;
    EXPECT_TRUE(EndianUtils::IS_LITTLE_ENDIAN || EndianUtils::IS_BIG_ENDIAN);
}

TEST_F(EndianUtilsTest, Swap16) {
    uint16_t value = 0x1234;
    uint16_t swapped = EndianUtils::swap_uint16(value);
    EXPECT_EQ(swapped, 0x3412);
}

TEST_F(EndianUtilsTest, Swap32) {
    uint32_t value = 0x12345678;
    uint32_t swapped = EndianUtils::swap_uint32(value);
    EXPECT_EQ(swapped, 0x78563412);
}

TEST_F(EndianUtilsTest, LittleEndianConversion) {
    uint16_t value = 0x1234;
    uint16_t le = EndianUtils::to_little_endian(value);
    uint16_t back = EndianUtils::from_little_endian(le);
    EXPECT_EQ(back, value);
}

// ============ BitPackUtils Tests ============

class BitPackUtilsTest : public ::testing::Test {
protected:
    uint8_t buffer[64] = {0};
};

TEST_F(BitPackUtilsTest, PackUnpackBits) {
    size_t offset = 0;
    offset = BitPackUtils::pack_bits(buffer, offset, 0b1010, 4);
    offset = BitPackUtils::pack_bits(buffer, offset, 0b1100, 4);
    
    size_t read_offset = 0;
    uint32_t val1 = BitPackUtils::unpack_bits(buffer, read_offset, 4);
    read_offset += 4;
    uint32_t val2 = BitPackUtils::unpack_bits(buffer, read_offset, 4);
    
    EXPECT_EQ(val1, 0b1010);
    EXPECT_EQ(val2, 0b1100);
}

TEST_F(BitPackUtilsTest, PackUnpackBool) {
    size_t offset = 0;
    offset = BitPackUtils::pack_bool(buffer, offset, true);
    offset = BitPackUtils::pack_bool(buffer, offset, false);
    
    size_t read_offset = 0;
    bool val1 = BitPackUtils::unpack_bool(buffer, read_offset);
    read_offset += 1;
    bool val2 = BitPackUtils::unpack_bool(buffer, read_offset);
    
    EXPECT_TRUE(val1);
    EXPECT_FALSE(val2);
}

TEST_F(BitPackUtilsTest, PackUnpackUint16) {
    size_t offset = 0;
    offset = BitPackUtils::pack_uint16(buffer, offset, 0x1234);
    offset = BitPackUtils::pack_uint16(buffer, offset, 0x5678);
    
    uint16_t val1 = BitPackUtils::unpack_uint16(buffer, 0);
    uint16_t val2 = BitPackUtils::unpack_uint16(buffer, 16);
    
    EXPECT_EQ(val1, 0x1234);
    EXPECT_EQ(val2, 0x5678);
}

TEST_F(BitPackUtilsTest, PackUnpackUint32) {
    size_t offset = 0;
    offset = BitPackUtils::pack_uint32(buffer, offset, 0x12345678);
    
    uint32_t val = BitPackUtils::unpack_uint32(buffer, 0);
    EXPECT_EQ(val, 0x12345678);
}

// ============ HandlerRegistry Tests ============

class HandlerRegistryTest : public ::testing::Test {
protected:
    HandlerRegistry registry;
};

TEST_F(HandlerRegistryTest, RegisterHandler) {
    auto handler = std::make_unique<messages::PingHandler>([](const messages::PingMessage&) {
        return true;
    });
    
    EXPECT_TRUE(registry.register_handler(std::move(handler)));
    EXPECT_TRUE(registry.has_handler(MessageType::PING));
}

TEST_F(HandlerRegistryTest, DuplicateRegistration) {
    auto handler1 = std::make_unique<messages::PingHandler>([](const messages::PingMessage&) {
        return true;
    });
    auto handler2 = std::make_unique<messages::PingHandler>([](const messages::PingMessage&) {
        return true;
    });
    
    EXPECT_TRUE(registry.register_handler(std::move(handler1)));
    EXPECT_FALSE(registry.register_handler(std::move(handler2)));
}

TEST_F(HandlerRegistryTest, UnregisterHandler) {
    auto handler = std::make_unique<messages::PingHandler>([](const messages::PingMessage&) {
        return true;
    });
    
    registry.register_handler(std::move(handler));
    EXPECT_TRUE(registry.has_handler(MessageType::PING));
    EXPECT_TRUE(registry.unregister_handler(MessageType::PING));
    EXPECT_FALSE(registry.has_handler(MessageType::PING));
}

TEST_F(HandlerRegistryTest, GetHandler) {
    auto handler = std::make_unique<messages::PingHandler>([](const messages::PingMessage&) {
        return true;
    });
    
    registry.register_handler(std::move(handler));
    auto* retrieved = registry.get_handler(MessageType::PING);
    
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->get_message_type(), MessageType::PING);
}

TEST_F(HandlerRegistryTest, HandlerCount) {
    EXPECT_EQ(registry.handler_count(), 0);
    
    registry.register_handler(std::make_unique<messages::PingHandler>([](const messages::PingMessage&) {
        return true;
    }));
    
    EXPECT_EQ(registry.handler_count(), 1);
    
    registry.register_handler(std::make_unique<messages::PongHandler>([](const messages::PongMessage&) {
        return true;
    }));
    
    EXPECT_EQ(registry.handler_count(), 2);
}
