#include "container_widget.h"
#include <cmath>

namespace telemetry {
namespace overlay {


std::shared_ptr<ContainerWidget> ContainerWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"ContainerWidget::create"};
    log.info("Creating ContainerWidget");

    auto widget = std::make_shared<ContainerWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "x") {
            widget->x_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "y") {
            widget->y_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "visible") {
            widget->visible_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for ContainerWidget", name);
        }
    }

    if (!widget->x_ || !widget->y_ ) {
        log.error("Missing required parameters (x, y)");
        return nullptr;
    }

    if (!widget->visible_) {
        log.debug("Visible parameter not set, defaulting to true");
        widget->visible_ = std::make_shared<BooleanParameter>(true);
    }

    return widget;
}

ContainerWidget::ContainerWidget()
        : Widget("ContainerWidget") {
}

void ContainerWidget::draw(time::microseconds_t timestamp, cairo_t* cr,
                        double x_offset, double y_offset) {
    visible_->update(timestamp);

    if (visible_->get_value(timestamp)) {
        x_->update(timestamp);
        y_->update(timestamp);

        double x = x_offset + x_->get_value(timestamp);
        double y = y_offset + y_->get_value(timestamp);

        Widget::draw(timestamp, cr, x, y);
    } else {
        log.debug("Visibility is false, skipping drawing");
    }
}

} // namespace overlay
} // namespace telemetry