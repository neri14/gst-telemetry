#include "root_widget.h"
#include <cmath>

namespace telemetry {
namespace overlay {


std::shared_ptr<RootWidget> RootWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"RootWidget::create"};
    log.info("Creating RootWidget");

    auto widget = std::make_shared<RootWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "width") {
            widget->width_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "height") {
            widget->height_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for RootWidget", name);
        }
    }

    if (!widget->width_ || !widget->height_ ) {
        log.error("Missing required parameters (width, height)");
        return nullptr;
    }

    return widget;
}

RootWidget::RootWidget()
        : Widget("RootWidget") {
}

int RootWidget::get_width(time::microseconds_t timestamp) {
    width_->update(timestamp);
    return static_cast<int>(std::round(width_->get_value(timestamp)));
}

int RootWidget::get_height(time::microseconds_t timestamp) {
    height_->update(timestamp);
    return static_cast<int>(std::round(height_->get_value(timestamp)));
}

void RootWidget::draw(time::microseconds_t timestamp, cairo_t* cr,
                        double x_offset, double y_offset) {
    Widget::draw(timestamp, cr, x_offset, y_offset);
}

} // namespace overlay
} // namespace telemetry