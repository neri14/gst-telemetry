#ifndef CHART_WIDGET_H
#define CHART_WIDGET_H

#include "element.h"
#include "common/text_align.h"
#include "common/color.h"

namespace telemetry {
namespace overlay {

struct ChartWidget: public Element {
    ChartWidget(std::shared_ptr<track::Track> track,
                int x,
                int y,
                int width,
                int height,
                rgba line_color,
                int line_width,
                rgba point_color,
                int point_size,
                const std::string& x_key,
                const std::string& y_key);
    virtual ~ChartWidget() = default;

    void draw(time::microseconds_t timestamp, cairo_t* cr) override;

private:
    void prepare();
    void draw_line(cairo_t* cr);
    void draw_point(time::microseconds_t timestamp, cairo_t* cr);
    std::pair<int, int> translate_xy(double x, double y) const;

    int width_;
    int height_;
    rgba line_color_;
    int line_width_;
    rgba point_color_;
    int point_radius_;
    std::string x_key_;
    track::field_id_t x_key_id_;
    std::string y_key_;
    track::field_id_t y_key_id_;

    bool valid = true;
    std::vector<std::pair<double, double>> points;

    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    double scale_x = 1.0;
    double scale_y = 1.0;
};

} // namespace overlay
} // namespace telemetry

#endif // CHART_WIDGET_H