#ifndef ALIGNMENT_PARAMETER_H
#define ALIGNMENT_PARAMETER_H

#include "parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include "backend/utils/text_align.h"
#include <string>
#include <limits>

namespace telemetry {
namespace overlay {

class AlignmentParameter : public Parameter {
public:
    static std::shared_ptr<AlignmentParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);
    
    AlignmentParameter(const std::string& key, std::shared_ptr<track::Track> track);
    AlignmentParameter(ETextAlign static_value);

    ~AlignmentParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    ETextAlign get_value(time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"AlignmentParameter"};

    UpdateStrategy update_strategy_;
    ETextAlign value_ = ETextAlign::Left;

    //used by TrackKey update strategy
    std::shared_ptr<track::Track> track_ = nullptr;
    track::field_id_t field_id = track::INVALID_FIELD;
};

} // namespace telemetry
} // namespace overlay

#endif // ALIGNMENT_PARAMETER_H
