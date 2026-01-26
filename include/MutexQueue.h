#pragma once

#include <queue>
#include <mutex>
#include <optional>

namespace core {

/**
 * @brief Traditional mutex-based queue for performance comparison
 * @tparam T Type of elements to store
 * @tparam Capacity Maximum number of elements
 */
template<typename T, size_t Capacity>
class MutexQueue {
public:
    /**
     * @brief Construct a mutex-based queue
     */
    MutexQueue() noexcept = default;

    // Delete copy operations
    MutexQueue(const MutexQueue&) = delete;
    MutexQueue& operator=(const MutexQueue&) = delete;

    // Delete move operations
    MutexQueue(MutexQueue&&) = delete;
    MutexQueue& operator=(MutexQueue&&) = delete;

    /**
     * @brief Try to enqueue an element
     * @param value Element to enqueue
     * @return true if enqueue succeeded, false if queue is full
     */
    bool try_enqueue(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_queue.size() >= Capacity) {
            return false;
        }
        
        m_queue.push(value);
        return true;
    }

    /**
     * @brief Try to dequeue an element
     * @param value Output parameter for dequeued element
     * @return true if dequeue succeeded, false if queue is empty
     */
    bool try_dequeue(T& value) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_queue.empty()) {
            return false;
        }
        
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    /**
     * @brief Check if queue is empty
     * @return true if queue is empty
     */
    [[nodiscard]] bool is_empty() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    /**
     * @brief Get approximate size of queue
     * @return Number of elements in queue
     */
    [[nodiscard]] size_t approximate_size() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    /**
     * @brief Get maximum capacity
     * @return Queue capacity
     */
    [[nodiscard]] static constexpr size_t capacity() noexcept
    {
        return Capacity;
    }

private:
    mutable std::mutex m_mutex;
    std::queue<T> m_queue;
};

} // namespace core
