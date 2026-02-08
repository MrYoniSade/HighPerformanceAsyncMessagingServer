# Usage Guide & Integration Examples

This guide demonstrates how to use the High-Performance Asynchronous Messaging Server components in your applications.

## Table of Contents

1. [Basic Usage](#basic-usage)
2. [Thread Pool Examples](#thread-pool-examples)
3. [Lock-Free Queue Examples](#lock-free-queue-examples)
4. [Network Server Examples](#network-server-examples)
5. [Protocol & Message Handling](#protocol--message-handling)
6. [Advanced Patterns](#advanced-patterns)

---

## Basic Usage

### Including Headers

```cpp
#include "ThreadPool.h"
#include "LockFreeQueue.h"
#include "AsyncServer.h"
#include "HandlerRegistry.h"
#include "BinaryProtocol.h"
```

### Namespace Alias (Optional)

```cpp
namespace cp = core::protocol;
namespace cn = core::net;
```

---

## Thread Pool Examples

### Simple Task Submission

```cpp
#include "ThreadPool.h"
using namespace core;

int main() {
    // Create a thread pool with 4 worker threads
    ThreadPool pool(4);
    
    // Submit a simple task
    auto future = pool.submit([]() {
        std::cout << "Task executed on thread: " 
                  << std::this_thread::get_id() << std::endl;
        return 42;
    });
    
    // Get the result
    int result = future.get();
    std::cout << "Result: " << result << std::endl;
    
    // Graceful shutdown
    pool.shutdown();
    
    return 0;
}
```

### Submitting Multiple Tasks

```cpp
ThreadPool pool(4);
std::vector<std::future<int>> futures;

// Submit multiple tasks
for (int i = 0; i < 100; ++i) {
    futures.push_back(pool.submit([i]() {
        // Do some work
        return i * i;
    }));
}

// Collect results
int sum = 0;
for (auto& future : futures) {
    sum += future.get();
}

std::cout << "Sum of squares: " << sum << std::endl;
pool.shutdown();
```

### Task with Exception Handling

```cpp
ThreadPool pool(4);

auto future = pool.submit([]() {
    if (/* error condition */) {
        throw std::runtime_error("Task failed");
    }
    return 100;
});

try {
    int result = future.get();
} catch (const std::exception& e) {
    std::cerr << "Task threw: " << e.what() << std::endl;
}

pool.shutdown();
```

---

## Lock-Free Queue Examples

### Basic Enqueue/Dequeue

```cpp
#include "LockFreeQueue.h"
using namespace core;

int main() {
    // Create a lock-free queue with capacity 64
    LockFreeQueue<int, 64> queue;
    
    // Enqueue values
    if (!queue.try_enqueue(42)) {
        std::cerr << "Queue full" << std::endl;
    }
    
    // Dequeue values
    int value;
    if (queue.try_dequeue(value)) {
        std::cout << "Dequeued: " << value << std::endl;
    }
    
    // Check status
    std::cout << "Queue size: " << queue.approximate_size() << std::endl;
    std::cout << "Queue empty: " << queue.is_empty() << std::endl;
    
    return 0;
}
```

### Producer-Consumer Pattern

```cpp
LockFreeQueue<int, 256> queue;
std::atomic<bool> done(false);

// Producer thread
std::thread producer([&]() {
    for (int i = 0; i < 1000; ++i) {
        while (!queue.try_enqueue(i)) {
            // Busy wait or yield
            std::this_thread::yield();
        }
    }
    done = true;
});

// Consumer thread
std::thread consumer([&]() {
    int count = 0;
    int value;
    while (!done || !queue.is_empty()) {
        if (queue.try_dequeue(value)) {
            count++;
        } else {
            std::this_thread::yield();
        }
    }
    std::cout << "Consumed " << count << " items" << std::endl;
});

producer.join();
consumer.join();
```

### Performance Comparison

```cpp
#include "MutexQueue.h"

void benchmark() {
    const int ITERATIONS = 1000000;
    
    // Benchmark LockFreeQueue
    LockFreeQueue<int, 4096> lf_queue;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        lf_queue.try_enqueue(i);
    }
    auto lf_time = std::chrono::high_resolution_clock::now() - start;
    
    // Benchmark MutexQueue
    MutexQueue<int, 4096> m_queue;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        m_queue.try_enqueue(i);
    }
    auto m_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "LockFreeQueue: " << lf_time.count() << "ns" << std::endl;
    std::cout << "MutexQueue: " << m_time.count() << "ns" << std::endl;
    std::cout << "Speedup: " << (double)m_time.count() / lf_time.count() << "x" << std::endl;
}
```

---

## Network Server Examples

### Simple Echo Server

```cpp
#include "AsyncServer.h"
using namespace core::net;

int main() {
    AsyncServer server(4);  // 4 worker threads
    
    if (!server.start("127.0.0.1", 5000)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Server listening on 127.0.0.1:5000" << std::endl;
    
    // This blocks until stop() is called
    server.run();
    
    return 0;
}
```

### Server with Custom Connection Handler

```cpp
class EchoServer {
private:
    AsyncServer server;
    
public:
    EchoServer(size_t num_threads = 4)
        : server(num_threads) {}
    
    bool start(const std::string& address, uint16_t port) {
        if (!server.start(address, port)) {
            return false;
        }
        
        std::cout << "Echo server started on " 
                  << address << ":" << port << std::endl;
        return true;
    }
    
    void run() {
        server.run();
    }
    
    void broadcast_message(const std::string& msg) {
        server.broadcast(
            reinterpret_cast<const uint8_t*>(msg.c_str()),
            msg.size()
        );
    }
};
```

### Broadcast Example

```cpp
AsyncServer server(4);
server.start("127.0.0.1", 5000);

// In a separate thread
std::thread broadcaster([&]() {
    for (int i = 0; i < 10; ++i) {
        std::string msg = "Broadcast message " + std::to_string(i);
        server.broadcast(
            reinterpret_cast<const uint8_t*>(msg.c_str()),
            msg.size()
        );
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
});

// Run server (blocking)
server.run();

broadcaster.join();
```

---

## Protocol & Message Handling

### Serializing Messages

```cpp
#include "BinaryProtocol.h"
#include "MessageSerializer.h"
#include "ProtocolMessages.h"
using namespace core::protocol;

int main() {
    // Create a Ping message
    messages::PingMessage ping;
    ping.sequence_id = 12345;
    ping.timestamp = std::chrono::system_clock::now()
                        .time_since_epoch().count();
    
    // Create frame header
    FrameHeader header;
    header.magic = PROTOCOL_MAGIC;
    header.version = PROTOCOL_VERSION;
    header.message_type = static_cast<uint8_t>(MessageType::PING);
    header.flags = 0;
    header.payload_length = sizeof(ping);
    header.reserved = 0;
    
    // Serialize to buffer
    core::net::NetworkBuffer buffer(256);
    if (MessageSerializer::serialize_frame(
            header,
            reinterpret_cast<const uint8_t*>(&ping),
            sizeof(ping),
            buffer)) {
        std::cout << "Serialized " << buffer.write_pos() << " bytes" << std::endl;
    }
    
    return 0;
}
```

### Deserializing Messages

```cpp
FrameHeader header;
std::vector<uint8_t> payload;

// Received data from network
uint8_t received_data[] = { /* ... */ };
size_t received_size = sizeof(received_data);

size_t consumed = MessageSerializer::deserialize_frame(
    received_data,
    received_size,
    header,
    payload
);

if (consumed > 0) {
    std::cout << "Deserialized message type: " 
              << (int)header.message_type << std::endl;
    std::cout << "Payload size: " << payload.size() << std::endl;
} else {
    std::cerr << "Failed to deserialize frame" << std::endl;
}
```

### Custom Message Handler

```cpp
// Define custom message
struct CustomMessage {
    uint32_t request_id;
    uint32_t data_size;
    uint8_t data[128];
};

// Create handler
class CustomHandler : public MessageHandler<CustomMessage> {
public:
    CustomHandler()
        : MessageHandler(MessageType::DATA, 
            [](const CustomMessage& msg) {
                std::cout << "Request ID: " << msg.request_id << std::endl;
                return true;
            }) {}
    
    bool deserialize_payload(const uint8_t* data,
                            size_t length,
                            CustomMessage& payload) noexcept override {
        if (length < sizeof(CustomMessage)) {
            return false;
        }
        std::memcpy(&payload, data, sizeof(CustomMessage));
        return true;
    }
};

// Register handler
HandlerRegistry registry;
registry.register_handler(std::make_unique<CustomHandler>());
```

### Message Routing

```cpp
HandlerRegistry registry;

// Register Ping handler
registry.register_handler(
    std::make_unique<messages::PingHandler>(
        [](const messages::PingMessage& msg) {
            std::cout << "Received Ping: " << msg.sequence_id << std::endl;
            // Send Pong response
            return true;
        }
    )
);

// Register Pong handler
registry.register_handler(
    std::make_unique<messages::PongHandler>(
        [](const messages::PongMessage& msg) {
            std::cout << "Received Pong: " << msg.sequence_id << std::endl;
            return true;
        }
    )
);

// Dispatch incoming message
FrameHeader header = /* from network */;
std::vector<uint8_t> payload = /* from network */;

if (registry.dispatch(
        static_cast<MessageType>(header.message_type),
        payload.data(),
        payload.size())) {
    std::cout << "Message handled successfully" << std::endl;
} else {
    std::cerr << "No handler for message type" << std::endl;
}
```

---

## Advanced Patterns

### Resource Pool Usage

```cpp
#include "ResourcePool.h"

struct DatabaseConnection {
    std::string server;
    uint16_t port;
    bool is_connected = false;
};

int main() {
    core::ResourcePool<DatabaseConnection> pool(10);
    
    {
        auto connection = pool.acquire();
        if (connection) {
            connection.value()->server = "localhost";
            connection.value()->port = 5432;
            std::cout << "Active connections: " 
                      << pool.active_resources() << std::endl;
        }
    } // Connection returned to pool
    
    std::cout << "Available connections: " 
              << pool.available_resources() << std::endl;
    
    return 0;
}
```

### RAII Buffer Management

```cpp
#include "BufferWrapper.h"

void process_data() {
    // Buffer automatically allocated
    core::BufferWrapper<uint8_t> buffer(4096);
    
    // Use buffer
    buffer[0] = 0xFF;
    buffer[1] = 0xAA;
    
    // Move semantics
    core::BufferWrapper<uint8_t> buffer2 = std::move(buffer);
    
    // buffer is now invalid
    // buffer2 still owns the memory
    
} // buffer2 automatically freed
```

### Logging with LogGuard

```cpp
#include "LogGuard.h"

void run_session() {
    core::LogGuard log("session.log");
    
    log.log("Session started");
    
    try {
        // Do work
        log.log("Processing request 1");
        log.log("Processing request 2");
        log.log("Requests completed");
    } catch (const std::exception& e) {
        log.log(std::string("Error: ") + e.what());
    }
    
} // Log file automatically closed
```

### Combined Example: Server with Protocol Handler

```cpp
class ProtocolServer {
private:
    AsyncServer server;
    HandlerRegistry handlers;
    ThreadPool worker_pool;
    
public:
    ProtocolServer(size_t num_threads = 4)
        : server(num_threads / 2)
        , worker_pool(num_threads / 2) {
        setup_handlers();
    }
    
    void setup_handlers() {
        handlers.register_handler(
            std::make_unique<messages::PingHandler>(
                [this](const messages::PingMessage& msg) {
                    // Process on thread pool
                    worker_pool.submit([msg]() {
                        std::cout << "Handling Ping: " 
                                  << msg.sequence_id << std::endl;
                    });
                    return true;
                }
            )
        );
        
        handlers.register_handler(
            std::make_unique<messages::EchoHandler>(
                [this](const messages::EchoMessage& msg) {
                    worker_pool.submit([this, msg]() {
                        // Echo back to all clients
                        std::cout << "Echoing " 
                                  << msg.length << " bytes" << std::endl;
                    });
                    return true;
                }
            )
        );
    }
    
    bool start(const std::string& addr, uint16_t port) {
        return server.start(addr, port);
    }
    
    void run() {
        server.run();
    }
    
    void shutdown() {
        // Server cleanup
        worker_pool.shutdown();
    }
    
    HandlerRegistry& get_handlers() {
        return handlers;
    }
};

int main() {
    ProtocolServer server(8);
    
    if (!server.start("127.0.0.1", 5000)) {
        std::cerr << "Failed to start" << std::endl;
        return 1;
    }
    
    std::cout << "Server running..." << std::endl;
    server.run();
    server.shutdown();
    
    return 0;
}
```

---

## Performance Tips

1. **Lock-Free Queue**: Use for high-throughput producer-consumer scenarios
2. **Thread Pool**: Keep worker threads <= CPU cores for CPU-bound work
3. **Message Handlers**: Keep handler logic light, offload to thread pool
4. **Buffer Reuse**: Pre-allocate buffers when possible
5. **Protocol**: CRC32 adds ~1?s overhead, consider for critical paths

---

## Common Patterns

### Request-Response Pattern

```cpp
// Server sends request with sequence ID
PingMessage request;
request.sequence_id = generate_id();

// Client responds with matching sequence ID
PongMessage response;
response.sequence_id = request.sequence_id;

// Correlate requests with responses
std::map<uint32_t, PendingRequest> pending;
```

### Pub-Sub Pattern

```cpp
// Register multiple handlers for different message types
HandlerRegistry handlers;

handlers.register_handler(create_ping_handler());
handlers.register_handler(create_data_handler());
handlers.register_handler(create_status_handler());

// Dispatch routes to appropriate handler
handlers.dispatch(message_type, payload, length);
```

### Command Pattern

```cpp
// Wrap command as message
struct CommandMessage {
    uint8_t command_type;
    uint8_t parameters[64];
};

// Handler executes command
handlers.register_handler(
    std::make_unique<CommandHandler>(
        [](const CommandMessage& cmd) {
            execute_command(cmd.command_type, cmd.parameters);
            return true;
        }
    )
);
```

---

This guide provides the foundation for integrating the High-Performance Asynchronous Messaging Server components into your applications.
