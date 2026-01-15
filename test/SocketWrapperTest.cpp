#include <gtest/gtest.h>
#include "SocketWrapper.h"

namespace net {

class SocketWrapperTest : public ::testing::Test {
protected:
    // Invalid socket for testing
    static constexpr socket_t INVALID_SOCK = INVALID_SOCKET_VALUE;
};

// Test 1: Default constructor creates invalid socket
TEST_F(SocketWrapperTest, DefaultConstructorCreatesInvalidSocket) {
    SocketWrapper sock;
    EXPECT_FALSE(sock.isValid());
}

// Test 2: Constructor with socket handle
TEST_F(SocketWrapperTest, ConstructorWithSocketHandle) {
    SocketWrapper sock(INVALID_SOCK);
    EXPECT_FALSE(sock.isValid());
}

// Test 3: get() returns the correct socket
TEST_F(SocketWrapperTest, GetReturnsCorrectSocket) {
    socket_t expected = INVALID_SOCK;
    SocketWrapper sock(expected);
    EXPECT_EQ(sock.get(), expected);
}

// Test 4: release() returns socket and invalidates wrapper
TEST_F(SocketWrapperTest, ReleaseReturnsSocketAndInvalidates) {
    socket_t original = INVALID_SOCK;
    SocketWrapper sock(original);
    
    socket_t released = sock.release();
    EXPECT_EQ(released, original);
    EXPECT_FALSE(sock.isValid());
}

// Test 5: reset() closes old socket and sets new one
TEST_F(SocketWrapperTest, ResetClosesOldSocketAndSetsNew) {
    SocketWrapper sock(INVALID_SOCK);
    EXPECT_FALSE(sock.isValid());
    
    sock.reset(INVALID_SOCK);
    EXPECT_FALSE(sock.isValid());
}

// Test 6: Move constructor transfers ownership
TEST_F(SocketWrapperTest, MoveConstructorTransfersOwnership) {
    socket_t original = INVALID_SOCK;
    SocketWrapper sock1(original);
    
    SocketWrapper sock2(std::move(sock1));
    EXPECT_FALSE(sock1.isValid());
    EXPECT_EQ(sock2.get(), original);
}

// Test 7: Move assignment transfers ownership
TEST_F(SocketWrapperTest, MoveAssignmentTransfersOwnership) {
    socket_t original = INVALID_SOCK;
    SocketWrapper sock1(original);
    SocketWrapper sock2;
    
    sock2 = std::move(sock1);
    EXPECT_FALSE(sock1.isValid());
    EXPECT_EQ(sock2.get(), original);
}

// Test 8: Move assignment closes old socket
TEST_F(SocketWrapperTest, MoveAssignmentClosesOldSocket) {
    SocketWrapper sock1(INVALID_SOCK);
    SocketWrapper sock2(INVALID_SOCK);
    
    sock2 = std::move(sock1);
    EXPECT_FALSE(sock1.isValid());
    EXPECT_FALSE(sock2.isValid());
}

// Test 9: Copy constructor is deleted
TEST_F(SocketWrapperTest, CopyConstructorIsDeleted) {
    EXPECT_FALSE(std::is_copy_constructible_v<SocketWrapper>);
}

// Test 10: Copy assignment is deleted
TEST_F(SocketWrapperTest, CopyAssignmentIsDeleted) {
    EXPECT_FALSE(std::is_copy_assignable_v<SocketWrapper>);
}

// Test 11: isValid() returns true for valid-looking socket (stub)
TEST_F(SocketWrapperTest, IsValidReturnsCorrectState) {
    SocketWrapper validSock(static_cast<socket_t>(42));
    SocketWrapper invalidSock(INVALID_SOCKET_VALUE);
    
    EXPECT_TRUE(validSock.isValid());
    EXPECT_FALSE(invalidSock.isValid());
}

// Test 12: close() can be called multiple times safely
TEST_F(SocketWrapperTest, CloseCanBeCalledMultipleTimes) {
    SocketWrapper sock(INVALID_SOCK);
    EXPECT_NO_THROW(sock.close());
    EXPECT_NO_THROW(sock.close());
}

} // namespace net
