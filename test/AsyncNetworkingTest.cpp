#include <gtest/gtest.h>
#include "NetworkBuffer.h"
#include "ConnectionManager.h"
#include <winsock2.h>

using namespace core::net;

// ============ NetworkBuffer Tests ============

class NetworkBufferTest : public ::testing::Test {
protected:
    NetworkBuffer buffer{256};
};

TEST_F(NetworkBufferTest, WriteReadByte) {
    EXPECT_TRUE(buffer.write_byte(0xFF));
    
    buffer.reset_read();
    uint8_t value = 0;
    EXPECT_TRUE(buffer.read_byte(value));
    EXPECT_EQ(value, 0xFF);
}

TEST_F(NetworkBufferTest, WriteReadUint16) {
    uint16_t original = 0x1234;
    EXPECT_TRUE(buffer.write_uint16(original));
    
    buffer.reset_read();
    uint16_t value = 0;
    EXPECT_TRUE(buffer.read_uint16(value));
    EXPECT_EQ(value, original);
}

TEST_F(NetworkBufferTest, WriteReadUint32) {
    uint32_t original = 0x12345678;
    EXPECT_TRUE(buffer.write_uint32(original));
    
    buffer.reset_read();
    uint32_t value = 0;
    EXPECT_TRUE(buffer.read_uint32(value));
    EXPECT_EQ(value, original);
}

TEST_F(NetworkBufferTest, WriteReadData) {
    uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_TRUE(buffer.write(data, 10));
    
    buffer.reset_read();
    uint8_t read_data[10];
    EXPECT_TRUE(buffer.read(read_data, 10));
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(read_data[i], data[i]);
    }
}

TEST_F(NetworkBufferTest, AvailableSpace) {
    EXPECT_EQ(buffer.available_write(), 256);
    EXPECT_EQ(buffer.available_read(), 0);
    
    buffer.write_byte(0xFF);
    EXPECT_EQ(buffer.available_write(), 255);
    EXPECT_EQ(buffer.available_read(), 1);
}

TEST_F(NetworkBufferTest, OverflowWrite) {
    uint8_t large_data[300];
    EXPECT_FALSE(buffer.write(large_data, 300));
}

TEST_F(NetworkBufferTest, ReadMoreThanAvailable) {
    buffer.write_byte(0xFF);
    
    buffer.reset_read();
    uint8_t data[10];
    EXPECT_FALSE(buffer.read(data, 10));
}

TEST_F(NetworkBufferTest, ResetPositions) {
    buffer.write_uint32(0x12345678);
    
    uint32_t value1;
    EXPECT_TRUE(buffer.read_uint32(value1));
    
    buffer.reset_read();
    uint32_t value2;
    EXPECT_TRUE(buffer.read_uint32(value2));
    
    EXPECT_EQ(value1, value2);
}

TEST_F(NetworkBufferTest, ClearBuffer) {
    buffer.write_uint32(0x12345678);
    EXPECT_EQ(buffer.write_pos(), 4);
    
    buffer.clear();
    EXPECT_EQ(buffer.write_pos(), 0);
    EXPECT_EQ(buffer.read_pos(), 0);
}

// ============ ConnectionManager Tests ============

class ConnectionManagerTest : public ::testing::Test {
protected:
    ConnectionManager manager;
};

TEST_F(ConnectionManagerTest, AddConnection) {
    SOCKET mock_socket = (SOCKET)1001;
    auto handler = std::make_unique<ConnectionHandler>(mock_socket, "127.0.0.1", 1234);
    
    EXPECT_TRUE(manager.add_connection(mock_socket, std::move(handler)));
    EXPECT_EQ(manager.get_connection_count(), 1);
}

TEST_F(ConnectionManagerTest, DuplicateConnection) {
    SOCKET mock_socket = (SOCKET)1001;
    auto handler1 = std::make_unique<ConnectionHandler>(mock_socket, "127.0.0.1", 1234);
    auto handler2 = std::make_unique<ConnectionHandler>(mock_socket, "127.0.0.1", 1234);
    
    EXPECT_TRUE(manager.add_connection(mock_socket, std::move(handler1)));
    EXPECT_FALSE(manager.add_connection(mock_socket, std::move(handler2)));
}

TEST_F(ConnectionManagerTest, RemoveConnection) {
    SOCKET mock_socket = (SOCKET)1001;
    auto handler = std::make_unique<ConnectionHandler>(mock_socket, "127.0.0.1", 1234);
    
    manager.add_connection(mock_socket, std::move(handler));
    EXPECT_TRUE(manager.remove_connection(mock_socket));
    EXPECT_EQ(manager.get_connection_count(), 0);
}

TEST_F(ConnectionManagerTest, RemoveNonexistent) {
    SOCKET mock_socket = (SOCKET)1001;
    EXPECT_FALSE(manager.remove_connection(mock_socket));
}

TEST_F(ConnectionManagerTest, HasConnection) {
    SOCKET mock_socket = (SOCKET)1001;
    auto handler = std::make_unique<ConnectionHandler>(mock_socket, "127.0.0.1", 1234);
    
    EXPECT_FALSE(manager.has_connection(mock_socket));
    manager.add_connection(mock_socket, std::move(handler));
    EXPECT_TRUE(manager.has_connection(mock_socket));
}

TEST_F(ConnectionManagerTest, GetConnection) {
    SOCKET mock_socket = (SOCKET)1001;
    auto handler = std::make_unique<ConnectionHandler>(mock_socket, "127.0.0.1", 1234);
    
    manager.add_connection(mock_socket, std::move(handler));
    auto* retrieved = manager.get_connection(mock_socket);
    
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->get_socket(), mock_socket);
}

TEST_F(ConnectionManagerTest, GetAllSockets) {
    std::vector<SOCKET> sockets;
    for (int i = 0; i < 5; ++i) {
        SOCKET mock_socket = (SOCKET)(1000 + i);
        auto handler = std::make_unique<ConnectionHandler>(mock_socket, "127.0.0.1", 1234 + i);
        manager.add_connection(mock_socket, std::move(handler));
    }
    
    auto all_sockets = manager.get_all_sockets();
    EXPECT_EQ(all_sockets.size(), 5);
}

TEST_F(ConnectionManagerTest, CloseAll) {
    for (int i = 0; i < 5; ++i) {
        SOCKET mock_socket = (SOCKET)(1000 + i);
        auto handler = std::make_unique<ConnectionHandler>(mock_socket, "127.0.0.1", 1234 + i);
        manager.add_connection(mock_socket, std::move(handler));
    }
    
    EXPECT_EQ(manager.get_connection_count(), 5);
    manager.close_all();
    EXPECT_EQ(manager.get_connection_count(), 0);
}
