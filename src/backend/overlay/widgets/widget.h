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

using draw_cb_t = std::function<void(int, int, cairo_surface_t*)>;

class Widget {
public:
    static std::shared_ptr<Widget> create(parameter_map_ptr parameters);
    Widget();
    virtual ~Widget() = default;

    void add_child(std::shared_ptr<Widget> child);

    virtual void draw(time::microseconds_t timestamp, double x_offset, double y_offset, draw_cb_t draw_cb);
    virtual unsigned int surface_count() const;

    inline static parameter_type_map_t parameter_types = {};

protected:
    mutable utils::logging::Logger log;

    Widget(const std::string& name); // for derived classes to set logger name

private:
    std::vector<std::shared_ptr<Widget>> children_;
    std::shared_ptr<track::Track> track_;
};

} // namespace overlay
} // namespace telemetry

#endif // WIDGET_H