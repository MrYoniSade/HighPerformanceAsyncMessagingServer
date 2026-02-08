#pragma once

#include "AsyncSocket.h"
#include "ConnectionHandler.h"
#include "ThreadPool.h"
#include <map>
#include <memory>
#include <atomic>
#include <thread>

namespace core {
namespace net {

/**
 * @brief Async TCP server handling multiple concurrent clients
 * 
 * Demonstrates:
 * - Multi-client TCP server
 * - Async I/O with WinSock events
 * - Integration with ThreadPool
 * - Connection lifecycle management
 */
class AsyncServer {
public:
    /**
     * @brief Construct async server
     * @param num_worker_threads Number of worker threads for connection processing
     */
    explicit AsyncServer(size_t num_worker_threads = 4);

    /**
     * @brief Destructor - stops server and closes connections
     */
    ~AsyncServer() noexcept;

    // Delete copy operations
    AsyncServer(const AsyncServer&) = delete;
    AsyncServer& operator=(const AsyncServer&) = delete;

    // Delete move operations
    AsyncServer(AsyncServer&&) = delete;
    AsyncServer& operator=(AsyncServer&&) = delete;

    /**
     * @brief Start listening for connections
     * @param listen_address Address to listen on
     * @param port Port to listen on
     * @return true if successful
     */
    bool start(const std::string& listen_address, uint16_t port) noexcept;

    /**
     * @brief Stop server and close all connections
     */
    void stop() noexcept;

    /**
     * @brief Run server main loop (blocking)
     * Processes client events and handles connections
     * @param timeout_ms Timeout in milliseconds for event wait (0 = infinite)
     */
    void run(unsigned long timeout_ms = INFINITE) noexcept;

    /**
     * @brief Check if server is running
     */
    [[nodiscard]] bool is_running() const noexcept
    {
        return m_is_running.load(std::memory_order_acquire);
    }

    /**
     * @brief Get number of active connections
     */
    [[nodiscard]] size_t get_connection_count() const noexcept
    {
        return m_connections.size();
    }

    /**
     * @brief Send data to a specific client
     * @param client_socket Socket handle of client
     * @param data Data to send
     * @param length Data length
     * @return true if successful
     */
    bool send_to_client(SOCKET client_socket, const uint8_t* data, size_t length) noexcept;

    /**
     * @brief Broadcast data to all connected clients
     * @param data Data to broadcast
     * @param length Data length
     */
    void broadcast(const uint8_t* data, size_t length) noexcept;

    /**
     * @brief Close a specific client connection
     * @param client_socket Socket to close
     */
    void close_client(SOCKET client_socket) noexcept;

private:
    std::unique_ptr<AsyncSocket> m_socket;
    std::unique_ptr<ThreadPool> m_thread_pool;
    
    WSAEVENT m_event_object;
    std::atomic<bool> m_is_running{false};

    // Map of socket -> connection handler
    std::map<SOCKET, std::unique_ptr<ConnectionHandler>> m_connections;
    mutable std::mutex m_connections_mutex;

    static constexpr size_t MAX_EVENTS = 64;
    static constexpr size_t MAX_CONNECTIONS = 1000;

    /**
     * @brief Process network events
     */
    void process_events() noexcept;

    /**
     * @brief Handle new client connection
     */
    void handle_new_connection() noexcept;

    /**
     * @brief Handle client read event
     */
    void handle_client_read(SOCKET client_socket) noexcept;

    /**
     * @brief Handle client write event
     */
    void handle_client_write(SOCKET client_socket) noexcept;

    /**
     * @brief Connection closed callback
     */
    void on_connection_closed(SOCKET client_socket) noexcept;
};

} // namespace net
} // namespace core
