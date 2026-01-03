#ifndef LINE_WIDGET_H
#define LINE_WIDGET_H

#include "widget.h"

#include "backend/utils/logging/logger.h"
#include "params/numeric_parameter.h"
#include "params/color_parameter.h"
#include "params/boolean_parameter.h"

namespace telemetry {
namespace overlay {

class LineWidget : public Widget {
public:
    static std::shared_ptr<LineWidget> create(parameter_map_ptr parameters);
    LineWidget();
    ~LineWidget() override = default;

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr,
                      double x_offset = 0, double y_offset = 0) override;

    inline static parameter_type_map_t parameter_types = {
        {"x", ParameterType::Numeric}, // x position
        {"y", ParameterType::Numeric}, // y position
        {"x2", ParameterType::Numeric}, // x2 position
        {"y2", ParameterType::Numeric}, // y2 position
        {"line-width", ParameterType::Numeric}, // line width
        {"line-color", ParameterType::Color}, // line color
        {"visible", ParameterType::Boolean}, // visibility condition
    };

private:
    mutable utils::logging::Logger log{"LineWidget"};

    std::shared_ptr<NumericParameter> x_ = nullptr;
    std::shared_ptr<NumericParameter> y_ = nullptr;
    std::shared_ptr<NumericParameter> x2_ = nullptr;
    std::shared_ptr<NumericParameter> y2_ = nullptr;
    std::shared_ptr<NumericParameter> line_width_ = nullptr;
    std::shared_ptr<ColorParameter> line_color_ = nullptr;
    std::shared_ptr<BooleanParameter> visible_ = nullptr;

    cairo_surface_t* cache = nullptr;
    bool cache_drawn = false;
    int cache_width = 0;
    int cache_height = 0;
    int margin_ = 0;

    int origin_x_ = 0;
    int origin_y_ = 0;
};

} // namespace overlay
} // namespace telemetry

#endif // LINE_WIDGET_H
