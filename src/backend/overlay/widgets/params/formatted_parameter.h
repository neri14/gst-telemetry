#ifndef FORMATTED_PARAMETER_H
#define FORMATTED_PARAMETER_H

#include "parameter.h"
#include "expression.h"
#include "string_parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>
#include <limits>

namespace telemetry {
namespace overlay {

class FormattedParameter : public Parameter {
public:
    static std::shared_ptr<FormattedParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);
    
    FormattedParameter(std::shared_ptr<Expression> expression);
    FormattedParameter(const std::string& key, std::shared_ptr<track::Track> track);
    FormattedParameter(const std::string& static_value);

    ~FormattedParameter() override = default;

    void set_format_subparameter(std::shared_ptr<StringParameter> format);

    bool update(time::microseconds_t timestamp) override;
    const std::string& get_value(time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"FormattedParameter"};

    UpdateStrategy update_strategy_;
    std::string value_ = "";

    //used by TrackKey update strategy
    std::shared_ptr<track::Track> track_ = nullptr;
    track::field_id_t field_id = track::INVALID_FIELD;

    //used by Expression update strategy
    std::shared_ptr<Expression> expression_ = nullptr;

    // format sub-parameter - used for TrackKey and Expression update strategies
    std::shared_ptr<StringParameter> format_ = nullptr;
};

} // namespace telemetry
} // namespace overlay

#endif // FORMATTED_PARAMETER_H
