#pragma once

#include <iostream>
#include <memory>
#include <string>

namespace core {

/**
 * @brief Demonstrates complete Rule of 5 implementation
 * 
 * The Rule of 5 states that if a class defines any of:
 * - destructor
 * - copy constructor
 * - copy assignment operator
 * - move constructor
 * - move assignment operator
 * 
 * Then it should explicitly define ALL of them (or explicitly delete them).
 * This class shows a complete, correct implementation.
 */
class RuleOfFiveDemo {
public:
    /**
     * @brief Constructor
     */
    explicit RuleOfFiveDemo(const std::string& name = "default")
        : m_name(std::make_unique<std::string>(name))
        , m_instance_count(++s_total_instances)
    {
        std::cout << "Constructor: " << *m_name << " (instance " << m_instance_count << ")" << std::endl;
    }

    /**
     * @brief Destructor (1 of 5)
     */
    ~RuleOfFiveDemo() noexcept
    {
        std::cout << "Destructor: " << (m_name ? *m_name : "moved") << std::endl;
    }

    /**
     * @brief Copy Constructor (2 of 5)
     * 
     * Deep copy the internal data.
     */
    RuleOfFiveDemo(const RuleOfFiveDemo& other)
        : m_name(std::make_unique<std::string>(*other.m_name))
        , m_instance_count(++s_total_instances)
    {
        std::cout << "Copy Constructor: " << *m_name << " (instance " << m_instance_count << ")" << std::endl;
    }

    /**
     * @brief Copy Assignment Operator (3 of 5)
     * 
     * Handles self-assignment safely.
     */
    RuleOfFiveDemo& operator=(const RuleOfFiveDemo& other)
    {
        if (this != &other) {
            m_name = std::make_unique<std::string>(*other.m_name);
            std::cout << "Copy Assignment: " << *m_name << std::endl;
        }
        return *this;
    }

    /**
     * @brief Move Constructor (4 of 5)
     * 
     * Efficiently steal resources from the source.
     */
    RuleOfFiveDemo(RuleOfFiveDemo&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_instance_count(++s_total_instances)
    {
        std::cout << "Move Constructor: (instance " << m_instance_count << ")" << std::endl;
    }

    /**
     * @brief Move Assignment Operator (5 of 5)
     * 
     * Efficiently steal resources and handle self-move.
     */
    RuleOfFiveDemo& operator=(RuleOfFiveDemo&& other) noexcept
    {
        if (this != &other) {
            m_name = std::move(other.m_name);
            std::cout << "Move Assignment" << std::endl;
        }
        return *this;
    }

    /**
     * @brief Get the name
     */
    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name ? *m_name : "";
    }

    /**
     * @brief Get instance number
     */
    [[nodiscard]] size_t instance_id() const noexcept
    {
        return m_instance_count;
    }

    /**
     * @brief Get total instances ever created
     */
    [[nodiscard]] static size_t total_instances() noexcept
    {
        return s_total_instances;
    }

private:
    std::unique_ptr<std::string> m_name;
    size_t m_instance_count;
    static size_t s_total_instances;
};

// Static member initialization
inline size_t RuleOfFiveDemo::s_total_instances = 0;

} // namespace core
