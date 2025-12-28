#ifndef VALUE_H
#define VALUE_H

#include <optional>
#include <string>
#include <format>
#include <variant>

#include "backend/utils/time.h"

namespace telemetry {
namespace track {

struct Value {
    std::optional<std::variant<std::string, double, bool, time::time_point_t>> data;

    bool is_string() const;
    bool is_double() const;
    bool is_bool() const;
    bool is_time_point() const;

    bool is_valid() const;
    operator bool() const;

    std::string as_string() const;//TODO format argument?
    double as_double() const;
    bool as_bool() const;
    time::time_point_t as_time_point() const;

    Value();
    Value(const std::string& str);
    Value(double d);
    Value(bool b);
    Value(time::time_point_t tp);
};

} // namespace track
} // namespace telemetry

#endif // VALUE_H
