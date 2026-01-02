#include "composite_text_widget.h"

extern "C" {
    #include <cairo.h>
    #include <pango/pangocairo.h>
}

namespace telemetry {
namespace overlay {
namespace defaults {
    const std::string format = "{}";
} // namespace defaults


std::shared_ptr<CompositeTextWidget> CompositeTextWidget::create(parameter_map_ptr parameters) {
    utils::logging::Logger log{"CompositeTextWidget::create"};
    log.info("Creating CompositeTextWidget");

    auto widget = std::make_shared<CompositeTextWidget>();

    for (const auto& [name, param] : *parameters) {
        if (name == "value-1") {
            widget->value_1_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "value-2") {
            widget->value_2_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "value-3") {
            widget->value_3_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "value-4") {
            widget->value_4_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "value-5") {
            widget->value_5_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "value-6") {
            widget->value_6_ = std::dynamic_pointer_cast<NumericParameter>(param);
        } else if (name == "format") {
            widget->format_ = std::dynamic_pointer_cast<StringParameter>(param);
        }
    }

    if (!widget->value_1_) {
        log.error("Missing required parameter 'value-1'");
        return nullptr;
    }

    if (!widget->format_) {
        widget->format_ = std::make_shared<StringParameter>(defaults::format);
        log.debug("Value format parameter not set, using default value");
    }
    
    for (int i=1; i<=6; ++i) {
        parameters->erase("value-" + std::to_string(i));
    }
    parameters->erase("format");
    StringWidget::load_params(widget, parameters);

    return widget;
}

CompositeTextWidget::CompositeTextWidget()
        : StringWidget("CompositeTextWidget") {
}

bool CompositeTextWidget::update_value(time::microseconds_t timestamp) {
    bool updated = false;

    for (auto& val_param : {value_1_, value_2_, value_3_, value_4_, value_5_, value_6_}) {
        if (val_param) {
            updated |= val_param->update(timestamp);
        }
    }
    updated |= format_->update(timestamp);

    if (updated) {
        try {
            std::vector<double> values;
            for (auto& val_param : {value_1_, value_2_, value_3_, value_4_, value_5_, value_6_}) {
                if (val_param) {
                    values.push_back(val_param->get_value(timestamp));
                } else {
                    values.push_back(0.0);
                }
            }

            value_ = std::vformat(
                format_->get_value(timestamp),
                std::make_format_args(values[0], values[1], values[2],
                                      values[3], values[4], values[5]));
            valid_ = true;
            return true;
        } catch (const std::exception& e) {
            log.error("Error formatting composite text: {}", e.what());
            valid_ = false;
        }
    }
    return false;
}

std::string CompositeTextWidget::get_value(time::microseconds_t timestamp) const {
    return value_;
}

} // namespace overlay
} // namespace telemetry