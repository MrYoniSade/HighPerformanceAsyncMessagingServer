#include "HandlerRegistry.h"
#include <iostream>

namespace core {
namespace protocol {

bool HandlerRegistry::register_handler(std::unique_ptr<IMessageHandler> handler) noexcept
{
    if (!handler) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    MessageType type = handler->get_message_type();

    // Check if already registered
    if (m_handlers.find(type) != m_handlers.end()) {
        std::cerr << "Handler for message type " << static_cast<int>(type) 
                  << " already registered" << std::endl;
        return false;
    }

    m_handlers[type] = std::move(handler);
    return true;
}

bool HandlerRegistry::unregister_handler(MessageType message_type) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_handlers.find(message_type);
    if (it == m_handlers.end()) {
        return false;
    }

    m_handlers.erase(it);
    return true;
}

IMessageHandler* HandlerRegistry::get_handler(MessageType message_type) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_handlers.find(message_type);
    if (it != m_handlers.end()) {
        return it->second.get();
    }

    return nullptr;
}

bool HandlerRegistry::dispatch(MessageType message_type,
                              const uint8_t* payload,
                              size_t length) noexcept
{
    auto handler = get_handler(message_type);
    if (!handler) {
        std::cerr << "No handler registered for message type " 
                  << static_cast<int>(message_type) << std::endl;
        return false;
    }

    return handler->handle(payload, length);
}

bool HandlerRegistry::has_handler(MessageType message_type) const noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_handlers.find(message_type) != m_handlers.end();
}

} // namespace protocol
} // namespace core
