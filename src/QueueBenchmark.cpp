#include "LockFreeQueue.h"
#include "MutexQueue.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iomanip>

using namespace core;

/**
 * @brief Benchmark throughput of enqueue operations
 */
template<typename QueueType>
double benchmark_enqueue(const std::string& name, int num_operations) {
    QueueType queue;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        queue.try_enqueue(i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    double ops_per_sec = (static_cast<double>(num_operations) / duration) * 1e6;
    
    std::cout << name << " Enqueue: " << std::fixed << std::setprecision(0) 
              << ops_per_sec << " ops/sec" << std::endl;
    
    return ops_per_sec;
}

/**
 * @brief Benchmark throughput of dequeue operations
 */
template<typename QueueType>
double benchmark_dequeue(const std::string& name, int num_operations) {
    QueueType queue;
    
    // Pre-fill the queue
    for (int i = 0; i < num_operations; ++i) {
        queue.try_enqueue(i);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int value;
    for (int i = 0; i < num_operations; ++i) {
        queue.try_dequeue(value);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    double ops_per_sec = (static_cast<double>(num_operations) / duration) * 1e6;
    
    std::cout << name << " Dequeue: " << std::fixed << std::setprecision(0) 
              << ops_per_sec << " ops/sec" << std::endl;
    
    return ops_per_sec;
}

/**
 * @brief Benchmark concurrent enqueue/dequeue with multiple threads
 */
template<typename QueueType>
double benchmark_concurrent(const std::string& name, 
                           int num_producers, 
                           int num_consumers,
                           int operations_per_producer) {
    QueueType queue;
    
    std::atomic<int> total_enqueued(0);
    std::atomic<int> total_dequeued(0);
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Producer threads
    for (int i = 0; i < num_producers; ++i) {
        threads.emplace_back([&queue, &total_enqueued, operations_per_producer, i]() {
            for (int j = 0; j < operations_per_producer; ++j) {
                int value = i * operations_per_producer + j;
                if (queue.try_enqueue(value)) {
                    total_enqueued.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }
    
    // Consumer threads
    for (int i = 0; i < num_consumers; ++i) {
        threads.emplace_back([&queue, &total_dequeued]() {
            int value;
            while (queue.try_dequeue(value)) {
                total_dequeued.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    int total_ops = total_enqueued.load() + total_dequeued.load();
    double ops_per_sec = (static_cast<double>(total_ops) / duration) * 1e6;
    
    std::cout << name << " Concurrent (" << num_producers << "P/" << num_consumers << "C): " 
              << std::fixed << std::setprecision(0) << ops_per_sec << " ops/sec" << std::endl;
    
    return ops_per_sec;
}

int main() {
    std::cout << "\n======== Queue Performance Benchmark ========\n" << std::endl;
    
    const int SMALL_OPS = 100000;
    const int MEDIUM_OPS = 1000000;
    
    // Single-threaded benchmarks
    std::cout << "--- Single-Threaded Benchmarks ---\n" << std::endl;
    
    double lf_enq = benchmark_enqueue<LockFreeQueue<int, 1024>>("LockFreeQueue", MEDIUM_OPS);
    double mx_enq = benchmark_enqueue<MutexQueue<int, 1024>>("MutexQueue", MEDIUM_OPS);
    std::cout << "Speedup: " << std::fixed << std::setprecision(2) << (lf_enq / mx_enq) << "x\n" << std::endl;
    
    double lf_deq = benchmark_dequeue<LockFreeQueue<int, 1024>>("LockFreeQueue", MEDIUM_OPS);
    double mx_deq = benchmark_dequeue<MutexQueue<int, 1024>>("MutexQueue", MEDIUM_OPS);
    std::cout << "Speedup: " << std::fixed << std::setprecision(2) << (lf_deq / mx_deq) << "x\n" << std::endl;
    
    // Concurrent benchmarks
    std::cout << "--- Multi-Threaded Benchmarks ---\n" << std::endl;
    
    double lf_conc = benchmark_concurrent<LockFreeQueue<int, 4096>>("LockFreeQueue", 2, 2, SMALL_OPS / 4);
    double mx_conc = benchmark_concurrent<MutexQueue<int, 4096>>("MutexQueue", 2, 2, SMALL_OPS / 4);
    std::cout << "Speedup: " << std::fixed << std::setprecision(2) << (lf_conc / mx_conc) << "x\n" << std::endl;
    
    double lf_conc_heavy = benchmark_concurrent<LockFreeQueue<int, 4096>>("LockFreeQueue", 4, 4, SMALL_OPS / 8);
    double mx_conc_heavy = benchmark_concurrent<MutexQueue<int, 4096>>("MutexQueue", 4, 4, SMALL_OPS / 8);
    std::cout << "Speedup: " << std::fixed << std::setprecision(2) << (lf_conc_heavy / mx_conc_heavy) << "x\n" << std::endl;
    
    std::cout << "========================================\n" << std::endl;
    
    return 0;
}
