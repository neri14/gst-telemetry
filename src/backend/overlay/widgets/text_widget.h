#ifndef TEXT_WIDGET_H
#define TEXT_WIDGET_H

#include "string_widget.h"

#include "backend/utils/logging/logger.h"
#include "params/formatted_parameter.h"
#include "params/string_parameter.h"

namespace telemetry {
namespace overlay {

class TextWidget : public StringWidget {
public:
    static std::shared_ptr<TextWidget> create(parameter_map_ptr parameters);
    TextWidget();
    ~TextWidget() override = default;

    inline static parameter_type_map_t parameter_types = []() {
        auto types = StringWidget::parameter_types;
        types["value"] = ParameterType::Formatted; // text value to display
        types["format"] = ParameterType::String; // string format for value parameter
        return types;
    }();

private:
    virtual bool update_value(time::microseconds_t timestamp);
    virtual std::string get_value(time::microseconds_t timestamp) const;

    std::shared_ptr<FormattedParameter> value_ = nullptr;
};

} // namespace overlay
} // namespace telemetry

#endif // TEXT_WIDGET_H
