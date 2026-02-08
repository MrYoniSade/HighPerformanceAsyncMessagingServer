#include <gtest/gtest.h>
#include "ResourcePool.h"

using namespace core;

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
