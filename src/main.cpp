#include <iostream>
#include "SocketWrapper.h"
#include "ThreadPool.h"
#include "LockFreeQueue.h"
#include "BufferWrapper.h"
#include "LogGuard.h"
#include "ResourcePool.h"
#include "RuleOfFiveDemo.h"
#include "AsyncServer.h"
#include "NetworkBuffer.h"
#include <vector>
#include <thread>
#include <chrono>

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

    // Demo Lock-Free Queue
    std::cout << "\n--- Lock-Free Queue Demo ---" << std::endl;
    {
        core::LockFreeQueue<int, 64> queue;
        std::cout << "Created LockFreeQueue with capacity " << queue.capacity() << std::endl;

        // Enqueue values
        std::cout << "Enqueuing 16 values..." << std::endl;
        for (int i = 0; i < 16; ++i) {
            if (queue.try_enqueue(i * 10)) {
                std::cout << "  Enqueued: " << i * 10 << std::endl;
            } else {
                std::cout << "  Failed to enqueue: " << i * 10 << " (queue full)" << std::endl;
            }
        }

        std::cout << "\nApproximate queue size: " << queue.approximate_size() << std::endl;

        // Dequeue values
        std::cout << "Dequeuing values..." << std::endl;
        int value;
        while (queue.try_dequeue(value)) {
            std::cout << "  Dequeued: " << value << std::endl;
        }

        std::cout << "Queue is empty: " << (queue.is_empty() ? "true" : "false") << std::endl;
    }

    // Demo BufferWrapper - RAII for buffers
    std::cout << "\n--- BufferWrapper Demo (RAII) ---" << std::endl;
    {
        core::BufferWrapper<uint8_t> buffer(256);
        std::cout << "Created buffer with size: " << buffer.size() << " bytes" << std::endl;
        
        buffer[0] = 0xFF;
        buffer[255] = 0xAA;
        std::cout << "Set buffer[0] = 0x" << std::hex << (int)buffer[0] << std::endl;
        std::cout << "Set buffer[255] = 0x" << (int)buffer[255] << std::dec << std::endl;
        
        // Move semantics
        core::BufferWrapper<uint8_t> buffer2 = std::move(buffer);
        std::cout << "After move, buffer2 is valid: " << (buffer2.is_valid() ? "true" : "false") << std::endl;
    } // Both buffers cleaned up automatically

    // Demo LogGuard - RAII for file handles
    std::cout << "\n--- LogGuard Demo (RAII) ---" << std::endl;
    {
        core::LogGuard log("demo_session.log");
        std::cout << "Created log file: " << log.filename() << std::endl;
        
        log.log("Session started");
        log.log("Processing data");
        log.log("Operation complete");
        
        std::cout << "Log file closed: " << (log.is_open() ? "false (still open)" : "true") << std::endl;
    } // Log file automatically closed

    // Demo ResourcePool - Smart Pointer patterns
    std::cout << "\n--- ResourcePool Demo (shared_ptr/weak_ptr) ---" << std::endl;
    {
        struct SimpleResource {
            int id = 0;
        };
        
        core::ResourcePool<SimpleResource> pool(3);
        std::cout << "Created ResourcePool with " << pool.total_resources() << " resources" << std::endl;
        
        {
            auto res1 = pool.acquire();
            auto res2 = pool.acquire();
            
            std::cout << "Active resources: " << pool.active_resources() << std::endl;
            std::cout << "Available resources: " << pool.available_resources() << std::endl;
        }
        
        std::cout << "After releasing: " << pool.available_resources() << " available" << std::endl;
    }

    // Demo Rule of Five
    std::cout << "\n--- Rule of Five Demo ---" << std::endl;
    {
        std::cout << "Creating object 1..." << std::endl;
        core::RuleOfFiveDemo obj1("Original");
        
        std::cout << "Copy constructing object 2..." << std::endl;
        core::RuleOfFiveDemo obj2 = obj1;
        
        std::cout << "Move constructing object 3..." << std::endl;
        core::RuleOfFiveDemo obj3 = std::move(obj1);
        
        std::cout << "Copy assigning to object 2..." << std::endl;
        obj2 = obj3;
        
        std::cout << "Total instances created: " << core::RuleOfFiveDemo::total_instances() << std::endl;
    } // All objects destroyed, destructors called

    // Demo Async Networking
    std::cout << "\n--- Async Networking Demo ---" << std::endl;
    {
        core::net::NetworkBuffer buffer(256);
        std::cout << "Created NetworkBuffer with capacity: " << buffer.size() << " bytes" << std::endl;
        
        // Write data
        buffer.write_byte(0xFF);
        buffer.write_uint16(0x1234);
        buffer.write_uint32(0x12345678);
        
        std::cout << "Written " << buffer.write_pos() << " bytes" << std::endl;
        std::cout << "Available to read: " << buffer.available_read() << " bytes" << std::endl;
        
        // Reset and read
        buffer.reset_read();
        uint8_t byte_val;
        uint16_t word_val;
        uint32_t dword_val;
        
        buffer.read_byte(byte_val);
        buffer.read_uint16(word_val);
        buffer.read_uint32(dword_val);
        
        std::cout << "Read byte: 0x" << std::hex << (int)byte_val << std::endl;
        std::cout << "Read word: 0x" << word_val << std::endl;
        std::cout << "Read dword: 0x" << dword_val << std::dec << std::endl;
    }

    std::cout << "\n=== All Demos Complete ===" << std::endl;
    
    return 0;
}