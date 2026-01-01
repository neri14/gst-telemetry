#include "numeric_parameter.h"

#include "backend/utils/string_utils.h"

namespace telemetry {
namespace overlay {

std::shared_ptr<NumericParameter> NumericParameter::create(
        const std::string& definition, std::shared_ptr<track::Track> track) {
    utils::logging::Logger log{"NumericParameter::create"};
    std::string def{definition};
    trim(def);

    log.debug("Creating NumericParameter with definition: {}", def);

    // if begins with "eval(" and ends with ")" - expression
    if (def.rfind("eval(", 0) == 0 && def.back() == ')') {
        auto expression = std::make_shared<Expression>(def.substr(5, def.size() - 6), track);
        if (expression) {
            log.debug("Created expression-based numeric parameter");
            return std::make_shared<NumericParameter>(expression);
        } else {
            log.warning("Failed to create expression from definition '{}'", def);
            return nullptr;
        }
    }

    // if begins with "key(" and ends with ")" - track key
    if (def.rfind("key(", 0) == 0 && def.back() == ')') {
        std::string key = def.substr(4, def.size() - 5);
        log.debug("Created track-key-based numeric parameter with key '{}'", key);
        return std::make_shared<NumericParameter>(key, track);
    }

    // otherwise try to parse as static double value
    try {
        double static_value = std::stod(def);
        log.debug("Created static numeric parameter with value {}", static_value);
        return std::make_shared<NumericParameter>(static_value);
    } catch (const std::invalid_argument&) {
        log.warning("Unparsable parameter definition '{}'", def);
        return nullptr;
    }
}

NumericParameter::NumericParameter(std::shared_ptr<Expression> expression)
        : update_strategy_(UpdateStrategy::Expression),
          expression_(expression) {
}

NumericParameter::NumericParameter(const std::string& key, std::shared_ptr<track::Track> track)
        : update_strategy_(UpdateStrategy::TrackKey),
          track_(track),
          field_id(track->get_field_id(key)) {
}

NumericParameter::NumericParameter(double static_value)
        : update_strategy_(UpdateStrategy::Static),
          value_(static_value) {
}

bool NumericParameter::update(time::microseconds_t timestamp) {
    switch (update_strategy_) {
        case UpdateStrategy::Static:
            return false; // static value does not change
        case UpdateStrategy::Expression:
            if (expression_) {
                double new_value = expression_->evaluate(timestamp);
                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        case UpdateStrategy::TrackKey:
            if (track_) {
                track::Value v = track_->get(field_id, timestamp);
                double new_value = v.as_double();
                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        default:
            log.warning("Unknown update strategy in NumericParameter");
            return false;
    }
}

double NumericParameter::get_value(time::microseconds_t timestamp) const {
    if (std::isnan(value_)) {
        log.warning("NumericParameter has NaN value at timestamp {}, defaulting to 0.0", timestamp);
        return 0.0;
    }
    return value_;
}

} // namespace telemetry
} // namespace overlay