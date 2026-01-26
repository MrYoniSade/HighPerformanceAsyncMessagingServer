# GoogleTest integration
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# SocketWrapper tests
add_executable(SocketWrapperTest SocketWrapperTest.cpp)
target_link_libraries(SocketWrapperTest gtest gtest_main)

if(WIN32)
    target_link_libraries(SocketWrapperTest ws2_32)
endif()

add_test(NAME SocketWrapperTest COMMAND SocketWrapperTest)