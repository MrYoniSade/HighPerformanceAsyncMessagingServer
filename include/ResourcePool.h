#pragma once

#include <memory>
#include <vector>
#include <optional>

namespace core {

/**
 * @brief Resource pool demonstrating shared_ptr, weak_ptr patterns
 * @tparam T Resource type
 * 
 * Demonstrates:
 * - shared_ptr for shared ownership
 * - weak_ptr for non-owning references
 * - Pool management with smart pointers
 */
template<typename T>
class ResourcePool {
public:
    using ResourcePtr = std::shared_ptr<T>;
    using WeakResourcePtr = std::weak_ptr<T>;

    /**
     * @brief Construct a resource pool with initial capacity
     * @param initial_capacity Number of resources to pre-allocate
     */
    explicit ResourcePool(size_t initial_capacity = 10)
        : m_resources(initial_capacity)
    {
        for (size_t i = 0; i < initial_capacity; ++i) {
            m_resources[i] = std::make_shared<T>();
        }
    }

    /**
     * @brief Destructor - all resources cleaned up automatically via shared_ptr
     */
    ~ResourcePool() noexcept = default;

    // Delete copy operations
    ResourcePool(const ResourcePool&) = delete;
    ResourcePool& operator=(const ResourcePool&) = delete;

    // Delete move operations
    ResourcePool(ResourcePool&&) = delete;
    ResourcePool& operator=(ResourcePool&&) = delete;

    /**
     * @brief Acquire a resource from the pool
     * @return Optional shared_ptr to a resource
     * 
     * Returns a shared_ptr, allowing multiple owners.
     * Resource is automatically cleaned up when last shared_ptr goes out of scope.
     */
    [[nodiscard]] std::optional<ResourcePtr> acquire()
    {
        for (auto& resource : m_resources) {
            // If only the pool holds a reference (use_count == 1), we can reuse it
            if (resource.use_count() == 1) {
                return resource;
            }
        }
        
        // Create a new resource if all are in use
        auto new_resource = std::make_shared<T>();
        m_resources.push_back(new_resource);
        return new_resource;
    }

    /**
     * @brief Acquire a resource and get a weak_ptr
     * @return Optional weak_ptr to a resource
     * 
     * Weak pointer doesn't extend resource lifetime.
     * Useful for non-owning references.
     */
    [[nodiscard]] std::optional<WeakResourcePtr> acquire_weak()
    {
        auto shared = acquire();
        if (shared) {
            return WeakResourcePtr(*shared);
        }
        return std::nullopt;
    }

    /**
     * @brief Get total resource count
     */
    [[nodiscard]] size_t total_resources() const noexcept
    {
        return m_resources.size();
    }

    /**
     * @brief Get count of resources currently in use (use_count > 1)
     */
    [[nodiscard]] size_t active_resources() const noexcept
    {
        size_t count = 0;
        for (const auto& resource : m_resources) {
            if (resource && resource.use_count() > 1) {
                count++;
            }
        }
        return count;
    }

    /**
     * @brief Get count of available (unused) resources
     */
    [[nodiscard]] size_t available_resources() const noexcept
    {
        return total_resources() - active_resources();
    }

    /**
     * @brief Clear all resources from the pool
     * 
     * Note: Resources with active external references will not be destroyed
     * until those references are released.
     */
    void clear() noexcept
    {
        m_resources.clear();
    }

private:
    std::vector<ResourcePtr> m_resources;
};

} // namespace core
