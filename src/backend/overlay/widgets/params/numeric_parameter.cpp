#include "numeric_parameter.h"

namespace telemetry {
namespace overlay {


NumericParameter::NumericParameter(const std::string& definition, std::shared_ptr<track::Track> track)
    : definition_(definition), track_(track) {
}

NumericParameter::NumericParameter(double static_value)
    : static_value_(static_value) {
}

bool NumericParameter::update(time::microseconds_t timestamp) {
    //FIXME to be implemented
    return true;
}

double NumericParameter::get_value(time::microseconds_t timestamp) const {
    // FIXME to be implemented
    return std::stod(definition_);
}


} // namespace telemetry
} // namespace overlay