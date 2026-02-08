#pragma once

#include <cstring>
#include <memory>
#include <stdexcept>

namespace core {

/**
 * @brief RAII wrapper for dynamically allocated buffers
 * @tparam T Element type
 * 
 * Demonstrates:
 * - RAII pattern for memory management
 * - Rule of 5 (constructor, destructor, move, copy)
 * - Automatic cleanup on destruction
 */
template<typename T = uint8_t>
class BufferWrapper {
public:
    /**
     * @brief Construct a buffer of given size
     * @param size Number of elements to allocate
     * @throws std::bad_alloc if allocation fails
     */
    explicit BufferWrapper(size_t size)
        : m_buffer(std::make_unique<T[]>(size))
        , m_size(size)
    {
        if (size == 0) {
            throw std::invalid_argument("BufferWrapper size must be greater than 0");
        }
    }

    /**
     * @brief Destructor - automatically frees buffer via unique_ptr
     */
    ~BufferWrapper() noexcept = default;

    // Copy operations - deleted (unique ownership)
    BufferWrapper(const BufferWrapper&) = delete;
    BufferWrapper& operator=(const BufferWrapper&) = delete;

    /**
     * @brief Move constructor
     */
    BufferWrapper(BufferWrapper&& other) noexcept
        : m_buffer(std::move(other.m_buffer))
        , m_size(other.m_size)
    {
        other.m_size = 0;
    }

    /**
     * @brief Move assignment operator
     */
    BufferWrapper& operator=(BufferWrapper&& other) noexcept
    {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_size = other.m_size;
            other.m_size = 0;
        }
        return *this;
    }

    /**
     * @brief Get pointer to buffer data
     */
    [[nodiscard]] T* data() noexcept
    {
        return m_buffer.get();
    }

    /**
     * @brief Get const pointer to buffer data
     */
    [[nodiscard]] const T* data() const noexcept
    {
        return m_buffer.get();
    }

    /**
     * @brief Get buffer size
     */
    [[nodiscard]] size_t size() const noexcept
    {
        return m_size;
    }

    /**
     * @brief Check if buffer is valid
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
        return m_buffer != nullptr && m_size > 0;
    }

    /**
     * @brief Array access operator
     */
    [[nodiscard]] T& operator[](size_t index)
    {
        if (index >= m_size) {
            throw std::out_of_range("BufferWrapper index out of range");
        }
        return m_buffer[index];
    }

    /**
     * @brief Const array access operator
     */
    [[nodiscard]] const T& operator[](size_t index) const
    {
        if (index >= m_size) {
            throw std::out_of_range("BufferWrapper index out of range");
        }
        return m_buffer[index];
    }

    /**
     * @brief Fill buffer with value
     */
    void fill(const T& value) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        for (size_t i = 0; i < m_size; ++i) {
            m_buffer[i] = value;
        }
    }

    /**
     * @brief Clear buffer (fill with zeros)
     */
    void clear() noexcept
    {
        std::memset(m_buffer.get(), 0, m_size * sizeof(T));
    }

private:
    std::unique_ptr<T[]> m_buffer;
    size_t m_size;
};

} // namespace core
