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

    if (get_function_name(def) == "eval") { // expression
        auto expression_str = get_function_argstr(def);
        auto expression = std::make_shared<Expression>(expression_str, track);
        if (expression) {
            log.debug("Created expression-based numeric parameter");
            return std::make_shared<NumericParameter>(expression, track);
        } else {
            log.warning("Failed to create expression from definition '{}'", def);
            return nullptr;
        }
    }

    if (get_function_name(def) == "key") { // track key value
        std::string key = get_function_argstr(def);
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

NumericParameter::NumericParameter(std::shared_ptr<Expression> expression, std::shared_ptr<track::Track> track)
        : update_strategy_(UpdateStrategy::Expression),
          track_(track),  
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

double NumericParameter::get_value(time::microseconds_t timestamp, bool allow_nan) const {
    if (std::isnan(value_) && !allow_nan) {
        log.warning("NumericParameter has NaN value at timestamp {}, defaulting to 0.0", timestamp);
        return 0.0;
    }
    return value_;
}

std::shared_ptr<NumericParameter::sections_t> NumericParameter::get_values(
            time::microseconds_t step, double min_value, double max_value) {
    std::shared_ptr<sections_t> sections = std::make_shared<sections_t>();

    value_map_t* values = nullptr;
    if (step == time::INVALID_TIME) {
        // get filtered sections at available timestamps
        for (auto ts : track_->get_trackpoint_timestamps()) {
            update(ts);
            if (value_ >= min_value && value_ <= max_value) {
                if (!values) {
                    sections->emplace_back();
                    values = &sections->back();
                }
                (*values)[ts] = value_;
            } else {
                values = nullptr; // break section
            }
        }
    } else {
        // get values at specified intervals
        time::microseconds_t from = track_->get_trackpoint_timestamps().front();
        time::microseconds_t to = track_->get_trackpoint_timestamps().back();

        for (time::microseconds_t ts = from; ts <= to; ts += step) {
            update(ts);
            if (value_ >= min_value && value_ <= max_value) {
                if (!values) {
                    sections->emplace_back();
                    values = &sections->back();
                }
                (*values)[ts] = value_;
            } else {
                values = nullptr; // break section
            }
        }
    }
    value_ = std::numeric_limits<double>::quiet_NaN(); // reset to NaN after values retrieval
    return sections;
}


std::shared_ptr<NumericParameter::sections_t> NumericParameter::get_values(
            std::vector<std::vector<time::microseconds_t>> timestamp_sections) {
    std::shared_ptr<sections_t> sections = std::make_shared<sections_t>();

    for (auto tsec : timestamp_sections) {
        sections->emplace_back();
        value_map_t* values = &sections->back();

        for (auto ts : tsec) {
            update(ts);
            (*values)[ts] = value_;
        }
    }

    value_ = std::numeric_limits<double>::quiet_NaN(); // reset to NaN after values retrieval
    return sections;
}

std::shared_ptr<NumericParameter::value_map_t> NumericParameter::get_all_values() {
    std::shared_ptr<value_map_t> values = std::make_shared<std::map<time::microseconds_t, double>>();

    for (auto ts : track_->get_trackpoint_timestamps()) {
        update(ts);
        (*values)[ts] = value_;
    }
    value_ = std::numeric_limits<double>::quiet_NaN(); // reset to NaN after values retrieval
    return values;
}

bool NumericParameter::is_static() const {
    return update_strategy_ == UpdateStrategy::Static;
}

} // namespace telemetry
} // namespace overlay