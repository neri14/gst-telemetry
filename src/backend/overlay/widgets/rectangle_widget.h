#ifndef RECTANGLE_WIDGET_H
#define RECTANGLE_WIDGET_H

#include "widget.h"

#include "backend/utils/logging/logger.h"
#include "params/numeric_parameter.h"
#include "params/color_parameter.h"
#include "params/boolean_parameter.h"

namespace telemetry {
namespace overlay {

class RectangleWidget : public Widget {
public:
    static std::shared_ptr<RectangleWidget> create(parameter_map_ptr parameters);
    RectangleWidget();
    ~RectangleWidget() override = default;

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr,
                      double x_offset = 0, double y_offset = 0) override;

    inline static parameter_type_map_t parameter_types = {
        {"x", ParameterType::Numeric}, // center x position
        {"y", ParameterType::Numeric}, // center y position
        {"width", ParameterType::Numeric}, // rectangle width
        {"height", ParameterType::Numeric}, // rectangle height
        {"color", ParameterType::Color}, // fill color
        {"border-width", ParameterType::Numeric}, // border width
        {"border-color", ParameterType::Color}, // border color
        {"visible", ParameterType::Boolean}, // visibility condition
    };

private:
    mutable utils::logging::Logger log{"RectangleWidget"};

    std::shared_ptr<NumericParameter> x_ = nullptr;
    std::shared_ptr<NumericParameter> y_ = nullptr;
    std::shared_ptr<NumericParameter> width_ = nullptr;
    std::shared_ptr<NumericParameter> height_ = nullptr;
    std::shared_ptr<ColorParameter> color_ = nullptr;
    std::shared_ptr<NumericParameter> border_width_ = nullptr;
    std::shared_ptr<ColorParameter> border_color_ = nullptr;
    std::shared_ptr<BooleanParameter> visible_ = nullptr;

    cairo_surface_t* cache = nullptr;
    bool cache_drawn = false;
    int cache_width = 0;
    int cache_height = 0;
    int margin_ = 0;
};

} // namespace overlay
} // namespace telemetry

#endif // RECTANGLE_WIDGET_H
