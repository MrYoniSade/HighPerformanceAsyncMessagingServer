#include <iostream>
#include "SocketWrapper.h"

int main() {
 
    // Demo RAII Socket Wrapper
    {
        net::SocketWrapper validSock(static_cast<net::socket_t>(42));
        std::cout << "Created socket wrapper with handle: " << validSock.get() << std::endl;
        std::cout << "Socket is valid: " << (validSock.isValid() ? "true" : "false") << std::endl;
    } // Socket automatically cleaned up here via RAII destructor
    
    {
        net::SocketWrapper invalidSock;
        std::cout << "Created invalid socket wrapper" << std::endl;
        std::cout << "Socket is valid: " << (invalidSock.isValid() ? "true" : "false") << std::endl;
    }
    
    // Demo move semantics
    net::SocketWrapper sock1(static_cast<net::socket_t>(123));
    net::SocketWrapper sock2 = std::move(sock1);
    std::cout << "\nAfter move:" << std::endl;
    std::cout << "sock1 is valid: " << (sock1.isValid() ? "true" : "false") << std::endl;
    std::cout << "sock2 is valid: " << (sock2.isValid() ? "true" : "false") << std::endl;
    std::cout << "sock2 handle: " << sock2.get() << std::endl;
    
    return 0;
}