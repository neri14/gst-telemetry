#include "boolean_parameter.h"

namespace telemetry {
namespace overlay {


BooleanParameter::BooleanParameter(const std::string& definition, std::shared_ptr<track::Track> track)
    : definition_(definition), track_(track) {
}

BooleanParameter::BooleanParameter(bool static_value)
    : static_value_(static_value) {
}

bool BooleanParameter::update(time::microseconds_t timestamp) {
    //FIXME to be implemented
    return true;
}

bool BooleanParameter::get_value(time::microseconds_t timestamp) const {
    // FIXME to be implemented
    return static_value_;
}


} // namespace telemetry
} // namespace overlay