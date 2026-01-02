#include "string_parameter.h"

#include "backend/utils/string_utils.h"

namespace telemetry {
namespace overlay {

std::shared_ptr<StringParameter> StringParameter::create(
        const std::string& definition, std::shared_ptr<track::Track> track) {
    utils::logging::Logger log{"StringParameter::create"};
    std::string def{definition};
    trim(def);

    log.debug("Creating StringParameter with definition: {}", def);

    if (get_function_name(def) == "key") { // track key value
        std::string key = get_function_argstr(def);
        log.debug("Created track-key-based string parameter with key '{}'", key);
        return std::make_shared<StringParameter>(key, track);
    }

    // otherwise use string value as is
    log.debug("Created static string parameter with value \"{}\"", definition);
    return std::make_shared<StringParameter>(definition);
}

StringParameter::StringParameter(const std::string& key, std::shared_ptr<track::Track> track)
        : update_strategy_(UpdateStrategy::TrackKey),
          track_(track),
          field_id(track->get_field_id(key)) {
}

StringParameter::StringParameter(const std::string& static_value)
        : update_strategy_(UpdateStrategy::Static),
          value_(static_value) {
}

bool StringParameter::update(time::microseconds_t timestamp) {
    switch (update_strategy_) {
        case UpdateStrategy::Static:
            return false; // static value does not change
        case UpdateStrategy::TrackKey:
            if (track_) {
                track::Value v = track_->get(field_id, timestamp);
                std::string new_value = v.as_string();
                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        default:
            log.warning("Unknown update strategy in StringParameter");
            return false;
    }
}

const std::string& StringParameter::get_value(time::microseconds_t timestamp) const {
    return value_;
}

} // namespace telemetry
} // namespace overlay