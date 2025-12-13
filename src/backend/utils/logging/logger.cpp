#include "logger.h"

namespace telemetry {
namespace utils {
namespace logging {

Logger::Logger(const std::string& name) :
    name_(name),
    backend_(LogBackend::get_instance())
{}

} // namespace logging
} // namespace utils
} // namespace telemetry
