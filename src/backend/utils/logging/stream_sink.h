#ifndef STREAM_SINK_H
#define STREAM_SINK_H

#include "sink.h"

namespace telemetry {
namespace utils {
namespace logging {

class StreamSink : public Sink {
public:
    StreamSink(std::ostream& stream);
    ~StreamSink() = default;

    void write(const std::string& msg) override;

private:
    std::ostream& stream_;
};

} // namespace loggingd
} // namespace utils
} // namespace telemetry

#endif // STREAM_SINK_H
