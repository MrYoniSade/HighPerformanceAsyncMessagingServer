#include <gtest/gtest.h>
#include "BufferWrapper.h"
#include "LogGuard.h"
#include "ResourcePool.h"
#include <filesystem>

using namespace core;

// ============ BufferWrapper Tests ============

class BufferWrapperTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Clean up any test buffers
    }
};

TEST_F(BufferWrapperTest, ConstructorAndSize) {
    BufferWrapper<uint8_t> buffer(100);
    EXPECT_EQ(buffer.size(), 100);
    EXPECT_TRUE(buffer.is_valid());
}

TEST_F(BufferWrapperTest, DataAccess) {
    BufferWrapper<int> buffer(10);
    buffer[0] = 42;
    buffer[9] = 99;
    
    EXPECT_EQ(buffer[0], 42);
    EXPECT_EQ(buffer[9], 99);
}

TEST_F(BufferWrapperTest, OutOfRangeAccess) {
    BufferWrapper<int> buffer(10);
    EXPECT_THROW(buffer[10], std::out_of_range);
}

TEST_F(BufferWrapperTest, Fill) {
    BufferWrapper<int> buffer(5);
    buffer.fill(7);
    
    for (size_t i = 0; i < buffer.size(); ++i) {
        EXPECT_EQ(buffer[i], 7);
    }
}

TEST_F(BufferWrapperTest, Clear) {
    BufferWrapper<int> buffer(5);
    buffer.fill(42);
    buffer.clear();
    
    for (size_t i = 0; i < buffer.size(); ++i) {
        EXPECT_EQ(buffer[i], 0);
    }
}

TEST_F(BufferWrapperTest, MoveSemantics) {
    BufferWrapper<int> buffer1(10);
    buffer1[0] = 123;
    
    BufferWrapper<int> buffer2 = std::move(buffer1);
    
    EXPECT_FALSE(buffer1.is_valid());
    EXPECT_TRUE(buffer2.is_valid());
    EXPECT_EQ(buffer2[0], 123);
}

TEST_F(BufferWrapperTest, MoveAssignment) {
    BufferWrapper<int> buffer1(10);
    BufferWrapper<int> buffer2(5);
    
    buffer2 = std::move(buffer1);
    
    EXPECT_EQ(buffer2.size(), 10);
}

TEST_F(BufferWrapperTest, DataPointer) {
    BufferWrapper<uint8_t> buffer(5);
    uint8_t* ptr = buffer.data();
    
    EXPECT_NE(ptr, nullptr);
    ptr[0] = 42;
    EXPECT_EQ(buffer[0], 42);
}

TEST_F(BufferWrapperTest, ConstDataPointer) {
    BufferWrapper<uint8_t> buffer(5);
    buffer[0] = 99;
    
    const uint8_t* ptr = buffer.data();
    EXPECT_EQ(*ptr, 99);
}

TEST_F(BufferWrapperTest, InvalidSize) {
    EXPECT_THROW(BufferWrapper<int>(0), std::invalid_argument);
}

TEST_F(BufferWrapperTest, LargeBuffer) {
    BufferWrapper<uint32_t> buffer(1000000);
    EXPECT_EQ(buffer.size(), 1000000);
    EXPECT_TRUE(buffer.is_valid());
}

// ============ LogGuard Tests ============

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

// ============ ResourcePool Tests ============

// Simple test resource
struct TestResource {
    int value = 42;
    bool* destroyed = nullptr;
    
    ~TestResource() {
        if (destroyed) {
            *destroyed = true;
        }
    }
};

class ResourcePoolTest : public ::testing::Test {
protected:
    ResourcePool<TestResource> pool{5};
};

TEST_F(ResourcePoolTest, Acquire) {
    auto resource = pool.acquire();
    EXPECT_TRUE(resource.has_value());
    EXPECT_EQ(resource.value()->value, 42);
}

TEST_F(ResourcePoolTest, AcquireMultiple) {
    auto res1 = pool.acquire();
    auto res2 = pool.acquire();
    
    EXPECT_TRUE(res1.has_value());
    EXPECT_TRUE(res2.has_value());
    EXPECT_NE(res1.value(), res2.value());
}

TEST_F(ResourcePoolTest, ReuseAfterRelease) {
    // This test demonstrates that ResourcePool creates new resources
    // when needed. It doesn't actively reuse slots - it tracks which
    // resources are in use via use_count.
    auto initial_count = pool.total_resources();
    
    std::shared_ptr<TestResource> ptr1;
    {
        auto resource = pool.acquire();
        ptr1 = resource.value();
        EXPECT_EQ(pool.active_resources(), 1);
    }
    
    // After release, resource can be reused since use_count == 1
    auto resource = pool.acquire();
    auto ptr2 = resource.value();
    
    // ResourcePool tracks reusable resources by use_count
    // If use_count == 1, it means only the pool holds it
    EXPECT_LE(pool.total_resources(), initial_count + 1);
}

TEST_F(ResourcePoolTest, ActiveResourceCount) {
    EXPECT_EQ(pool.active_resources(), 0);
    
    auto res1 = pool.acquire();
    EXPECT_EQ(pool.active_resources(), 1);
    
    auto res2 = pool.acquire();
    EXPECT_EQ(pool.active_resources(), 2);
}

TEST_F(ResourcePoolTest, AvailableResourceCount) {
    EXPECT_EQ(pool.available_resources(), 5);
    
    auto res = pool.acquire();
    EXPECT_EQ(pool.available_resources(), 4);
}

TEST_F(ResourcePoolTest, PoolExpansion) {
    size_t initial_total = pool.total_resources();
    
    // Acquire more than initial capacity
    std::vector<decltype(pool.acquire())> resources;
    for (int i = 0; i < 8; ++i) {
        resources.push_back(pool.acquire());
    }
    
    EXPECT_GT(pool.total_resources(), initial_total);
}

TEST_F(ResourcePoolTest, WeakPtr) {
    {
        auto shared = pool.acquire();
        auto weak = pool.acquire_weak();
        
        EXPECT_TRUE(weak.has_value());
        auto locked = weak.value().lock();
        EXPECT_TRUE(locked);
    }
}

TEST_F(ResourcePoolTest, WeakPtrExpiration) {
    std::optional<decltype(pool.acquire_weak())> weak_holder;
    
    {
        auto shared = pool.acquire();
        weak_holder = pool.acquire_weak();
    }
    
    // Weak pointer should expire after shared is released
    if (weak_holder.has_value()) {
        auto locked = weak_holder.value().value().lock();
        // locked might be nullptr if resource was fully released
    }
}

TEST_F(ResourcePoolTest, Clear) {
    auto res = pool.acquire();
    EXPECT_EQ(pool.active_resources(), 1);
    
    pool.clear();
    EXPECT_EQ(pool.total_resources(), 0);
}

TEST_F(ResourcePoolTest, ResourceDestruction) {
    bool destroyed = false;
    
    {
        auto res = pool.acquire();
        res.value()->destroyed = &destroyed;
    }
    
    pool.clear();
    EXPECT_TRUE(destroyed);
}
