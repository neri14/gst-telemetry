#include "chart_widget.h"
#include "backend/utils/color.h"
#include <cmath>

extern "C" {
    #include <cairo.h>
}

//FIXME major refactoring required to cleanup the code

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
        } else if (name == "value-time-step") {
            widget->value_time_step_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "stretch-to-fill") {
            widget->stretch_to_fill_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "min-x") {
            widget->min_x_param_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "max-x") {
            widget->max_x_param_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "min-y") {
            widget->min_y_param_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "max-y") {
            widget->max_y_param_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "filter-value") {
            widget->filter_value_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "filter-max") {
            widget->filter_max_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "filter-min") {
            widget->filter_min_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "zoom-to-filter-x") {
            widget->zoom_to_filter_x_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else if (name == "zoom-to-filter-y") {
            widget->zoom_to_filter_y_ = std::dynamic_pointer_cast<BooleanParameter>(param);
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

    if (!widget->zoom_to_filter_x_) {
        log.debug("Zoom-to-filter-x parameter not set, defaulting to false");
        widget->zoom_to_filter_x_ = std::make_shared<BooleanParameter>(false);
    }
    if (!widget->zoom_to_filter_y_) {
        log.debug("Zoom-to-filter-y parameter not set, defaulting to false");
        widget->zoom_to_filter_y_ = std::make_shared<BooleanParameter>(false);
    }

    return widget;
}

ChartWidget::ChartWidget()
        : Widget("ChartWidget") {
}

void ChartWidget::draw(time::microseconds_t timestamp,
                       double x_offset, double y_offset, draw_cb_t draw_cb) {
    // calculate visibility
    visible_->update(timestamp);

    if (visible_->get_value(timestamp)) {
        // if cache is not drawn, mark for redraw
        bool invalidate_line_cache = !line_cache_drawn_;
        bool invalidate_point_cache = !point_cache_drawn_;

        // update parameters that can affect cache validity
        if (width_ && width_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (height_ && height_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (value_time_step_ && value_time_step_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (stretch_to_fill_ && stretch_to_fill_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (min_x_param_ && min_x_param_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (max_x_param_ && max_x_param_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (min_y_param_ && min_y_param_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (max_y_param_ && max_y_param_->update(timestamp)) {
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
        // update filtering parameters
        if (filter_value_ && filter_value_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (filter_max_ && filter_max_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (filter_min_ && filter_min_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (zoom_to_filter_x_ && zoom_to_filter_x_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }
        if (zoom_to_filter_y_ && zoom_to_filter_y_->update(timestamp)) {
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }

        // read values needed for cache size calculation
        double width = width_->get_value(timestamp);
        double height = height_->get_value(timestamp);
        double line_width = line_width_->get_value(timestamp);
        double point_size = point_size_ ? point_size_->get_value(timestamp) : 0.0;
        stretch_chart_ = stretch_to_fill_->get_value(timestamp);

        // calculate margin and cache size
        margin_ = 2*static_cast<int>(std::ceil(std::max(line_width, point_size)));
        int min_cache_width = static_cast<int>(std::ceil(width)) + 2*margin_;
        int min_cache_height = static_cast<int>(std::ceil(height)) + 2*margin_;

        if (min_cache_width > cache_width_ || min_cache_height > cache_height_) {
            // cache size change requires full redraw
            cache_width_ = min_cache_width;
            cache_height_ = min_cache_height;
            invalidate_line_cache = true;
            invalidate_point_cache = true;
        }

        time::microseconds_t value_step = time::INVALID_TIME;
        if (value_time_step_) {
            double step_val = value_time_step_->get_value(timestamp);
            if (step_val > 0.0) {
                value_step = time::s_to_us(step_val);;
            }
        }

        // redraw line cache if needed
        if (invalidate_line_cache) {
            // read filter values
            bool filter_active = !!filter_value_;
            bool filter_zoom_x = filter_active && zoom_to_filter_x_ && zoom_to_filter_x_->get_value(timestamp);
            bool filter_zoom_y = filter_active && zoom_to_filter_y_ && zoom_to_filter_y_->get_value(timestamp);

            double filter_min = filter_min_ ? filter_min_->get_value(timestamp) : std::numeric_limits<double>::min();
            double filter_max = filter_max_ ? filter_max_->get_value(timestamp) : std::numeric_limits<double>::max();
            auto filter_values = filter_value_ ? filter_value_->get_values(value_step, filter_min, filter_max) : nullptr;

            std::shared_ptr<NumericParameter::sections_t> x_values = nullptr;
            std::shared_ptr<NumericParameter::sections_t> y_values = nullptr;

            if (filter_active && filter_values){
                std::vector<std::vector<time::microseconds_t>> timestamps = {};
                for (const auto& section : *filter_values) {
                    std::vector<time::microseconds_t> ts_section = {};
                    for (const auto& [ts, _] : section) {
                        ts_section.push_back(ts);
                    }
                    timestamps.push_back(ts_section);
                }
                x_values = x_value_->get_values(timestamps);
                y_values = y_value_->get_values(timestamps);
            } else {
                x_values = x_value_->get_values(value_step);
                y_values = y_value_->get_values(value_step);
            }

            lock_x_minmax_ = false;
            if (min_x_param_ && max_x_param_) {
                min_x_ = min_x_param_->get_value(timestamp);
                max_x_ = max_x_param_->get_value(timestamp);

                if (min_x_ < max_x_) {
                    lock_x_minmax_ = true;
                }
            }
            lock_y_minmax_ = false;
            if (min_y_param_ && max_y_param_) {
                min_y_ = min_y_param_->get_value(timestamp);
                max_y_ = max_y_param_->get_value(timestamp);
                if (min_y_ < max_y_) {
                    lock_y_minmax_ = true;
                }
            }

            recalculate_extremes(filter_zoom_x ? x_values : x_value_->get_values(),
                                 filter_zoom_y ? y_values : y_value_->get_values());

            if (invalid_) {
                log.error("ChartWidget is in invalid state, aborting drawing");
                return;
            }

            redraw_line_cache(width, height,
                line_color_->get_value(timestamp), line_width, x_values, y_values);
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

        // redraw point cache if needed
        if (invalidate_point_cache) {
            redraw_point_cache(width, height,
                point_color_ ? point_color_->get_value(timestamp) : color::transparent,
                point_size, x_value, y_value);
            point_cache_drawn_ = true;
        }

        // calculate widget position
        x_->update(timestamp);
        y_->update(timestamp);

        double x = x_offset + x_->get_value(timestamp);
        double y = y_offset + y_->get_value(timestamp);

        double draw_x = x - margin_;
        double draw_y = y - margin_;

        // draw caches onto surface
        if (line_cache_) {
            draw_cb(static_cast<int>(std::round(draw_x)), static_cast<int>(std::round(draw_y)), line_cache_);
        }
        if (point_cache_) {
            draw_cb(static_cast<int>(std::round(draw_x)), static_cast<int>(std::round(draw_y)), point_cache_);
        }

        // draw childern relative to chart top-left
        Widget::draw(timestamp, x, y, draw_cb);
    } else {
        log.debug("Visibility is false, skipping drawing");
    }
}

unsigned int ChartWidget::surface_count() const {
    return Widget::surface_count() + 2;
}

void ChartWidget::redraw_line_cache(double width, double height,
                                    rgb line_color, double line_width,
                                    std::shared_ptr<NumericParameter::sections_t> x_values,
                                    std::shared_ptr<NumericParameter::sections_t> y_values) {
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

        auto find_y_value = [&](time::microseconds_t ts) -> double {
            for (const auto& section : *y_values) {
                auto it = section.find(ts);
                if (it != section.end()) {
                    return it->second;
                }
            }
            return std::numeric_limits<double>::quiet_NaN();
        };

        for (const auto& section : *x_values) {
            bool last_point_valid = false;
            for (const auto& [ts, x_val] : section) {
                double y_val = find_y_value(ts);

                if (std::isnan(x_val) || std::isnan(y_val)) {
                    cairo_stroke(cache_cr); // finish current section
                    last_point_valid = false;
                    continue; // skip NaN values
                }

                auto [x_pos, y_pos] = translate(x_val, y_val, width, height);
                x_pos += margin_;
                y_pos += margin_;

                if (last_point_valid && cairo_has_current_point(cache_cr)) {
                    cairo_line_to(cache_cr, x_pos, y_pos);
                } else {
                    cairo_move_to(cache_cr, x_pos, y_pos);
                }
                last_point_valid = true;
            }
            cairo_stroke(cache_cr); // finish section
        }
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
        surface_height = cairo_image_surface_get_height(point_cache_);
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
        point_cache_drawn_ = true;
    }

    cairo_destroy(cache_cr);
}

void ChartWidget::recalculate_extremes(std::shared_ptr<NumericParameter::sections_t> x_values,
                                       std::shared_ptr<NumericParameter::sections_t> y_values) {
    invalid_ = false;// reset invalid state
    if (lock_x_minmax_ && lock_y_minmax_) {
        // both min and max are locked, no need to recalculate
        return;
    }

    if (!x_values || x_values->empty() || !y_values || y_values->empty()) {
        log.error("Extremes recalculation failure - no x or y values available");
        invalid_ = true;
        return;
    }

    if (!lock_x_minmax_) {  
        min_x_ = std::numeric_limits<double>::max();
        max_x_ = std::numeric_limits<double>::min();
        for (const auto& section: *x_values) {
            for (const auto& [ts, x_val] : section) {
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
        }
        if (min_x_ >= max_x_) {
            log.error("Extremes recalculation failure - min_x ({}) >= max_x ({})", min_x_, max_x_);
            invalid_ = true;
        }
    }

    if (!lock_y_minmax_) {
        min_y_ = std::numeric_limits<double>::max();
        max_y_ = std::numeric_limits<double>::min();
        for (const auto& section: *y_values) {
            for (const auto& [ts, y_val] : section) {
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
        }
        if (min_y_ >= max_y_) {
            log.error("Extremes recalculation failure - min_y ({}) >= max_y ({})", min_y_, max_y_);
            invalid_ = true;
        }
    }
}

std::pair<double, double> ChartWidget::translate(double x_value, double y_value, double width, double height) const {
    double x_range = max_x_ - min_x_;
    double y_range = max_y_ - min_y_;

    double x_scale = width / x_range;
    double y_scale = height / y_range;

    double x_offset = 0;
    double y_offset = 0;

    if (!stretch_chart_) {
        if (x_scale < y_scale) {
            double used_height = y_range * x_scale;
            y_offset = (height - used_height) / 2.0;

            y_scale = x_scale;
        } else if (y_scale < x_scale) {
            double used_width = x_range * y_scale;
            x_offset = (width - used_width) / 2.0;

            x_scale = y_scale;
        }
    }

    double x_pos = (x_value - min_x_) * x_scale + x_offset;
    double y_pos = height - y_offset - (y_value - min_y_) * y_scale; // invert y axis and apply centering offset
    return {x_pos, y_pos};
}

} // namespace overlay
} // namespace telemetry