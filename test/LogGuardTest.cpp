#include <gtest/gtest.h>
#include "LogGuard.h"
#include <filesystem>

using namespace core;

class LogGuardTest : public ::testing::Test {
protected:
    std::string test_log_file = "test_log_guard.txt";
    
    void TearDown() override {
        // Clean up log file
        std::filesystem::remove(test_log_file);
    }
};

TEST_F(LogGuardTest, ConstructorDestructor) {
    {
        LogGuard guard(test_log_file);
        EXPECT_TRUE(guard.is_open());
    }
    
    // File should still exist after guard destruction
    EXPECT_TRUE(std::filesystem::exists(test_log_file));
}

TEST_F(LogGuardTest, LogMessage) {
    {
        LogGuard guard(test_log_file);
        guard.log("Test message");
    }
    
    // Check that file contains message
    std::ifstream file(test_log_file);
    std::string content((std::istreambuf_iterator<char>(file)), 
                        std::istreambuf_iterator<char>());
    
    EXPECT_NE(content.find("Test message"), std::string::npos);
}

TEST_F(LogGuardTest, MultipleMessages) {
    {
        LogGuard guard(test_log_file);
        guard.log("Message 1");
        guard.log("Message 2");
        guard.log("Message 3");
    }
    
    std::ifstream file(test_log_file);
    std::string content((std::istreambuf_iterator<char>(file)), 
                        std::istreambuf_iterator<char>());
    
    EXPECT_NE(content.find("Message 1"), std::string::npos);
    EXPECT_NE(content.find("Message 2"), std::string::npos);
    EXPECT_NE(content.find("Message 3"), std::string::npos);
}

TEST_F(LogGuardTest, Filename) {
    LogGuard guard(test_log_file);
    EXPECT_EQ(guard.filename(), test_log_file);
}

TEST_F(LogGuardTest, InvalidFile) {
    EXPECT_THROW(LogGuard("invalid/path/that/does/not/exist.txt"), std::runtime_error);
}
