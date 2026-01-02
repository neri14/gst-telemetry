#ifndef NUMERIC_PARAMETER_H
#define NUMERIC_PARAMETER_H

#include "parameter.h"
#include "expression.h"
#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>
#include <limits>

namespace telemetry {
namespace overlay {

class NumericParameter : public Parameter {
public:
    static std::shared_ptr<NumericParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);
    
    NumericParameter(std::shared_ptr<Expression> expression);
    NumericParameter(const std::string& key, std::shared_ptr<track::Track> track);
    NumericParameter(double static_value);

    ~NumericParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    double get_value(time::microseconds_t timestamp) const;

private:
    mutable utils::logging::Logger log{"NumericParameter"};

    UpdateStrategy update_strategy_;
    double value_ = std::numeric_limits<double>::quiet_NaN();

    //used by TrackKey update strategy
    std::shared_ptr<track::Track> track_ = nullptr;
    track::field_id_t field_id = track::INVALID_FIELD;

    //used by Expression update strategy
    std::shared_ptr<Expression> expression_ = nullptr;
};

} // namespace telemetry
} // namespace overlay

#endif // NUMERIC_PARAMETER_H
