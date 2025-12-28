#include "chart_widget.h"
#include <format>
#include <cmath>

namespace telemetry {
namespace overlay {

ChartWidget::ChartWidget(std::shared_ptr<track::Track> track,
                         int x,
                         int y,
                         int width,
                         int height,
                         rgba line_color,
                         int line_width,
                         rgba point_color,
                         int point_size,
                         const std::string& x_key,
                         const std::string& y_key)
    : Element(track, x, y),
      width_(width),
      height_(height),
      line_color_(line_color),
      line_width_(line_width),
      point_color_(point_color),
      point_radius_(point_size/2),
      x_key_(x_key),
      x_key_id_(track->get_field_id(x_key)),
      y_key_(y_key),
      y_key_id_(track->get_field_id(y_key)) {
    prepare();
}

void ChartWidget::draw(time::microseconds_t timestamp, cairo_t* cr) {
    draw_line(cr); //TODO add caching - line needs to be drawn only once
    draw_point(timestamp, cr); //TODO add caching - point needs to be redrawn only if it's position changes

    Element::draw(timestamp, cr); // Call base class draw to handle children
}

void ChartWidget::prepare() {
    for (auto timestamp : track_->get_trackpoint_timestamps()) {
        track::Value x_value = track_->get(x_key_id_, timestamp);
        track::Value y_value = track_->get(y_key_id_, timestamp);

        if (x_value.is_valid() && y_value.is_valid() &&
            x_value.is_double() && y_value.is_double()) {
            double xv = x_value.as_double();
            double yv = y_value.as_double();

            if (xv < min_x) min_x = xv;
            if (xv > max_x) max_x = xv;
            if (yv < min_y) min_y = yv;
            if (yv > max_y) max_y = yv;

            points.emplace_back(xv, yv);
        }
    }

    if (min_x >= max_x || min_y >= max_y) {
        valid = false;
        return; // Not enough points to draw a line
    }

    scale_x = width_ / (max_x - min_x);
    scale_y = -1 * height_ / (max_y - min_y);
}

void ChartWidget::draw_line(cairo_t* cr) {
    if (!valid) {
        return;
    }

    cairo_save(cr);

    cairo_set_source_rgba(cr, line_color_.r, line_color_.g, line_color_.b, line_color_.a);
    cairo_set_line_width(cr, line_width_);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

    int last_x = INT_MAX;
    int last_y = INT_MAX;
    for (auto [fx, fy] : points) {
        auto [xi,yi] = translate_xy(fx,fy);

        if (INT_MAX == last_x || INT_MAX == last_y) {
            cairo_move_to(cr, xi, yi);
        } else if (last_x != xi || last_y != yi) {
            cairo_line_to(cr, xi, yi);
        }

        last_x = xi;
        last_y = yi;
    }

    cairo_stroke(cr);
    cairo_restore(cr);
}

void ChartWidget::draw_point(time::microseconds_t timestamp, cairo_t* cr) {
    if (!valid) {
        return;
    }

    auto xval = track_->get(x_key_id_, timestamp);
    auto yval = track_->get(y_key_id_, timestamp);

    if (!xval.is_valid() || !yval.is_valid() ||
        !xval.is_double() || !yval.is_double()) {
        return;
    }

    auto [xi,yi] = translate_xy(xval.as_double(), yval.as_double());

    cairo_move_to(cr, xi, yi + point_radius_);
    cairo_arc(cr, xi, yi, point_radius_, 0.0, 2*M_PI);

    cairo_set_source_rgba(cr, point_color_.r, point_color_.g, point_color_.b, point_color_.a);
    cairo_fill(cr);
}

std::pair<int, int> ChartWidget::translate_xy(double xv, double yv) const {
    return std::make_pair(static_cast<int>(std::round(((xv-min_x)*scale_x) + x)),
                          static_cast<int>(std::round(((yv-min_y)*scale_y) + y + height_)));

}

} // namespace overlay
} // namespace telemetry
