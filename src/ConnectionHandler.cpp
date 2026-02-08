#include "ConnectionHandler.h"
#include <iostream>
#include <cstring>

namespace core {
namespace net {

ConnectionHandler::ConnectionHandler(SOCKET client_socket, const std::string& client_address, uint16_t client_port)
    : m_client_socket(client_socket)
    , m_client_address(client_address)
    , m_client_port(client_port)
{
}

ConnectionHandler::~ConnectionHandler() noexcept
{
    close();
}

bool ConnectionHandler::handle_read_event() noexcept
{
    if (!m_is_active) {
        return false;
    }

    int bytes_received = recv(m_client_socket, reinterpret_cast<char*>(m_read_buffer.data()), 
                             static_cast<int>(m_read_buffer.size()), 0);

    if (bytes_received == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "recv() failed: " << error << std::endl;
            m_is_active = false;
            if (m_on_connection_closed) {
                m_on_connection_closed();
            }
            return false;
        }
        return false;
    }

    if (bytes_received == 0) {
        // Connection closed by client
        std::cout << "Client " << m_client_address << ":" << m_client_port << " closed connection" << std::endl;
        m_is_active = false;
        if (m_on_connection_closed) {
            m_on_connection_closed();
        }
        return false;
    }

    m_bytes_received += bytes_received;

    if (m_on_data_received) {
        m_on_data_received(m_read_buffer.data(), bytes_received);
    }

    return true;
}

bool ConnectionHandler::handle_write_event() noexcept
{
    if (!m_is_active || m_write_pos == 0) {
        return false;
    }

    int bytes_sent = send(m_client_socket, reinterpret_cast<const char*>(m_write_buffer.data()), 
                         static_cast<int>(m_write_pos), 0);

    if (bytes_sent == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "send() failed: " << error << std::endl;
            m_is_active = false;
            return false;
        }
        return false;
    }

    m_bytes_sent += bytes_sent;

    // Remove sent data from buffer
    if (bytes_sent < static_cast<int>(m_write_pos)) {
        std::memmove(m_write_buffer.data(), 
                    m_write_buffer.data() + bytes_sent,
                    m_write_pos - bytes_sent);
    }

    m_write_pos -= bytes_sent;
    return m_write_pos > 0;
}

bool ConnectionHandler::send_data(const uint8_t* data, size_t length) noexcept
{
    if (!m_is_active || !data || length == 0) {
        return false;
    }

    // Check if we have space in write buffer
    if (m_write_pos + length > m_write_buffer.size()) {
        std::cerr << "Write buffer full" << std::endl;
        return false;
    }

    // Copy data to write buffer
    std::memcpy(m_write_buffer.data() + m_write_pos, data, length);
    m_write_pos += length;

    // Try to send immediately
    return handle_write_event() || m_write_pos > 0;
}

void ConnectionHandler::close() noexcept
{
    if (m_client_socket != INVALID_SOCKET) {
        shutdown(m_client_socket, SD_BOTH);
        closesocket(m_client_socket);
        m_client_socket = INVALID_SOCKET;
    }
    m_is_active = false;
}

} // namespace net
} // namespace core
