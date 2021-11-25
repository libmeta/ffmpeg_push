//
// Created by TBD on 2020/8/28.
//

#ifndef XLIVE_SEMAPHORE_HPP
#define XLIVE_SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>
#include <atomic>

#include "time.hpp"

namespace xlab {

class Semaphore {
private:
    Semaphore(const Semaphore &) = delete;

    Semaphore &operator=(const Semaphore &) = delete;

    Semaphore &operator=(Semaphore &&sema) = delete;

    Semaphore(Semaphore &&sema) = delete;

public:

    Semaphore(int init_val = 0);

    ~Semaphore();

    void Wait();

    bool TryWait(std::chrono::nanoseconds wait_time = std::chrono::nanoseconds::zero());

    template<typename Rep, typename Period>
    bool TimedWait(const std::chrono::duration<Rep, Period> &duration);

    bool TimedWait(const Time::Interval &interval) {
        return TimedWait(interval.ToChrono<std::chrono::nanoseconds>());
    }

    bool WaitUntil(const std::chrono::time_point<std::chrono::steady_clock> &abs_time);

    bool WaitUntil(const Time::Point &point) {
        return WaitUntil(point.ToStedyTimePoint());
    }

    void Post(int count = 1);

    void ClearPost();

private:
    int64_t curCount;
    std::mutex mutex;
    std::condition_variable condVar;
};

}

#include "semaphore.inl"

#endif //XLIVE_SEMAPHORE_HPP


