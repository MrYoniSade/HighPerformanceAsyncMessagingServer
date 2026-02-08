#include "BitPackUtils.h"
#include <algorithm>

namespace core {
namespace protocol {

size_t BitPackUtils::pack_bits(uint8_t* buffer,
                              size_t offset,
                              uint32_t value,
                              size_t num_bits) noexcept
{
    if (num_bits == 0 || num_bits > 32) {
        return offset;
    }

    size_t byte_pos = offset / 8;
    size_t bit_pos = offset % 8;

    // Mask value to requested bits
    uint32_t mask = (1ULL << num_bits) - 1;
    value &= mask;

    size_t bits_written = 0;
    while (bits_written < num_bits) {
        size_t bits_in_current_byte = 8 - bit_pos;
        size_t bits_to_write = std::min(bits_in_current_byte, num_bits - bits_written);

        uint8_t byte_mask = (1U << bits_to_write) - 1;
        uint8_t bits_value = (value >> bits_written) & byte_mask;
        
        buffer[byte_pos] |= (bits_value << bit_pos);

        bits_written += bits_to_write;
        bit_pos += bits_to_write;

        if (bit_pos >= 8) {
            byte_pos++;
            bit_pos = 0;
        }
    }

    return offset + num_bits;
}

uint32_t BitPackUtils::unpack_bits(const uint8_t* buffer,
                                  size_t offset,
                                  size_t num_bits) noexcept
{
    if (num_bits == 0 || num_bits > 32) {
        return 0;
    }

    size_t byte_pos = offset / 8;
    size_t bit_pos = offset % 8;

    uint32_t result = 0;
    size_t bits_read = 0;

    while (bits_read < num_bits) {
        size_t bits_in_current_byte = 8 - bit_pos;
        size_t bits_to_read = std::min(bits_in_current_byte, num_bits - bits_read);

        uint8_t byte_mask = (1U << bits_to_read) - 1;
        uint8_t bits_value = (buffer[byte_pos] >> bit_pos) & byte_mask;
        
        result |= (static_cast<uint32_t>(bits_value) << bits_read);

        bits_read += bits_to_read;
        bit_pos += bits_to_read;

        if (bit_pos >= 8) {
            byte_pos++;
            bit_pos = 0;
        }
    }

    return result;
}

size_t BitPackUtils::pack_bool(uint8_t* buffer,
                              size_t offset,
                              bool value) noexcept
{
    return pack_bits(buffer, offset, value ? 1 : 0, 1);
}

bool BitPackUtils::unpack_bool(const uint8_t* buffer,
                              size_t offset) noexcept
{
    return unpack_bits(buffer, offset, 1) != 0;
}

size_t BitPackUtils::pack_uint8(uint8_t* buffer,
                               size_t offset,
                               uint8_t value) noexcept
{
    return pack_bits(buffer, offset, value, 8);
}

uint8_t BitPackUtils::unpack_uint8(const uint8_t* buffer,
                                  size_t offset) noexcept
{
    return static_cast<uint8_t>(unpack_bits(buffer, offset, 8));
}

size_t BitPackUtils::pack_uint16(uint8_t* buffer,
                                size_t offset,
                                uint16_t value) noexcept
{
    // Little-endian: low byte first
    offset = pack_bits(buffer, offset, value & 0xFF, 8);
    return pack_bits(buffer, offset, (value >> 8) & 0xFF, 8);
}

uint16_t BitPackUtils::unpack_uint16(const uint8_t* buffer,
                                    size_t offset) noexcept
{
    uint16_t low = unpack_bits(buffer, offset, 8);
    uint16_t high = unpack_bits(buffer, offset + 8, 8);
    return low | (high << 8);
}

size_t BitPackUtils::pack_uint32(uint8_t* buffer,
                                size_t offset,
                                uint32_t value) noexcept
{
    // Little-endian: bytes in order
    offset = pack_bits(buffer, offset, value & 0xFF, 8);
    offset = pack_bits(buffer, offset, (value >> 8) & 0xFF, 8);
    offset = pack_bits(buffer, offset, (value >> 16) & 0xFF, 8);
    return pack_bits(buffer, offset, (value >> 24) & 0xFF, 8);
}

uint32_t BitPackUtils::unpack_uint32(const uint8_t* buffer,
                                    size_t offset) noexcept
{
    uint32_t b0 = unpack_bits(buffer, offset, 8);
    uint32_t b1 = unpack_bits(buffer, offset + 8, 8);
    uint32_t b2 = unpack_bits(buffer, offset + 16, 8);
    uint32_t b3 = unpack_bits(buffer, offset + 24, 8);
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

} // namespace protocol
} // namespace core
