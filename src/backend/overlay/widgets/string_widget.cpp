#include "string_widget.h"
#include "backend/utils/color.h"
#include "backend/utils/text_align.h"
#include "text_render.h"
#include "trace/trace.h"
#include <cmath>

extern "C" {
    #include <cairo.h>
}

namespace telemetry {
namespace overlay {
namespace defaults {
    const std::string format = "{}";
    const rgb color = color::white;
    const int border_width = 0; // if border not defined - no border
    const rgb border_color = color::black;
    const std::string font = "Arial";
    const int font_size = 12;
    const ETextAlign alignment = ETextAlign::Left;
} // namespace defaults

bool StringWidget::load_params(std::shared_ptr<StringWidget> widget, parameter_map_ptr parameters) {
    utils::logging::Logger log{"StringWidget::load_params"};
    log.info("Loading common StringWidget params");

    for (const auto& [name, param] : *parameters) {
        if (name == "x") {
            widget->x_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y") {
            widget->y_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "font-name") {
            widget->font_name_ = std::dynamic_pointer_cast<StringParameter>(param);
        } else if (name == "font-size") {
            widget->font_size_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "align") {
            widget->align_ = std::dynamic_pointer_cast<AlignmentParameter>(param);
        }  else if (name == "color") {
            widget->color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "border-width") {
            widget->border_width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "border-color") {
            widget->border_color_ = std::dynamic_pointer_cast<ColorParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for StringWidget", name);
        }
    }

    if (!widget->x_ || !widget->y_ ) {
        log.error("Missing required parameters (x, y, value)");
        return false;
    }

    if (!widget->font_name_) {
        log.debug("Font name parameter not set, using default value");
        widget->font_name_ = std::make_shared<StringParameter>(defaults::font);
    }
    if (!widget->font_size_) {
        log.debug("Font size parameter not set, using default value");
        widget->font_size_ = std::make_shared<NumericParameter>(defaults::font_size);
    }
    if (!widget->align_) {
        log.debug("Align parameter not set, using default value");
        widget->align_ = std::make_shared<AlignmentParameter>(defaults::alignment);
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

    return true;
}

StringWidget::StringWidget(const std::string& name)
        : Widget(name) {
}

void StringWidget::draw(time::microseconds_t timestamp,
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

        // draw childern relative to text anchor point (only if text visible)
        Widget::draw(timestamp, schedule_drawing_cb, x, y);
    } else {
        log.debug("Visibility is false, skipping drawing");
    }
}

void StringWidget::draw_impl(Surface& surface, time::microseconds_t timestamp, double x, double y) {
    TRACE_EVENT_BEGIN(EV_STRING_WIDGET_DRAW);

    bool cache_update_needed = !cache_drawn;
    // since we recalculate the params only if widget is visible
    // change to params that impact cache will be detected here
    // if they changed while widget was not visible
    for (auto& param : std::vector<parameter_ptr_t>{font_name_, font_size_, align_, color_, border_width_, border_color_}) {
        if (param->update(timestamp)) {
            cache_update_needed = true;
        }
    }

    if (update_value(timestamp)) {
        cache_update_needed = true;
    }

    int margin = static_cast<int>(border_width_->get_value(timestamp) + font_size_->get_value(timestamp) * 0.5);

    if (cache_update_needed) {
        TRACE_EVENT_BEGIN(EV_STRING_WIDGET_UPDATE_CACHE);

        std::string text = get_value(timestamp);
        int font_size = static_cast<int>(std::round(font_size_->get_value(timestamp)));

        int chars = static_cast<int>(text.length());
        int line_breaks = std::count(text.begin(), text.end(), '\n') + std::count(text.begin(), text.end(), '\r');

        //naive overestimation of surface size needed
        int width = chars * font_size + 2 * margin;
        int height = 2 * (line_breaks + 1) * font_size + 2 * margin;

        if (!cache || width > cache_width || height > cache_height) {
            // bigger widget size require allocating bigger cache
            if (cache) {
                cairo_surface_destroy(cache);
                cache = nullptr;
            }
            cache_width = width;
            cache_height = height;
            cache = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cache_width, cache_height);
            cache_drawn = false;
            log.info("Allocated new StringWidget cache surface: {}x{}", cache_width, cache_height);
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

        draw_text(cache_cr, cache_width, cache_height, margin, text,
                    std::format("{} {}", font_name_->get_value(timestamp), font_size),
                    align_->get_value(timestamp),
                    color_->get_value(timestamp),
                    border_width_->get_value(timestamp),
                    border_color_->get_value(timestamp));
        cairo_destroy(cache_cr);
        cache_drawn = true;

        TRACE_EVENT_END(EV_STRING_WIDGET_UPDATE_CACHE);
    }

    double draw_x = x;
    double draw_y = y - margin;

    switch (align_->get_value(timestamp)) {
        case ETextAlign::Left:
            draw_x -= margin;
            break;
        case ETextAlign::Center:
            draw_x -= cache_width / 2.0;
            break;
        case ETextAlign::Right:
            draw_x -= cache_width - margin;
            break;
    }

    surface.x = draw_x;
    surface.y = draw_y;
    surface.surface = cache;

    TRACE_EVENT_END(EV_STRING_WIDGET_DRAW);
}

} // namespace overlay
} // namespace telemetry