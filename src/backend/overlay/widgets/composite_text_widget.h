#ifndef COMPOSITE_TEXT_WIDGET_H
#define COMPOSITE_TEXT_WIDGET_H

#include "string_widget.h"

#include "backend/utils/logging/logger.h"
#include "params/timestamp_parameter.h"
#include "params/string_parameter.h"

namespace telemetry {
namespace overlay {

class CompositeTextWidget : public StringWidget {
public:
    static std::shared_ptr<CompositeTextWidget> create(parameter_map_ptr parameters);
    CompositeTextWidget();
    ~CompositeTextWidget() override = default;

    inline static parameter_type_map_t parameter_types = []() {
        auto types = StringWidget::parameter_types;
        types["value-1"] = ParameterType::Numeric; // first numeric value
        types["value-2"] = ParameterType::Numeric; // second numeric value
        types["value-3"] = ParameterType::Numeric; // third numeric value
        types["value-4"] = ParameterType::Numeric; // fourth numeric value
        types["value-5"] = ParameterType::Numeric; // fifth numeric value
        types["value-6"] = ParameterType::Numeric; // sixth numeric value
        types["format"] = ParameterType::String; // string format for value parameters
        return types;
    }();

private:
    virtual bool update_value(time::microseconds_t timestamp);
    virtual std::string get_value(time::microseconds_t timestamp) const;

    std::string value_ = "";
    bool valid_ = false;

    std::shared_ptr<NumericParameter> value_1_ = nullptr;
    std::shared_ptr<NumericParameter> value_2_ = nullptr;
    std::shared_ptr<NumericParameter> value_3_ = nullptr;
    std::shared_ptr<NumericParameter> value_4_ = nullptr;
    std::shared_ptr<NumericParameter> value_5_ = nullptr;
    std::shared_ptr<NumericParameter> value_6_ = nullptr;
    std::shared_ptr<StringParameter> format_ = nullptr;
};

} // namespace overlay
} // namespace telemetry

#endif // COMPOSITE_TEXT_WIDGET_H
