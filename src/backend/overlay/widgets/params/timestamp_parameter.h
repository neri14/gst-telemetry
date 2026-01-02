#ifndef TIMESTAMP_PARAMETER_H
#define TIMESTAMP_PARAMETER_H

#include "parameter.h"
#include "string_parameter.h"
#include "numeric_parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>
#include <limits>

namespace telemetry {
namespace overlay {

class TimestampParameter : public Parameter {
public:
    static std::shared_ptr<TimestampParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);

    TimestampParameter(const std::string& key, std::shared_ptr<track::Track> track);

    ~TimestampParameter() override = default;

    void set_format_subparameter(std::shared_ptr<StringParameter> format);
    void set_precision_subparameter(std::shared_ptr<NumericParameter> precision);
    void set_timezone_subparameter(std::shared_ptr<StringParameter> timezone);

    bool update(time::microseconds_t timestamp) override;
    const std::string& get_value(time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"TimestampParameter"};

    UpdateStrategy update_strategy_;
    std::string value_ = "";

    //used by TrackKey update strategy
    std::shared_ptr<track::Track> track_ = nullptr;
    track::field_id_t field_id = track::INVALID_FIELD;

    // sub-parameters
    std::shared_ptr<StringParameter> format_ = nullptr;
    std::shared_ptr<NumericParameter> precision_ = nullptr;
    std::shared_ptr<StringParameter> timezone_ = nullptr;
};

} // namespace telemetry
} // namespace overlay

#endif // TIMESTAMP_PARAMETER_H
