#ifndef BOOLEAN_PARAMETER_H
#define BOOLEAN_PARAMETER_H

#include "parameter.h"
#include "numeric_parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>

namespace telemetry {
namespace overlay {

class BooleanParameter : public Parameter {
public:
    static std::shared_ptr<BooleanParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);

    BooleanParameter(std::shared_ptr<NumericParameter> param, bool negate = false);
    BooleanParameter(const std::string& key, std::shared_ptr<track::Track> track,
                     bool negate = false, bool if_exists=false);
    BooleanParameter(bool static_value);
    ~BooleanParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    bool get_value(time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"BooleanParameter"};

    UpdateStrategy update_strategy_;
    bool valid_ = false;
    bool value_ = false;

    // used by TrackKey, TrackKeyExistance and Expression update strategy
    bool negate_ = false;

    // used by TrackKey and TrackKeyExistance update strategy
    std::shared_ptr<track::Track> track_ = nullptr;
    track::field_id_t field_id = track::INVALID_FIELD;

    // used by SubParameter update strategy
    std::shared_ptr<NumericParameter> sub_param_ = nullptr;
};

} // namespace telemetry
} // namespace overlay

#endif // BOOLEAN_PARAMETER_H
