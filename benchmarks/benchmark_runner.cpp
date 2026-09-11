#include <concurrent/lock_free_skip_list.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

int main() {
    using Clock = std::chrono::high_resolution_clock;

    concurrent::LockFreeSkipList<int, std::string> skip_list(20);
    const int operations = 50000;

    auto start = Clock::now();
    for (int i = 0; i < operations; ++i) {
        skip_list.insert(i, std::to_string(i));
    }
    auto insert_done = Clock::now();

    for (int i = 0; i < operations; ++i) {
        skip_list.contains(i);
    }
    auto lookup_done = Clock::now();

    std::cout << "Inserted " << operations << " items in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(insert_done - start).count()
              << " ms\n";
    std::cout << "Looked up " << operations << " items in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(lookup_done - insert_done).count()
              << " ms\n";

    return 0;
}
