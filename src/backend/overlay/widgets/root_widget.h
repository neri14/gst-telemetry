#ifndef ROOT_WIDGET_H
#define ROOT_WIDGET_H

#include "widget.h"

#include "backend/utils/logging/logger.h"
#include "params/numeric_parameter.h"

namespace telemetry {
namespace overlay {

class RootWidget : public Widget {
public:
    static std::shared_ptr<RootWidget> create(parameter_map_ptr parameters);
    RootWidget();
    ~RootWidget() override = default;

    virtual void draw(time::microseconds_t timestamp, cairo_t* cr,
                      double x_offset = 0, double y_offset = 0) override;

    inline static parameter_type_map_t parameter_types = {
        {"width", ParameterType::Numeric}, // layout width
        {"height", ParameterType::Numeric}, // layout height
    };

    int get_width(time::microseconds_t timestamp);
    int get_height(time::microseconds_t timestamp);

private:
    mutable utils::logging::Logger log{"RootWidget"};

    std::shared_ptr<NumericParameter> width_ = nullptr;
    std::shared_ptr<NumericParameter> height_ = nullptr;
};

} // namespace overlay
} // namespace telemetry

#endif // ROOT_WIDGET_H
