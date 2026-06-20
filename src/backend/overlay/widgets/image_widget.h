#ifndef IMAGE_WIDGET_H
#define IMAGE_WIDGET_H

#include "widget.h"

#include "backend/utils/logging/logger.h"
#include "params/string_parameter.h"
#include "params/numeric_parameter.h"
#include "params/boolean_parameter.h"

namespace telemetry {
namespace overlay {

class ImageWidget : public Widget {
public:
    static std::shared_ptr<ImageWidget> create(parameter_map_ptr parameters);
    ImageWidget();
    virtual ~ImageWidget() override = default;

    void draw(time::microseconds_t timestamp,
              schedule_drawing_cb_t schedule_drawing_cb,
              double x_offset, double y_offset) override;
            
    inline static parameter_type_map_t parameter_types = {
        {"path", ParameterType::String}, // image file path
        {"x", ParameterType::Numeric}, // top left corner x position
        {"y", ParameterType::Numeric}, // top left corner y position
        {"scale", ParameterType::Numeric}, // scale factor (1.0 = original size)
        {"visible", ParameterType::Boolean}, // visibility condition
    };

private:
    mutable utils::logging::Logger log{"ImageWidget"};

    void draw_impl(Surface& surface, time::microseconds_t timestamp, double x, double y);

    std::shared_ptr<StringParameter> path_ = nullptr;
    std::shared_ptr<NumericParameter> x_ = nullptr;
    std::shared_ptr<NumericParameter> y_ = nullptr;
    // std::shared_ptr<NumericParameter> rotation_ = nullptr;
    std::shared_ptr<NumericParameter> scale_ = nullptr;
    std::shared_ptr<BooleanParameter> visible_ = nullptr;

    cairo_surface_t* image_ = nullptr;

    cairo_surface_t* cache = nullptr;
    bool cache_drawn = false;
    int cache_width = 0;
    int cache_height = 0;
};

} // namespace overlay
} // namespace telemetry

#endif // IMAGE_WIDGET_H
