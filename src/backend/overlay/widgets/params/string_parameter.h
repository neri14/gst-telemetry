#ifndef STRING_PARAMETER_H
#define STRING_PARAMETER_H

#include "parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>
#include <limits>

namespace telemetry {
namespace overlay {

class StringParameter : public Parameter {
public:
    static std::shared_ptr<StringParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);
    
    StringParameter(const std::string& key, std::shared_ptr<track::Track> track);
    StringParameter(const std::string& static_value);

    ~StringParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    const std::string& get_value(time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"StringParameter"};

    UpdateStrategy update_strategy_;
    std::string value_ = "";

    //used by TrackKey update strategy
    std::shared_ptr<track::Track> track_ = nullptr;
    track::field_id_t field_id = track::INVALID_FIELD;
};

} // namespace telemetry
} // namespace overlay

#endif // STRING_PARAMETER_H
