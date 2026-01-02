#ifndef COLOR_PARAMETER_H
#define COLOR_PARAMETER_H

#include "parameter.h"
#include "numeric_parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include "backend/utils/color.h"
#include <string>

namespace telemetry {
namespace overlay {

class ColorParameter : public Parameter {
public:
    static std::shared_ptr<ColorParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);

    ColorParameter(std::shared_ptr<NumericParameter> r_param,
                   std::shared_ptr<NumericParameter> g_param,
                   std::shared_ptr<NumericParameter> b_param,
                   std::shared_ptr<NumericParameter> a_param);
    ColorParameter(const std::string& key, std::shared_ptr<track::Track> track);
    ColorParameter(rgb static_value);

    ~ColorParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    rgb get_value(time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"ColorParameter"};

    UpdateStrategy update_strategy_;
    rgb value_ = color::invalid;

    // used by TrackKey update strategy
    std::shared_ptr<track::Track> track_ = nullptr;
    track::field_id_t field_id = track::INVALID_FIELD;

    // used by SubParameter update strategy
    std::shared_ptr<NumericParameter> r_param_ = nullptr;
    std::shared_ptr<NumericParameter> g_param_ = nullptr;
    std::shared_ptr<NumericParameter> b_param_ = nullptr;
    std::shared_ptr<NumericParameter> a_param_ = nullptr;
};

} // namespace telemetry
} // namespace overlay

#endif // COLOR_PARAMETER_H


