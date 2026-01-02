#ifndef TIMESTAMP_WIDGET_H
#define TIMESTAMP_WIDGET_H

#include "string_widget.h"

#include "backend/utils/logging/logger.h"
#include "params/timestamp_parameter.h"
#include "params/string_parameter.h"

namespace telemetry {
namespace overlay {

class TimestampWidget : public StringWidget {
public:
    static std::shared_ptr<TimestampWidget> create(parameter_map_ptr parameters);
    TimestampWidget();
    ~TimestampWidget() override = default;

    inline static parameter_type_map_t parameter_types = []() {
        auto types = StringWidget::parameter_types;
        types["value"] = ParameterType::Timestamp; // timestamp value to display
        types["format"] = ParameterType::String; // string format for value parameter
        types["precision"] = ParameterType::Numeric; // precision for fractional seconds (0,3,6) (other values will be rounded up to nearest valid)
        types["timezone"] = ParameterType::String; // timezone for formatting
        return types;
    }();

private:
    virtual bool update_value(time::microseconds_t timestamp);
    virtual std::string get_value(time::microseconds_t timestamp) const;

    std::shared_ptr<TimestampParameter> value_ = nullptr;
};

} // namespace overlay
} // namespace telemetry

#endif // TIMESTAMP_WIDGET_H
