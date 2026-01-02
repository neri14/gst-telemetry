#ifndef WIDGET_H
#define WIDGET_H

#include <cairo.h>
#include <memory>
#include <vector>
#include "backend/utils/time.h"
#include "backend/track/track.h"
#include "backend/utils/logging/logger.h"
#include "params/parameter.h"

namespace telemetry {
namespace overlay {

class Widget {
public:
    static std::shared_ptr<Widget> create(parameter_map_ptr parameters);
    Widget();
    virtual ~Widget() = default;

    void add_child(std::shared_ptr<Widget> child);

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr,
                      double x_offset = 0, double y_offset = 0);

    inline static parameter_type_map_t parameter_types = {};

protected:
    Widget(std::string name); // for derived classes to set logger name

private:
    mutable utils::logging::Logger log;

    std::vector<std::shared_ptr<Widget>> children_;
    std::shared_ptr<track::Track> track_;
};

} // namespace overlay
} // namespace telemetry

#endif // WIDGET_H