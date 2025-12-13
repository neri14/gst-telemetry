#ifndef STREAM_SINK_H
#define STREAM_SINK_H

#include "sink.h"

namespace telemetry {
namespace utils {
namespace logging {

class stream_sink : public sink {
public:
    stream_sink(std::ostream& stream);
    ~stream_sink() = default;

    void write(const std::string& msg) override;

private:
    std::ostream& stream_;
};

} // namespace logging
} // namespace utils
} // namespace telemetry

#endif // STREAM_SINK_H
