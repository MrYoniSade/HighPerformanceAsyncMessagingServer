#include "AsyncSocket.h"
#include <iostream>
#include <sstream>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

namespace core {
namespace net {

std::atomic<int> AsyncSocket::s_winsock_count(0);

AsyncSocket::AsyncSocket(const std::string& address, uint16_t port)
    : m_socket(INVALID_SOCKET)
    , m_address(address)
    , m_port(port)
{
}

AsyncSocket::~AsyncSocket() noexcept
{
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

bool AsyncSocket::initialize_winsock() noexcept
{
    if (s_winsock_count.fetch_add(1) > 0) {
        // Already initialized
        return true;
    }

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    if (result != 0) {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        s_winsock_count.fetch_sub(1);
        return false;
    }

    return true;
}

void AsyncSocket::cleanup_winsock() noexcept
{
    if (s_winsock_count.fetch_sub(1) == 1) {
        WSACleanup();
    }
}

bool AsyncSocket::create_listening_socket(const std::string& listen_address, uint16_t port, int backlog) noexcept
{
    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Set socket to non-blocking mode
    unsigned long mode = 1;
    if (ioctlsocket(m_socket, FIONBIO, &mode) == SOCKET_ERROR) {
        std::cerr << "ioctlsocket() failed: " << WSAGetLastError() << std::endl;
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    // Bind socket
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(listen_address.c_str());
    
    if (sockaddr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "inet_addr() failed: " << WSAGetLastError() << std::endl;
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    if (bind(m_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR) {
        std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    // Listen for connections
    if (listen(m_socket, backlog) == SOCKET_ERROR) {
        std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    m_address = listen_address;
    m_port = port;
    return true;
}

SOCKET AsyncSocket::accept_connection(std::string& client_addr, uint16_t& client_port) noexcept
{
    sockaddr_in client_sockaddr;
    int client_sockaddr_len = sizeof(client_sockaddr);

    SOCKET client_socket = accept(m_socket, (struct sockaddr*)&client_sockaddr, &client_sockaddr_len);

    if (client_socket == INVALID_SOCKET) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "accept() failed: " << error << std::endl;
        }
        return INVALID_SOCKET;
    }

    // Extract client address and port
    char addr_buf[16]; // "xxx.xxx.xxx.xxx" fits in 15 chars + null
    strcpy_s(addr_buf, sizeof(addr_buf), inet_ntoa(client_sockaddr.sin_addr));
    client_addr = addr_buf;
    client_port = ntohs(client_sockaddr.sin_port);

    return client_socket;
}

bool AsyncSocket::connect(const std::string& remote_address, uint16_t remote_port) noexcept
{
    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Set to non-blocking
    unsigned long mode = 1;
    if (ioctlsocket(m_socket, FIONBIO, &mode) == SOCKET_ERROR) {
        std::cerr << "ioctlsocket() failed: " << WSAGetLastError() << std::endl;
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    // Connect
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(remote_port);
    server_addr.sin_addr.s_addr = inet_addr(remote_address.c_str());

    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "inet_addr() failed" << std::endl;
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    int result = ::connect(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        // WSAEWOULDBLOCK is expected for non-blocking sockets
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "connect() failed: " << error << std::endl;
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }
    }

    return true;
}

bool AsyncSocket::set_async_mode(WSAEVENT hEventObject, long lNetworkEvents) noexcept
{
    if (WSAEventSelect(m_socket, hEventObject, lNetworkEvents) == SOCKET_ERROR) {
        std::cerr << "WSAEventSelect() failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

int AsyncSocket::send_data(SOCKET socket, const uint8_t* data, int length) noexcept
{
    int result = send(socket, reinterpret_cast<const char*>(data), length, 0);
    
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "send() failed: " << error << std::endl;
        }
    }

    return result;
}

int AsyncSocket::recv_data(SOCKET socket, uint8_t* buffer, int buffer_size) noexcept
{
    int result = recv(socket, reinterpret_cast<char*>(buffer), buffer_size, 0);

    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "recv() failed: " << error << std::endl;
        }
    }

    return result;
}

void AsyncSocket::close_client_socket(SOCKET socket) noexcept
{
    if (socket != INVALID_SOCKET) {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
    }
}

std::string AsyncSocket::get_last_error() const noexcept
{
    int error = WSAGetLastError();
    std::ostringstream oss;
    oss << "WSA Error: " << error;
    return oss.str();
}

} // namespace net
} // namespace core
