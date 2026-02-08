#pragma once

#include <cstdint>
#include <cstring>

namespace core {
namespace protocol {

/**
 * @brief Utilities for bit-packing and bit-unpacking operations
 * 
 * Demonstrates:
 * - Efficient bit-level operations
 * - Endianness handling for multi-byte values
 * - Compile-time bit manipulation
 */
class BitPackUtils {
public:
    /**
     * @brief Pack bits into a buffer
     * @param buffer Target buffer
     * @param offset Bit offset to start writing
     * @param value Value to pack
     * @param num_bits Number of bits to pack (1-32)
     * @return New bit offset after packing
     */
    static size_t pack_bits(uint8_t* buffer,
                           size_t offset,
                           uint32_t value,
                           size_t num_bits) noexcept;

    /**
     * @brief Unpack bits from a buffer
     * @param buffer Source buffer
     * @param offset Bit offset to start reading
     * @param num_bits Number of bits to unpack (1-32)
     * @return Unpacked value
     */
    static uint32_t unpack_bits(const uint8_t* buffer,
                               size_t offset,
                               size_t num_bits) noexcept;

    /**
     * @brief Pack a boolean into bits
     */
    static size_t pack_bool(uint8_t* buffer,
                           size_t offset,
                           bool value) noexcept;

    /**
     * @brief Unpack a boolean from bits
     */
    static bool unpack_bool(const uint8_t* buffer,
                           size_t offset) noexcept;

    /**
     * @brief Pack an 8-bit value
     */
    static size_t pack_uint8(uint8_t* buffer,
                            size_t offset,
                            uint8_t value) noexcept;

    /**
     * @brief Unpack an 8-bit value
     */
    static uint8_t unpack_uint8(const uint8_t* buffer,
                               size_t offset) noexcept;

    /**
     * @brief Pack a 16-bit value (little-endian)
     */
    static size_t pack_uint16(uint8_t* buffer,
                             size_t offset,
                             uint16_t value) noexcept;

    /**
     * @brief Unpack a 16-bit value (little-endian)
     */
    static uint16_t unpack_uint16(const uint8_t* buffer,
                                 size_t offset) noexcept;

    /**
     * @brief Pack a 32-bit value (little-endian)
     */
    static size_t pack_uint32(uint8_t* buffer,
                             size_t offset,
                             uint32_t value) noexcept;

    /**
     * @brief Unpack a 32-bit value (little-endian)
     */
    static uint32_t unpack_uint32(const uint8_t* buffer,
                                 size_t offset) noexcept;

    /**
     * @brief Align offset to next byte boundary
     */
    [[nodiscard]] static constexpr size_t align_to_byte(size_t bit_offset) noexcept {
        return (bit_offset + 7) / 8 * 8;
    }

    /**
     * @brief Get byte offset from bit offset
     */
    [[nodiscard]] static constexpr size_t byte_offset(size_t bit_offset) noexcept {
        return bit_offset / 8;
    }

    /**
     * @brief Get bit position within byte
     */
    [[nodiscard]] static constexpr size_t bit_in_byte(size_t bit_offset) noexcept {
        return bit_offset % 8;
    }
};

} // namespace protocol
} // namespace core
