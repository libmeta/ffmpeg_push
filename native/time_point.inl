//
// Created by TBD on 2021/11/12.
//

#ifndef XLIVE_TIME_POINT_INL
#define XLIVE_TIME_POINT_INL

namespace xlab {

namespace Time {

#pragma mark - Point Functions

constexpr Point::Point(const value_type &timePoint) noexcept
    : value(timePoint) {
}

template<typename Rep, typename Period>
constexpr Point::Point(const std::chrono::duration<Rep, Period> &td) noexcept:
    value(std::chrono::steady_clock::time_point(td)) {
}

template<typename Duration>
constexpr Point::Point(const std::chrono::time_point<value_type::clock, Duration> &timePoint) noexcept
    : value(std::chrono::time_point_cast<value_type::clock, value_type::duration>(timePoint)) {
}

constexpr Point::Point(const Time::Interval &interval) noexcept
    : value(std::chrono::steady_clock::time_point(interval.Value())) {
}

template<typename ChronoInterval>
constexpr Point Point::FromRawValue(int64_t rawValue) noexcept {
    return std::move(Point(ChronoInterval(rawValue)));
}

inline Point Point::Now() noexcept {
    return std::move(Point(std::chrono::steady_clock::now()));
}

constexpr Point Point::Min() noexcept {
    return std::move(Point(value_type::min()));
}

constexpr Point Point::Max() noexcept {
    return std::move(Point(value_type::max()));
}

constexpr std::chrono::steady_clock::time_point Point::ToStedyTimePoint() const noexcept {
    return value;
}

constexpr Point Point::After(const Interval &timeInterval) const {
    return Point(value + timeInterval.ToChrono<std::chrono::nanoseconds>());
}

constexpr Point Point::Before(const Interval &timeInterval) const {
    return Point(value - timeInterval.ToChrono<std::chrono::nanoseconds>());
}

constexpr Interval Point::ToInterval() noexcept {
    return Interval(std::chrono::nanoseconds(value.time_since_epoch()));
}

template<typename ChronoInterval>
constexpr int64_t Point::RawValue() const noexcept {
    if constexpr (std::is_same_v<ChronoInterval, value_type::duration>) {
        return static_cast<int64_t>(value.time_since_epoch().count());
    }
    return static_cast<int64_t>(std::chrono::duration_cast<ChronoInterval>(value.time_since_epoch()).count());
}

constexpr Point &Point::operator+=(const Interval &d) {
    this->value += d.Value();
    return *this;
}

constexpr Point &Point::operator-=(const Interval &d) {
    this->value -= d.Value();
    return *this;
}

}

}

#endif //XLIVE_TIME_POINT_INL
