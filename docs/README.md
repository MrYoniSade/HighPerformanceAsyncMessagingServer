# High-Performance Asynchronous Messaging Server

A **modern C++20 backend system** showcasing advanced concurrency, networking, and compile-time programming techniques. Production-ready architecture demonstrating deep mastery of systems programming.

## ?? Project Highlights

? **90 Unit Tests** - Comprehensive coverage of all components  
? **Zero-Copy Design** - Lock-free queues, move semantics, RAII  
? **High Performance** - 2-3x speedup over mutex-based alternatives  
? **Type-Safe** - Template-based handlers with compile-time message IDs  
? **Production-Ready** - Error handling, checksums, protocol validation  
? **Cross-Platform** - Windows WinSock2 with portable abstractions  

---

## ?? Test Results

```
? 100% tests passed (8/8 test suites)
- SocketWrapperTest (1 test)
- ThreadPoolTest (13 tests)
- QueueTest (15 tests)
- BufferWrapperTest (11 tests)
- LogGuardTest (5 tests)
- ResourcePoolTest (10 tests)
- AsyncNetworkingTest (15 tests)
- ProtocolTest (20 tests)

Total: 90 tests, 100% passing
```

---

## ??? Architecture

### System Components

```
Async TCP Server (Multi-Client, WinSock2)
         ?
ConnectionHandler (Per-Client)
         ?
NetworkBuffer ? MessageSerializer ? BinaryProtocol (CRC32)
         ?
HandlerRegistry (Thread-Safe Message Routing)
         ?
MessageHandler<T> (Type-Safe Dispatch)
         ?
ThreadPool (Worker Threads)
         ?
LockFreeQueue (Lock-Free Task Distribution)
```

### Technology Stack

| Layer | Component | Status |
|-------|-----------|--------|
| **Core** | ThreadPool, LockFreeQueue, RAII | ? Complete |
| **Networking** | AsyncSocket, ConnectionHandler, AsyncServer | ? Complete |
| **Protocol** | BinaryProtocol, MessageSerializer, CRC32 | ? Complete |
| **Routing** | MessageHandler, HandlerRegistry | ? Complete |
| **Utilities** | BitPackUtils, EndianUtils, NetworkBuffer | ? Complete |
| **Testing** | GoogleTest (90 tests) | ? Complete |

---

## ? Key Features

### Week 1: Core Infrastructure

#### **Custom Thread Pool**
- Worker thread management with futures
- Graceful shutdown
- Task queue system
- Exception-safe

#### **Lock-Free Queue (MPMC)**
- Atomic CAS operations
- ABA prevention with 64-bit versioning
- Ring buffer design
- **2-3x faster** than mutex queues

#### **RAII & Smart Pointers**
- BufferWrapper for automatic memory management
- LogGuard for file resource management
- ResourcePool with shared_ptr/weak_ptr patterns
- Rule of 5 implementation

### Week 2: Networking & Protocol

#### **Async TCP Server**
- WinSock2 event-based I/O
- Multi-client connection handling
- Per-client buffering
- Integrated with ThreadPool

#### **Binary Protocol**
- Frame format: Header (8) + Payload + Checksum (4)
- Message types: PING, PONG, ECHO, DATA, STATUS
- CRC32 validation
- Frame flags: ACK_REQUIRED, COMPRESSED, ENCRYPTED

#### **Message Routing**
- Template-based polymorphic handlers
- Type-safe message dispatch
- Compile-time message IDs
- Thread-safe handler registry

#### **Advanced Utilities**
- Bit-packing/unpacking at sub-byte level
- Endianness conversion (constexpr)
- Efficient network serialization

---

## ?? Quick Start

### Build

```bash
# Clone and build
git clone https://github.com/MrYoniSade/HighPerformanceAsyncMessagingServer.git
cd HighPerformanceAsyncMessagingServer

mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Run tests
ctest --output-on-failure

# Run demo
./HighPerfServer
```

### Demo Output

```
--- Lock-Free Queue Demo ---
Created LockFreeQueue with capacity 63
Enqueuing 16 values...
  Enqueued: 0
  Enqueued: 10
  ...
Approximate queue size: 16

--- Binary Protocol Demo ---
Protocol Magic: 0xab
Protocol Version: 1
Created Ping message (seq=12345)
Serialized frame size: 28 bytes
Deserialized frame successfully

--- Message Routing Demo ---
Created Handler Registry
Registered 2 handlers, total: 2
Dispatching Ping message...
  [Handler] Received Ping: seq=999

--- Bit Packing Demo ---
Packed 1 bool + 4 bits + 1 uint8 + 1 uint16
Unpacked values: bool=1, bits=0xa, byte=0xff, word=0x1234

=== All Demos Complete ===
```

---

## ?? Performance Benchmarks

### Lock-Free Queue Performance

```
Single-Threaded Enqueue:
  LockFreeQueue: 66.7M ops/sec
  MutexQueue:    33.8M ops/sec
  Speedup: 1.97x

Single-Threaded Dequeue:
  LockFreeQueue: 71.6M ops/sec
  MutexQueue:    34.8M ops/sec
  Speedup: 2.06x

Multi-Threaded (2P/2C):
  LockFreeQueue: 12.0M ops/sec
  MutexQueue:    4.3M ops/sec
  Speedup: 2.82x

Multi-Threaded (4P/4C):
  LockFreeQueue: 10.5M ops/sec
  MutexQueue:    4.2M ops/sec
  Speedup: 2.51x
```

### Protocol Overhead

```
Minimum Frame: 12 bytes (8-byte header + 4-byte checksum)
Ping Message: 28 bytes total
Payload Efficiency: ~70%
CRC32 Calculation: <1?s for typical messages
```

---

## ?? Design Patterns Used

### RAII (Resource Acquisition Is Initialization)
```cpp
{
    BufferWrapper buffer(256);
    LogGuard log("session.log");
} // Automatic cleanup
```

### Smart Pointers
```cpp
ResourcePool<MyResource> pool(10);
auto resource = pool.acquire();  // shared_ptr
// Auto-cleanup when out of scope
```

### Lock-Free Programming
```cpp
LockFreeQueue<T, 64> queue;
queue.try_enqueue(value);    // Lock-free push
queue.try_dequeue(value);    // Lock-free pop
```

### Template-Based Message Handling
```cpp
HandlerRegistry registry;
registry.register_handler(
    std::make_unique<MessageHandler<PingMessage>>(
        [](const PingMessage& msg) { /*...*/ }
    )
);
```

### Event-Based Async I/O
```cpp
WSAEventSelect(socket, hEvent, FD_READ | FD_WRITE);
WSAWaitForMultipleEvents(1, &hEvent, FALSE, timeout, FALSE);
```

---

## ?? Project Structure

```
HighPerformanceAsyncMessagingServer/
??? CMakeLists.txt
??? docs/
?   ??? README.md
??? include/
?   ??? Core/
?   ?   ??? ThreadPool.h
?   ?   ??? LockFreeQueue.h
?   ?   ??? BufferWrapper.h
?   ?   ??? LogGuard.h
?   ?   ??? ResourcePool.h
?   ?   ??? RuleOfFiveDemo.h
?   ??? Networking/
?   ?   ??? AsyncSocket.h
?   ?   ??? ConnectionHandler.h
?   ?   ??? AsyncServer.h
?   ?   ??? ConnectionManager.h
?   ?   ??? NetworkBuffer.h
?   ??? Protocol/
?       ??? BinaryProtocol.h
?       ??? MessageSerializer.h
?       ??? BitPackUtils.h
?       ??? EndianUtils.h
?       ??? MessageHandler.h
?       ??? HandlerRegistry.h
?       ??? ProtocolMessages.h
??? src/
?   ??? main.cpp
?   ??? ThreadPool.cpp
?   ??? AsyncSocket.cpp
?   ??? ConnectionHandler.cpp
?   ??? AsyncServer.cpp
?   ??? ConnectionManager.cpp
?   ??? BinaryProtocol.cpp
?   ??? MessageSerializer.cpp
?   ??? BitPackUtils.cpp
?   ??? HandlerRegistry.cpp
?   ??? QueueBenchmark.cpp
??? test/
    ??? SocketWrapperTest.cpp
    ??? ThreadPoolTest.cpp
    ??? QueueTest.cpp
    ??? BufferWrapperTest.cpp
    ??? LogGuardTest.cpp
    ??? ResourcePoolTest.cpp
    ??? AsyncNetworkingTest.cpp
    ??? ProtocolTest.cpp
```

---

## ?? Build Options

### Visual Studio
```bash
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

### Ninja (Recommended)
```bash
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Unix Makefiles
```bash
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make
```

---

## ?? C++20 Features Demonstrated

- **Concepts** - Template constraints and validation
- **Ranges** - Modern algorithm compositions
- **Designated Initializers** - Clean struct initialization
- **Bit Fields** - Efficient memory packing
- **constexpr** - Compile-time computations

---

## ?? Learning Outcomes

This project demonstrates mastery of:

? **Concurrency**
- Lock-free programming with atomics
- Memory ordering (acquire/release)
- Thread pool design
- Event-based async I/O

? **Systems Programming**
- WinSock2 socket programming
- Buffer management
- Protocol design
- Resource lifecycle management

? **Modern C++**
- RAII pattern
- Smart pointers
- Move semantics
- Template specialization
- Type safety

? **Software Design**
- SOLID principles
- Design patterns
- Clean architecture
- Test-driven development

---

## ?? Support

### Common Operations

```cpp
// Create and run server
AsyncServer server(4);  // 4 worker threads
server.start("127.0.0.1", 5000);
// server.run();  // Blocking call

// Create lock-free queue
LockFreeQueue<int, 64> queue;
queue.try_enqueue(42);
int value;
queue.try_dequeue(value);

// Register message handler
HandlerRegistry registry;
registry.register_handler(
    std::make_unique<PingHandler>(
        [](const PingMessage& msg) {
            std::cout << "Ping received: " << msg.sequence_id << std::endl;
            return true;
        }
    )
);

// Dispatch message
registry.dispatch(MessageType::PING, payload, length);

// Thread pool
ThreadPool pool(4);
auto future = pool.submit([]() { return 42; });
int result = future.get();
pool.shutdown();
```

---

## ?? License & Attribution

**Educational Portfolio Project**

Demonstrates production-grade C++20 programming techniques suitable for:
- Systems programming roles
- Backend engineering positions
- High-performance computing
- Real-time systems development

---

## ? Project Statistics

- **Lines of Code**: ~3,500 (headers + implementation)
- **Test Coverage**: 90 tests across 8 test suites
- **Build Time**: <5 seconds
- **Memory Overhead**: Minimal (RAII, zero-copy)
- **Dependencies**: Only C++ standard library + GoogleTest
- **Platforms**: Windows (WinSock2), portable to Unix

---

**Status**: ? Complete & Production-Ready  
**Last Updated**: 2024-02-08  
**Test Pass Rate**: 100% (90/90 tests)  
**Build Status**: ? Successful
