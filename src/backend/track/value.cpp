#include "value.h"

namespace telemetry {
namespace track {

bool Value::is_string() const {
    return data && std::holds_alternative<std::string>(*data);
}

bool Value::is_double() const {
    return data && std::holds_alternative<double>(*data);
}

bool Value::is_bool() const {
    return data && std::holds_alternative<bool>(*data);
}

bool Value::is_time_point() const {
    return data && std::holds_alternative<time::time_point_t>(*data);
}

bool Value::is_valid() const {
    return data.has_value();
}

Value::operator bool() const {
    return is_valid();
}

std::string Value::as_string() const {
    if (is_string()) {
        return std::get<std::string>(*data);
    }
    if (is_double()) {
        return std::format("{}", std::get<double>(*data));
    }
    if (is_bool()) {
        return std::get<bool>(*data) ? "true" : "false";
    }
    if (is_time_point()) {
        return std::format("{}", std::get<time::time_point_t>(*data));
    }
    return "";
}

double Value::as_double() const {
    if (is_double()) {
        return std::get<double>(*data);
    }
    if (is_bool()) {
        return std::get<bool>(*data) ? 1.0 : 0.0;
    }
    return 0.0;
}

bool Value::as_bool() const {
    if (is_bool()) {
        return std::get<bool>(*data);
    }
    return false;
}

time::time_point_t Value::as_time_point() const {
    if (is_time_point()) {
        return std::get<time::time_point_t>(*data);
    }
    return time::INVALID_TIME_POINT;
}

Value::Value() : data(std::nullopt) {
}

Value::Value(const std::string& str) : data(str) {
}

Value::Value(double d) : data(d) {
}

Value::Value(bool b) : data(b) {
}

Value::Value(time::time_point_t tp) : data(tp) {
}

} // namespace track
} // namespace telemetry
