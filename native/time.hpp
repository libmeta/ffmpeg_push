//
// Created by TBD on 2021/11/11.
//

#ifndef XLIVE_TIME_HPP
#define XLIVE_TIME_HPP

namespace xlab {

namespace Time {

class Interval;

struct Point final {
private:
    using value_type = std::chrono::steady_clock::time_point;
    value_type value{};

public:
    explicit constexpr Point(const value_type &timePoint = value_type{}) noexcept;

    template<typename Rep, typename Period>
    explicit constexpr Point(const std::chrono::duration<Rep, Period> &td) noexcept;

    template<typename Duration>
    explicit constexpr Point(const std::chrono::time_point<value_type::clock, Duration> &timePoint) noexcept;

    explicit constexpr Point(const Time::Interval &interval) noexcept;


public:
    constexpr auto &Value() const noexcept {
        return value;
    }

public:
    template<typename ChronoInterval>
    static constexpr Point FromRawValue(int64_t rawValue) noexcept;

public:
    static Point Now() noexcept;

    static constexpr Point Min() noexcept;

    static constexpr Point Max() noexcept;

public:
    constexpr std::chrono::steady_clock::time_point ToStedyTimePoint() const noexcept;

    constexpr Point After(const Interval &timeInterval) const;

    constexpr Point Before(const Interval &timeInterval) const;

    constexpr Interval ToInterval() noexcept;

    template<typename ChronoInterval>
    constexpr int64_t RawValue() const noexcept;

public:
    constexpr Point &operator+=(const Interval &d);

    constexpr Point &operator-=(const Interval &d);
};


struct Interval final {
private:
    using value_type = std::chrono::nanoseconds;
    value_type value{};

public:
    explicit constexpr Interval(const value_type &td = value_type{}) noexcept;

    template<typename Rep, typename Period>
    explicit constexpr Interval(const std::chrono::duration<Rep, Period> &td) noexcept;

public:
    constexpr auto &Value() const noexcept {
        return value;
    }

public:
    template<typename ChronoInterval>
    static constexpr Interval FromRawValue(int64_t rawValue) noexcept;

public:
    static constexpr Interval Zero() noexcept;

    static constexpr Interval Min() noexcept;

    static constexpr Interval Max() noexcept;

public:
    template<typename ChronoInterval>
    constexpr ChronoInterval ToChrono() const noexcept;

    constexpr Point ToPoint() const noexcept;

    template<typename ChronoInterval>
    constexpr int64_t RawValue() const noexcept;

public:
    constexpr Interval &operator+=(const Interval &d);

    constexpr Interval &operator-=(const Interval &d);
};

}

}

#include "time_point.inl"
#include "time_interval.inl"
#include "time_operator.inl"
#include "time_literals.inl"
#include "time_thread.inl"

#endif //XLIVE_TIME_HPP
