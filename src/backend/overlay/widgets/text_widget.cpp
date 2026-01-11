#include "text_widget.h"

extern "C" {
    #include <cairo.h>
    #include <pango/pangocairo.h>
}

namespace telemetry {
namespace overlay {
namespace defaults {
    const std::string format = "{}";
} // namespace defaults


std::shared_ptr<TextWidget> TextWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"TextWidget::create"};
    log.info("Creating TextWidget");

    auto widget = std::make_shared<TextWidget>();

    std::shared_ptr<StringParameter> format = nullptr;

    for (const auto& [name, param] : *parameters) {
        if (name == "value") {
            widget->value_ = std::dynamic_pointer_cast<FormattedParameter>(param);
        } else if (name == "format") {
            format = std::dynamic_pointer_cast<StringParameter>(param);
        } else if (name == "override-time") {
            auto num_param = std::dynamic_pointer_cast<NumericParameter>(param);
            if (num_param) {
                widget->override_time_set = true;
                widget->override_time_ = num_param->get_value(0);
            } else {
                log.error("Invalid parameter type for 'override-time', expected NumericParameter");
            }
        }
    }

    if (!widget->value_) {
        log.error("Missing required parameter 'value'");
        return nullptr;
    }

    if (!format) {
        format = std::make_shared<StringParameter>(defaults::format);
        log.debug("Value format parameter not set, using default value");
    }
    widget->value_->set_format_subparameter(format);

    parameters->erase("value");
    parameters->erase("format");
    parameters->erase("override-time");
    StringWidget::load_params(widget, parameters);

    return widget;
}

TextWidget::TextWidget()
        : StringWidget("TextWidget") {
}

bool TextWidget::update_value(time::microseconds_t timestamp) {
    if (override_time_set) {
        timestamp = static_cast<time::microseconds_t>(override_time_ * 1'000'000);
    }
    return value_->update(timestamp);
}

std::string TextWidget::get_value(time::microseconds_t timestamp) const {
    if (override_time_set) {
        timestamp = static_cast<time::microseconds_t>(override_time_ * 1'000'000);
    }
    return value_->get_value(timestamp);
}

} // namespace overlay
} // namespace telemetry