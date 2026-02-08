#pragma once

#include "ConnectionHandler.h"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace core {
namespace net {

/**
 * @brief Manages all active client connections
 * 
 * Demonstrates:
 * - Thread-safe connection registry
 * - Connection lifecycle management
 * - Statistics and monitoring
 */
class ConnectionManager {
public:
    /**
     * @brief Construct connection manager
     */
    ConnectionManager() = default;

    /**
     * @brief Destructor - closes all connections
     */
    ~ConnectionManager() noexcept;

    // Delete copy operations
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    // Delete move operations
    ConnectionManager(ConnectionManager&&) = delete;
    ConnectionManager& operator=(ConnectionManager&&) = delete;

    /**
     * @brief Add a new connection
     * @param client_socket Socket handle
     * @param handler Connection handler
     * @return true if added, false if socket already exists
     */
    bool add_connection(SOCKET client_socket, std::unique_ptr<ConnectionHandler> handler) noexcept;

    /**
     * @brief Remove a connection
     * @param client_socket Socket to remove
     * @return true if removed, false if not found
     */
    bool remove_connection(SOCKET client_socket) noexcept;

    /**
     * @brief Get connection handler
     * @param client_socket Socket to find
     * @return Pointer to handler, nullptr if not found
     */
    [[nodiscard]] ConnectionHandler* get_connection(SOCKET client_socket) noexcept;

    /**
     * @brief Check if connection exists
     */
    [[nodiscard]] bool has_connection(SOCKET client_socket) const noexcept;

    /**
     * @brief Get all active sockets
     */
    [[nodiscard]] std::vector<SOCKET> get_all_sockets() const noexcept;

    /**
     * @brief Get connection count
     */
    [[nodiscard]] size_t get_connection_count() const noexcept;

    /**
     * @brief Close all connections
     */
    void close_all() noexcept;

    /**
     * @brief Get total bytes received across all connections
     */
    [[nodiscard]] size_t get_total_bytes_received() const noexcept;

    /**
     * @brief Get total bytes sent across all connections
     */
    [[nodiscard]] size_t get_total_bytes_sent() const noexcept;

    /**
     * @brief Get statistics for a connection
     * @param client_socket Socket to query
     * @return Pair of (bytes_received, bytes_sent)
     */
    [[nodiscard]] std::pair<size_t, size_t> get_connection_stats(SOCKET client_socket) const noexcept;

private:
    std::map<SOCKET, std::unique_ptr<ConnectionHandler>> m_connections;
    mutable std::mutex m_mutex;
};

} // namespace net
} // namespace core
