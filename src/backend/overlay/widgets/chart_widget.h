#ifndef CHART_WIDGET_H
#define CHART_WIDGET_H

#include "widget.h"

#include "params/numeric_parameter.h"
#include "params/color_parameter.h"
#include "params/boolean_parameter.h"

#include <limits>

namespace telemetry {
namespace overlay {

class ChartWidget : public Widget {
public:
    static std::shared_ptr<ChartWidget> create(parameter_map_ptr parameters);
    ChartWidget();
    ~ChartWidget() override = default;

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr,
                      double x_offset = 0, double y_offset = 0) override;

    inline static parameter_type_map_t parameter_types = {
        {"x", ParameterType::Numeric}, // center x position
        {"y", ParameterType::Numeric}, // center y position
        {"width", ParameterType::Numeric}, // chart width
        {"height", ParameterType::Numeric}, // chart height
        {"line-color", ParameterType::Color}, // line color
        {"line-width", ParameterType::Numeric}, // line width
        {"point-color", ParameterType::Color}, // point color
        {"point-size", ParameterType::Numeric}, // point size
        {"x-value", ParameterType::Numeric}, // track key for x values
        {"y-value", ParameterType::Numeric}, // track key for y values
        {"stretch-to-fill", ParameterType::Boolean}, // whether to stretch chart to fill widget size
        {"visible", ParameterType::Boolean}, // visibility condition
    };

private:
    void redraw_line_cache(double width, double height,
                       rgb line_color, double line_width,
                       std::map<time::microseconds_t, double> x_values,
                       std::map<time::microseconds_t, double> y_values);
    void redraw_point_cache(double width, double height,
                        rgb point_color, double point_size,
                        double x_value, double y_value);

    void recalculate_extremes(std::map<time::microseconds_t, double> x_values,
                              std::map<time::microseconds_t, double> y_values);
    std::pair<double, double> translate(double x_value, double y_value, double width, double height) const;

    std::shared_ptr<NumericParameter> x_ = nullptr;
    std::shared_ptr<NumericParameter> y_ = nullptr;
    std::shared_ptr<NumericParameter> width_ = nullptr;
    std::shared_ptr<NumericParameter> height_ = nullptr;
    std::shared_ptr<ColorParameter> line_color_ = nullptr;
    std::shared_ptr<NumericParameter> line_width_ = nullptr;
    std::shared_ptr<ColorParameter> point_color_ = nullptr;
    std::shared_ptr<NumericParameter> point_size_ = nullptr;
    std::shared_ptr<NumericParameter> x_value_ = nullptr;
    std::shared_ptr<NumericParameter> y_value_ = nullptr;
    std::shared_ptr<BooleanParameter> stretch_to_fill_ = nullptr;
    std::shared_ptr<BooleanParameter> visible_ = nullptr;

    cairo_surface_t* line_cache_ = nullptr;
    bool line_cache_drawn_ = false;
        
    cairo_surface_t* point_cache_ = nullptr;
    bool point_cache_drawn_ = false;

    int margin_ = 0;
    int cache_width_ = 0;
    int cache_height_ = 0;

    double min_x_ = std::numeric_limits<double>::max();
    double max_x_ = std::numeric_limits<double>::min();
    double min_y_ = std::numeric_limits<double>::max();
    double max_y_ = std::numeric_limits<double>::min();
    bool stretch_chart_ = false;

    bool invalid_ = false;
};

} // namespace overlay
} // namespace telemetry

#endif // CHART_WIDGET_H
