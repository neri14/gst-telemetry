#include "line_widget.h"
#include "backend/utils/color.h"
#include "trace/trace.h"
#include <cmath>

extern "C" {
    #include <cairo.h>
}

namespace telemetry {
namespace overlay {
namespace defaults {
    const rgb color = color::white;
    const int width = 2;
} // namespace defaults

std::shared_ptr<LineWidget> LineWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"LineWidget::create"};
    log.info("Creating LineWidget");

    auto widget = std::make_shared<LineWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "x") {
            widget->x_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y") {
            widget->y_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "x2") {
            widget->x2_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y2") {
            widget->y2_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "line-width") {
            widget->line_width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "line-color") {
            widget->line_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for LineWidget", name);
        }
    }

    if (!widget->x_ || !widget->y_ || !widget->x2_ || !widget->y2_) {
        log.error("Missing required parameters (x, y, x2, y2)");
        return nullptr;
    }

    if (!widget->line_width_) {
        log.debug("Line width parameter not set, using default value");
        widget->line_width_ = std::make_shared<NumericParameter>(defaults::width);
    }
    if (!widget->line_color_) {
        log.debug("Line color parameter not set, using default value");
        widget->line_color_ = std::make_shared<ColorParameter>(defaults::color);
    }
    if (!widget->visible_) {
        log.debug("Visible parameter not set, defaulting to true");
        widget->visible_ = std::make_shared<BooleanParameter>(true);
    }

    return widget;
}

LineWidget::LineWidget()
        : Widget("LineWidget") {
}

void LineWidget::draw(time::microseconds_t timestamp,
                      schedule_drawing_cb_t schedule_drawing_cb,
                      double x_offset, double y_offset) {
    visible_->update(timestamp);

    if (visible_->get_value(timestamp)) {

        bool coords_changed = false;
        for (auto& param : std::vector<parameter_ptr_t>{x_, y_, x2_, y2_}) {
            if (param->update(timestamp)) {
                coords_changed = true;
            }
        }

        schedule_drawing_cb([this, timestamp, coords_changed, x_offset, y_offset](Surface& surface) {
            this->draw_impl(surface, timestamp, coords_changed, x_offset, y_offset);
        });

        // draw childern relative to first point
        double x = x_offset + x_->get_value(timestamp);
        double y = y_offset + y_->get_value(timestamp);
        Widget::draw(timestamp, schedule_drawing_cb, x, y);
    } else {
        log.debug("Visibility is false, skipping drawing");
    }
}

void LineWidget::draw_impl(Surface& surface, time::microseconds_t timestamp,
                           bool coords_changed, double x_offset, double y_offset) {
    TRACE_EVENT_BEGIN(EV_LINE_WIDGET_DRAW);

    bool cache_update_needed = !cache_drawn || coords_changed;
    for (auto& param : std::vector<parameter_ptr_t>{x2_, y2_, line_width_, line_color_}) {
        if (param->update(timestamp)) {
            cache_update_needed = true;
        }
    }

    if (cache_update_needed) {
        TRACE_EVENT_BEGIN(EV_LINE_WIDGET_UPDATE_CACHE);
        // get parameter values
        double x1 = x_->get_value(timestamp);
        double y1 = y_->get_value(timestamp);
        double x2 = x2_->get_value(timestamp);
        double y2 = y2_->get_value(timestamp);
        double line_width = line_width_->get_value(timestamp);
        rgb line_color = line_color_->get_value(timestamp);

        margin_ = line_width * 2;

        // determine required surface size
        int min_x = std::floor(std::min(x1, x2) - line_width / 2) - margin_;
        int min_y = std::floor(std::min(y1, y2) - line_width / 2) - margin_;
        int max_x = std::ceil(std::max(x1, x2) + line_width / 2) + margin_;
        int max_y = std::ceil(std::max(y1, y2) + line_width / 2) + margin_;
        int surface_width = max_x - min_x;
        int surface_height = max_y - min_y;

        // create or resize cache surface if needed
        if (!cache || surface_width != cache_width || surface_height != cache_height) {
            if (cache) {
                cairo_surface_destroy(cache);
            }
            cache = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, surface_width, surface_height);
            cache_width = surface_width;
            cache_height = surface_height;
            log.info("Allocated new line cache surface: {}x{}", surface_width, surface_height);
        }

        cairo_t* cache_cr = cairo_create(cache);

        if (cache_drawn) {
            // clear cache
            cairo_save(cache_cr);
            cairo_set_operator(cache_cr, CAIRO_OPERATOR_CLEAR);
            cairo_paint(cache_cr);
            cairo_restore(cache_cr);
            cache_drawn = false;
        }

        // draw line onto cache surface
        cairo_set_source_rgba(cache_cr, line_color.r, line_color.g, line_color.b, line_color.a);
        cairo_set_line_width(cache_cr, line_width);
        cairo_move_to(cache_cr, x1 - min_x, y1 - min_y);
        cairo_line_to(cache_cr, x2 - min_x, y2 - min_y);
        cairo_stroke(cache_cr);
        cairo_destroy(cache_cr);

        origin_x_ = min_x;
        origin_y_ = min_y;
        cache_drawn = true;

        TRACE_EVENT_END(EV_LINE_WIDGET_UPDATE_CACHE);
    }

    surface.x = origin_x_ + x_offset;;
    surface.y = origin_y_ + y_offset;
    surface.surface = cache;

    TRACE_EVENT_END(EV_LINE_WIDGET_DRAW);
}

} // namespace overlay
} // namespace telemetry