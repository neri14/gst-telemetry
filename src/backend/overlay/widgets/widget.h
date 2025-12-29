#ifndef WIDGET_H
#define WIDGET_H

#include <cairo.h>
#include <memory>
#include <vector>
#include "backend/utils/time.h"
#include "backend/track/track.h"
#include "backend/utils/logging/logger.h"

namespace telemetry {
namespace overlay {

class Widget {
public:
    Widget(std::shared_ptr<track::Track> track);
    Widget(std::string name, std::shared_ptr<track::Track> track);
    virtual ~Widget() = default;

    void add_child(std::shared_ptr<Widget> child);

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr,  int x_offset = 0, int y_offset = 0);

private:
    mutable utils::logging::Logger log;

    std::vector<std::shared_ptr<Widget>> children_;
    std::shared_ptr<track::Track> track_;
};

} // namespace overlay
} // namespace telemetry

#endif // WIDGET_H