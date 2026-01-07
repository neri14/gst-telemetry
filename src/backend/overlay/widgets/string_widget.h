#ifndef STRING_WIDGET_H
#define STRING_WIDGET_H

#include "widget.h"

#include "backend/utils/logging/logger.h"
#include "params/numeric_parameter.h"
#include "params/string_parameter.h"
#include "params/formatted_parameter.h"
#include "params/color_parameter.h"
#include "params/alignment_parameter.h"
#include "params/boolean_parameter.h"

namespace telemetry {
namespace overlay {

class StringWidget : public Widget {
public:
    static bool load_params(std::shared_ptr<StringWidget> widget, parameter_map_ptr parameters);
    StringWidget(const std::string& name);
    ~StringWidget() override = default;

    virtual void draw(time::microseconds_t timestamp,
        double x_offset, double y_offset, draw_cb_t draw_cb) override;
    virtual unsigned int surface_count() const override;

    inline static parameter_type_map_t parameter_types = {
        {"x", ParameterType::Numeric}, // center x position
        {"y", ParameterType::Numeric}, // center y position
        {"font-name", ParameterType::String}, // font family/name
        {"font-size", ParameterType::Numeric}, // font size in points
        {"align", ParameterType::Alignment}, // text alignment: left, center, right
        {"color", ParameterType::Color}, // text color
        {"border-width", ParameterType::Numeric}, // border width
        {"border-color", ParameterType::Color}, // border color
        {"visible", ParameterType::Boolean}, // visibility condition
    };

private:
    virtual bool update_value(time::microseconds_t timestamp) = 0;
    virtual std::string get_value(time::microseconds_t timestamp) const = 0;

    void draw_text(cairo_t* cr, int width, int height, int margin,
                   const std::string& text,
                   const std::string& font,
                   ETextAlign align,
                   rgb color,
                   double border_width,
                   rgb border_color);

    std::shared_ptr<NumericParameter> x_ = nullptr;
    std::shared_ptr<NumericParameter> y_ = nullptr;
    std::shared_ptr<StringParameter> font_name_ = nullptr;
    std::shared_ptr<NumericParameter> font_size_ = nullptr;
    std::shared_ptr<AlignmentParameter> align_ = nullptr;
    std::shared_ptr<ColorParameter> color_ = nullptr;
    std::shared_ptr<NumericParameter> border_width_ = nullptr;
    std::shared_ptr<ColorParameter> border_color_ = nullptr;
    std::shared_ptr<BooleanParameter> visible_ = nullptr;

    cairo_surface_t* cache = nullptr;
    bool cache_drawn = false;
    int cache_width = 0;
    int cache_height = 0;
};

} // namespace overlay
} // namespace telemetry

#endif // STRING_WIDGET_H
