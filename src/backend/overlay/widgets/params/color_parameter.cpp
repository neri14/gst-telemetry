#include "color_parameter.h"

namespace telemetry {
namespace overlay {


ColorParameter::ColorParameter(const std::string& definition, std::shared_ptr<track::Track> track)
    : definition_(definition), track_(track) {
}

ColorParameter::ColorParameter(rgb static_value)
    : static_value_(static_value) {
}

bool ColorParameter::update(time::microseconds_t timestamp) {
    //FIXME to be implemented
    return true;
}

rgb ColorParameter::get_value(time::microseconds_t timestamp) const {
    // FIXME to be implemented
    return color_from_string(definition_);
}


} // namespace telemetry
} // namespace overlay