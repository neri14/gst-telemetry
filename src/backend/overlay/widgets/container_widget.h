#ifndef CONTAINER_WIDGET_H
#define CONTAINER_WIDGET_H

#include "widget.h"

#include "backend/utils/logging/logger.h"
#include "params/numeric_parameter.h"
#include "params/boolean_parameter.h"

namespace telemetry {
namespace overlay {

class ContainerWidget : public Widget {
public:
    static std::shared_ptr<ContainerWidget> create(parameter_map_ptr parameters);
    ContainerWidget();
    ~ContainerWidget() override = default;

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr,
                      double x_offset = 0, double y_offset = 0) override;

    inline static parameter_type_map_t parameter_types = {
        {"x", ParameterType::Numeric}, // x position
        {"y", ParameterType::Numeric}, // y position
        {"visible", ParameterType::Boolean}, // visibility condition
    };

private:
    mutable utils::logging::Logger log{"ContainerWidget"};

    std::shared_ptr<NumericParameter> x_ = nullptr;
    std::shared_ptr<NumericParameter> y_ = nullptr;
    std::shared_ptr<BooleanParameter> visible_ = nullptr;
};

} // namespace overlay
} // namespace telemetry

#endif // CONTAINER_WIDGET_H
