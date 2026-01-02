#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "backend/track/track.h"
#include "backend/utils/time.h"
#include <string>
#include <memory>
#include <exprtk.hpp>

namespace telemetry {
namespace overlay {

class Expression {
public:
    Expression(const std::string& expression_str, std::shared_ptr<track::Track> track);
    ~Expression() = default;

    double evaluate(time::microseconds_t timestamp);

    bool is_valid() const;

private:
    mutable utils::logging::Logger log{"Expression"};

    bool valid_expr_ = false;
    double value_ = std::numeric_limits<double>::quiet_NaN();

    exprtk::expression<double> expression_;
    std::map<track::field_id_t, double> variables_;
    std::shared_ptr<track::Track> track_;

};


} // namespace overlay
} // namespace telemetry

#endif // EXPRESSION_H
