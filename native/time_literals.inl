//
// Created by TBD on 2021/11/12.
//

#ifndef XLIVE_TIME_LITERALS_INL
#define XLIVE_TIME_LITERALS_INL

namespace xlab {

#pragma mark - Literals
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuser-defined-literals"

constexpr Time::Interval operator ""ns(unsigned long long ns) {
    return Time::Interval(std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(ns)));
}

constexpr Time::Interval operator ""us(unsigned long long us) {
    return Time::Interval(std::chrono::microseconds(static_cast<std::chrono::microseconds::rep>(us)));
}

constexpr Time::Interval operator ""ms(unsigned long long ms) {
    return Time::Interval(std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(ms)));
}

constexpr Time::Interval operator ""s(unsigned long long s) {
    return Time::Interval(std::chrono::seconds(static_cast<std::chrono::seconds::rep>(s)));
}

constexpr Time::Interval operator ""min(unsigned long long m) {
    return Time::Interval(std::chrono::minutes(static_cast<std::chrono::minutes::rep>(m)));
}

constexpr Time::Interval operator ""h(unsigned long long h) {
    return Time::Interval(std::chrono::hours(static_cast<std::chrono::hours::rep>(h)));
}

#if _LIBCPP_STD_VER > 17 && !defined(_LIBCPP_HAS_NO_CXX20_CHRONO_LITERALS)
constexpr Time::Interval operator ""d(unsigned long long d) noexcept {
    return Time::Interval(std::chrono::day(static_cast<std::chrono::day::rep>(d)));
}

constexpr Time::Interval operator ""y(unsigned long long y) noexcept {
    return Time::Interval(std::chrono::year(static_cast<std::chrono::year::rep>(y)));
}
#endif

#pragma clang diagnostic pop

using nanoseconds = std::chrono::nanoseconds;   // 纳秒
using microseconds = std::chrono::microseconds; // 微秒
using milliseconds = std::chrono::milliseconds; // 毫秒
using seconds = std::chrono::seconds;
using minutes = std::chrono::minutes;
using hours = std::chrono::hours;

using ns = nanoseconds;     // 纳秒
using us = microseconds;    // 微秒
using ms = milliseconds;    // 毫秒
using s = seconds;
using min = minutes;

#if _LIBCPP_STD_VER > 17
using day = std::chrono::day;
using year = std::chrono::year;
#endif

}

#endif //XLIVE_TIME_LITERALS_INL
