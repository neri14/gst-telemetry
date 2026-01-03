#include "chart_widget.h"
#include "backend/utils/color.h"
#include <cmath>

extern "C" {
    #include <cairo.h>
}

namespace telemetry {
namespace overlay {
namespace defaults {
    const rgb line_color = color::white;
    const int line_width = 2;
} // namespace defaults

std::shared_ptr<ChartWidget> ChartWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"ChartWidget::create"};
    log.info("Creating ChartWidget");

    auto widget = std::make_shared<ChartWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "x") {
            widget->x_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y") {
            widget->y_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "width") {
            widget->width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "height") {
            widget->height_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "line-color") {
            widget->line_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "line-width") {
            widget->line_width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "point-color") {
            widget->point_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "point-size") {
            widget->point_size_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "x-value") {
            widget->x_value_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y-value") {
            widget->y_value_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "stretch-to-fill") {
            widget->stretch_to_fill_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for ChartWidget", name);
        }
    }

    if (!widget->x_ || !widget->y_ || !widget->width_ || !widget->height_ || !widget->x_value_ || !widget->y_value_) {
        log.error("Missing required parameters (x, y, width, height, x-value, y-value)");
        return nullptr;
    }

    if (!widget->line_color_) {
        log.debug("Line color parameter not set, using default value");
        widget->line_color_ = std::make_shared<ColorParameter>(defaults::line_color);
    }
    if (!widget->line_width_) {
        log.debug("Line width parameter not set, using default value");
        widget->line_width_ = std::make_shared<NumericParameter>(defaults::line_width);
    }
    if (!widget->stretch_to_fill_) {
        log.debug("Stretch-to-fit parameter not set, defaulting to true");
        widget->stretch_to_fill_ = std::make_shared<BooleanParameter>(true);
    }
    if (!widget->visible_) {
        log.debug("Visible parameter not set, defaulting to true");
        widget->visible_ = std::make_shared<BooleanParameter>(true);
    }

    return widget;
}

ChartWidget::ChartWidget()
        : Widget("ChartWidget") {
}

void ChartWidget::draw(time::microseconds_t timestamp, cairo_t* cr,
                        double x_offset, double y_offset) {
    visible_->update(timestamp);
    //visibility change does not invalidate cache

    if (visible_->get_value(timestamp)) {
        bool invalidate_line_cache = !line_cache_drawn_;
        bool invalidate_point_cache = !point_cache_drawn_;

        if (width_ && width_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (height_ && height_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (stretch_to_fill_ && stretch_to_fill_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (line_color_ && line_color_->update(timestamp)) {
            invalidate_line_cache = true;
        }
        if (line_width_ && line_width_->update(timestamp)) {
            invalidate_line_cache = true;
        }
        if (point_color_ && point_color_->update(timestamp)) {
            invalidate_point_cache = true;
        }
        if (point_size_ && point_size_->update(timestamp)) {
            invalidate_point_cache = true;
        }
        double width = width_->get_value(timestamp);
        double height = height_->get_value(timestamp);
        rgb line_color = line_color_->get_value(timestamp);
        double line_width = line_width_->get_value(timestamp);
        rgb point_color = point_color_->get_value(timestamp);
        double point_size = point_size_->get_value(timestamp);
        stretch_chart_ = stretch_to_fill_->get_value(timestamp);

        margin_ = 2*static_cast<int>(std::ceil(std::max(line_width, point_size)));
        int new_cache_width = static_cast<int>(std::ceil(width)) + 2*margin_;
        int new_cache_height = static_cast<int>(std::ceil(height)) + 2*margin_;

        if (new_cache_width != cache_width_ || new_cache_height != cache_height_) {
            // cache size change requires full redraw
            cache_width_ = new_cache_width;
            cache_height_ = new_cache_height;
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }

        if (invalidate_line_cache) {
            auto x_values = x_value_->get_all_values();
            auto y_values = y_value_->get_all_values();
            recalculate_extremes(x_values, y_values);
            if (invalid_) {
                log.error("ChartWidget is in invalid state, aborting drawing");
                return;
            }
            redraw_line_cache(width, height, line_color, line_width, x_values, y_values);
            line_cache_drawn_ = true;
        }

        // recalutate x and y values after possible track cache update
        // (since extremes recalculation invalidates cached values)
        if (x_value_ && x_value_->update(timestamp)) {
            invalidate_point_cache = true;
        }
        if (y_value_ && y_value_->update(timestamp)) {
            invalidate_point_cache = true;
        }
        double x_value = x_value_->get_value(timestamp, true);
        double y_value = y_value_->get_value(timestamp, true);

        if (invalidate_point_cache) {
            redraw_point_cache(width, height, point_color, point_size, x_value, y_value);
            point_cache_drawn_ = true;
        }

        x_->update(timestamp);
        y_->update(timestamp);

        double x = x_offset + x_->get_value(timestamp) - margin_;
        double y = y_offset + y_->get_value(timestamp) - margin_;

        if (line_cache_) {
            cairo_set_source_surface(cr, line_cache_, x, y);
            cairo_paint(cr);
        }
        if (point_cache_) {
            cairo_set_source_surface(cr, point_cache_, x, y);
            cairo_paint(cr);
        }

    } else {
        log.debug("Visibility is false, skipping drawing");
    }
}

void ChartWidget::redraw_line_cache(double width, double height,
                                    rgb line_color, double line_width,
                                    std::map<time::microseconds_t, double> x_values,
                                    std::map<time::microseconds_t, double> y_values) {
    int surface_width = 0;
    int surface_height = 0;

    if (line_cache_) {
        surface_width = cairo_image_surface_get_width(line_cache_);
        surface_height = cairo_image_surface_get_height(line_cache_);
    }

    if (!line_cache_ || surface_width < cache_width_ || surface_height < cache_height_) {
        // bigger widget size require allocating bigger cache
        if (line_cache_) {
            cairo_surface_destroy(line_cache_);
            line_cache_ = nullptr;
        }
        line_cache_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cache_width_, cache_height_);
        line_cache_drawn_ = false;
        log.info("Allocated new line cache surface: {}x{}", cache_width_, cache_height_);
    }

    cairo_t* cache_cr = cairo_create(line_cache_);
    if (line_cache_drawn_) {
        // clear cache
        cairo_save(cache_cr);
        cairo_set_operator(cache_cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint(cache_cr);
        cairo_restore(cache_cr);
        line_cache_drawn_ = false;
    }

    //draw line
    if (line_width > 0) {
        cairo_set_line_cap(cache_cr, CAIRO_LINE_CAP_SQUARE);
        cairo_set_line_join(cache_cr, CAIRO_LINE_JOIN_BEVEL);

        cairo_set_line_width(cache_cr, line_width);
        cairo_set_source_rgba(cache_cr, line_color.r, line_color.g, line_color.b, line_color.a);

        for (const auto& [ts, x_val] : x_values) {
            auto y_it = y_values.find(ts);
            if (y_it == y_values.end()) {
                continue; // no corresponding y value
            }
            double y_val = y_it->second;
            if (std::isnan(x_val) || std::isnan(y_val)) {
                continue; // skip NaN values
            }

            auto [x_pos, y_pos] = translate(x_val, y_val, width, height);
            x_pos += margin_;
            y_pos += margin_;

            if (cairo_has_current_point(cache_cr)) {
                cairo_line_to(cache_cr, x_pos, y_pos);
            } else {
                cairo_move_to(cache_cr, x_pos, y_pos);
            }
        }
        cairo_stroke(cache_cr);
    }

    cairo_destroy(cache_cr);
    line_cache_drawn_ = true;
}

void ChartWidget::redraw_point_cache(double width, double height,
                                  rgb point_color, double point_size,
                                  double x_value, double y_value) {
    int surface_width = 0;
    int surface_height = 0;

    if (point_cache_) {
        surface_width = cairo_image_surface_get_width(point_cache_);
        surface_height = cairo_image_surface_get_height(line_cache_);
    }

    if (!point_cache_ || surface_width < cache_width_ || surface_height < cache_height_) {
        // bigger widget size require allocating bigger cache
        if (point_cache_) {
            cairo_surface_destroy(point_cache_);
            point_cache_ = nullptr;
        }
        point_cache_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cache_width_, cache_height_);
        point_cache_drawn_ = false;
        log.info("Allocated new point cache surface: {}x{}", cache_width_, cache_height_);
    }

    cairo_t* cache_cr = cairo_create(point_cache_);
    if (point_cache_drawn_) {
        // clear cache
        cairo_save(cache_cr);
        cairo_set_operator(cache_cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint(cache_cr);
        cairo_restore(cache_cr);
        point_cache_drawn_ = false;
    }

    //draw point
    if (point_size > 0 && !std::isnan(x_value) && !std::isnan(y_value)) {
        cairo_set_line_width(cache_cr, 1.0);
        cairo_set_source_rgba(cache_cr, point_color.r, point_color.g, point_color.b, point_color.a);
        auto [x_pos, y_pos] = translate(x_value, y_value, width, height);
        x_pos += margin_;
        y_pos += margin_;

        cairo_arc(cache_cr, x_pos, y_pos, point_size / 2.0, 0, 2 * M_PI);
        cairo_fill(cache_cr);
    }

    cairo_destroy(cache_cr);
    point_cache_drawn_ = true;
}

void ChartWidget::recalculate_extremes(std::map<time::microseconds_t, double> x_values,
                                       std::map<time::microseconds_t, double> y_values) {
    if (x_values.empty() || y_values.empty()) {
        log.error("Extremes recalculation failure - no x or y values available");
        invalid_ = true;
        return;
    }

    min_x_ = std::numeric_limits<double>::max();
    max_x_ = std::numeric_limits<double>::min();
    for (const auto& [ts, x_val] : x_values) {
        if (std::isnan(x_val)) {
            continue; // skip NaN values
        }

        if (x_val < min_x_) {
            min_x_ = x_val;
        }
        if (x_val > max_x_) {
            max_x_ = x_val;
        }
    }

    min_y_ = std::numeric_limits<double>::max();
    max_y_ = std::numeric_limits<double>::min();
    for (const auto& [ts, y_val] : y_values) {
        if (std::isnan(y_val)) {
            continue; // skip NaN values
        }

        if (y_val < min_y_) {
            min_y_ = y_val;
        }
        if (y_val > max_y_) {
            max_y_ = y_val;
        }
    }

    if (min_x_ >= max_x_) {
        log.error("Extremes recalculation failure - min_x ({}) >= max_x ({})", min_x_, max_x_);
        invalid_ = true;
    } else if (min_y_ >= max_y_) {
        log.error("Extremes recalculation failure - min_y ({}) >= max_y ({})", min_y_, max_y_);
        invalid_ = true;
    } else {
        invalid_ = false;
    }
}

std::pair<double, double> ChartWidget::translate(double x_value, double y_value, double width, double height) const {
    double x_range = max_x_ - min_x_;
    double y_range = max_y_ - min_y_;

    double x_scale = width / x_range;
    double y_scale = height / y_range;

    if (!stretch_chart_) {
        double scale = std::min(x_scale, y_scale);
        x_scale = scale;
        y_scale = scale;

        //FIXME add centering to middle of widget
    }

    double x_pos = (x_value - min_x_) * x_scale;
    double y_pos = (height - (y_value - min_y_) * y_scale); // invert y axis

    return {x_pos, y_pos};
}

} // namespace overlay
} // namespace telemetry