#pragma once

#include "MessageHandler.h"
#include <map>
#include <memory>
#include <mutex>

namespace core {
namespace protocol {

/**
 * @brief Registry for message handlers
 * 
 * Demonstrates:
 * - Message routing dispatch
 * - Handler lifecycle management
 * - Thread-safe handler registration
 */
class HandlerRegistry {
public:
    /**
     * @brief Construct handler registry
     */
    HandlerRegistry() = default;

    /**
     * @brief Destructor
     */
    ~HandlerRegistry() = default;

    // Delete copy
    HandlerRegistry(const HandlerRegistry&) = delete;
    HandlerRegistry& operator=(const HandlerRegistry&) = delete;

    // Delete move
    HandlerRegistry(HandlerRegistry&&) = delete;
    HandlerRegistry& operator=(HandlerRegistry&&) = delete;

    /**
     * @brief Register a message handler
     * @param handler Handler to register
     * @return true if registered, false if type already registered
     */
    bool register_handler(std::unique_ptr<IMessageHandler> handler) noexcept;

    /**
     * @brief Unregister a handler
     * @param message_type Message type to unregister
     * @return true if found and removed
     */
    bool unregister_handler(MessageType message_type) noexcept;

    /**
     * @brief Get handler for message type
     * @param message_type Message type
     * @return Pointer to handler, nullptr if not found
     */
    [[nodiscard]] IMessageHandler* get_handler(MessageType message_type) noexcept;

    /**
     * @brief Dispatch message to appropriate handler
     * @param message_type Message type
     * @param payload Payload data
     * @param length Payload length
     * @return true if handled successfully
     */
    bool dispatch(MessageType message_type,
                 const uint8_t* payload,
                 size_t length) noexcept;

    /**
     * @brief Check if handler is registered
     */
    [[nodiscard]] bool has_handler(MessageType message_type) const noexcept;

    /**
     * @brief Get number of registered handlers
     */
    [[nodiscard]] size_t handler_count() const noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_handlers.size();
    }

    /**
     * @brief Clear all handlers
     */
    void clear() noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_handlers.clear();
    }

private:
    std::map<MessageType, std::unique_ptr<IMessageHandler>> m_handlers;
    mutable std::mutex m_mutex;
};

} // namespace protocol
} // namespace core
