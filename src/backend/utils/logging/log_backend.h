#ifndef LOG_BACKEND_H
#define LOG_BACKEND_H

#include "log_level.h"
#include "sink.h"

#include <string>
#include <memory>
#include <vector>
#include <utility>

namespace telemetry {
namespace utils {
namespace logging {

class LogBackend {
public:
    static LogBackend& get_instance();

    const void log(const std::string& logger_name, ELogLevel level, const std::string& msg);

    void add_sink(std::shared_ptr<Sink> sink, ELogLevel level = ELogLevel::Debug);
    void set_log_level(std::shared_ptr<Sink> sink, ELogLevel level);
    void remove_sink(std::shared_ptr<Sink> sink);

private:
    LogBackend();
    ~LogBackend() = default;
    LogBackend(const LogBackend&) = delete;

    std::vector<std::pair<ELogLevel, std::shared_ptr<Sink>>> sinks_;
};

} // namespace logging
} // namespace utils
} // namespace telemetry

#endif // LOG_BACKEND_H
