#pragma once

#include "BinaryProtocol.h"
#include "NetworkBuffer.h"
#include <memory>
#include <optional>
#include <vector>

namespace core {
namespace protocol {

/**
 * @brief Serializes and deserializes binary protocol messages
 * 
 * Demonstrates:
 * - Frame serialization with header + payload + checksum
 * - Protocol validation
 * - Error handling
 */
class MessageSerializer {
public:
    /**
     * @brief Serialize a message frame
     * @param header Frame header
     * @param payload Payload data
     * @param payload_length Payload length
     * @param buffer Output buffer
     * @return true if successful
     */
    static bool serialize_frame(const FrameHeader& header,
                               const uint8_t* payload,
                               uint16_t payload_length,
                               net::NetworkBuffer& buffer) noexcept;

    /**
     * @brief Deserialize a frame header
     * @param data Input data
     * @param length Data length
     * @param header Output header
     * @return Bytes consumed if successful, 0 if not enough data
     */
    static size_t deserialize_header(const uint8_t* data,
                                    size_t length,
                                    FrameHeader& header) noexcept;

    /**
     * @brief Deserialize a complete frame
     * @param data Input data
     * @param length Data length
     * @param header Output header
     * @param payload Output payload
     * @return Bytes consumed if successful, 0 if not enough data
     */
    static size_t deserialize_frame(const uint8_t* data,
                                   size_t length,
                                   FrameHeader& header,
                                   std::vector<uint8_t>& payload) noexcept;

    /**
     * @brief Calculate frame size from header
     */
    [[nodiscard]] static size_t calculate_frame_size(const FrameHeader& header) noexcept {
        return FRAME_HEADER_SIZE + header.payload_length + CHECKSUM_SIZE;
    }

    /**
     * @brief Validate a complete frame
     */
    [[nodiscard]] static bool validate_frame(const uint8_t* frame_data,
                                           size_t frame_size) noexcept;

private:
    /**
     * @brief Serialize header to buffer
     */
    static bool serialize_header(const FrameHeader& header,
                                net::NetworkBuffer& buffer) noexcept;

    /**
     * @brief Serialize payload and checksum
     */
    static bool serialize_payload_and_checksum(const uint8_t* payload,
                                              uint16_t payload_length,
                                              net::NetworkBuffer& buffer,
                                              uint32_t& out_checksum) noexcept;
};

} // namespace protocol
} // namespace core
