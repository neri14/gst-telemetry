#include "formatted_parameter.h"

#include "backend/utils/string_utils.h"
#include <format>

namespace telemetry {
namespace overlay {

std::shared_ptr<FormattedParameter> FormattedParameter::create(
        const std::string& definition, std::shared_ptr<track::Track> track) {
    utils::logging::Logger log{"FormattedParameter::create"};
    std::string def{definition};
    trim(def);

    log.debug("Creating FormattedParameter with definition: {}", def);

    if (get_function_name(def) == "eval") { // expression
        auto expression_str = get_function_argstr(def);
        auto expression = std::make_shared<Expression>(expression_str, track);
        if (expression) {
            log.debug("Created expression-based formatted parameter");
            return std::make_shared<FormattedParameter>(expression);
        } else {
            log.warning("Failed to create expression from definition '{}'", def);
            return nullptr;
        }
    }

    if (get_function_name(def) == "key") { // track key value
        std::string key = get_function_argstr(def);
        log.debug("Created track-key-based formatted parameter with key '{}'", key);
        return std::make_shared<FormattedParameter>(key, track);
    }

    // otherwise use string value as is
    log.debug("Created static formatted parameter with value \"{}\"", definition);
    return std::make_shared<FormattedParameter>(definition);
}

FormattedParameter::FormattedParameter(std::shared_ptr<Expression> expression)
        : update_strategy_(UpdateStrategy::Expression),
          expression_(expression) {
}

FormattedParameter::FormattedParameter(const std::string& key, std::shared_ptr<track::Track> track)
        : update_strategy_(UpdateStrategy::TrackKey),
          track_(track),
          field_id(track->get_field_id(key)) {
}

FormattedParameter::FormattedParameter(const std::string& static_value)
        : update_strategy_(UpdateStrategy::Static),
          value_(static_value) {
}

void FormattedParameter::set_format_subparameter(std::shared_ptr<StringParameter> format) {
    format_ = format;
}

bool FormattedParameter::update(time::microseconds_t timestamp) {
    switch (update_strategy_) {
        case UpdateStrategy::Static:
            return false; // static value does not change
        case UpdateStrategy::Expression:
            if (expression_) {
                double expr_result = expression_->evaluate(timestamp);
                std::string new_value = "";

                if (format_) {
                    format_->update(timestamp);
                    new_value = std::vformat(format_->get_value(timestamp), std::make_format_args(expr_result));
                } else {
                    new_value = std::vformat("{}", std::make_format_args(expr_result));
                }

                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        case UpdateStrategy::TrackKey:
            if (track_) {
                track::Value v = track_->get(field_id, timestamp);
                std::string new_value = "";

                if (format_) {
                    format_->update(timestamp);
                    new_value = v.as_string(format_->get_value(timestamp));
                } else {
                    new_value = v.as_string();
                }

                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        default:
            log.warning("Unknown update strategy in FormattedParameter");
            return false;
    }
}

const std::string& FormattedParameter::get_value(time::microseconds_t timestamp) const {
    return value_;
}

} // namespace telemetry
} // namespace overlay
