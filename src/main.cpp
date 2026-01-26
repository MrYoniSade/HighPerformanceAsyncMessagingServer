#include <iostream>
#include "SocketWrapper.h"
#include "ThreadPool.h"
#include <vector>

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

    // Demo ThreadPool
    std::cout << "\n--- ThreadPool Demo ---" << std::endl;
    {
        core::ThreadPool pool(4);
        std::cout << "Created ThreadPool with " << pool.getNumThreads() << " worker threads" << std::endl;

        // Submit some tasks
        std::vector<std::future<int>> futures;
        for (int i = 0; i < 8; ++i) {
            futures.push_back(pool.submit([i]() {
                std::cout << "Task " << i << " executing on thread " << std::this_thread::get_id() << std::endl;
                return i * i;
            }));
        }

        // Wait for all tasks and collect results
        std::cout << "Collecting results..." << std::endl;
        for (size_t i = 0; i < futures.size(); ++i) {
            int result = futures[i].get();
            std::cout << "Task " << i << " result: " << result << std::endl;
        }

        // Graceful shutdown
        pool.shutdown();
        std::cout << "ThreadPool shutdown complete" << std::endl;
    }
    
    return 0;
}