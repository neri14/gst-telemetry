#include "conditional_widget.h"
#include <cmath>

namespace telemetry {
namespace overlay {


std::shared_ptr<ConditionalWidget> ConditionalWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"ConditionalWidget::create"};
    log.info("Creating ConditionalWidget");

    auto widget = std::make_shared<ConditionalWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "condition") {
            widget->condition_ = std::dynamic_pointer_cast<BooleanParameter>(param);
        } else {
            log.warning("Unknown parameter '{}' for ConditionalWidget", name);
        }
    }
    
    if (!widget->condition_) {
        log.error("Missing required parameter 'condition'");
        return nullptr;
    }

    return widget;
}

ConditionalWidget::ConditionalWidget()
        : Widget("ConditionalWidget") {
}

void ConditionalWidget::draw(time::microseconds_t timestamp, cairo_t* cr,
                        double x_offset, double y_offset) {
    condition_->update(timestamp);

    if (condition_->get_value(timestamp)) {
        Widget::draw(timestamp, cr, x_offset, y_offset);
    } else {
        log.debug("Condition is false, skipping drawing");
    }
}

} // namespace overlay
} // namespace telemetry