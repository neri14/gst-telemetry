#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <string>
#include <chrono>

namespace telemetry {
namespace time {

using microseconds_t = int64_t;
using seconds_t = double;

static constexpr microseconds_t INVALID_TIME = INT64_MAX;

using time_point_t = std::chrono::time_point<std::chrono::system_clock>;
static constexpr time_point_t INVALID_TIME_POINT = time_point_t::max();

double us_to_s(microseconds_t us);
microseconds_t s_to_us(seconds_t s);

time_point_t parse_iso8601(const std::string& timestamp);

} // namespace time
} // namespace telemetry

#endif // TIME_H