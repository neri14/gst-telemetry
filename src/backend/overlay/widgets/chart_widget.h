#ifndef CHART_WIDGET_H
#define CHART_WIDGET_H

#include "widget.h"

#include "params/numeric_parameter.h"
#include "params/color_parameter.h"
#include "params/boolean_parameter.h"
#include "params/string_parameter.h"

#include <limits>
#include <vector>

namespace telemetry {
namespace overlay {

// Where an extreme-value marker's label is placed relative to its circle.
// top/bottom labels are horizontally centered; *-left labels are right-aligned
// (text ends at the marker); *-right labels are left-aligned (text starts at
// the marker).
enum class ELabelPosition {
    Top,
    Bottom,
    Left,
    Right,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

// Bundles the style parameters for one direction (max or min) of local-extreme
// marker. Populated in ChartWidget::create() with the same required/optional +
// default-fallback handling as every other ChartWidget parameter.
struct MarkerStyle {
    std::shared_ptr<BooleanParameter> enabled = nullptr;
    std::shared_ptr<ColorParameter> color = nullptr;
    std::shared_ptr<NumericParameter> radius = nullptr;
    std::shared_ptr<NumericParameter> border_width = nullptr;
    std::shared_ptr<ColorParameter> border_color = nullptr;
    std::shared_ptr<ColorParameter> label_color = nullptr;
    std::shared_ptr<StringParameter> label_position = nullptr;
};

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

        {"marker-window", ParameterType::Numeric}, // x-axis window width for local-extreme detection (required if max-marker or min-marker is set)
        {"marker-format", ParameterType::String}, // std::vformat format string applied to the marked y-value
        {"marker-font-name", ParameterType::String}, // marker label font name
        {"marker-font-size", ParameterType::Numeric}, // marker label font size
        {"marker-label-offset", ParameterType::Numeric}, // gap between marker edge and label
        {"marker-label-border-width", ParameterType::Numeric}, // marker label outline width
        {"marker-label-border-color", ParameterType::Color}, // marker label outline color

        {"max-marker", ParameterType::Boolean}, // enable local-maximum markers
        {"max-marker-color", ParameterType::Color}, // local-maximum marker fill color
        {"max-marker-radius", ParameterType::Numeric}, // local-maximum marker radius
        {"max-marker-border-width", ParameterType::Numeric}, // local-maximum marker ring width
        {"max-marker-border-color", ParameterType::Color}, // local-maximum marker ring color
        {"max-marker-label-color", ParameterType::Color}, // local-maximum label color (defaults to marker border color)
        {"max-marker-label-position", ParameterType::String}, // local-maximum label position (top/bottom/left/right/top-left/top-right/bottom-left/bottom-right)

        {"min-marker", ParameterType::Boolean}, // enable local-minimum markers
        {"min-marker-color", ParameterType::Color}, // local-minimum marker fill color
        {"min-marker-radius", ParameterType::Numeric}, // local-minimum marker radius
        {"min-marker-border-width", ParameterType::Numeric}, // local-minimum marker ring width
        {"min-marker-border-color", ParameterType::Color}, // local-minimum marker ring color
        {"min-marker-label-color", ParameterType::Color}, // local-minimum label color (defaults to marker border color)
        {"min-marker-label-position", ParameterType::String}, // local-minimum label position (top/bottom/left/right/top-left/top-right/bottom-left/bottom-right)
    };

private:
    // one flattened, gap-free, x-ascending point used for local-extrema detection
    struct ExtremePoint {
        time::microseconds_t ts;
        double x_val;
        double y_val;
    };

    void draw_impl(Surface& surface, time::microseconds_t timestamp, double x, double y);

    void redraw_line_cache(double width, double height, double line_width,
                       std::shared_ptr<NumericParameter::sections_t> x_values,
                       std::shared_ptr<NumericParameter::sections_t> y_values);
    void draw_background(cairo_t* cache_cr, double width, double height, double line_width,
                         std::shared_ptr<NumericParameter::sections_t> x_values,
                         std::shared_ptr<NumericParameter::sections_t> y_values);
    void draw_line(cairo_t* cache_cr, double width, double height, double line_width,
                  std::shared_ptr<NumericParameter::sections_t> x_values,
                  std::shared_ptr<NumericParameter::sections_t> y_values);


    void redraw_point_cache(double width, double height,
                        rgb point_color, double point_size,
                        rgb point_border_color, double point_border_width,
                        double x_value, double y_value);

    void redraw_extremes_cache(double width, double height, time::microseconds_t timestamp,
                               std::shared_ptr<NumericParameter::sections_t> x_values,
                               std::shared_ptr<NumericParameter::sections_t> y_values);
    void draw_direction_markers(cairo_t* cache_cr, double width, double height, time::microseconds_t timestamp,
                                const NumericParameter::sections_t& x_values,
                                const NumericParameter::sections_t& y_values,
                                bool find_max, const MarkerStyle& style);
    void draw_marker(cairo_t* cache_cr, double x_pos, double y_pos,
                     double radius, rgb color, double border_width, rgb border_color,
                     const std::string& label, const std::string& font,
                     rgb label_color, double label_border_width, rgb label_border_color,
                     ELabelPosition position, double label_offset) const;
    std::vector<ExtremePoint> find_local_extrema(const NumericParameter::sections_t& x_values,
                                                 const NumericParameter::sections_t& y_values,
                                                 double window, bool find_max) const;

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

    std::shared_ptr<NumericParameter> marker_window_ = nullptr;
    std::shared_ptr<StringParameter> marker_format_ = nullptr;
    std::shared_ptr<StringParameter> marker_font_name_ = nullptr;
    std::shared_ptr<NumericParameter> marker_font_size_ = nullptr;
    std::shared_ptr<NumericParameter> marker_label_offset_ = nullptr;
    std::shared_ptr<NumericParameter> marker_label_border_width_ = nullptr;
    std::shared_ptr<ColorParameter> marker_label_border_color_ = nullptr;

    MarkerStyle max_marker_style_;
    MarkerStyle min_marker_style_;

    cairo_surface_t* line_cache_ = nullptr;
    bool line_cache_drawn_ = false;
        
    cairo_surface_t* point_cache_ = nullptr;
    bool point_cache_drawn_ = false;

    cairo_surface_t* extremes_cache_ = nullptr;
    bool extremes_cache_drawn_ = false;

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
