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

    virtual void draw(time::microseconds_t timestamp,
                      schedule_drawing_cb_t schedule_drawing_cb,
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
        {"point-border-color", ParameterType::Color}, // point border color
        {"point-border-width", ParameterType::Numeric}, // point border width
        {"background-below", ParameterType::Color}, // background below color
        {"x-value", ParameterType::Numeric}, // track key for x values
        {"y-value", ParameterType::Numeric}, // track key for y values
        {"value-time-step", ParameterType::Numeric}, // time step for values
        {"stretch-to-fill", ParameterType::Boolean}, // whether to stretch chart to fill widget size
        {"min-x", ParameterType::Numeric}, // minimum x value (overrides auto-scaling)
        {"max-x", ParameterType::Numeric}, // maximum x value (overrides auto-scaling)
        {"min-y", ParameterType::Numeric}, // minimum y value (overrides auto-scaling)
        {"max-y", ParameterType::Numeric}, // maximum y value (overrides auto-scaling)
        {"visible", ParameterType::Boolean}, // visibility condition
        {"filter-value", ParameterType::Numeric}, // value to filter by
        {"filter-max", ParameterType::Numeric}, // maximum filter-value accepted
        {"filter-min", ParameterType::Numeric}, // minimum filter-value accepted
        {"zoom-to-filter-x", ParameterType::Boolean}, // whether to zoom x to filtered values only
        {"zoom-to-filter-y", ParameterType::Boolean}, // whether to zoom y to filtered values only
    };

private:
    void draw_impl(Surface& surface, time::microseconds_t timestamp, double x, double y);

    void redraw_line_cache(double width, double height, double line_width,
                       std::shared_ptr<NumericParameter::sections_t> x_values,
                       std::shared_ptr<NumericParameter::sections_t> y_values);
    void redraw_point_cache(double width, double height, 
                        rgb point_color, double point_size,
                        rgb point_border_color, double point_border_width,
                        double x_value, double y_value);

    void recalculate_extremes(std::shared_ptr<NumericParameter::sections_t> x_values,
                              std::shared_ptr<NumericParameter::sections_t> y_values);
    std::pair<double, double> translate(double x_value, double y_value, double width, double height) const;

    std::shared_ptr<NumericParameter> x_ = nullptr;
    std::shared_ptr<NumericParameter> y_ = nullptr;
    std::shared_ptr<NumericParameter> width_ = nullptr;
    std::shared_ptr<NumericParameter> height_ = nullptr;
    std::shared_ptr<ColorParameter> line_color_ = nullptr;
    std::shared_ptr<NumericParameter> line_width_ = nullptr;
    std::shared_ptr<ColorParameter> point_color_ = nullptr;
    std::shared_ptr<NumericParameter> point_size_ = nullptr;
    std::shared_ptr<ColorParameter> point_border_color_ = nullptr;
    std::shared_ptr<NumericParameter> point_border_width_ = nullptr;
    std::shared_ptr<ColorParameter> background_below_ = nullptr;

    std::shared_ptr<NumericParameter> x_value_ = nullptr;
    std::shared_ptr<NumericParameter> y_value_ = nullptr;
    std::shared_ptr<NumericParameter> value_time_step_ = nullptr;
    std::shared_ptr<BooleanParameter> stretch_to_fill_ = nullptr;

    std::shared_ptr<NumericParameter> min_x_param_ = nullptr;
    std::shared_ptr<NumericParameter> max_x_param_ = nullptr;
    std::shared_ptr<NumericParameter> min_y_param_ = nullptr;
    std::shared_ptr<NumericParameter> max_y_param_ = nullptr;

    std::shared_ptr<BooleanParameter> visible_ = nullptr;

    std::shared_ptr<NumericParameter> filter_value_ = nullptr;
    std::shared_ptr<NumericParameter> filter_max_ = nullptr;
    std::shared_ptr<NumericParameter> filter_min_ = nullptr;
    std::shared_ptr<BooleanParameter> zoom_to_filter_x_ = nullptr;
    std::shared_ptr<BooleanParameter> zoom_to_filter_y_ = nullptr;

    cairo_surface_t* line_cache_ = nullptr;
    bool line_cache_drawn_ = false;
        
    cairo_surface_t* point_cache_ = nullptr;
    bool point_cache_drawn_ = false;

    cairo_surface_t* combined_cache_ = nullptr;
    bool combined_cache_drawn_ = false;

    int margin_ = 0;
    int cache_width_ = 0;
    int cache_height_ = 0;

    double min_x_ = std::numeric_limits<double>::max();
    double max_x_ = std::numeric_limits<double>::min();
    double min_y_ = std::numeric_limits<double>::max();
    double max_y_ = std::numeric_limits<double>::min();

    bool lock_x_minmax_ = false;
    bool lock_y_minmax_ = false;

    bool stretch_chart_ = false;

    std::shared_ptr<NumericParameter::sections_t> last_filter_values_ = nullptr;

    bool invalid_ = false;
};

} // namespace overlay
} // namespace telemetry

#endif // CHART_WIDGET_H
