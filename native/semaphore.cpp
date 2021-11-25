
#include "semaphore.hpp"

namespace xlab {

Semaphore::Semaphore(int init_val) : curCount(init_val) {
}

Semaphore::~Semaphore() = default;

void Semaphore::Wait() {
    while (true) {
        std::unique_lock<std::mutex> lock{mutex};
        if (curCount > 0) {
            --curCount;
            break;
        }
        condVar.wait(lock);
        if (curCount > 0) {
            --curCount;
            break;
        }
    }
}

bool Semaphore::TryWait(std::chrono::nanoseconds wait_time) {
    std::unique_lock<std::mutex> lock{mutex};
    if (curCount > 0) {
        --curCount;
        return true;
    }

    condVar.wait_for(lock, wait_time);
    if (curCount > 0) {
        --curCount;
        return true;
    }

    return false;
}

bool Semaphore::WaitUntil(const std::chrono::time_point<std::chrono::steady_clock> &abs_time) {
    std::unique_lock<std::mutex> lock{mutex};
    if (curCount > 0) {
        --curCount;
        return true;
    }

    condVar.wait_until(lock, abs_time);
    if (curCount > 0) {
        --curCount;
        return true;
    }

    return false;
}

void Semaphore::Post(int count) {
    if (count <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock{mutex};
    curCount += count;
    condVar.notify_one();
}

void Semaphore::ClearPost() {
    std::unique_lock<std::mutex> lock{mutex};
    if (curCount > 0) {
        curCount = 0;
    }
    condVar.wait_for(lock, std::chrono::nanoseconds::zero());
}

}

