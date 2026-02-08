#include "MessageSerializer.h"
#include <cstring>
#include <iostream>

namespace core {
namespace protocol {

bool MessageSerializer::serialize_frame(const FrameHeader& header,
                                       const uint8_t* payload,
                                       uint16_t payload_length,
                                       net::NetworkBuffer& buffer) noexcept
{
    if (!header.is_valid()) {
        std::cerr << "Invalid frame header" << std::endl;
        return false;
    }

    if (!serialize_header(header, buffer)) {
        return false;
    }

    uint32_t checksum = 0;
    if (!serialize_payload_and_checksum(payload, payload_length, buffer, checksum)) {
        return false;
    }

    return true;
}

bool MessageSerializer::serialize_header(const FrameHeader& header,
                                        net::NetworkBuffer& buffer) noexcept
{
    return buffer.write_byte(header.magic) &&
           buffer.write_byte(header.version) &&
           buffer.write_byte(header.message_type) &&
           buffer.write_byte(header.flags) &&
           buffer.write_uint16(header.payload_length) &&
           buffer.write_uint16(header.reserved);
}

bool MessageSerializer::serialize_payload_and_checksum(const uint8_t* payload,
                                                      uint16_t payload_length,
                                                      net::NetworkBuffer& buffer,
                                                      uint32_t& out_checksum) noexcept
{
    // Write payload
    if (!buffer.write(payload, payload_length)) {
        std::cerr << "Failed to write payload" << std::endl;
        return false;
    }

    // Calculate checksum on header + payload
    size_t checksum_start = FRAME_HEADER_SIZE;
    const uint8_t* data_to_checksum = buffer.data() + checksum_start;
    size_t data_length = payload_length;

    out_checksum = crc32::calculate(data_to_checksum, data_length);

    // Write checksum (little-endian)
    uint8_t checksum_bytes[4] = {
        static_cast<uint8_t>(out_checksum & 0xFF),
        static_cast<uint8_t>((out_checksum >> 8) & 0xFF),
        static_cast<uint8_t>((out_checksum >> 16) & 0xFF),
        static_cast<uint8_t>((out_checksum >> 24) & 0xFF)
    };

    return buffer.write(checksum_bytes, 4);
}

size_t MessageSerializer::deserialize_header(const uint8_t* data,
                                            size_t length,
                                            FrameHeader& header) noexcept
{
    if (length < FRAME_HEADER_SIZE) {
        return 0; // Not enough data
    }

    header.magic = data[0];
    header.version = data[1];
    header.message_type = data[2];
    header.flags = data[3];
    header.payload_length = static_cast<uint16_t>(data[4]) |
                           (static_cast<uint16_t>(data[5]) << 8);
    header.reserved = static_cast<uint16_t>(data[6]) |
                     (static_cast<uint16_t>(data[7]) << 8);

    if (!header.is_valid()) {
        std::cerr << "Invalid header" << std::endl;
        return 0;
    }

    return FRAME_HEADER_SIZE;
}

size_t MessageSerializer::deserialize_frame(const uint8_t* data,
                                           size_t length,
                                           FrameHeader& header,
                                           std::vector<uint8_t>& payload) noexcept
{
    // Deserialize header
    size_t header_bytes = deserialize_header(data, length, header);
    if (header_bytes == 0) {
        return 0;
    }

    // Check if we have the full frame
    size_t frame_size = calculate_frame_size(header);
    if (length < frame_size) {
        return 0; // Not enough data yet
    }

    // Extract and validate checksum
    const uint8_t* checksum_data = data + FRAME_HEADER_SIZE + header.payload_length;
    uint32_t received_checksum = static_cast<uint32_t>(checksum_data[0]) |
                                (static_cast<uint32_t>(checksum_data[1]) << 8) |
                                (static_cast<uint32_t>(checksum_data[2]) << 16) |
                                (static_cast<uint32_t>(checksum_data[3]) << 24);

    uint32_t calculated_checksum = crc32::calculate(
        data + FRAME_HEADER_SIZE, 
        header.payload_length
    );

    if (received_checksum != calculated_checksum) {
        std::cerr << "Checksum mismatch" << std::endl;
        return 0;
    }

    // Extract payload
    payload.assign(
        data + FRAME_HEADER_SIZE,
        data + FRAME_HEADER_SIZE + header.payload_length
    );

    return frame_size;
}

bool MessageSerializer::validate_frame(const uint8_t* frame_data,
                                      size_t frame_size) noexcept
{
    if (frame_size < MIN_FRAME_SIZE) {
        return false;
    }

    FrameHeader header;
    size_t header_size = deserialize_header(frame_data, frame_size, header);
    if (header_size == 0) {
        return false;
    }

    // Check total frame size
    if (frame_size != calculate_frame_size(header)) {
        return false;
    }

    // Validate checksum
    const uint8_t* checksum_data = frame_data + FRAME_HEADER_SIZE + header.payload_length;
    uint32_t received_checksum = static_cast<uint32_t>(checksum_data[0]) |
                                (static_cast<uint32_t>(checksum_data[1]) << 8) |
                                (static_cast<uint32_t>(checksum_data[2]) << 16) |
                                (static_cast<uint32_t>(checksum_data[3]) << 24);

    uint32_t calculated_checksum = crc32::calculate(
        frame_data + FRAME_HEADER_SIZE,
        header.payload_length
    );

    return received_checksum == calculated_checksum;
}

} // namespace protocol
} // namespace core
