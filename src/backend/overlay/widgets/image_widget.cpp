#include "image_widget.h"
#include "trace/trace.h"
#include <cmath>

extern "C" {
    #include <cairo.h>
}


namespace telemetry {
namespace overlay {
namespace defaults {
    const double scale = 1.0;
} // namespace defaults

std::shared_ptr<ImageWidget> ImageWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"ImageWidget::create"};
    log.info("Creating ImageWidget");

    auto widget = std::make_shared<ImageWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "path") {
            widget->path_ = std::dynamic_pointer_cast<StringParameter>(param);
        } else if (name == "x") {
            widget->x_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y") {
            widget->y_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "scale") {
            widget->scale_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for ImageWidget", name);
        }
    }

    if (!widget->x_ || !widget->y_ || !widget->path_) {
        log.error("Missing required parameters (x, y, path)");
        return nullptr;
    }

    if (!widget->scale_) {
        log.debug("Scale parameter not set, using default value");
        widget->scale_ = std::make_shared<NumericParameter>(defaults::scale);
    }
    if (!widget->visible_) {
        log.debug("Visible parameter not set, defaulting to true");
        widget->visible_ = std::make_shared<BooleanParameter>(true);
    }

    return widget;
}

ImageWidget::ImageWidget()
        : Widget("ImageWidget") {
}

void ImageWidget::draw(time::microseconds_t timestamp,
                       schedule_drawing_cb_t schedule_drawing_cb,
                       double x_offset, double y_offset) {
    visible_->update(timestamp);

    if (visible_->get_value(timestamp)) {
        x_->update(timestamp);
        y_->update(timestamp);

        double x = x_offset + x_->get_value(timestamp);
        double y = y_offset + y_->get_value(timestamp);

        schedule_drawing_cb([this, timestamp, x, y](Surface& surface) {
            this->draw_impl(surface, timestamp, x, y);
        });

        // draw childern relative to top left corner of image
        // (only if image is visible)
        Widget::draw(timestamp, schedule_drawing_cb, x, y);
    } else {
        log.debug("Visibility is false, skipping drawing");
    }
}

void ImageWidget::draw_impl(Surface& surface, time::microseconds_t timestamp, double x, double y) {
    TRACE_EVENT_BEGIN(EV_IMAGE_WIDGET_DRAW);

    bool cache_update_needed = !cache_drawn;
    for (auto& param : std::vector<parameter_ptr_t>{x_, y_, scale_}) {
        if (param->update(timestamp)) {
            cache_update_needed = true;
        }
    }

    if (cache_update_needed) {
        TRACE_EVENT_BEGIN(EV_IMAGE_WIDGET_UPDATE_CACHE);

        double width = 100; //TODO get image width * scale
        double height = 100; //TODO get image height * scale

        if (!cache || width > cache_width || height > cache_height) {
            // bigger image size require allocating bigger cache
            if (cache) {
                cairo_surface_destroy(cache);
                cache = nullptr;
            }
            cache_width = static_cast<int>(std::ceil(width));
            cache_height = static_cast<int>(std::ceil(height));
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

        //TODO draw image to cache_cr

        cairo_surface_flush(cache);
        cache_drawn = true;

        cairo_destroy(cache_cr);

        TRACE_EVENT_END(EV_IMAGE_WIDGET_UPDATE_CACHE);
    }

    surface.x = x;
    surface.y = y;
    surface.surface = cache;

    TRACE_EVENT_END(EV_IMAGE_WIDGET_DRAW);
}

} // namespace overlay
} // namespace telemetry