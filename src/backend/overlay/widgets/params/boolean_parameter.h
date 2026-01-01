#ifndef BOOLEAN_PARAMETER_H
#define BOOLEAN_PARAMETER_H

#include "parameter.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>

namespace telemetry {
namespace overlay {

class BooleanParameter : public Parameter {
public:
    BooleanParameter(const std::string& definition, std::shared_ptr<track::Track> track);
    BooleanParameter(bool static_value);
    ~BooleanParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    bool get_value(time::microseconds_t timestamp) const;

private:
    std::string definition_;
    std::shared_ptr<track::Track> track_;

    bool static_value_ = false;
};

} // namespace telemetry
} // namespace overlay

#endif // BOOLEAN_PARAMETER_H



