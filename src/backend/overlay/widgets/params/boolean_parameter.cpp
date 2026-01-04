#include "boolean_parameter.h"
#include "backend/utils/string_utils.h"
#include <limits>
#include <cmath>

namespace telemetry {
namespace overlay {

namespace helper {
    bool interpret(const std::string& str) {
        std::string s{str};
        trim(s);
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (s.empty() || s == "false" || s == "no") {
            return false;
        }
        return true;
    }

    bool interpret(double value) {
        return std::abs(value) > std::numeric_limits<double>::epsilon();
    }
}

std::shared_ptr<BooleanParameter> BooleanParameter::create(
            const std::string& definition, std::shared_ptr<track::Track> track) {
    utils::logging::Logger log{"BooleanParameter::create"};
    std::string def{definition};
    trim(def);

    log.debug("Creating BooleanParameter with definition: {}", def);

    bool negate = false;
    if (get_function_name(def) == "not") {
        negate = true;
        def = get_function_argstr(def);
    }

    if (get_function_name(def) == "key") { // track key
        std::string key = get_function_argstr(def);
        auto param = std::make_shared<BooleanParameter>(key, track, negate);
        log.debug("Created track-key-based boolean parameter with key '{}'", key);
        return param;
    }

    if (get_function_name(def) == "exists") { // track key existance
        std::string key = get_function_argstr(def);
        auto param = std::make_shared<BooleanParameter>(key, track, negate, true);
        log.debug("Created track-key-existance-based boolean parameter with key '{}'", key);
        return param;
    }

    if (get_function_name(def) == "eval") { // expression
        auto expression_param = NumericParameter::create(def, track);
        if (expression_param) {
            auto param = std::make_shared<BooleanParameter>(expression_param, negate);
            log.debug("Created expression-based boolean parameter");
            return param;
        } else {
            log.warning("Failed to create expression from definition '{}'", def);
            return nullptr;
        }
    }

    // otherwise try to parse as static boolean value
    try {
        double dvalue = std::stod(def);
        bool val = helper::interpret(dvalue);
        if (negate) {
            val = !val;
        }
        log.debug("Created static boolean parameter ({}) from numeric definition: {}", val, definition);
        return std::make_shared<BooleanParameter>(val);
    } catch (const std::invalid_argument&) {
        //not a numeric value - continue
    }

    // finally: interpret as string
    bool val = helper::interpret(def);
    if (negate) {
        val = !val;
    }
    log.debug("Created static boolean parameter ({}) from string definition: {}", val, definition);
    return std::make_shared<BooleanParameter>(val);

    return nullptr;
}

BooleanParameter::BooleanParameter(std::shared_ptr<NumericParameter> param, bool negate)
        : update_strategy_(UpdateStrategy::SubParameter),
          negate_(negate),
          sub_param_(param) {
}

BooleanParameter::BooleanParameter(const std::string& key, std::shared_ptr<track::Track> track, bool negate, bool if_exists)
        : update_strategy_(if_exists ? UpdateStrategy::TrackKeyExistance : UpdateStrategy::TrackKey),
          negate_(negate),
          track_(track),
          field_id(track->get_field_id(key)) {
}

BooleanParameter::BooleanParameter(bool static_value)
        : update_strategy_(UpdateStrategy::Static),
          valid_(true),
          value_(static_value) {
}

bool BooleanParameter::update(time::microseconds_t timestamp) {
    switch (update_strategy_) {
        case UpdateStrategy::Static:
            return false; // static value does not change
        case UpdateStrategy::TrackKey:
            if (track_) {
                track::Value v = track_->get(field_id, timestamp);
                
                bool new_value = false;
                if (!v.is_valid()) {
                    new_value = false;
                } else if (v.is_bool()) {
                    new_value = v.as_bool();
                } else if (v.is_double()) {
                    new_value = helper::interpret(v.as_double());
                } else if (v.is_string() || v.is_time_point()) {
                    new_value = helper::interpret(v.as_string());
                } else {
                    log.debug("Track key did not yield a valid type for boolean interpretation at timestamp {}", timestamp);
                }

                if (negate_) {
                    new_value = !new_value;
                }

                bool changed = !valid_; //if wasnt valid yet - consider changed
                if (new_value != value_) {
                    value_ = new_value;
                    changed = true;
                }
                
                valid_ = true;
                return changed;
            }
            return false;
        case UpdateStrategy::TrackKeyExistance:
            if (track_) {
                bool new_value = value_;
                if (field_id == track::INVALID_FIELD) {
                    new_value = false;
                } else {
                    track::Value v = track_->get(field_id, timestamp);
                    new_value = v.is_valid();
                }

                if (negate_) {
                    new_value = !new_value;
                }

                bool changed = !valid_; //if wasnt valid yet - consider changed
                if (new_value != value_) {
                    value_ = new_value;
                    changed = true;
                }
                valid_ = true;
                return changed;
            }
            return false;
        case UpdateStrategy::SubParameter:
            if (sub_param_ && (!valid_ || sub_param_->update(timestamp))) {
                double sub_value = sub_param_->get_value(timestamp);
                bool new_value = helper::interpret(sub_value);

                if (negate_) {
                    new_value = !new_value;
                }

                bool changed = !valid_; //if wasnt valid yet - consider changed
                if (new_value != value_) {
                    value_ = new_value;
                    changed = true;
                }
                valid_ = true;
                return changed;
            }
            return false;
        default:
            log.warning("Unsupported update strategy in BooleanParameter");
            return false;
    }

    return true;
}

bool BooleanParameter::get_value(time::microseconds_t timestamp) const {
    if (!valid_) {
        log.warning("BooleanParameter value requested but parameter is not valid");
        return false;
    }
    return value_;
}


} // namespace telemetry
} // namespace overlay