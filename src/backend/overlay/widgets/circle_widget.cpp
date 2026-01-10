#include "circle_widget.h"
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

std::shared_ptr<CircleWidget> CircleWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"CircleWidget::create"};
    log.info("Creating CircleWidget");

    auto widget = std::make_shared<CircleWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "x") {
            widget->x_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y") {
            widget->y_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "radius") {
            widget->radius_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "color") {
            widget->color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "border-width") {
            widget->border_width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "border-color") {
            widget->border_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for CircleWidget", name);
        }
    }

    if (!widget->x_ || !widget->y_ || !widget->radius_ ) {
        log.error("Missing required parameters (x, y, radius)");
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

CircleWidget::CircleWidget()
        : Widget("CircleWidget") {
}

void CircleWidget::draw(time::microseconds_t timestamp,
                        schedule_drawing_cb_t schedule_drawing_cb,
                        double x_offset, double y_offset) {
    visible_->update(timestamp);
    //visibility change does not invalidate cache

    if (visible_->get_value(timestamp)) {
        x_->update(timestamp);
        y_->update(timestamp);

        double x = x_offset + x_->get_value(timestamp);
        double y = y_offset + y_->get_value(timestamp);

        schedule_drawing_cb([this, timestamp, x, y](Surface& surface) {
            this->draw_impl(surface, timestamp, x, y);
        });

        // draw childern relative to circle center
        // (only if circle is visible)
        Widget::draw(timestamp, schedule_drawing_cb, x, y);
    } else {
        log.debug("Visibility is false, skipping drawing");
    }
}

void CircleWidget::draw_impl(Surface& surface, time::microseconds_t timestamp, double x, double y) {
    TRACE_EVENT_BEGIN(EV_CIRCLE_WIDGET_DRAW);

    bool cache_update_needed = !cache_drawn;
    // since we recalculate the params only if widget is visible
    // change to params that impact cache will be detected here
    // if they changed while widget was not visible
    for (auto& param : std::vector<parameter_ptr_t>{radius_, color_, border_width_, border_color_}) {
        if (param->update(timestamp)) {
            cache_update_needed = true;
        }
    }

    if (cache_update_needed) {
        TRACE_EVENT_BEGIN(EV_CIRCLE_WIDGET_UPDATE_CACHE);
        double radius = radius_->get_value(timestamp);
        double border_width = border_width_->get_value(timestamp);

        int size = static_cast<int>(std::ceill(2 * radius + 2 * border_width));
        if (!cache || size > cache_width || size > cache_height) {
            // bigger widget size require allocating bigger cache
            if (cache) {
                cairo_surface_destroy(cache);
                cache = nullptr;
            }
            cache_width = size;
            cache_height = size;
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

        if (radius > 0) {
            rgb color = color_->get_value(timestamp);
            rgb border_color = border_color_->get_value(timestamp);

            cairo_arc(cache_cr, cache_width/2.0, cache_height/2.0, radius, 0, 2 * M_PI);

            cairo_set_source_rgba(cache_cr, color.r, color.g, color.b, color.a);
            cairo_fill_preserve(cache_cr);

            if (border_width > 0) {
                cairo_set_line_width(cache_cr, border_width);
                cairo_set_source_rgba(cache_cr, border_color.r, border_color.g, border_color.b, border_color.a);
                cairo_stroke(cache_cr);
            }//border width 0 is not an error - it means no border (silently ignore negative border)

            cairo_surface_flush(cache);
            cache_drawn = true;
        } else {
            log.warning("CircleWidget radius is non-positive ({}), skipping drawing", radius);
        }

        cairo_destroy(cache_cr);
        TRACE_EVENT_END(EV_CIRCLE_WIDGET_UPDATE_CACHE);
    }

    surface.x = x - cache_width / 2.0;
    surface.y = y - cache_height / 2.0;
    surface.surface = cache;

    TRACE_EVENT_END(EV_CIRCLE_WIDGET_DRAW);
}

} // namespace overlay
} // namespace telemetry