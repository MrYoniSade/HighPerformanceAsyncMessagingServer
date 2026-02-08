#include "AsyncServer.h"
#include <iostream>
#include <algorithm>

namespace core {
namespace net {

AsyncServer::AsyncServer(size_t num_worker_threads)
    : m_socket(std::make_unique<AsyncSocket>("127.0.0.1", 0))
    , m_thread_pool(std::make_unique<ThreadPool>(num_worker_threads))
    , m_event_object(WSACreateEvent())
{
}

AsyncServer::~AsyncServer() noexcept
{
    stop();

    if (m_event_object != WSA_INVALID_EVENT) {
        WSACloseEvent(m_event_object);
    }
}

bool AsyncServer::start(const std::string& listen_address, uint16_t port) noexcept
{
    if (!AsyncSocket::initialize_winsock()) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return false;
    }

    if (!m_socket->create_listening_socket(listen_address, port)) {
        std::cerr << "Failed to create listening socket" << std::endl;
        AsyncSocket::cleanup_winsock();
        return false;
    }

    if (!m_socket->set_async_mode(m_event_object, FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE)) {
        std::cerr << "Failed to set async mode" << std::endl;
        AsyncSocket::cleanup_winsock();
        return false;
    }

    m_is_running.store(true, std::memory_order_release);
    std::cout << "AsyncServer listening on " << listen_address << ":" << port << std::endl;

    return true;
}

void AsyncServer::stop() noexcept
{
    m_is_running.store(false, std::memory_order_release);

    {
        std::lock_guard<std::mutex> lock(m_connections_mutex);
        m_connections.clear();
    }

    if (m_thread_pool) {
        m_thread_pool->shutdown();
    }

    AsyncSocket::cleanup_winsock();
}

void AsyncServer::run(unsigned long timeout_ms) noexcept
{
    if (!m_is_running) {
        return;
    }

    while (m_is_running) {
        // Wait for network events
        DWORD dwRet = WSAWaitForMultipleEvents(1, &m_event_object, FALSE, timeout_ms, FALSE);

        if (dwRet == WSA_WAIT_TIMEOUT) {
            continue;
        }

        if (dwRet == WSA_WAIT_FAILED) {
            std::cerr << "WSAWaitForMultipleEvents failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        // Process all events
        process_events();

        // Reset event object for next iteration
        WSAResetEvent(m_event_object);
    }
}

void AsyncServer::process_events() noexcept
{
    WSANETWORKEVENTS networkEvents;

    // Check listening socket for new connections
    {
        int result = WSAEnumNetworkEvents(m_socket->get_socket(), m_event_object, &networkEvents);
        if (result != SOCKET_ERROR) {
            if (networkEvents.lNetworkEvents & FD_ACCEPT) {
                if (networkEvents.iErrorCode[FD_ACCEPT_BIT] == 0) {
                    handle_new_connection();
                }
            }
        }
    }

    // Check client sockets for read/write events
    {
        std::lock_guard<std::mutex> lock(m_connections_mutex);
        
        for (auto it = m_connections.begin(); it != m_connections.end(); ) {
            SOCKET client_socket = it->first;
            auto& handler = it->second;

            WSANETWORKEVENTS clientEvents;
            int result = WSAEnumNetworkEvents(client_socket, WSA_INVALID_EVENT, &clientEvents);

            if (result != SOCKET_ERROR) {
                if (clientEvents.lNetworkEvents & FD_READ) {
                    if (clientEvents.iErrorCode[FD_READ_BIT] == 0) {
                        handle_client_read(client_socket);
                    }
                }

                if (clientEvents.lNetworkEvents & FD_WRITE) {
                    if (clientEvents.iErrorCode[FD_WRITE_BIT] == 0) {
                        handle_client_write(client_socket);
                    }
                }

                if (clientEvents.lNetworkEvents & FD_CLOSE) {
                    on_connection_closed(client_socket);
                    it = m_connections.erase(it);
                    continue;
                }
            }

            if (!handler->is_active()) {
                it = m_connections.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void AsyncServer::handle_new_connection() noexcept
{
    if (m_connections.size() >= MAX_CONNECTIONS) {
        std::cerr << "Max connections reached" << std::endl;
        return;
    }

    std::string client_address;
    uint16_t client_port = 0;

    SOCKET client_socket = m_socket->accept_connection(client_address, client_port);
    if (client_socket == INVALID_SOCKET) {
        return;
    }

    std::cout << "New connection from " << client_address << ":" << client_port << std::endl;

    auto handler = std::make_unique<ConnectionHandler>(client_socket, client_address, client_port);

    // Set up callbacks
    handler->set_data_received_callback([this, client_socket](const uint8_t* data, size_t length) {
        std::cout << "Received " << length << " bytes from " << client_socket << std::endl;
        // Echo the data back
        send_to_client(client_socket, data, length);
    });

    handler->set_connection_closed_callback([this, client_socket]() {
        on_connection_closed(client_socket);
    });

    std::lock_guard<std::mutex> lock(m_connections_mutex);
    m_connections[client_socket] = std::move(handler);
}

void AsyncServer::handle_client_read(SOCKET client_socket) noexcept
{
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    
    auto it = m_connections.find(client_socket);
    if (it != m_connections.end()) {
        it->second->handle_read_event();
    }
}

void AsyncServer::handle_client_write(SOCKET client_socket) noexcept
{
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    
    auto it = m_connections.find(client_socket);
    if (it != m_connections.end()) {
        it->second->handle_write_event();
    }
}

void AsyncServer::on_connection_closed(SOCKET client_socket) noexcept
{
    std::cout << "Connection closed: " << client_socket << std::endl;
}

bool AsyncServer::send_to_client(SOCKET client_socket, const uint8_t* data, size_t length) noexcept
{
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    
    auto it = m_connections.find(client_socket);
    if (it != m_connections.end()) {
        return it->second->send_data(data, length);
    }

    return false;
}

void AsyncServer::broadcast(const uint8_t* data, size_t length) noexcept
{
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    
    for (auto& pair : m_connections) {
        pair.second->send_data(data, length);
    }
}

void AsyncServer::close_client(SOCKET client_socket) noexcept
{
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    
    auto it = m_connections.find(client_socket);
    if (it != m_connections.end()) {
        it->second->close();
        m_connections.erase(it);
    }
}

} // namespace net
} // namespace core
