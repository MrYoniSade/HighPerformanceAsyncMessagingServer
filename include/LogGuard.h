#pragma once

#include <memory>
#include <fstream>
#include <iostream>
#include <chrono>
#include <sstream>

namespace core {

/**
 * @brief RAII wrapper for log file resource management
 * 
 * Demonstrates:
 * - RAII for file handle management
 * - Automatic resource cleanup
 * - Scoped logging context
 */
class LogGuard {
public:
    /**
     * @brief Construct a log guard - opens log file
     * @param filename Path to log file
     * @throws std::runtime_error if file cannot be opened
     */
    explicit LogGuard(const std::string& filename)
        : m_filename(filename)
    {
        m_file.open(filename, std::ios::app);
        if (!m_file.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
        log("LogGuard initialized");
    }

    /**
     * @brief Destructor - closes log file and writes final message
     */
    ~LogGuard() noexcept
    {
        try {
            if (m_file.is_open()) {
                log("LogGuard destroyed");
                m_file.close();
            }
        } catch (...) {
            // Suppress exceptions in destructor
        }
    }

    // Delete copy operations
    LogGuard(const LogGuard&) = delete;
    LogGuard& operator=(const LogGuard&) = delete;

    // Delete move operations (resource ownership is tied to object lifetime)
    LogGuard(LogGuard&&) = delete;
    LogGuard& operator=(LogGuard&&) = delete;

    /**
     * @brief Write a message to the log
     * @param message Message to log
     */
    void log(const std::string& message) noexcept
    {
        try {
            if (m_file.is_open()) {
                auto now = std::chrono::system_clock::now();
                auto time = std::chrono::system_clock::to_time_t(now);
                
                char timebuf[64];
                std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
                
                m_file << "[" << timebuf << "] " << message << std::endl;
                m_file.flush();
            }
        } catch (...) {
            // Suppress exceptions
        }
    }

    /**
     * @brief Get filename
     */
    [[nodiscard]] const std::string& filename() const noexcept
    {
        return m_filename;
    }

    /**
     * @brief Check if log file is open
     */
    [[nodiscard]] bool is_open() const noexcept
    {
        return m_file.is_open();
    }

private:
    std::string m_filename;
    mutable std::ofstream m_file;
};

} // namespace core
