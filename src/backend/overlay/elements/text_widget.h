#ifndef TEXT_WIDGET_H
#define TEXT_WIDGET_H

#include "element.h"
#include "common/text_align.h"
#include "common/color.h"

namespace telemetry {
namespace overlay {

struct TextWidget: public Element {
    TextWidget(std::shared_ptr<track::Track> track,
               int x,
               int y,
               ETextAlign align,
               const std::string& font,
               rgba color,
               rgba border_color,
               int border_width,
               const std::string& text);
    virtual ~TextWidget() = default;

    void draw(time::microseconds_t timestamp, cairo_t* cr) override;

private:
    ETextAlign align_;
    std::string font_;
    rgba color_;
    rgba border_color_;
    int border_width_;
    std::string text_;
};

} // namespace overlay
} // namespace telemetry

#endif // TEXT_WIDGET_H