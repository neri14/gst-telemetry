#include "numeric_parameter.h"

namespace telemetry {
namespace overlay {

std::shared_ptr<NumericParameter> NumericParameter::create(
        const std::string& definition, std::shared_ptr<track::Track> track) {
    utils::logging::Logger log{"NumericParameter::create"};
    log.debug("Creating NumericParameter with definition: {}", definition);

    // if begins with "eval(" and ends with ")" - expression
    if (definition.rfind("eval(", 0) == 0 && definition.back() == ')') {
        //TODO to be implemented
        log.warning("Expression-based parameters not yet implemented");
        return nullptr;
    }

    // if begins with "key(" and ends with ")" - track key
    if (definition.rfind("key(", 0) == 0 && definition.back() == ')') {
        std::string key = definition.substr(4, definition.size() - 5);
        log.debug("Created track-key-based numeric parameter with key '{}'", key);
        return std::make_shared<NumericParameter>(key, track);
    }

    try {
        double static_value = std::stod(definition);
        log.debug("Created static numeric parameter with value {}", static_value);
        return std::make_shared<NumericParameter>(static_value);
    } catch (const std::invalid_argument&) {
        log.warning("Unparsable parameter definition '{}'", definition);
        return nullptr;
    }
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
            //TODO to be implemented - return true only if value_ changed
            return true;
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
            return false;
    }
}

double NumericParameter::get_value(time::microseconds_t timestamp) const {
    return value_;
}

    // --> class: NumericParameter
    // for numeric attributes (x, y, radius, border-width):
    //    numeric value        -> use as is
    //    "eval(...)"          -> evaluate string inside parentheses as expression
    //    "key(...)"           -> get value from track at timestamp using key name inside parentheses
    //                            (note it can be achieved by eval(...) too, but this has less overhead)

} // namespace telemetry
} // namespace overlay