#include "ConnectionManager.h"
#include <iostream>

namespace core {
namespace net {

ConnectionManager::~ConnectionManager() noexcept
{
    close_all();
}

bool ConnectionManager::add_connection(SOCKET client_socket, std::unique_ptr<ConnectionHandler> handler) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_connections.find(client_socket) != m_connections.end()) {
        return false; // Already exists
    }

    m_connections[client_socket] = std::move(handler);
    return true;
}

bool ConnectionManager::remove_connection(SOCKET client_socket) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(client_socket);
    if (it == m_connections.end()) {
        return false; // Not found
    }

    m_connections.erase(it);
    return true;
}

ConnectionHandler* ConnectionManager::get_connection(SOCKET client_socket) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(client_socket);
    if (it != m_connections.end()) {
        return it->second.get();
    }

    return nullptr;
}

bool ConnectionManager::has_connection(SOCKET client_socket) const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connections.find(client_socket) != m_connections.end();
}

std::vector<SOCKET> ConnectionManager::get_all_sockets() const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<SOCKET> sockets;
    sockets.reserve(m_connections.size());
    
    for (const auto& pair : m_connections) {
        sockets.push_back(pair.first);
    }

    return sockets;
}

size_t ConnectionManager::get_connection_count() const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connections.size();
}

void ConnectionManager::close_all() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.clear();
}

size_t ConnectionManager::get_total_bytes_received() const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t total = 0;
    for (const auto& pair : m_connections) {
        total += pair.second->get_bytes_received();
    }

    return total;
}

size_t ConnectionManager::get_total_bytes_sent() const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t total = 0;
    for (const auto& pair : m_connections) {
        total += pair.second->get_bytes_sent();
    }

    return total;
}

std::pair<size_t, size_t> ConnectionManager::get_connection_stats(SOCKET client_socket) const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_connections.find(client_socket);
    if (it != m_connections.end()) {
        return {it->second->get_bytes_received(), it->second->get_bytes_sent()};
    }

    return {0, 0};
}

} // namespace net
} // namespace core
