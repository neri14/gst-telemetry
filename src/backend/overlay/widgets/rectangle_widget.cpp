#include "rectangle_widget.h"
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
    const int border_width = 0; // if border not defined - no border
    const rgb border_color = color::black;
} // namespace defaults

std::shared_ptr<RectangleWidget> RectangleWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"RectangleWidget::create"};
    log.info("Creating RectangleWidget");

    auto widget = std::make_shared<RectangleWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "x") {
            widget->x_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y") {
            widget->y_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "width") {
            widget->width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "height") {
            widget->height_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "color") {
            widget->color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "border-width") {
            widget->border_width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "border-color") {
            widget->border_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for RectangleWidget", name);
        }
    }

    if (!widget->x_ || !widget->y_ || !widget->width_ || !widget->height_) {
        log.error("Missing required parameters (x, y, width, height)");
        return nullptr;
    }

    if (!widget->color_) {
        log.debug("Color parameter not set, using default value");
        widget->color_ = std::make_shared<ColorParameter>(defaults::color);
    }
    if (!widget->border_width_) {
        log.debug("Border width parameter not set, using default value");
        widget->border_width_ = std::make_shared<NumericParameter>(defaults::border_width);
    }
    if (!widget->border_color_) {
        log.debug("Border color parameter not set, using default value");
        widget->border_color_ = std::make_shared<ColorParameter>(defaults::border_color);
    }
    if (!widget->visible_) {
        log.debug("Visible parameter not set, defaulting to true");
        widget->visible_ = std::make_shared<BooleanParameter>(true);
    }

    return widget;
}

RectangleWidget::RectangleWidget()
        : Widget("RectangleWidget") {
}

void RectangleWidget::draw(time::microseconds_t timestamp, cairo_t* cr,
                        double x_offset, double y_offset) {
    visible_->update(timestamp);

    if (visible_->get_value(timestamp)) {
        TRACE_EVENT_BEGIN(EV_RECTANGLE_WIDGET_DRAW);

        for (auto& param : {x_, y_}) {
            param->update(timestamp);
        }

        bool cache_update_needed = !cache_drawn;
        for (auto& param : std::vector<parameter_ptr_t>{width_, height_, color_, border_width_, border_color_}) {
            if (param->update(timestamp)) {
                cache_update_needed = true;
            }
        }

        if (cache_update_needed) {
            TRACE_EVENT_BEGIN(EV_RECTANGLE_WIDGET_UPDATE_CACHE);

            double width = width_->get_value(timestamp);
            double height = height_->get_value(timestamp);
            double border_width = border_width_->get_value(timestamp);

            margin_ = 2*static_cast<int>(std::ceil(border_width));

            int surface_width = static_cast<int>(std::ceil(width + 2 * margin_));
            int surface_height = static_cast<int>(std::ceil(height + 2 * margin_));

            if (!cache || surface_width > cache_width || surface_height > cache_height) {
                // bigger widget size require allocating bigger cache
                if (cache) {
                    cairo_surface_destroy(cache);
                    cache = nullptr;
                }
                cache_width = surface_width;
                cache_height = surface_height;
                cache = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cache_width, cache_height);
                cache_drawn = false;
                log.info("Allocated new cache surface: {}x{}", cache_width, cache_height);
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

            rgb color = color_->get_value(timestamp);
            rgb border_color = border_color_->get_value(timestamp);

            cairo_rectangle(cache_cr, margin_, margin_, width, height);

            cairo_set_source_rgba(cache_cr, color.r, color.g, color.b, color.a);
            cairo_fill_preserve(cache_cr);

            if (border_width > 0) {
                cairo_set_line_width(cache_cr, border_width);
                cairo_set_source_rgba(cache_cr, border_color.r, border_color.g, border_color.b, border_color.a);
                cairo_stroke(cache_cr);
            }//border width 0 means no border (silently ignore negative border)

            cairo_surface_flush(cache);
            cache_drawn = true;

            cairo_destroy(cache_cr);

            TRACE_EVENT_END(EV_RECTANGLE_WIDGET_UPDATE_CACHE);
        }

        double x = x_offset + x_->get_value(timestamp);
        double y = y_offset + y_->get_value(timestamp);

        TRACE_EVENT_BEGIN(EV_RECTANGLE_WIDGET_DRAW_CACHE);
        cairo_set_source_surface(cr, cache, x - margin_, y - margin_);
        cairo_paint(cr);
        TRACE_EVENT_END(EV_RECTANGLE_WIDGET_DRAW_CACHE);

        TRACE_EVENT_END(EV_RECTANGLE_WIDGET_DRAW);

        // draw childern relative to circle center
        // (only if circle is visible)
        Widget::draw(timestamp, cr, x, y);
    } else {
        log.debug("Visibility is false, skipping drawing");
    }
}

} // namespace overlay
} // namespace telemetry