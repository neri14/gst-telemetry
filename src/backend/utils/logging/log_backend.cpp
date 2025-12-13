#include "log_backend.h"

#include "stream_sink.h"

#include <format>
#include <iostream>
#include <map>

namespace telemetry {
namespace utils {
namespace logging {

namespace consts {
const std::map<ELogLevel, std::string> level_str = {
    {ELogLevel::Debug,   "DBG"},
    {ELogLevel::Info,    "INF"},
    {ELogLevel::Warning, "WRN"},
    {ELogLevel::Error,   "ERR"}
};
} // namespace consts

static int get_debug_level()
{
    const char* s = std::getenv("TELEMETRY_DEBUG");
    if (!s) return 0;

    errno = 0;
    char* end = nullptr;
    long v = std::strtol(s, &end, 10);

    // not a valid integer or out of range
    if (end == s || *end != '\0' || errno == ERANGE)
        return 0;

    if (v == 1 || v == 2)
        return static_cast<int>(v);
    if (v > 2)
        return 2;
    return 0;
}

LogBackend::LogBackend()
{
    int dbg_level = get_debug_level();
    ELogLevel level = ELogLevel::Warning;
    if (dbg_level == 1)
        level = ELogLevel::Info;
    else if (dbg_level == 2)
        level = ELogLevel::Debug;

    add_sink(std::make_shared<StreamSink>(std::cout), level);
}

LogBackend& LogBackend::get_instance()
{
    static LogBackend instance;
    return instance;
}

const void LogBackend::log(const std::string& logger_name, ELogLevel level, const std::string& msg)
{
    std::string fmsg = std::format("[{}] {}: {}", consts::level_str.at(level), logger_name, msg);

    for(auto [s_level, s] : sinks_) {
        if (s_level <= level)
            s->write(fmsg);
    }
}

void LogBackend::add_sink(std::shared_ptr<Sink> sink, ELogLevel level)
{
    sinks_.push_back(std::make_pair(level, sink));
}

void LogBackend::set_log_level(std::shared_ptr<Sink> sink, ELogLevel level)
{
    std::for_each(sinks_.begin(), sinks_.end(), [sink, level](auto& s) {
        if (s.second == sink)
            s.first = level;
    });
}

void LogBackend::remove_sink(std::shared_ptr<Sink> sink)
{
    sinks_.erase(std::remove_if(sinks_.begin(), sinks_.end(), [sink](auto& s) {
        return s.second == sink;
    }), sinks_.end());
}

} // namespace logging
} // namespace utils
} // namespace telemetry
