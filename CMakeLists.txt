cmake_minimum_required(VERSION 3.20)
project(HighPerformanceAsyncMessagingServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add executables
add_executable(HighPerfServer src/main.cpp src/ThreadPool.cpp src/AsyncSocket.cpp src/ConnectionHandler.cpp src/AsyncServer.cpp src/ConnectionManager.cpp)

# Link winsock2 on Windows
if(WIN32)
    target_link_libraries(HighPerfServer ws2_32)
endif()

# GoogleTest integration
enable_testing()
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Unit tests
# Define test targets with a reusable function
function(add_test_target test_name test_sources extra_sources)
    add_executable(${test_name} ${test_sources} ${extra_sources})
    target_link_libraries(${test_name} gtest gtest_main)
    
    # Link winsock2 on Windows for all tests
    if(WIN32)
        target_link_libraries(${test_name} ws2_32)
    endif()
    
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Register test targets
add_test_target(SocketWrapperTest "test/SocketWrapperTest.cpp" "")
add_test_target(ThreadPoolTest "test/ThreadPoolTest.cpp" "src/ThreadPool.cpp")
add_test_target(QueueTest "test/QueueTest.cpp" "")
add_test_target(BufferWrapperTest "test/BufferWrapperTest.cpp" "")
add_test_target(LogGuardTest "test/LogGuardTest.cpp" "")
add_test_target(ResourcePoolTest "test/ResourcePoolTest.cpp" "")
add_test_target(AsyncNetworkingTest "test/AsyncNetworkingTest.cpp" "src/AsyncSocket.cpp;src/ConnectionHandler.cpp;src/ConnectionManager.cpp")

# Benchmark executable
add_executable(QueueBenchmark src/QueueBenchmark.cpp)