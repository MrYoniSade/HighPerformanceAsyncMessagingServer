# Project Architecture & Design Documentation

Comprehensive architectural overview of the High-Performance Asynchronous Messaging Server.

## Table of Contents

1. [System Architecture](#system-architecture)
2. [Component Design](#component-design)
3. [Data Flow](#data-flow)
4. [Threading Model](#threading-model)
5. [Protocol Design](#protocol-design)
6. [Design Decisions](#design-decisions)
7. [Extension Points](#extension-points)

---

## System Architecture

### Layered Architecture

```
???????????????????????????????????????????????????????????
?           Application Layer                            ?
?     (User-defined message handlers & business logic)   ?
???????????????????????????????????????????????????????????
                       ?
???????????????????????????????????????????????????????????
?         Message Processing Layer                        ?
?  - HandlerRegistry (routing)                            ?
?  - MessageHandler<T> (type-safe dispatch)              ?
?  - Protocol messages (Ping, Pong, Echo, Data, Status)  ?
???????????????????????????????????????????????????????????
                       ?
???????????????????????????????????????????????????????????
?         Protocol Layer                                 ?
?  - MessageSerializer (frame parsing)                   ?
?  - BinaryProtocol (frame format)                       ?
?  - BitPackUtils (bit operations)                       ?
?  - EndianUtils (byte order)                            ?
?  - CRC32 (checksums)                                   ?
???????????????????????????????????????????????????????????
                       ?
???????????????????????????????????????????????????????????
?         Network Layer                                  ?
?  - AsyncServer (TCP server)                            ?
?  - ConnectionHandler (per-client)                      ?
?  - ConnectionManager (registry)                        ?
?  - NetworkBuffer (serialization)                       ?
?  - AsyncSocket (WinSock wrapper)                       ?
???????????????????????????????????????????????????????????
                       ?
???????????????????????????????????????????????????????????
?         Infrastructure Layer                           ?
?  - ThreadPool (worker management)                      ?
?  - LockFreeQueue (MPMC queue)                          ?
?  - RAII Wrappers (BufferWrapper, LogGuard)             ?
?  - ResourcePool (smart pointer patterns)               ?
?  - RuleOfFiveDemo (memory management)                  ?
???????????????????????????????????????????????????????????
                       ?
???????????????????????????????????????????????????????????
?         OS Layer                                       ?
?  - WinSock2 (Windows sockets)                          ?
?  - Threads (OS thread management)                      ?
?  - Atomics (CPU-level synchronization)                 ?
???????????????????????????????????????????????????????????
```

### Component Dependency Graph

```
User Code
  ?
  ??? AsyncServer ??????????
  ?                        ?
  ??? HandlerRegistry ??????
  ?                        ?
  ??? MessageHandler<T>    ?
  ?                        ?
  ??? ThreadPool           ?
  ?                        ?
  ??? LockFreeQueue        ?
                           ?
                    ????????
                    ?
            ???????????????????????
            ?                     ?
    ?????????????????     ???????????????????
    ? ConnectionMgr ?     ? MessageSerializer
    ?               ?     ?                  ?
    ?????????????????     ????????????????????
           ?                     ?
    ?????????????????     ???????????????????
    ?ConnectionHdlr ?     ? BinaryProtocol   ?
    ?               ?     ?                  ?
    ?????????????????     ????????????????????
           ?                     ?
    ?????????????????     ???????????????????
    ? AsyncSocket   ?     ? BitPackUtils     ?
    ?               ?     ? EndianUtils      ?
    ?????????????????     ????????????????????
```

---

## Component Design

### Tier 1: Infrastructure (Lowest Level)

#### **LockFreeQueue<T, Cap>**

```cpp
template<typename T, size_t Cap>
class LockFreeQueue {
    // Ring buffer design
    std::array<T, Cap> buffer;
    
    // 64-bit versioned index (ABA prevention)
    // Upper 32 bits: version counter
    // Lower 32 bits: position in ring
    std::atomic<uint64_t> head;
    std::atomic<uint64_t> tail;
    
    // Compare-and-swap loop
    bool try_enqueue(const T& value);
    bool try_dequeue(T& value);
};
```

**Key Points:**
- No locks, no allocations
- CAS operations only
- O(1) enqueue/dequeue
- Wait-free dequeue, lock-free enqueue

#### **ThreadPool**

```cpp
class ThreadPool {
    std::vector<std::thread> workers;
    LockFreeQueue<Task> task_queue;
    std::atomic<bool> shutdown_requested;
    
    std::future<T> submit(std::function<T()> task);
    void shutdown();
};
```

**Key Points:**
- Worker threads wait on queue
- std::future for result retrieval
- Exception propagation
- Graceful shutdown

### Tier 2: RAII & Memory Management

#### **BufferWrapper<T>**

```cpp
template<typename T = uint8_t>
class BufferWrapper {
    std::unique_ptr<T[]> buffer;
    size_t size;
    
    // Rule of 5
    ~BufferWrapper();
    BufferWrapper(const BufferWrapper&) = delete;
    BufferWrapper& operator=(const BufferWrapper&) = delete;
    BufferWrapper(BufferWrapper&&);
    BufferWrapper& operator=(BufferWrapper&&);
};
```

**Key Points:**
- RAII automatic cleanup
- Move semantics for efficiency
- Exception-safe

#### **ResourcePool<T>**

```cpp
template<typename T>
class ResourcePool {
    std::map<shared_ptr<T>> resources;
    
    // Track active resources via use_count()
    std::optional<shared_ptr<T>> acquire();
    std::optional<weak_ptr<T>> acquire_weak();
};
```

**Key Points:**
- Shared ownership pattern
- Automatic cleanup
- Reuse across lifetimes

### Tier 3: Networking

#### **AsyncSocket**

```cpp
class AsyncSocket {
    SOCKET socket;
    WSAEVENT event;
    
    bool create_listening_socket(const std::string& address, uint16_t port);
    SOCKET accept_connection(std::string& client_addr, uint16_t& client_port);
    bool set_async_mode(WSAEVENT hEventObject, long lNetworkEvents);
};
```

**Key Points:**
- WinSock2 async I/O
- Event-based notification
- Non-blocking operations

#### **ConnectionHandler**

```cpp
class ConnectionHandler {
    SOCKET client_socket;
    BufferWrapper<uint8_t> read_buffer;
    BufferWrapper<uint8_t> write_buffer;
    
    bool handle_read_event();
    bool handle_write_event();
    bool send_data(const uint8_t* data, size_t length);
};
```

**Key Points:**
- Per-connection state
- Buffered I/O
- Callback-based events

#### **AsyncServer**

```cpp
class AsyncServer {
    std::unique_ptr<AsyncSocket> socket;
    std::map<SOCKET, ConnectionHandler> connections;
    std::unique_ptr<ThreadPool> pool;
    
    bool start(const std::string& address, uint16_t port);
    void run();
    bool dispatch(SOCKET client, const uint8_t* data, size_t length);
};
```

**Key Points:**
- Multi-client support
- Event multiplexing
- ThreadPool integration

### Tier 4: Protocol

#### **BinaryProtocol**

```cpp
struct FrameHeader {
    uint8_t magic;              // 0xAB
    uint8_t version;            // 0x01
    uint8_t message_type;       // MessageType enum
    uint8_t flags;              // FrameFlags
    uint16_t payload_length;    // 0-65535
    uint16_t reserved;          // Future use
};

// Complete frame: Header (8) + Payload (variable) + Checksum (4)
```

**Key Points:**
- Standardized format
- CRC32 validation
- Compact (8-byte header)

#### **MessageSerializer**

```cpp
class MessageSerializer {
    // Frame layout handling
    static bool serialize_frame(...);
    static size_t deserialize_frame(...);
    
    // Checksum validation
    static bool validate_frame(...);
    
    // Size calculation
    static size_t calculate_frame_size(...);
};
```

**Key Points:**
- Protocol parsing
- Error handling
- Checksum validation

#### **BitPackUtils**

```cpp
class BitPackUtils {
    static size_t pack_bits(uint8_t* buffer, size_t offset,
                           uint32_t value, size_t num_bits);
    static uint32_t unpack_bits(const uint8_t* buffer,
                               size_t offset, size_t num_bits);
    
    // Helper methods for uint8/16/32
    static size_t pack_uint8(...);
    static uint8_t unpack_uint8(...);
    // ...
};
```

**Key Points:**
- Sub-byte operations
- Bit-level packing
- Endian-safe

### Tier 5: Message Routing

#### **MessageHandler<T>**

```cpp
template<typename T>
class MessageHandler : public IMessageHandler {
    MessageType message_type;
    std::function<bool(const T&)> callback;
    
    bool handle(const uint8_t* payload, size_t length) override;
    virtual bool deserialize_payload(...);
};
```

**Key Points:**
- Template specialization
- Type safety at compile-time
- Custom deserialization

#### **HandlerRegistry**

```cpp
class HandlerRegistry {
    std::map<MessageType, unique_ptr<IMessageHandler>> handlers;
    std::mutex mutex;
    
    bool register_handler(unique_ptr<IMessageHandler> handler);
    bool dispatch(MessageType type, const uint8_t* payload, size_t length);
};
```

**Key Points:**
- Thread-safe dispatch
- Polymorphic handler support
- Type enumeration routing

---

## Data Flow

### Incoming Message Processing

```
1. Network Packet Arrives
        ?
        ?
2. AsyncSocket receives FD_READ event
        ?
        ?
3. ConnectionHandler::handle_read_event()
   ?? recv() into read_buffer
   ?? Extract complete frames
        ?
        ?
4. MessageSerializer::deserialize_frame()
   ?? Parse header
   ?? Validate checksum
   ?? Extract payload
        ?
        ?
5. HandlerRegistry::dispatch()
   ?? Lookup handler by message type
   ?? Call handler
        ?
        ?
6. MessageHandler<T>::handle()
   ?? deserialize_payload() ? type T
   ?? Call application callback
        ?
        ?
7. Application Logic
   ?? Process message
   ?? Generate response (optional)
        ?
        ?
8. Response Queued
   ?? ConnectionHandler::send_data()
   ?? MessageSerializer::serialize_frame()
        ?
        ?
9. AsyncSocket transmits
```

### Outgoing Message Transmission

```
1. Application creates message
   ?? Serialize to buffer via MessageSerializer
        ?
        ?
2. AsyncServer::send_to_client()
        ?
        ?
3. ConnectionHandler::send_data()
   ?? Copy to write_buffer
   ?? Try immediate send
        ?
        ?
4. Handle FD_WRITE event
   ?? send() remaining data
   ?? Update buffer position
        ?
        ?
5. Complete transmission
```

---

## Threading Model

### Thread Pool Architecture

```
Main Thread
    ?
    ??? ThreadPool Control
    ?   ??? Worker Thread 1 ???
    ?   ??? Worker Thread 2   ?
    ?   ??? Worker Thread 3   ???? LockFreeQueue (Tasks)
    ?   ??? Worker Thread N   ?
    ?                         ?
    ?
    ??? AsyncServer Main Loop
        ??? WSAWaitForMultipleEvents()
        ??? Process Network Events
        ??? Delegate to ThreadPool
```

### Synchronization Strategy

```
Critical Sections:
?????????????????????????????????????????????????
LockFreeQueue        ? Atomics + CAS (no locks)
ConnectionManager    ? std::mutex (low contention)
HandlerRegistry      ? std::mutex (low contention)
Individual buffers   ? No synchronization (per-thread)

Result: High concurrency, minimal lock contention
```

### Memory Ordering

```cpp
// ThreadPool task submission
std::memory_order_release  // Publish task to queue

// LockFreeQueue operations
std::memory_order_acquire  // Read from other producers
std::memory_order_release  // Publish to consumers

// Connection state
std::atomic<bool> is_active;  // Acquire/Release semantics
```

---

## Protocol Design

### Binary Frame Format

```
Offset  Size    Field           Description
?????????????????????????????????????????????????
0       1       Magic           Protocol identifier (0xAB)
1       1       Version         Protocol version (0x01)
2       1       Message Type    Type ID (enum)
3       1       Flags           ACK_REQUIRED, COMPRESSED, etc.
4-5     2       Payload Length  0-65535 bytes
6-7     2       Reserved        Future extension
8-X     var     Payload         Message data
X+1-X+4 4      Checksum        CRC32 (little-endian)

Total size: 8 + payload_length + 4 bytes
```

### Message Type Enumeration

```cpp
enum class MessageType : uint8_t {
    PING = 0x01,    // Connectivity check
    PONG = 0x02,    // Ping response
    ECHO = 0x03,    // Echo request
    DATA = 0x04,    // General data
    STATUS = 0x05,  // Status update
    MAX = 0xFF
};
```

### Flags Usage

```cpp
enum class FrameFlags : uint8_t {
    NONE       = 0x00,  // No flags
    ACK_REQ    = 0x01,  // Acknowledgment required
    COMPRESS   = 0x02,  // Payload compressed
    ENCRYPT    = 0x04   // Payload encrypted
};
```

---

## Design Decisions

### 1. Lock-Free Queue vs Mutex Queue

**Chosen: Lock-Free Queue**

Reasons:
- 2-3x better throughput
- Consistent latency (deterministic)
- No context switches
- Scales better with contention

Trade-off: More complex implementation

### 2. Event-Based Async vs Thread-Per-Client

**Chosen: Event-Based**

Reasons:
- Scales to thousands of connections
- Lower memory per connection
- Better CPU utilization
- Modern approach (IOCP, epoll)

Trade-off: More complex state management

### 3. Binary Protocol vs Text-Based

**Chosen: Binary**

Reasons:
- Smaller message size
- Faster parsing
- Type-safe
- CRC32 validation

Trade-off: Less human-readable

### 4. Template-Based Dispatch vs Virtual Methods

**Chosen: Templates (with Virtual Base)**

Reasons:
- Type safety at compile-time
- Zero-cost abstraction
- No virtual function overhead
- Extensible pattern

Trade-off: Code generation for each type

### 5. RAII vs Manual Resource Management

**Chosen: RAII**

Reasons:
- Exception-safe
- No resource leaks
- Automatic cleanup
- Clear ownership semantics

Trade-off: Small overhead from unique_ptr

### 6. C++20 vs Earlier Standards

**Chosen: C++20**

Reasons:
- Modern language features
- Better compiler optimizations
- constexpr capabilities
- Ranges and concepts

Trade-off: Requires newer compiler

---

## Extension Points

### Adding New Message Types

```cpp
// 1. Define structure
struct MyMessage {
    uint32_t id;
    std::array<uint8_t, 128> data;
};

// 2. Create handler
class MyHandler : public MessageHandler<MyMessage> {
public:
    MyHandler() : MessageHandler(MessageType::DATA, callback) {}
    
    bool deserialize_payload(...) override {
        // Custom deserialization
    }
};

// 3. Register
registry.register_handler(std::make_unique<MyHandler>());
```

### Custom Connection Logic

```cpp
class CustomConnectionHandler : public ConnectionHandler {
protected:
    void on_data_received(const uint8_t* data, size_t length) override {
        // Custom processing
    }
    
    void on_connection_closed() override {
        // Cleanup
    }
};
```

### Protocol Enhancements

```cpp
// Add compression support
MessageSerializer::serialize_frame(...) {
    if (header.has_flag(FrameFlags::COMPRESS)) {
        payload = compress(payload);
    }
    // ...
}

// Add encryption
MessageSerializer::serialize_frame(...) {
    if (header.has_flag(FrameFlags::ENCRYPT)) {
        payload = encrypt(payload, key);
    }
    // ...
}
```

### Performance Monitoring

```cpp
class InstrumentedAsyncServer : public AsyncServer {
    std::atomic<uint64_t> messages_received{0};
    std::atomic<uint64_t> bytes_received{0};
    
    void on_message_received(size_t size) {
        messages_received++;
        bytes_received += size;
    }
};
```

---

## Summary

The architecture achieves:

? **Modularity** - Clear separation of concerns across tiers  
? **Performance** - Optimized at each level (lock-free, async I/O, binary protocol)  
? **Extensibility** - Clean extension points for custom logic  
? **Maintainability** - Well-documented, testable components  
? **Scalability** - Handles thousands of concurrent connections  
? **Safety** - RAII, type safety, thread-safe primitives  

This design is suitable for production systems requiring high performance, reliability, and maintainability.
