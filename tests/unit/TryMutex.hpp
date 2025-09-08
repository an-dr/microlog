#pragma once
#include <atomic>

class TryMutex {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:
    // returns true if lock acquired, false if already locked
    bool try_lock() {
        return !flag.test_and_set(std::memory_order_acquire);
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }
};
