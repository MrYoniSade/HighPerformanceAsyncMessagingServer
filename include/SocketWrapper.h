#pragma once

#include <memory>
#include <utility>

#ifdef _WIN32
    #include <winsock2.h>
    using socket_t = SOCKET;
    constexpr socket_t INVALID_SOCKET_VALUE = INVALID_SOCKET;
#else
    #include <sys/socket.h>
    #include <unistd.h>
    using socket_t = int;
    constexpr socket_t INVALID_SOCKET_VALUE = -1;
#endif

namespace net {

// Re-export socket_t in namespace
using socket_t = ::socket_t;
constexpr auto INVALID_SOCKET_VALUE = ::INVALID_SOCKET_VALUE;

class SocketWrapper {
public:
    explicit SocketWrapper(socket_t socket = INVALID_SOCKET_VALUE) noexcept
        : m_socket(socket) {}

    // Destructor: RAII - closes the socket
    ~SocketWrapper() noexcept {
        close();
    }

    // Delete copy operations
    SocketWrapper(const SocketWrapper&) = delete;
    SocketWrapper& operator=(const SocketWrapper&) = delete;

    // Move operations
    SocketWrapper(SocketWrapper&& other) noexcept
        : m_socket(other.release()) {}

    SocketWrapper& operator=(SocketWrapper&& other) noexcept {
        if (this != &other) {
            close();
            m_socket = other.release();
        }
        return *this;
    }

    // Check if socket is valid
    [[nodiscard]] bool isValid() const noexcept {
        return m_socket != INVALID_SOCKET_VALUE;
    }

    // Get underlying socket handle
    [[nodiscard]] socket_t get() const noexcept {
        return m_socket;
    }

    // Release ownership without closing
    [[nodiscard]] socket_t release() noexcept {
        socket_t temp = m_socket;
        m_socket = INVALID_SOCKET_VALUE;
        return temp;
    }

    // Reset with a new socket
    void reset(socket_t socket = INVALID_SOCKET_VALUE) noexcept {
        close();
        m_socket = socket;
    }

    // Explicit close
    void close() noexcept {
        if (isValid()) {
#ifdef _WIN32
            closesocket(m_socket);
#else
            ::close(m_socket);
#endif
            m_socket = INVALID_SOCKET_VALUE;
        }
    }

private:
    socket_t m_socket;
};

} // namespace net
