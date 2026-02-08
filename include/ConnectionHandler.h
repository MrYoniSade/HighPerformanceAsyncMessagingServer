#pragma once

#include <winsock2.h>
#include <memory>
#include <string>
#include <functional>
#include <atomic>
#include "BufferWrapper.h"

namespace core {
namespace net {

/**
 * @brief Handles a single client connection
 * 
 * Demonstrates:
 * - Per-connection state management
 * - RAII for connection resources
 * - Async read/write operations
 * - Callback-based event handling
 */
class ConnectionHandler {
public:
    using DataReceivedCallback = std::function<void(const uint8_t*, size_t)>;
    using ConnectionClosedCallback = std::function<void()>;

    /**
     * @brief Construct a connection handler
     * @param client_socket Connected client socket
     * @param client_address Client IP address
     * @param client_port Client port number
     */
    ConnectionHandler(SOCKET client_socket, const std::string& client_address, uint16_t client_port);

    /**
     * @brief Destructor - closes connection
     */
    ~ConnectionHandler() noexcept;

    // Delete copy operations
    ConnectionHandler(const ConnectionHandler&) = delete;
    ConnectionHandler& operator=(const ConnectionHandler&) = delete;

    // Delete move operations
    ConnectionHandler(ConnectionHandler&&) = delete;
    ConnectionHandler& operator=(ConnectionHandler&&) = delete;

    /**
     * @brief Set callback for data received
     */
    void set_data_received_callback(DataReceivedCallback callback) noexcept
    {
        m_on_data_received = callback;
    }

    /**
     * @brief Set callback for connection closed
     */
    void set_connection_closed_callback(ConnectionClosedCallback callback) noexcept
    {
        m_on_connection_closed = callback;
    }

    /**
     * @brief Handle read event - attempt to receive data
     * @return true if data was received, false if connection closed or no data available
     */
    bool handle_read_event() noexcept;

    /**
     * @brief Handle write event - attempt to send buffered data
     * @return true if data was sent, false if would block
     */
    bool handle_write_event() noexcept;

    /**
     * @brief Send data to client
     * @param data Data to send
     * @param length Data length
     * @return true if queued/sent, false if error
     */
    bool send_data(const uint8_t* data, size_t length) noexcept;

    /**
     * @brief Get client address
     */
    [[nodiscard]] const std::string& get_client_address() const noexcept
    {
        return m_client_address;
    }

    /**
     * @brief Get client port
     */
    [[nodiscard]] uint16_t get_client_port() const noexcept
    {
        return m_client_port;
    }

    /**
     * @brief Get client socket
     */
    [[nodiscard]] SOCKET get_socket() const noexcept
    {
        return m_client_socket;
    }

    /**
     * @brief Check if connection is active
     */
    [[nodiscard]] bool is_active() const noexcept
    {
        return m_is_active.load(std::memory_order_acquire);
    }

    /**
     * @brief Get bytes received
     */
    [[nodiscard]] size_t get_bytes_received() const noexcept
    {
        return m_bytes_received;
    }

    /**
     * @brief Get bytes sent
     */
    [[nodiscard]] size_t get_bytes_sent() const noexcept
    {
        return m_bytes_sent;
    }

    /**
     * @brief Close connection gracefully
     */
    void close() noexcept;

private:
    static constexpr size_t BUFFER_SIZE = 4096;

    SOCKET m_client_socket;
    std::string m_client_address;
    uint16_t m_client_port;
    std::atomic<bool> m_is_active{true};

    BufferWrapper<uint8_t> m_read_buffer{BUFFER_SIZE};
    BufferWrapper<uint8_t> m_write_buffer{BUFFER_SIZE};
    size_t m_write_pos{0};

    size_t m_bytes_received{0};
    size_t m_bytes_sent{0};

    DataReceivedCallback m_on_data_received;
    ConnectionClosedCallback m_on_connection_closed;
};

} // namespace net
} // namespace core
