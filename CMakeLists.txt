cmake_minimum_required(VERSION 3.20)
project(HighPerformanceAsyncMessagingServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Main executable
add_executable(HighPerfServer src/main.cpp)

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
add_executable(SocketWrapperTest test/SocketWrapperTest.cpp)
target_link_libraries(SocketWrapperTest gtest gtest_main)

# Link winsock2 on Windows for tests
if(WIN32)
    target_link_libraries(SocketWrapperTest ws2_32)
endif()

add_test(NAME SocketWrapperTest COMMAND SocketWrapperTest)