#include "conditional_element.h"

#include "backend/track/value.h"

namespace telemetry {
namespace overlay {

ConditionalElement::ConditionalElement(
        std::shared_ptr<track::Track> track,
        int x, int y, const std::string& key,
        std::optional<EOperator> oper, std::optional<double> value)
    : Element(track, x, y),
      key_(key),
      key_id_(track->get_field_id(key)),
      oper_(oper),
      value_(value) {
}

void ConditionalElement::draw(time::microseconds_t timestamp, cairo_t* cr) {
    track::Value val = track_->get(key_id_, timestamp);
    bool condition_met = false;
    if (!oper_) {
        condition_met = val.as_bool();
    } else if (val.is_double() && value_) {
        double v = val.as_double();
        switch (*oper_) {
            case EOperator::EQUAL:
                condition_met = (v == *value_);
                break;
            case EOperator::NOT_EQUAL:
                condition_met = (v != *value_);
                break;
            case EOperator::LESS:
                condition_met = (v < *value_);
                break;
            case EOperator::LESS_EQUAL:
                condition_met = (v <= *value_);
                break;
            case EOperator::GREATER:
                condition_met = (v > *value_);
                break;
            case EOperator::GREATER_EQUAL:
                condition_met = (v >= *value_);
                break;
            default:
                break;
        }
    }

    if (condition_met) {
        for (auto& child : children) {
            child->draw(timestamp, cr);
        }
    }
}

} // namespace overlay
} // namespace telemetry
