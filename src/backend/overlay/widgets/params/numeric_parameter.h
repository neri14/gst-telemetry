#ifndef NUMERIC_PARAMETER_H
#define NUMERIC_PARAMETER_H

#include "parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>

namespace telemetry {
namespace overlay {

class NumericParameter : public Parameter {
public:
    NumericParameter(const std::string& definition, std::shared_ptr<track::Track> track);
    NumericParameter(double static_value);
    ~NumericParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    double get_value(time::microseconds_t timestamp) const;

private:
    std::string definition_;
    std::shared_ptr<track::Track> track_;
    double static_value_ = 0.0;
};

} // namespace telemetry
} // namespace overlay

#endif // NUMERIC_PARAMETER_H
