#pragma once

#include "MessageHandler.h"
#include <cstring>
#include <array>

namespace core {
namespace protocol {
namespace messages {

/**
 * @brief Ping message - simple connectivity check
 */
struct PingMessage {
    uint32_t sequence_id;
    uint64_t timestamp;

    static constexpr MessageType TYPE = MessageType::PING;
};

/**
 * @brief Pong message - response to ping
 */
struct PongMessage {
    uint32_t sequence_id;
    uint64_t timestamp;
    uint64_t echo_time; // When ping was received

    static constexpr MessageType TYPE = MessageType::PONG;
};

/**
 * @brief Echo message - echo back any data
 */
struct EchoMessage {
    static constexpr size_t MAX_DATA = 256;
    std::array<uint8_t, MAX_DATA> data;
    uint16_t length;

    static constexpr MessageType TYPE = MessageType::ECHO;
};

/**
 * @brief Data message - general purpose data transfer
 */
struct DataMessage {
    uint16_t data_type;
    uint16_t data_id;
    std::array<uint8_t, 512> data;
    uint16_t data_length;

    static constexpr MessageType TYPE = MessageType::DATA;
};

/**
 * @brief Status message - status updates
 */
struct StatusMessage {
    uint8_t status_code;
    uint16_t error_code;
    std::array<char, 64> message;

    static constexpr MessageType TYPE = MessageType::STATUS;
};

/**
 * @brief Specialized handler for Ping messages
 */
class PingHandler : public MessageHandler<PingMessage> {
public:
    explicit PingHandler(HandlerFunc handler)
        : MessageHandler(MessageType::PING, handler)
    {
    }

    bool deserialize_payload(const uint8_t* data,
                            size_t length,
                            PingMessage& payload) noexcept override {
        if (length < sizeof(PingMessage)) {
            return false;
        }

        std::memcpy(&payload, data, sizeof(PingMessage));
        return true;
    }
};

/**
 * @brief Specialized handler for Pong messages
 */
class PongHandler : public MessageHandler<PongMessage> {
public:
    explicit PongHandler(HandlerFunc handler)
        : MessageHandler(MessageType::PONG, handler)
    {
    }

    bool deserialize_payload(const uint8_t* data,
                            size_t length,
                            PongMessage& payload) noexcept override {
        if (length < sizeof(PongMessage)) {
            return false;
        }

        std::memcpy(&payload, data, sizeof(PongMessage));
        return true;
    }
};

/**
 * @brief Specialized handler for Echo messages
 */
class EchoHandler : public MessageHandler<EchoMessage> {
public:
    explicit EchoHandler(HandlerFunc handler)
        : MessageHandler(MessageType::ECHO, handler)
    {
    }

    bool deserialize_payload(const uint8_t* data,
                            size_t length,
                            EchoMessage& payload) noexcept override {
        if (length > EchoMessage::MAX_DATA + sizeof(uint16_t)) {
            return false;
        }

        // Extract length
        if (length < sizeof(uint16_t)) {
            return false;
        }

        payload.length = *reinterpret_cast<const uint16_t*>(data);
        
        // Extract data
        if (length < sizeof(uint16_t) + payload.length) {
            return false;
        }

        std::memcpy(payload.data.data(), data + sizeof(uint16_t), payload.length);
        return true;
    }
};

/**
 * @brief Specialized handler for Data messages
 */
class DataHandler : public MessageHandler<DataMessage> {
public:
    explicit DataHandler(HandlerFunc handler)
        : MessageHandler(MessageType::DATA, handler)
    {
    }

    bool deserialize_payload(const uint8_t* data,
                            size_t length,
                            DataMessage& payload) noexcept override {
        if (length < 4) { // At least type (2) + id (2)
            return false;
        }

        std::memcpy(&payload.data_type, data, sizeof(uint16_t));
        std::memcpy(&payload.data_id, data + 2, sizeof(uint16_t));
        std::memcpy(&payload.data_length, data + 4, sizeof(uint16_t));

        if (payload.data_length > payload.data.size()) {
            return false;
        }

        if (length < 6 + payload.data_length) {
            return false;
        }

        std::memcpy(payload.data.data(), data + 6, payload.data_length);
        return true;
    }
};

/**
 * @brief Specialized handler for Status messages
 */
class StatusHandler : public MessageHandler<StatusMessage> {
public:
    explicit StatusHandler(HandlerFunc handler)
        : MessageHandler(MessageType::STATUS, handler)
    {
    }

    bool deserialize_payload(const uint8_t* data,
                            size_t length,
                            StatusMessage& payload) noexcept override {
        if (length < 3) { // status_code (1) + error_code (2)
            return false;
        }

        payload.status_code = data[0];
        payload.error_code = *reinterpret_cast<const uint16_t*>(data + 1);

        // Extract message
        size_t msg_len = length - 3;
        if (msg_len > payload.message.size() - 1) {
            msg_len = payload.message.size() - 1;
        }

        std::memcpy(payload.message.data(), data + 3, msg_len);
        payload.message[msg_len] = '\0';

        return true;
    }
};

} // namespace messages
} // namespace protocol
} // namespace core
