#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>
#include <vector>
#include <atomic>

namespace core {

class ThreadPool final {
public:
    /**
     * @brief Construct a ThreadPool with specified number of worker threads
     * @param numThreads Number of worker threads to create. If 0, uses hardware concurrency
     */
    explicit ThreadPool(size_t numThreads = 0);

    /**
     * @brief Destructor that gracefully shuts down the thread pool
     */
    ~ThreadPool();

    // Delete copy operations
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Move operations
    ThreadPool(ThreadPool&& other) noexcept;
    ThreadPool& operator=(ThreadPool&& other) noexcept;

    /**
     * @brief Submit a task to the thread pool
     * @tparam Func Type of the callable
     * @tparam Args Types of arguments to forward to the function
     * @param func Callable to execute
     * @param args Arguments to pass to func
     * @return std::future<T> where T is the return type of func
     */
    template<typename Func, typename... Args>
    auto submit(Func&& func, Args&&... args) -> std::future<std::invoke_result_t<Func, Args...>>
    {
        using return_type = std::invoke_result_t<Func, Args...>;

        // Create a packaged_task that captures the function and arguments
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);

            // Check if thread pool is shutdown
            if (m_shutdown) {
                throw std::runtime_error("Cannot submit tasks to a shutdown ThreadPool");
            }

            // Enqueue the task
            m_task_queue.push([task]() { (*task)(); });
        }

        // Notify one waiting worker thread
        m_condition_variable.notify_one();

        return result;
    }

    /**
     * @brief Wait for all submitted tasks to complete and shutdown the thread pool
     */
    void shutdown();

    /**
     * @brief Check if the thread pool is shutdown
     * @return true if shutdown, false otherwise
     */
    [[nodiscard]] bool isShutdown() const noexcept {
        return m_shutdown.load(std::memory_order_acquire);
    }

    /**
     * @brief Get the number of worker threads
     * @return Number of active worker threads
     */
    [[nodiscard]] size_t getNumThreads() const noexcept {
        return m_workers.size();
    }

private:
    using Task = std::function<void()>;

    /**
     * @brief Worker thread main loop
     */
    void workerLoop();

    std::vector<std::thread> m_workers;           // Worker thread handles
    std::queue<Task> m_task_queue;                // Task queue
    mutable std::mutex m_queue_mutex;             // Synchronizes access to queue
    std::condition_variable m_condition_variable; // Notifies workers of new tasks
    std::atomic<bool> m_shutdown{false};          // Flag indicating shutdown requested
};

} // namespace core
