#include "alignment_parameter.h"

#include "backend/utils/string_utils.h"

namespace telemetry {
namespace overlay {

std::shared_ptr<AlignmentParameter> AlignmentParameter::create(
        const std::string& definition, std::shared_ptr<track::Track> track) {
    utils::logging::Logger log{"AlignmentParameter::create"};
    std::string def{definition};
    trim(def);

    log.debug("Creating AlignmentParameter with definition: {}", def);

    if (get_function_name(def) == "key") { // track key value
        std::string key = get_function_argstr(def);
        log.debug("Created track-key-based alignment parameter with key '{}'", key);
        return std::make_shared<AlignmentParameter>(key, track);
    }

    // otherwise use string value as is
    log.debug("Created static alignment parameter from value \"{}\"", def);
    return std::make_shared<AlignmentParameter>(text_align_from_string(def));
}

AlignmentParameter::AlignmentParameter(const std::string& key, std::shared_ptr<track::Track> track)
        : update_strategy_(UpdateStrategy::TrackKey),
          track_(track),
          field_id(track->get_field_id(key)) {
}

AlignmentParameter::AlignmentParameter(ETextAlign static_value)
        : update_strategy_(UpdateStrategy::Static),
          value_(static_value) {
}

bool AlignmentParameter::update(time::microseconds_t timestamp) {
    switch (update_strategy_) {
        case UpdateStrategy::Static:
            return false; // static value does not change
        case UpdateStrategy::TrackKey:
            if (track_) {
                track::Value v = track_->get(field_id, timestamp);
                ETextAlign new_value = text_align_from_string(v.as_string());
                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        default:
            log.warning("Unknown update strategy in AlignmentParameter");
            return false;
    }
}

ETextAlign AlignmentParameter::get_value(time::microseconds_t timestamp) const {
    return value_;
}

} // namespace telemetry
} // namespace overlay