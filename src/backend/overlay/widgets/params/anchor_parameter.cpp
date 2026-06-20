#include "anchor_parameter.h"

#include "backend/utils/string_utils.h"

namespace telemetry {
namespace overlay {

std::shared_ptr<AnchorParameter> AnchorParameter::create(
        const std::string& definition, std::shared_ptr<track::Track> track) {
    utils::logging::Logger log{"AnchorParameter::create"};
    std::string def{definition};
    trim(def);

    log.debug("Creating AnchorParameter with definition: {}", def);

    if (get_function_name(def) == "key") { // track key value
        std::string key = get_function_argstr(def);
        log.debug("Created track-key-based anchor parameter with key '{}'", key);
        return std::make_shared<AnchorParameter>(key, track);
    }

    // otherwise use string value as is
    log.debug("Created static anchor parameter from value \"{}\"", def);
    return std::make_shared<AnchorParameter>(anchor_point_from_string(def));
}

AnchorParameter::AnchorParameter(const std::string& key, std::shared_ptr<track::Track> track)
        : update_strategy_(UpdateStrategy::TrackKey),
          track_(track),
          field_id(track->get_field_id(key)) {
}

AnchorParameter::AnchorParameter(EAnchorPoint static_value)
        : update_strategy_(UpdateStrategy::Static),
          value_(static_value) {
}

bool AnchorParameter::update(time::microseconds_t timestamp) {
    switch (update_strategy_) {
        case UpdateStrategy::Static:
            return false; // static value does not change
        case UpdateStrategy::TrackKey:
            if (track_) {
                track::Value v = track_->get(field_id, timestamp);
                EAnchorPoint new_value = anchor_point_from_string(v.as_string());
                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        default:
            log.warning("Unknown update strategy in AnchorParameter");
            return false;
    }
}

EAnchorPoint AnchorParameter::get_value(time::microseconds_t timestamp) const {
    return value_;
}

} // namespace telemetry
} // namespace overlay
