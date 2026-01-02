#include "timestamp_parameter.h"

#include "backend/utils/string_utils.h"
#include <format>

namespace telemetry {
namespace overlay {

template <typename Duration>
std::string format_timestamp_cast(
                time::time_point_t tp,
                const std::string& format,
                const std::string& timezone) {
    auto tpcast = std::chrono::time_point_cast<Duration>(tp);
    auto tpzoned = std::chrono::zoned_time{timezone, tpcast};
    return std::vformat(format, std::make_format_args(tpzoned));
}

std::string format_timestamp(
                time::time_point_t tp,
                const std::string& format,
                int precision,
                const std::string& timezone) {
    if (precision <= 0) {
        return format_timestamp_cast<std::chrono::seconds>(tp, format, timezone);
    } else if (precision <= 3) {
        return format_timestamp_cast<std::chrono::milliseconds>(tp, format, timezone);
    } else {
        return format_timestamp_cast<std::chrono::microseconds>(tp, format, timezone);
    }
}

std::shared_ptr<TimestampParameter> TimestampParameter::create(
        const std::string& definition, std::shared_ptr<track::Track> track) {
    utils::logging::Logger log{"TimestampParameter::create"};
    std::string def{definition};
    trim(def);

    log.debug("Creating TimestampParameter with definition: {}", def);

    if (get_function_name(def) == "key") { // track key value
        std::string key = get_function_argstr(def);
        log.debug("Created track-key-based timestamp parameter with key '{}'", key);
        return std::make_shared<TimestampParameter>(key, track);
    }

    log.error("Invalid definition '{}' for TimestampParameter", def);
    return nullptr;
}

TimestampParameter::TimestampParameter(const std::string& key, std::shared_ptr<track::Track> track)
        : update_strategy_(UpdateStrategy::TrackKey),
          track_(track),
          field_id(track->get_field_id(key)) {
}

void TimestampParameter::set_format_subparameter(std::shared_ptr<StringParameter> format) {
    format_ = format;
}

void TimestampParameter::set_precision_subparameter(std::shared_ptr<NumericParameter> precision) {
    precision_ = precision;
}

void TimestampParameter::set_timezone_subparameter(std::shared_ptr<StringParameter> timezone) {
    timezone_ = timezone;
}

bool TimestampParameter::update(time::microseconds_t timestamp) {
    switch (update_strategy_) {
        case UpdateStrategy::TrackKey:
            if (track_) {
                track::Value v = track_->get(field_id, timestamp);
                if (!v.is_time_point()) {
                    log.warning("TimestampParameter: value for field_id {} at timestamp {} is not a time_point", field_id, timestamp);
                    return false;
                }

                format_->update(timestamp);
                precision_->update(timestamp);
                timezone_->update(timestamp);

                std::string new_value = format_timestamp(
                    v.as_time_point(),
                    format_->get_value(timestamp),
                    static_cast<int>(precision_->get_value(timestamp)),
                    timezone_->get_value(timestamp));

                if (new_value != value_) {
                    value_ = new_value;
                    return true;
                }
            }
            return false;
        default:
            log.warning("Unknown update strategy in TimestampParameter");
            return false;
    }
}

const std::string& TimestampParameter::get_value(time::microseconds_t timestamp) const {
    return value_;
}

} // namespace telemetry
} // namespace overlay
