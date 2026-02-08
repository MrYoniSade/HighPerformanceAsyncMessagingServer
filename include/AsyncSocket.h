#pragma once

#include <winsock2.h>
#include <ws2ipdef.h>
#include <string>
#include <memory>
#include <functional>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

namespace core {
namespace net {

/**
 * @brief RAII wrapper for async WinSock operations
 * 
 * Demonstrates:
 * - WinSock2 async socket operations
 * - RAII resource management for sockets
 * - Event-based notification
 */
class AsyncSocket {
public:
    using EventHandler = std::function<void(WSANETWORKEVENTS)>;

    /**
     * @brief Construct an async socket
     * @param address IP address to bind/connect to
     * @param port Port number
     */
    AsyncSocket(const std::string& address, uint16_t port);

    /**
     * @brief Destructor - closes socket and unregisters events
     */
    ~AsyncSocket() noexcept;

    // Delete copy operations
    AsyncSocket(const AsyncSocket&) = delete;
    AsyncSocket& operator=(const AsyncSocket&) = delete;

    // Delete move operations (resource tied to lifetime)
    AsyncSocket(AsyncSocket&&) = delete;
    AsyncSocket& operator=(AsyncSocket&&) = delete;

    /**
     * @brief Initialize Winsock
     * @return true if successful
     */
    static bool initialize_winsock() noexcept;

    /**
     * @brief Cleanup Winsock
     */
    static void cleanup_winsock() noexcept;

    /**
     * @brief Create a listening socket
     * @param listen_address Address to listen on
     * @param port Port to listen on
     * @param backlog Connection backlog
     * @return true if successful
     */
    bool create_listening_socket(const std::string& listen_address, uint16_t port, int backlog = 5) noexcept;

    /**
     * @brief Accept incoming connection
     * @param client_addr Output: client address
     * @param client_port Output: client port
     * @return SOCKET handle for accepted connection, INVALID_SOCKET on error
     */
    [[nodiscard]] SOCKET accept_connection(std::string& client_addr, uint16_t& client_port) noexcept;

    /**
     * @brief Connect to remote server
     * @param remote_address Remote server address
     * @param remote_port Remote server port
     * @return true if successful
     */
    bool connect(const std::string& remote_address, uint16_t remote_port) noexcept;

    /**
     * @brief Set socket to async mode with event notifications
     * @param hEventObject Windows event object for notifications
     * @param lNetworkEvents Events to monitor (FD_READ, FD_WRITE, etc.)
     * @return true if successful
     */
    bool set_async_mode(WSAEVENT hEventObject, long lNetworkEvents) noexcept;

    /**
     * @brief Send data on socket
     * @param socket Socket handle
     * @param data Data to send
     * @param length Data length
     * @return Bytes sent, SOCKET_ERROR on error
     */
    [[nodiscard]] int send_data(SOCKET socket, const uint8_t* data, int length) noexcept;

    /**
     * @brief Receive data on socket
     * @param socket Socket handle
     * @param buffer Buffer to receive into
     * @param buffer_size Buffer size
     * @return Bytes received, SOCKET_ERROR on error, 0 on graceful close
     */
    [[nodiscard]] int recv_data(SOCKET socket, uint8_t* buffer, int buffer_size) noexcept;

    /**
     * @brief Close a connected socket (not the listening socket)
     * @param socket Socket to close
     */
    void close_client_socket(SOCKET socket) noexcept;

    /**
     * @brief Get listening socket
     */
    [[nodiscard]] SOCKET get_socket() const noexcept
    {
        return m_socket;
    }

    /**
     * @brief Check if socket is valid
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
        return m_socket != INVALID_SOCKET;
    }

    /**
     * @brief Get last error message
     */
    [[nodiscard]] std::string get_last_error() const noexcept;

private:
    SOCKET m_socket;
    std::string m_address;
    uint16_t m_port;
    static std::atomic<int> s_winsock_count;

    [[nodiscard]] std::string get_address_from_socket(SOCKET s) const noexcept;
    [[nodiscard]] uint16_t get_port_from_socket(SOCKET s) const noexcept;
};

} // namespace net
} // namespace core
