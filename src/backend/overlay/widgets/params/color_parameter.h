#ifndef COLOR_PARAMETER_H
#define COLOR_PARAMETER_H

#include "parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include "backend/utils/color.h"
#include <string>

namespace telemetry {
namespace overlay {

class ColorParameter : public Parameter {
public:
    ColorParameter(const std::string& definition, std::shared_ptr<track::Track> track);
    ColorParameter(rgb static_value);
    ~ColorParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    rgb get_value(time::microseconds_t timestamp) const;

private:
    std::string definition_;
    std::shared_ptr<track::Track> track_;
    rgb static_value_ = color::black;
};

} // namespace telemetry
} // namespace overlay

#endif // COLOR_PARAMETER_H
