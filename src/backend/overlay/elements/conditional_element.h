#ifndef CONDITIONAL_ELEMENT_H
#define CONDITIONAL_ELEMENT_H

#include <optional>
#include <string>
#include <vector>
#include "backend/utils/time.h"
#include "backend/track/track.h"
#include "element.h"

namespace telemetry {
namespace overlay {

enum class EOperator {
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL
};

inline std::optional<EOperator> operator_from_string(const std::string& str) {
    if (str == "==") return EOperator::EQUAL;
    if (str == "!=") return EOperator::NOT_EQUAL;
    if (str == "<") return EOperator::LESS;
    if (str == "<=") return EOperator::LESS_EQUAL;
    if (str == ">") return EOperator::GREATER;
    if (str == ">=") return EOperator::GREATER_EQUAL;
    return std::nullopt;
}

struct ConditionalElement: public Element {
    ConditionalElement(std::shared_ptr<track::Track> track,
        int x, int y, const std::string& key,
        std::optional<EOperator> oper, std::optional<double> value);
    virtual ~ConditionalElement() = default;

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr);

protected:
    std::string key_;
    track::field_id_t key_id_;
    std::optional<EOperator> oper_;
    std::optional<double> value_;
};

} // namespace overlay
} // namespace telemetry

#endif // CONDITIONAL_ELEMENT_H