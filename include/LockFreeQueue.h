#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include <stdexcept>

namespace core {

/**
 * @brief Multi-Producer, Multi-Consumer lock-free queue using a ring buffer
 * @tparam T Type of elements to store
 * @tparam Capacity Maximum number of elements (must be power of 2)
 * 
 * Features:
 * - No locks, no mutexes
 * - Atomic operations with proper memory ordering
 * - ABA problem prevention with versioned indices
 * - Bounded capacity (Capacity - 1 usable slots to distinguish full from empty)
 */
template<typename T, size_t Capacity>
class LockFreeQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
    static_assert(Capacity > 1, "Capacity must be greater than 1");

public:
    /**
     * @brief Construct a lock-free queue
     */
    LockFreeQueue() noexcept
        : m_buffer(std::make_unique<T[]>(Capacity))
        , m_enqueue_pos(0)
        , m_dequeue_pos(0)
    {
    }

    // Delete copy operations
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    // Delete move operations (queue should be statically allocated)
    LockFreeQueue(LockFreeQueue&&) = delete;
    LockFreeQueue& operator=(LockFreeQueue&&) = delete;

    /**
     * @brief Try to enqueue an element
     * @param value Element to enqueue
     * @return true if enqueue succeeded, false if queue is full
     */
    bool try_enqueue(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        uint64_t pos = m_enqueue_pos.load(std::memory_order_acquire);

        while (true) {
            uint64_t dequeue_pos = m_dequeue_pos.load(std::memory_order_acquire);
            uint64_t next_pos = pos + 1;

            // Check if queue is full (wraparound safety)
            // We can't store when (next_pos % Capacity) == (dequeue_pos % Capacity)
            // because that would make it indistinguishable from empty
            if (((next_pos) & MASK) == (dequeue_pos & MASK)) {
                return false;
            }

            // Try to claim enqueue position
            if (m_enqueue_pos.compare_exchange_weak(pos, next_pos,
                                                   std::memory_order_release,
                                                   std::memory_order_acquire))
            {
                // Successfully claimed position, write value
                size_t index = (pos) & MASK;
                m_buffer[index] = value;
                return true;
            }
        }
    }

    /**
     * @brief Try to dequeue an element
     * @param value Output parameter for dequeued element
     * @return true if dequeue succeeded, false if queue is empty
     */
    bool try_dequeue(T& value) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        uint64_t pos = m_dequeue_pos.load(std::memory_order_acquire);

        while (true) {
            uint64_t enqueue_pos = m_enqueue_pos.load(std::memory_order_acquire);

            // Queue is empty if dequeue position catches enqueue position
            if ((pos & MASK) == (enqueue_pos & MASK)) {
                return false;
            }

            // Try to claim dequeue position
            if (m_dequeue_pos.compare_exchange_weak(pos, pos + 1,
                                                   std::memory_order_release,
                                                   std::memory_order_acquire))
            {
                // Successfully claimed position, read value
                size_t index = (pos) & MASK;
                value = std::move(m_buffer[index]);
                return true;
            }
        }
    }

    /**
     * @brief Check if queue is empty
     * @return true if queue appears empty at this moment
     */
    [[nodiscard]] bool is_empty() const noexcept
    {
        auto enqueue = m_enqueue_pos.load(std::memory_order_acquire);
        auto dequeue = m_dequeue_pos.load(std::memory_order_acquire);
        return (enqueue & MASK) == (dequeue & MASK);
    }

    /**
     * @brief Approximate size of queue
     * @return Approximate number of elements in queue
     */
    [[nodiscard]] size_t approximate_size() const noexcept
    {
        uint64_t enqueue = m_enqueue_pos.load(std::memory_order_acquire);
        uint64_t dequeue = m_dequeue_pos.load(std::memory_order_acquire);
        return static_cast<size_t>(enqueue - dequeue);
    }

    /**
     * @brief Get maximum usable capacity
     * @return Queue usable capacity (Capacity - 1)
     */
    [[nodiscard]] static constexpr size_t capacity() noexcept
    {
        return Capacity - 1;
    }

private:
    static constexpr uint64_t MASK = Capacity - 1;

    std::unique_ptr<T[]> m_buffer;              // Ring buffer storage
    std::atomic<uint64_t> m_enqueue_pos{0};     // Enqueue position (64-bit for wraparound)
    std::atomic<uint64_t> m_dequeue_pos{0};     // Dequeue position (64-bit for wraparound)
};

} // namespace core
