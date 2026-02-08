#pragma once

#include <cstdint>
#include <bit>

namespace core {
namespace protocol {

/**
 * @brief Endianness utilities and detection
 * 
 * Demonstrates:
 * - Compile-time endianness detection
 * - Little-endian/Big-endian conversion
 * - Cross-platform byte swapping
 */
class EndianUtils {
public:
    /**
     * @brief Byte order enumeration
     */
    enum class ByteOrder {
        LITTLE_ENDIAN,
        BIG_ENDIAN
    };

    /**
     * @brief Detect system byte order at compile-time (C++20)
     */
    [[nodiscard]] static constexpr ByteOrder detect_byte_order() noexcept {
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            return ByteOrder::LITTLE_ENDIAN;
        #else
            return ByteOrder::BIG_ENDIAN;
        #endif
#elif defined(_WIN32)
        // Windows is typically little-endian
        return ByteOrder::LITTLE_ENDIAN;
#else
        // Safe default
        return ByteOrder::LITTLE_ENDIAN;
#endif
    }

    // Current system byte order - Windows is always little-endian
    static constexpr ByteOrder SYSTEM_BYTE_ORDER = ByteOrder::LITTLE_ENDIAN;
    static constexpr bool IS_LITTLE_ENDIAN = true;
    static constexpr bool IS_BIG_ENDIAN = false;

    /**
     * @brief Swap bytes in 16-bit value
     */
    [[nodiscard]] static constexpr uint16_t swap_uint16(uint16_t value) noexcept {
        return static_cast<uint16_t>(
            ((value & 0xFF) << 8) |
            ((value >> 8) & 0xFF)
        );
    }

    /**
     * @brief Swap bytes in 32-bit value
     */
    [[nodiscard]] static constexpr uint32_t swap_uint32(uint32_t value) noexcept {
        return ((value & 0xFF) << 24) |
               (((value >> 8) & 0xFF) << 16) |
               (((value >> 16) & 0xFF) << 8) |
               ((value >> 24) & 0xFF);
    }

    /**
     * @brief Swap bytes in 64-bit value
     */
    [[nodiscard]] static constexpr uint64_t swap_uint64(uint64_t value) noexcept {
        return ((value & 0xFF) << 56) |
               (((value >> 8) & 0xFF) << 48) |
               (((value >> 16) & 0xFF) << 40) |
               (((value >> 24) & 0xFF) << 32) |
               (((value >> 32) & 0xFF) << 24) |
               (((value >> 40) & 0xFF) << 16) |
               (((value >> 48) & 0xFF) << 8) |
               ((value >> 56) & 0xFF);
    }

    /**
     * @brief Convert to little-endian
     */
    [[nodiscard]] static constexpr uint16_t to_little_endian(uint16_t value) noexcept {
        return IS_LITTLE_ENDIAN ? value : swap_uint16(value);
    }

    [[nodiscard]] static constexpr uint32_t to_little_endian(uint32_t value) noexcept {
        return IS_LITTLE_ENDIAN ? value : swap_uint32(value);
    }

    [[nodiscard]] static constexpr uint64_t to_little_endian(uint64_t value) noexcept {
        return IS_LITTLE_ENDIAN ? value : swap_uint64(value);
    }

    /**
     * @brief Convert from little-endian
     */
    [[nodiscard]] static constexpr uint16_t from_little_endian(uint16_t value) noexcept {
        return IS_LITTLE_ENDIAN ? value : swap_uint16(value);
    }

    [[nodiscard]] static constexpr uint32_t from_little_endian(uint32_t value) noexcept {
        return IS_LITTLE_ENDIAN ? value : swap_uint32(value);
    }

    [[nodiscard]] static constexpr uint64_t from_little_endian(uint64_t value) noexcept {
        return IS_LITTLE_ENDIAN ? value : swap_uint64(value);
    }

    /**
     * @brief Convert to big-endian
     */
    [[nodiscard]] static constexpr uint16_t to_big_endian(uint16_t value) noexcept {
        return IS_BIG_ENDIAN ? value : swap_uint16(value);
    }

    [[nodiscard]] static constexpr uint32_t to_big_endian(uint32_t value) noexcept {
        return IS_BIG_ENDIAN ? value : swap_uint32(value);
    }

    [[nodiscard]] static constexpr uint64_t to_big_endian(uint64_t value) noexcept {
        return IS_BIG_ENDIAN ? value : swap_uint64(value);
    }

    /**
     * @brief Convert from big-endian
     */
    [[nodiscard]] static constexpr uint16_t from_big_endian(uint16_t value) noexcept {
        return IS_BIG_ENDIAN ? value : swap_uint16(value);
    }

    [[nodiscard]] static constexpr uint32_t from_big_endian(uint32_t value) noexcept {
        return IS_BIG_ENDIAN ? value : swap_uint32(value);
    }

    [[nodiscard]] static constexpr uint64_t from_big_endian(uint64_t value) noexcept {
        return IS_BIG_ENDIAN ? value : swap_uint64(value);
    }
};

} // namespace protocol
} // namespace core
