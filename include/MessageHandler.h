#pragma once

#include "BinaryProtocol.h"
#include "NetworkBuffer.h"
#include <memory>
#include <functional>

namespace core {
namespace protocol {

/**
 * @brief Base handler interface for messages
 * 
 * Demonstrates:
 * - Template-based polymorphic message handling
 * - Type-safe message dispatch
 * - CRTP pattern optional
 */
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;

    /**
     * @brief Get message type handled by this handler
     */
    [[nodiscard]] virtual MessageType get_message_type() const noexcept = 0;

    /**
     * @brief Handle incoming message
     * @param payload Message payload
     * @param length Payload length
     * @return true if handled successfully
     */
    virtual bool handle(const uint8_t* payload, size_t length) noexcept = 0;
};

/**
 * @brief Template-based message handler
 * @tparam T Message payload type
 */
template<typename T>
class MessageHandler : public IMessageHandler {
public:
    using PayloadType = T;
    using HandlerFunc = std::function<bool(const T&)>;

    /**
     * @brief Construct handler with callback
     * @param message_type Type ID for this message
     * @param handler Callback function
     */
    explicit MessageHandler(MessageType message_type, HandlerFunc handler)
        : m_message_type(message_type)
        , m_handler(handler)
    {
    }

    /**
     * @brief Get handled message type
     */
    [[nodiscard]] MessageType get_message_type() const noexcept override {
        return m_message_type;
    }

    /**
     * @brief Handle message by deserializing and calling callback
     */
    bool handle(const uint8_t* payload, size_t length) noexcept override {
        if (!payload || length < sizeof(T)) {
            return false;
        }

        // Deserialize payload
        T msg;
        if (!deserialize_payload(payload, length, msg)) {
            return false;
        }

        // Call handler
        if (m_handler) {
            return m_handler(msg);
        }

        return true;
    }

    /**
     * @brief Override to customize deserialization
     */
    virtual bool deserialize_payload(const uint8_t* data,
                                    size_t length,
                                    T& payload) noexcept {
        // Default: simple memcpy for POD types
        if (length >= sizeof(T)) {
            std::memcpy(&payload, data, sizeof(T));
            return true;
        }
        return false;
    }

protected:
    MessageType m_message_type;
    HandlerFunc m_handler;
};

/**
 * @brief Compile-time message ID generator
 * 
 * Usage:
 * constexpr auto PING_ID = MessageID<0x01>;
 * constexpr auto PONG_ID = MessageID<0x02>;
 */
template<uint8_t ID>
struct MessageID {
    static constexpr uint8_t value = ID;
    static constexpr MessageType type = static_cast<MessageType>(ID);
};

/**
 * @brief Generic message wrapper for type-safe handling
 */
template<typename T, uint8_t MSG_ID>
struct Message {
    static constexpr uint8_t ID = MSG_ID;
    static constexpr MessageType TYPE = static_cast<MessageType>(MSG_ID);
    
    T data;
};

} // namespace protocol
} // namespace core
