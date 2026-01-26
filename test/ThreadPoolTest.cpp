#include <gtest/gtest.h>
#include "ThreadPool.h"
#include <chrono>
#include <atomic>

using namespace core;

class ThreadPoolTest : public ::testing::Test {
protected:
    // Create a default thread pool before each test
    void SetUp() override {
        pool = std::make_unique<ThreadPool>(2);
    }

    // Shutdown the thread pool after each test
    void TearDown() override {
        if (pool && !pool->isShutdown()) {
            pool->shutdown();
        }
    }

    std::unique_ptr<ThreadPool> pool;
};

// Test basic task submission and execution
TEST_F(ThreadPoolTest, SubmitAndExecuteTask) {
    std::atomic<int> result(0);

    auto future = pool->submit([&result]() {
        result.store(42, std::memory_order_release);
        return 42;
    });

    int value = future.get();
    EXPECT_EQ(value, 42);
    EXPECT_EQ(result.load(), 42);
}

// Test submit with arguments
TEST_F(ThreadPoolTest, SubmitWithArguments) {
    auto future = pool->submit([](int a, int b) {
        return a + b;
    }, 10, 20);

    int result = future.get();
    EXPECT_EQ(result, 30);
}

// Test multiple task submissions
TEST_F(ThreadPoolTest, MultipleTasksExecution) {
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool->submit([i]() {
            return i * 2;
        }));
    }

    for (size_t i = 0; i < futures.size(); ++i) {
        int result = futures[i].get();
        EXPECT_EQ(result, static_cast<int>(i) * 2);
    }
}

// Test task that returns void
TEST_F(ThreadPoolTest, TaskReturningVoid) {
    std::atomic<bool> executed(false);

    auto future = pool->submit([&executed]() {
        executed.store(true, std::memory_order_release);
    });

    future.get();
    EXPECT_TRUE(executed.load());
}

// Test concurrent execution
TEST_F(ThreadPoolTest, ConcurrentExecution) {
    std::atomic<int> counter(0);
    std::vector<std::future<void>> futures;

    for (int i = 0; i < 5; ++i) {
        futures.push_back(pool->submit([&counter]() {
            counter.fetch_add(1, std::memory_order_acq_rel);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    EXPECT_EQ(counter.load(), 5);
}

// Test shutdown
TEST_F(ThreadPoolTest, Shutdown) {
    EXPECT_FALSE(pool->isShutdown());
    pool->shutdown();
    EXPECT_TRUE(pool->isShutdown());

    // Cannot submit after shutdown
    EXPECT_THROW(pool->submit([]() { return 1; }), std::runtime_error);
}

// Test that shutdown completes pending tasks
TEST_F(ThreadPoolTest, ShutdownCompletesTask) {
    std::atomic<bool> executed(false);

    auto future = pool->submit([&executed]() {
        executed.store(true, std::memory_order_release);
    });

    pool->shutdown();

    // Task should have been executed before shutdown completed
    EXPECT_TRUE(executed.load());
}

// Test constructor with explicit thread count
TEST_F(ThreadPoolTest, ExplicitThreadCount) {
    auto custom_pool = std::make_unique<ThreadPool>(4);
    EXPECT_EQ(custom_pool->getNumThreads(), 4);
    custom_pool->shutdown();
}

// Test default constructor (uses hardware concurrency)
TEST_F(ThreadPoolTest, DefaultThreadCount) {
    auto default_pool = std::make_unique<ThreadPool>();
    EXPECT_GT(default_pool->getNumThreads(), 0);
    EXPECT_LE(default_pool->getNumThreads(), std::thread::hardware_concurrency() + 1);
    default_pool->shutdown();
}

// Test move semantics
TEST_F(ThreadPoolTest, MoveSemantics) {
    auto pool1 = std::make_unique<ThreadPool>(2);
    size_t threads = pool1->getNumThreads();

    ThreadPool pool2 = std::move(*pool1);

    EXPECT_EQ(pool2.getNumThreads(), threads);
    EXPECT_TRUE(pool1->isShutdown());
    
    pool2.shutdown();
}

// Test exception handling in task
TEST_F(ThreadPoolTest, ExceptionInTask) {
    auto future = pool->submit([]() {
        throw std::runtime_error("Test exception");
        return 42;
    });

    // The exception should be stored in the future
    EXPECT_THROW(future.get(), std::runtime_error);
}

// Test many rapid task submissions
TEST_F(ThreadPoolTest, RapidSubmissions) {
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 100; ++i) {
        futures.push_back(pool->submit([i]() {
            return i;
        }));
    }

    for (size_t i = 0; i < futures.size(); ++i) {
        int result = futures[i].get();
        EXPECT_EQ(result, static_cast<int>(i));
    }
}

// Test lambda with capture
TEST_F(ThreadPoolTest, LambdaWithCapture) {
    int value = 100;
    std::string prefix = "Result: ";

    auto future = pool->submit([value, prefix]() {
        return prefix + std::to_string(value);
    });

    std::string result = future.get();
    EXPECT_EQ(result, "Result: 100");
}
