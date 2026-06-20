#ifndef ANCHOR_PARAMETER_H
#define ANCHOR_PARAMETER_H

#include "parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include "backend/utils/anchor_point.h"
#include <string>
#include <limits>

namespace telemetry {
namespace overlay {

class AnchorParameter : public Parameter {
public:
    static std::shared_ptr<AnchorParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);
    
    AnchorParameter(const std::string& key, std::shared_ptr<track::Track> track);
    AnchorParameter(EAnchorPoint static_value);

    ~AnchorParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    EAnchorPoint get_value(time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"AnchorParameter"};

    UpdateStrategy update_strategy_;
    EAnchorPoint value_ = defaults::anchor_point;

    //used by TrackKey update strategy
    std::shared_ptr<track::Track> track_ = nullptr;
    track::field_id_t field_id = track::INVALID_FIELD;
};

} // namespace telemetry
} // namespace overlay

#endif // ANCHOR_PARAMETER_H
