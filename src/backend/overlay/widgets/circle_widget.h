#ifndef CIRCLE_WIDGET_H
#define CIRCLE_WIDGET_H

#include "widget.h"

#include "backend/utils/logging/logger.h"
#include "params/numeric_parameter.h"
#include "params/color_parameter.h"
#include "params/boolean_parameter.h"

namespace telemetry {
namespace overlay {

class CircleWidget : public Widget {
public:
    static std::shared_ptr<CircleWidget> create(parameter_map_ptr parameters);
    CircleWidget();
    ~CircleWidget() override = default;

    virtual void draw(time::microseconds_t timestamp,
                      schedule_drawing_cb_t schedule_drawing_cb,
                      double x_offset = 0, double y_offset = 0) override;

    inline static parameter_type_map_t parameter_types = {
        {"x", ParameterType::Numeric}, // center x position
        {"y", ParameterType::Numeric}, // center y position
        {"radius", ParameterType::Numeric}, // circle radius
        {"color", ParameterType::Color}, // fill color
        {"border-width", ParameterType::Numeric}, // border width
        {"border-color", ParameterType::Color}, // border color
        {"visible", ParameterType::Boolean}, // visibility condition
    };

private:
    mutable utils::logging::Logger log{"CircleWidget"};

    void draw_impl(Surface& surface, time::microseconds_t timestamp, double x, double y);

    std::shared_ptr<NumericParameter> x_ = nullptr;
    std::shared_ptr<NumericParameter> y_ = nullptr;
    std::shared_ptr<NumericParameter> radius_ = nullptr;
    std::shared_ptr<ColorParameter> color_ = nullptr;
    std::shared_ptr<NumericParameter> border_width_ = nullptr;
    std::shared_ptr<ColorParameter> border_color_ = nullptr;
    std::shared_ptr<BooleanParameter> visible_ = nullptr;

    cairo_surface_t* cache = nullptr;
    bool cache_drawn = false;
    int cache_width = 0;
    int cache_height = 0;
};

} // namespace overlay
} // namespace telemetry

#endif // CIRCLE_WIDGET_H
