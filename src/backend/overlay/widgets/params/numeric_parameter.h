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
    using value_map_t = std::map<time::microseconds_t, double>;
    using sections_t = std::vector<value_map_t>;

    static std::shared_ptr<NumericParameter> create(
        const std::string& definition, std::shared_ptr<track::Track> track);
    
    NumericParameter(std::shared_ptr<Expression> expression, std::shared_ptr<track::Track> track);
    NumericParameter(const std::string& key, std::shared_ptr<track::Track> track);
    NumericParameter(double static_value);

    ~NumericParameter() override = default;

    bool update(time::microseconds_t timestamp) override;
    double get_value(time::microseconds_t timestamp, bool allow_nan = false) const;

    std::shared_ptr<sections_t> get_values(
                time::microseconds_t step = time::INVALID_TIME,
                double min_value = std::numeric_limits<double>::min(),
                double max_value = std::numeric_limits<double>::max());

    std::shared_ptr<sections_t> get_values(std::vector<std::vector<time::microseconds_t>> timestamp_sections);
    
    std::shared_ptr<value_map_t> get_all_values();

    bool is_static() const;

private:
    mutable utils::logging::Logger log{"NumericParameter"};

    UpdateStrategy update_strategy_;
    double value_ = std::numeric_limits<double>::quiet_NaN();

    //used by TrackKey and Expression update strategies
    std::shared_ptr<track::Track> track_ = nullptr;

    //used by TrackKey update strategy
    track::field_id_t field_id = track::INVALID_FIELD;

    //used by Expression update strategy
    std::shared_ptr<Expression> expression_ = nullptr;
};

} // namespace telemetry
} // namespace overlay

#endif // NUMERIC_PARAMETER_H
