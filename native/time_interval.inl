//
// Created by TBD on 2021/11/12.
//

#ifndef XLIVE_TIME_INTERVAL_INL
#define XLIVE_TIME_INTERVAL_INL

namespace xlab {

namespace Time {

#pragma mark - Interval Functions

constexpr Interval::Interval(const value_type &td) noexcept
    : value(td) {
}

template<typename Rep, typename Period>
constexpr Interval::Interval(const std::chrono::duration<Rep, Period> &td) noexcept
    : value(std::chrono::duration_cast<std::chrono::nanoseconds>(td)) {
}

template<typename ChronoInterval>
constexpr Interval Interval::FromRawValue(int64_t rawValue) noexcept {
    return std::move(Interval(ChronoInterval(rawValue)));
}

constexpr Interval Interval::Zero() noexcept {
    return std::move(Interval(value_type::zero()));
}

constexpr Interval Interval::Min() noexcept {
    return std::move(Interval(value_type::min()));
}

constexpr Interval Interval::Max() noexcept {
    return std::move(Interval(value_type::max()));
}


template<typename ChronoInterval>
constexpr ChronoInterval Interval::ToChrono() const noexcept {
    if constexpr (std::is_same_v<ChronoInterval, decltype(value)>) {
        return value;
    }
    return std::chrono::duration_cast<ChronoInterval>(value);
}

constexpr Point Interval::ToPoint() const noexcept {
    return Point(value);
}

template<typename ChronoInterval>
constexpr int64_t Interval::RawValue() const noexcept {
    return static_cast<int64_t>(ToChrono<ChronoInterval>().count());
}

constexpr Interval &Interval::operator+=(const Interval &d) {
    this->value += d.value;
    return *this;
}

constexpr Interval &Interval::operator-=(const Interval &d) {
    this->value -= d.value;
    return *this;
}

}

}

#endif //XLIVE_TIME_INTERVAL_INL
