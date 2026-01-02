#include "timestamp_widget.h"

extern "C" {
    #include <cairo.h>
    #include <pango/pangocairo.h>
}

namespace telemetry {
namespace overlay {
namespace defaults {
    const std::string format = "{}";
    const int precision = 0;
    const std::string timezone = "UTC";
} // namespace defaults


std::shared_ptr<TimestampWidget> TimestampWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"TimestampWidget::create"};
    log.info("Creating TimestampWidget");

    auto widget = std::make_shared<TimestampWidget>();

    std::shared_ptr<StringParameter> format = nullptr;
    std::shared_ptr<NumericParameter> precision = nullptr;
    std::shared_ptr<StringParameter> timezone = nullptr;

    for (const auto& [name, param] : *parameters) {
        if (name == "value") {
            widget->value_ = std::dynamic_pointer_cast<TimestampParameter>(param);
        } else if (name == "format") {
            format = std::dynamic_pointer_cast<StringParameter>(param);
        } else if (name == "precision") {
            precision = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "timezone") {
            timezone = std::dynamic_pointer_cast<StringParameter>(param);
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
    if (!precision) {
        precision = std::make_shared<NumericParameter>(defaults::precision);
        log.debug("Precision parameter not set, using default value");
    }
    if (!timezone) {
        timezone = std::make_shared<StringParameter>(defaults::timezone);
        log.debug("Timezone parameter not set, using default value");
    }

    widget->value_->set_format_subparameter(format);
    widget->value_->set_precision_subparameter(precision);
    widget->value_->set_timezone_subparameter(timezone);

    parameters->erase("value");
    parameters->erase("format");
    StringWidget::load_params(widget, parameters);

    return widget;
}

TimestampWidget::TimestampWidget()
        : StringWidget("TimestampWidget") {
}

bool TimestampWidget::update_value(time::microseconds_t timestamp) {
    return value_->update(timestamp);
}

std::string TimestampWidget::get_value(time::microseconds_t timestamp) const {
    return value_->get_value(timestamp);
}

} // namespace overlay
} // namespace telemetry