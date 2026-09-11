#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <new>
#include <thread>
#include <vector>

struct alignas(64) Node
{
    Node* next;
};

class SimplePool
{
public:
    SimplePool(size_t chunk_size, size_t chunk_count)
        : chunk_size_(std::max(chunk_size, sizeof(Node))),
          chunk_count_(chunk_count),
          storage_(nullptr)
    {
        const size_t total = chunk_size_ * chunk_count_;
        storage_ = static_cast<std::byte*>(::operator new[](total, std::align_val_t(alignof(Node))));

        Node* list = nullptr;
        for (size_t i = chunk_count_; i > 0; --i)
        {
            Node* node = reinterpret_cast<Node*>(storage_ + (i - 1) * chunk_size_);
            node->next = list;
            list = node;
        }

        head_.store(list, std::memory_order_release);
    }

    ~SimplePool()
    {
        if (storage_ != nullptr)
        {
            ::operator delete[](storage_, std::align_val_t(alignof(Node)));
        }
    }

    void* allocate()
    {
        Node* current = head_.load(std::memory_order_acquire);
        while (current != nullptr)
        {
            Node* next = current->next;
            if (head_.compare_exchange_weak(
                    current,
                    next,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire))
            {
                current->next = nullptr;
                return current;
            }
        }

        return nullptr;
    }

    void deallocate(void* pointer)
    {
        Node* node = static_cast<Node*>(pointer);
        Node* expected = head_.load(std::memory_order_acquire);
        do
        {
            node->next = expected;
        }
        while (!head_.compare_exchange_weak(
            expected,
            node,
            std::memory_order_acq_rel,
            std::memory_order_acquire));
    }

private:
    size_t chunk_size_;
    size_t chunk_count_;
    std::byte* storage_;
    alignas(64) std::atomic<Node*> head_;
};

struct BenchmarkResult
{
    double elapsed_seconds = 0.0;
    double throughput_ops_per_second = 0.0;
    double average_latency_ns = 0.0;
    size_t peak_live_blocks = 0;
};

static BenchmarkResult run_pool_bench(size_t thread_count, size_t ops_per_thread)
{
    SimplePool pool(64, thread_count * ops_per_thread + 1000);
    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    std::vector<size_t> peak_values(thread_count, 0);

    for (size_t i = 0; i < thread_count; ++i)
    {
        threads.emplace_back([&pool, ops_per_thread, &peak_values, i]() {
            std::vector<void*> items;
            items.reserve(ops_per_thread);
            size_t local_peak = 0;

            for (size_t j = 0; j < ops_per_thread; ++j)
            {
                void* p = pool.allocate();
                if (p != nullptr)
                {
                    items.push_back(p);
                    local_peak = std::max(local_peak, items.size());
                }
            }

            for (void* p : items)
            {
                pool.deallocate(p);
            }

            peak_values[i] = local_peak;
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end - start;

    BenchmarkResult result{};
    result.elapsed_seconds = elapsed.count();
    const size_t total_ops = thread_count * ops_per_thread;

    if (result.elapsed_seconds > 0.0)
    {
        result.throughput_ops_per_second = static_cast<double>(total_ops) / result.elapsed_seconds;
        result.average_latency_ns = (result.elapsed_seconds * 1e9) / static_cast<double>(total_ops);
    }

    result.peak_live_blocks = *std::max_element(peak_values.begin(), peak_values.end());
    return result;
}

int main()
{
    std::cout << "Concurrent Memory Pool Allocator benchmark\n";

    const size_t thread_count = 4;
    const size_t ops_per_thread = 12000;

    const BenchmarkResult result = run_pool_bench(thread_count, ops_per_thread);

    std::cout << "threads: " << thread_count << "\n";
    std::cout << "ops per thread: " << ops_per_thread << "\n";
    std::cout << "elapsed time: " << result.elapsed_seconds << " seconds\n";
    std::cout << "throughput: " << result.throughput_ops_per_second << " ops/s\n";
    std::cout << "avg latency: " << result.average_latency_ns << " ns/op\n";
    std::cout << "peak live blocks: " << result.peak_live_blocks << "\n";

    return 0;
}
