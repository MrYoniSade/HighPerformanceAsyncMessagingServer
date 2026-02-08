#pragma once

#include "BufferWrapper.h"
#include <cstring>
#include <stdexcept>

namespace core {
namespace net {

/**
 * @brief Network packet buffer for serialization/deserialization
 * 
 * Demonstrates:
 * - RAII-based buffer management
 * - Efficient memory usage with BufferWrapper
 * - Read/write position tracking
 */
class NetworkBuffer {
public:
    /**
     * @brief Construct network buffer
     * @param initial_capacity Initial buffer capacity
     */
    explicit NetworkBuffer(size_t initial_capacity = 4096)
        : m_buffer(initial_capacity)
        , m_write_pos(0)
        , m_read_pos(0)
    {
    }

    /**
     * @brief Write data to buffer
     * @param data Data to write
     * @param length Data length
     * @return true if successful
     */
    bool write(const uint8_t* data, size_t length) noexcept
    {
        if (m_write_pos + length > m_buffer.size()) {
            return false; // Buffer full
        }

        std::memcpy(m_buffer.data() + m_write_pos, data, length);
        m_write_pos += length;
        return true;
    }

    /**
     * @brief Write a single byte
     */
    bool write_byte(uint8_t byte) noexcept
    {
        return write(&byte, 1);
    }

    /**
     * @brief Write 16-bit integer (little-endian)
     */
    bool write_uint16(uint16_t value) noexcept
    {
        uint8_t bytes[2] = {
            static_cast<uint8_t>(value & 0xFF),
            static_cast<uint8_t>((value >> 8) & 0xFF)
        };
        return write(bytes, 2);
    }

    /**
     * @brief Write 32-bit integer (little-endian)
     */
    bool write_uint32(uint32_t value) noexcept
    {
        uint8_t bytes[4] = {
            static_cast<uint8_t>(value & 0xFF),
            static_cast<uint8_t>((value >> 8) & 0xFF),
            static_cast<uint8_t>((value >> 16) & 0xFF),
            static_cast<uint8_t>((value >> 24) & 0xFF)
        };
        return write(bytes, 4);
    }

    /**
     * @brief Read data from buffer
     * @param data Output buffer
     * @param length Length to read
     * @return true if successful
     */
    bool read(uint8_t* data, size_t length) noexcept
    {
        if (m_read_pos + length > m_write_pos) {
            return false; // Not enough data
        }

        std::memcpy(data, m_buffer.data() + m_read_pos, length);
        m_read_pos += length;
        return true;
    }

    /**
     * @brief Read a single byte
     */
    bool read_byte(uint8_t& value) noexcept
    {
        return read(&value, 1);
    }

    /**
     * @brief Read 16-bit integer (little-endian)
     */
    bool read_uint16(uint16_t& value) noexcept
    {
        uint8_t bytes[2];
        if (!read(bytes, 2)) {
            return false;
        }

        value = static_cast<uint16_t>(bytes[0]) |
               (static_cast<uint16_t>(bytes[1]) << 8);
        return true;
    }

    /**
     * @brief Read 32-bit integer (little-endian)
     */
    bool read_uint32(uint32_t& value) noexcept
    {
        uint8_t bytes[4];
        if (!read(bytes, 4)) {
            return false;
        }

        value = static_cast<uint32_t>(bytes[0]) |
               (static_cast<uint32_t>(bytes[1]) << 8) |
               (static_cast<uint32_t>(bytes[2]) << 16) |
               (static_cast<uint32_t>(bytes[3]) << 24);
        return true;
    }

    /**
     * @brief Get raw buffer data
     */
    [[nodiscard]] uint8_t* data() noexcept
    {
        return m_buffer.data();
    }

    /**
     * @brief Get const buffer data
     */
    [[nodiscard]] const uint8_t* data() const noexcept
    {
        return m_buffer.data();
    }

    /**
     * @brief Get buffer size
     */
    [[nodiscard]] size_t size() const noexcept
    {
        return m_buffer.size();
    }

    /**
     * @brief Get write position
     */
    [[nodiscard]] size_t write_pos() const noexcept
    {
        return m_write_pos;
    }

    /**
     * @brief Get read position
     */
    [[nodiscard]] size_t read_pos() const noexcept
    {
        return m_read_pos;
    }

    /**
     * @brief Get available bytes to read
     */
    [[nodiscard]] size_t available_read() const noexcept
    {
        return m_write_pos - m_read_pos;
    }

    /**
     * @brief Get available space to write
     */
    [[nodiscard]] size_t available_write() const noexcept
    {
        return m_buffer.size() - m_write_pos;
    }

    /**
     * @brief Reset buffer positions
     */
    void reset() noexcept
    {
        m_write_pos = 0;
        m_read_pos = 0;
    }

    /**
     * @brief Reset read position (for re-reading)
     */
    void reset_read() noexcept
    {
        m_read_pos = 0;
    }

    /**
     * @brief Clear buffer
     */
    void clear() noexcept
    {
        m_buffer.clear();
        reset();
    }

private:
    BufferWrapper<uint8_t> m_buffer;
    size_t m_write_pos;
    size_t m_read_pos;
};

} // namespace net
} // namespace core
