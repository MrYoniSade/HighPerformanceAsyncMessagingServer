#pragma once

#include <cstdint>
#include <array>
#include <optional>

namespace core {
namespace protocol {

/**
 * @brief Binary Protocol Constants and Definitions
 * 
 * Protocol Format (Little-Endian):
 * ???????????????????????????????????????????
 * ? Frame Header (8 bytes)                  ?
 * ???????????????????????????????????????????
 * ? Magic (1 byte): 0xAB                    ?
 * ? Version (1 byte): 0x01                  ?
 * ? Message Type (1 byte): Type ID          ?
 * ? Flags (1 byte): Options                 ?
 * ? Payload Length (2 bytes): Length        ?
 * ? Reserved (2 bytes): Future use          ?
 * ???????????????????????????????????????????
 * ? Payload (Variable)                      ?
 * ???????????????????????????????????????????
 * ? Checksum (4 bytes): CRC32               ?
 * ???????????????????????????????????????????
 */

// Protocol Magic Number
constexpr uint8_t PROTOCOL_MAGIC = 0xAB;

// Protocol Version
constexpr uint8_t PROTOCOL_VERSION = 0x01;

// Frame sizes
constexpr size_t FRAME_HEADER_SIZE = 8;
constexpr size_t CHECKSUM_SIZE = 4;
constexpr size_t MIN_FRAME_SIZE = FRAME_HEADER_SIZE + CHECKSUM_SIZE;
constexpr size_t MAX_PAYLOAD_SIZE = 65535; // 2^16 - 1

// Message Type IDs (compile-time known)
enum class MessageType : uint8_t {
    PING = 0x01,
    PONG = 0x02,
    ECHO = 0x03,
    DATA = 0x04,
    STATUS = 0x05,
    MAX = 0xFF
};

// Frame Flags
enum class FrameFlags : uint8_t {
    NONE = 0x00,
    ACK_REQUIRED = 0x01,
    COMPRESSED = 0x02,
    ENCRYPTED = 0x04
};

/**
 * @brief Frame Header structure
 */
struct FrameHeader {
    uint8_t magic;              // Protocol magic byte
    uint8_t version;            // Protocol version
    uint8_t message_type;       // Message type ID
    uint8_t flags;              // Frame flags
    uint16_t payload_length;    // Length of payload
    uint16_t reserved;          // Reserved for future use

    /**
     * @brief Validate header consistency
     */
    [[nodiscard]] bool is_valid() const noexcept {
        return magic == PROTOCOL_MAGIC && 
               version == PROTOCOL_VERSION &&
               payload_length <= MAX_PAYLOAD_SIZE;
    }

    /**
     * @brief Check if flag is set
     */
    [[nodiscard]] bool has_flag(FrameFlags flag) const noexcept {
        return (flags & static_cast<uint8_t>(flag)) != 0;
    }

    /**
     * @brief Set a flag
     */
    void set_flag(FrameFlags flag) noexcept {
        flags |= static_cast<uint8_t>(flag);
    }

    /**
     * @brief Clear a flag
     */
    void clear_flag(FrameFlags flag) noexcept {
        flags &= ~static_cast<uint8_t>(flag);
    }
};

/**
 * @brief CRC32 calculation for checksums
 */
namespace crc32 {
    /**
     * @brief Calculate CRC32 checksum
     * @param data Data buffer
     * @param length Data length
     * @return CRC32 checksum
     */
    [[nodiscard]] uint32_t calculate(const uint8_t* data, size_t length) noexcept;
}

} // namespace protocol
} // namespace core
