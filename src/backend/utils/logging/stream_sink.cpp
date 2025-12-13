#include "stream_sink.h"

#include <format>

namespace telemetry {
namespace utils {
namespace logging {

StreamSink::StreamSink(std::ostream& stream) :
    stream_(stream)
{}

void StreamSink::write(const std::string& msg)
{
    stream_ << std::format("{}\n", msg);
}

} // namespace logging
} // namespace utils
} // namespace telemetry
