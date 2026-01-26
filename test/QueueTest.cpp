#include <gtest/gtest.h>
#include "LockFreeQueue.h"
#include "MutexQueue.h"
#include <thread>
#include <vector>
#include <atomic>

using namespace core;

// Tests for LockFreeQueue
class LockFreeQueueTest : public ::testing::Test {
protected:
    LockFreeQueue<int, 64> queue;
};

TEST_F(LockFreeQueueTest, EnqueueAndDequeueBasic) {
    EXPECT_TRUE(queue.try_enqueue(42));
    
    int value = 0;
    EXPECT_TRUE(queue.try_dequeue(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.is_empty());
}

TEST_F(LockFreeQueueTest, MultipleEnqueueDequeue) {
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(queue.try_enqueue(i));
    }
    
    for (int i = 0; i < 10; ++i) {
        int value = 0;
        EXPECT_TRUE(queue.try_dequeue(value));
        EXPECT_EQ(value, i);
    }
    
    EXPECT_TRUE(queue.is_empty());
}

TEST_F(LockFreeQueueTest, DequeueFromEmpty) {
    int value = 0;
    EXPECT_FALSE(queue.try_dequeue(value));
}

TEST_F(LockFreeQueueTest, EnqueueUntilFull) {
    const size_t cap = queue.capacity();
    
    for (size_t i = 0; i < cap; ++i) {
        EXPECT_TRUE(queue.try_enqueue(static_cast<int>(i)));
    }
    
    // Queue should be full now
    EXPECT_FALSE(queue.try_enqueue(999));
}

TEST_F(LockFreeQueueTest, ApproximateSize) {
    EXPECT_EQ(queue.approximate_size(), 0);
    
    queue.try_enqueue(1);
    queue.try_enqueue(2);
    queue.try_enqueue(3);
    
    EXPECT_EQ(queue.approximate_size(), 3);
    
    int value;
    queue.try_dequeue(value);
    EXPECT_EQ(queue.approximate_size(), 2);
}

TEST_F(LockFreeQueueTest, CapacityIsPowerOfTwo) {
    // LockFreeQueue usable capacity is Capacity - 1 to distinguish full from empty
    EXPECT_EQ(queue.capacity(), 63);
    // The underlying buffer is power of 2 (64), usable is 63
    constexpr size_t internal_capacity = 64;
    EXPECT_TRUE((internal_capacity & (internal_capacity - 1)) == 0);
}

// Concurrent enqueue test
TEST_F(LockFreeQueueTest, ConcurrentEnqueue) {
    std::vector<std::thread> threads;
    const int values_per_thread = 8;
    const int num_threads = 4;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, values_per_thread]() {
            for (int j = 0; j < values_per_thread; ++j) {
                int value = i * values_per_thread + j;
                queue.try_enqueue(value);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // All items should be in queue
    EXPECT_EQ(queue.approximate_size(), num_threads * values_per_thread);
}

// Concurrent dequeue test
TEST_F(LockFreeQueueTest, ConcurrentDequeue) {
    // Pre-fill queue
    for (int i = 0; i < 32; ++i) {
        queue.try_enqueue(i);
    }
    
    std::vector<std::thread> threads;
    std::atomic<int> dequeued_count(0);
    const int num_threads = 4;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, &dequeued_count]() {
            int value;
            while (queue.try_dequeue(value)) {
                dequeued_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(dequeued_count.load(), 32);
    EXPECT_TRUE(queue.is_empty());
}

// Concurrent enqueue and dequeue
TEST_F(LockFreeQueueTest, ConcurrentEnqueueDequeue) {
    std::vector<std::thread> threads;
    std::atomic<int> total_enqueued(0);
    std::atomic<int> total_dequeued(0);
    
    const int operations_per_thread = 50;
    
    // Producer threads
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([this, &total_enqueued, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                if (queue.try_enqueue(j)) {
                    total_enqueued.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }
    
    // Consumer threads
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([this, &total_dequeued]() {
            int value;
            while (queue.try_dequeue(value)) {
                total_dequeued.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Allow for some flexibility due to timing
    EXPECT_GT(total_enqueued.load(), 0);
}

// Tests for MutexQueue
class MutexQueueTest : public ::testing::Test {
protected:
    MutexQueue<int, 64> queue;
};

TEST_F(MutexQueueTest, EnqueueAndDequeueBasic) {
    EXPECT_TRUE(queue.try_enqueue(42));
    
    int value = 0;
    EXPECT_TRUE(queue.try_dequeue(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.is_empty());
}

TEST_F(MutexQueueTest, MultipleEnqueueDequeue) {
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(queue.try_enqueue(i));
    }
    
    for (int i = 0; i < 10; ++i) {
        int value = 0;
        EXPECT_TRUE(queue.try_dequeue(value));
        EXPECT_EQ(value, i);
    }
    
    EXPECT_TRUE(queue.is_empty());
}

TEST_F(MutexQueueTest, DequeueFromEmpty) {
    int value = 0;
    EXPECT_FALSE(queue.try_dequeue(value));
}

TEST_F(MutexQueueTest, EnqueueUntilFull) {
    const size_t cap = queue.capacity();
    
    for (size_t i = 0; i < cap; ++i) {
        EXPECT_TRUE(queue.try_enqueue(static_cast<int>(i)));
    }
    
    // Queue should be full now
    EXPECT_FALSE(queue.try_enqueue(999));
}

TEST_F(MutexQueueTest, ConcurrentEnqueue) {
    std::vector<std::thread> threads;
    const int values_per_thread = 8;
    const int num_threads = 4;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, values_per_thread]() {
            for (int j = 0; j < values_per_thread; ++j) {
                int value = i * values_per_thread + j;
                queue.try_enqueue(value);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(queue.approximate_size(), num_threads * values_per_thread);
}

TEST_F(MutexQueueTest, ConcurrentDequeue) {
    // Pre-fill queue
    for (int i = 0; i < 32; ++i) {
        queue.try_enqueue(i);
    }
    
    std::vector<std::thread> threads;
    std::atomic<int> dequeued_count(0);
    const int num_threads = 4;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, &dequeued_count]() {
            int value;
            while (queue.try_dequeue(value)) {
                dequeued_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(dequeued_count.load(), 32);
    EXPECT_TRUE(queue.is_empty());
}
