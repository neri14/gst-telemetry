#include "value_widget.h"
#include <format>

extern "C" {
    #include <pango/pangocairo.h>
}

namespace telemetry {
namespace overlay {

ValueWidget::ValueWidget(std::shared_ptr<track::Track> track,
                       int x,
                       int y,
                       ETextAlign align,
                       const std::string& font,
                       rgba color,
                       rgba border_color,
                       int border_width,
                       const std::string& key,
                       const std::string& format)
    : Element(track, x, y),
      align_(align),
      font_(font),
      color_(color),
      border_color_(border_color),
      border_width_(border_width),
      key_(key),
      key_id_(track->get_field_id(key)),
      format_(format) {
}

void ValueWidget::draw(time::microseconds_t timestamp, cairo_t* cr) {
    std::string str = "-";
    track::Value value = track_->get(key_id_, timestamp);

    if (value.is_valid()) {
        if (value.is_double()) {
            double val = value.as_double();
            str = std::vformat(format_, std::make_format_args(val));
        } else if (value.is_time_point()) {
            auto tp = value.as_time_point();
            str = std::vformat(format_, std::make_format_args(tp));
        } else if (value.is_string()) {
            str = value.as_string();
        }
    }

    //TODO draw to cache and use cached surface - invalidate cache only if str changes
    //TODO create common text drawing function for TextWidget and ValueWidget (and others?)

    //setup
    PangoLayout *layout = pango_cairo_create_layout(cr);

    //set font
    PangoFontDescription *pfont = pango_font_description_from_string(font_.c_str());
    pango_layout_set_font_description(layout, pfont);
    pango_font_description_free(pfont);
    pfont = nullptr;

    //  set text and alignment
    pango_layout_set_text(layout, str.c_str(), -1);
    pango_layout_set_alignment(layout, to_pango_align(align_));

    //  setup offset depending on alignment
    int w,h;
    pango_layout_get_pixel_size(layout, &w, &h);

    int offset = 0;
    if (align_ == ETextAlign::Right) {
        offset = w;
    } else if (align_ == ETextAlign::Center) {
        offset = w/2;
    }

    // draw border
    cairo_move_to(cr, x - offset, y);

    cairo_set_source_rgba(cr, border_color_.r, border_color_.g, border_color_.b, border_color_.a);
    cairo_set_line_width(cr, border_width_*2);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

    pango_cairo_layout_path(cr, layout);
    cairo_stroke(cr);

    // draw background
    cairo_move_to(cr, x - offset, y);

    cairo_set_source_rgba(cr, color_.r, color_.g, color_.b, color_.a);
    pango_cairo_show_layout (cr, layout);

    // cleanup
    g_object_unref(layout);

    Element::draw(timestamp, cr); // Call base class draw to handle children
}

} // namespace overlay
} // namespace telemetry