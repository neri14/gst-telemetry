#ifndef CONDITIONAL_WIDGET_H
#define CONDITIONAL_WIDGET_H

#include "widget.h"

#include "backend/utils/logging/logger.h"
#include "params/boolean_parameter.h"

namespace telemetry {
namespace overlay {

class ConditionalWidget : public Widget {
public:
    static std::shared_ptr<ConditionalWidget> create(parameter_map_ptr parameters);
    ConditionalWidget();
    ~ConditionalWidget() override = default;

    virtual void draw(time::microseconds_t timestamp,
                      schedule_drawing_cb_t schedule_drawing_cb,
                      double x_offset = 0, double y_offset = 0) override;

    inline static parameter_type_map_t parameter_types = {
        {"condition", ParameterType::Boolean}, // condition
    };

private:
    mutable utils::logging::Logger log{"ConditionalWidget"};

    std::shared_ptr<BooleanParameter> condition_ = nullptr;
};

} // namespace overlay
} // namespace telemetry

#endif // CONDITIONAL_WIDGET_H
