#ifndef VALUE_WIDGET_H
#define VALUE_WIDGET_H

#include "element.h"
#include "common/text_align.h"
#include "common/color.h"

namespace telemetry {
namespace overlay {

struct ValueWidget: public Element {
    ValueWidget(std::shared_ptr<track::Track> track,
               int x,
               int y,
               ETextAlign align,
               const std::string& font,
               rgba color,
               rgba border_color,
               int border_width,
               const std::string& key,
               const std::string& format);
    virtual ~ValueWidget() = default;

    void draw(time::microseconds_t timestamp, cairo_t* cr) override;

private:
    ETextAlign align_;
    std::string font_;
    rgba color_;
    rgba border_color_;
    int border_width_;
    std::string key_;
    track::field_id_t key_id_;
    std::string format_;
};

} // namespace overlay
} // namespace telemetry

#endif // VALUE_WIDGET_H