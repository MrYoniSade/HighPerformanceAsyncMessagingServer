#include "ThreadPool.h"
#include <iostream>

namespace core {

ThreadPool::ThreadPool(size_t numThreads)
{
    // Use hardware concurrency if numThreads is 0
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) {
            numThreads = 2; // Fallback if detection fails
        }
    }

    // Create worker threads
    m_workers.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this]() { workerLoop(); });
    }
}

ThreadPool::~ThreadPool()
{
    // Ensure shutdown is called
    if (!m_shutdown) {
        shutdown();
    }
}

ThreadPool::ThreadPool(ThreadPool&& other) noexcept
    : m_workers(std::move(other.m_workers)),
      m_task_queue(std::move(other.m_task_queue)),
      m_shutdown(other.m_shutdown.load())
{
    other.m_shutdown = true; // Mark moved-from as shutdown to prevent double-shutdown
}

ThreadPool& ThreadPool::operator=(ThreadPool&& other) noexcept
{
    if (this != &other) {
        if (!m_shutdown) {
            shutdown();
        }
        m_workers = std::move(other.m_workers);
        m_task_queue = std::move(other.m_task_queue);
        m_shutdown.store(other.m_shutdown.load());
        other.m_shutdown = true;
    }
    return *this;
}

void ThreadPool::workerLoop()
{
    while (true) {
        Task task;

        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);

            // Wait for a task or shutdown signal
            m_condition_variable.wait(lock, [this]() {
                return !m_task_queue.empty() || m_shutdown;
            });

            // Exit if shutdown and queue is empty
            if (m_shutdown && m_task_queue.empty()) {
                break;
            }

            // Get the next task from the queue
            if (!m_task_queue.empty()) {
                task = std::move(m_task_queue.front());
                m_task_queue.pop();
            } else {
                // Spurious wakeup or shutdown during grab attempt
                continue;
            }
        }

        // Execute the task outside the lock
        if (task) {
            try {
                task();
            } catch (const std::exception& e) {
                // Log exception but continue processing tasks
                std::cerr << "ThreadPool task exception: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "ThreadPool task unknown exception" << std::endl;
            }
        }
    }
}

void ThreadPool::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_shutdown.store(true, std::memory_order_release);
    }

    // Notify all workers to wake up and check shutdown
    m_condition_variable.notify_all();

    // Join all worker threads
    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

} // namespace core
