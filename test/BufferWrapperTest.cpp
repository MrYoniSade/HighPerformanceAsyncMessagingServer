#include <gtest/gtest.h>
#include "BufferWrapper.h"

using namespace core;

class BufferWrapperTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Clean up any test buffers
    }
};

TEST_F(BufferWrapperTest, ConstructorAndSize) {
    BufferWrapper<uint8_t> buffer(100);
    EXPECT_EQ(buffer.size(), 100);
    EXPECT_TRUE(buffer.is_valid());
}

TEST_F(BufferWrapperTest, DataAccess) {
    BufferWrapper<int> buffer(10);
    buffer[0] = 42;
    buffer[9] = 99;
    
    EXPECT_EQ(buffer[0], 42);
    EXPECT_EQ(buffer[9], 99);
}

TEST_F(BufferWrapperTest, OutOfRangeAccess) {
    BufferWrapper<int> buffer(10);
    EXPECT_THROW(buffer[10], std::out_of_range);
}

TEST_F(BufferWrapperTest, Fill) {
    BufferWrapper<int> buffer(5);
    buffer.fill(7);
    
    for (size_t i = 0; i < buffer.size(); ++i) {
        EXPECT_EQ(buffer[i], 7);
    }
}

TEST_F(BufferWrapperTest, Clear) {
    BufferWrapper<int> buffer(5);
    buffer.fill(42);
    buffer.clear();
    
    for (size_t i = 0; i < buffer.size(); ++i) {
        EXPECT_EQ(buffer[i], 0);
    }
}

TEST_F(BufferWrapperTest, MoveSemantics) {
    BufferWrapper<int> buffer1(10);
    buffer1[0] = 123;
    
    BufferWrapper<int> buffer2 = std::move(buffer1);
    
    EXPECT_FALSE(buffer1.is_valid());
    EXPECT_TRUE(buffer2.is_valid());
    EXPECT_EQ(buffer2[0], 123);
}

TEST_F(BufferWrapperTest, MoveAssignment) {
    BufferWrapper<int> buffer1(10);
    BufferWrapper<int> buffer2(5);
    
    buffer2 = std::move(buffer1);
    
    EXPECT_EQ(buffer2.size(), 10);
}

TEST_F(BufferWrapperTest, DataPointer) {
    BufferWrapper<uint8_t> buffer(5);
    uint8_t* ptr = buffer.data();
    
    EXPECT_NE(ptr, nullptr);
    ptr[0] = 42;
    EXPECT_EQ(buffer[0], 42);
}

TEST_F(BufferWrapperTest, ConstDataPointer) {
    BufferWrapper<uint8_t> buffer(5);
    buffer[0] = 99;
    
    const uint8_t* ptr = buffer.data();
    EXPECT_EQ(*ptr, 99);
}

TEST_F(BufferWrapperTest, InvalidSize) {
    EXPECT_THROW(BufferWrapper<int>(0), std::invalid_argument);
}

TEST_F(BufferWrapperTest, LargeBuffer) {
    BufferWrapper<uint32_t> buffer(1000000);
    EXPECT_EQ(buffer.size(), 1000000);
    EXPECT_TRUE(buffer.is_valid());
}
