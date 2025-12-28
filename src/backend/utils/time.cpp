#include "time.h"

namespace telemetry {
namespace time {

seconds_t us_to_s(microseconds_t us) {
    return static_cast<seconds_t>(us / 1'000'000.0);
}

microseconds_t s_to_us(seconds_t s) {
    return static_cast<microseconds_t>(s * 1'000'000.0);
}

time_point_t parse_iso8601(const std::string& timestamp) {
    std::istringstream ss(timestamp);
    std::chrono::sys_time<std::chrono::milliseconds> tp;
    
    // Parse format: "2025-12-09T15:17:04.000Z"
    std::chrono::from_stream(ss, "%Y-%m-%dT%H:%M:%S", tp);
    
    if (ss.fail()) {
        return INVALID_TIME_POINT;
    }
    
    // Handle fractional seconds (.000)
    if (ss.peek() == '.') {
        ss.ignore();
        int ms = 0;
        ss >> ms;
        tp += std::chrono::milliseconds(ms);
    }
    
    return tp;
}

} // namespace time
} // namespace telemetry
