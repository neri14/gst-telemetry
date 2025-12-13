#ifndef LOGGER_H
#define LOGGER_H

#include <format>
#include <string>

#include "log_level.h"
#include "log_backend.h"

namespace telemetry {
namespace utils {
namespace logging {

class Logger {
public:
    Logger(const std::string& name);
    ~Logger() = default;

    template<typename... _Args>
    const void debug(const std::string_view& msg, _Args&&... args)
    {
        log(ELogLevel::Debug, msg, args...);
    }

    template<typename... _Args>
    void info(const std::string_view& msg, _Args&&... args)
    {
        log(ELogLevel::Info, msg, args...);
    }

    template<typename... _Args>
    void warning(const std::string_view& msg, _Args&&... args)
    {
        log(ELogLevel::Warning, msg, args...);
    }

    template<typename... _Args>
    void error(const std::string_view& msg, _Args&&... args)
    {
        log(ELogLevel::Error, msg, args...);
    }

private:
    template<typename... _Args>
    const void log(ELogLevel level, const std::string_view& msg, _Args&&... args)
    {
        backend_.log(name_, level, std::vformat(msg, std::make_format_args(args...)));
    }

    const std::string name_;
    LogBackend& backend_;
};


} // namespace logging
} // namespace utils
} // namespace telemetry

#endif // LOGGER_H