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


    // --> class: ColorParameter
    // for color attributes (bg-color, border-color):
    //    string value         -> color name from predefined colors or "#RRGGBB" / "#RRGGBBAA" hex code
    //    "key(...)"           -> get string value from track at timestamp using key name inside parentheses (interpreted as above)
    //    "rgb(r,g,b)"         -> color from r,g,b values (0-1.0)
    //    "rgba(r,g,b,a)"      -> color from r,g,b,a values (0-1.0)
    //             each of r,g,b,a is interpreted like numeric attributes - same rules apply
    //             resulting value is clamped to 0.0-1.0 range
