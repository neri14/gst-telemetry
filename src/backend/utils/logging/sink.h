#ifndef SINK_H
#define SINK_H

#include <string>

namespace telemetry {
namespace utils {
namespace logging {

class Sink {
public:
    Sink() = default;
    virtual ~Sink() = default;
    virtual void write(const std::string& msg) = 0;
};

} // namespace logging
} // namespace utils
} // namespace telemetry

#endif // SINK_H
