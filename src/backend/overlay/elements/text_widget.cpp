#include "text_widget.h"

extern "C" {
    #include <pango/pangocairo.h>
}

namespace telemetry {
namespace overlay {

TextWidget::TextWidget(std::shared_ptr<track::Track> track,
                       int x,
                       int y,
                       ETextAlign align,
                       const std::string& font,
                       rgba color,
                       rgba border_color,
                       int border_width,
                       const std::string& text)
    : Element(track, x, y),
      align_(align),
      font_(font),
      color_(color),
      border_color_(border_color),
      border_width_(border_width),
      text_(text) {
}

void TextWidget::draw(time::microseconds_t timestamp, cairo_t* cr) {
    //TODO draw to cache and use cached surface since this widget is fully static

    //setup
    PangoLayout *layout = pango_cairo_create_layout(cr);

    //set font
    PangoFontDescription *pfont = pango_font_description_from_string(font_.c_str());
    pango_layout_set_font_description(layout, pfont);
    pango_font_description_free(pfont);
    pfont = nullptr;

    //  set text and alignment
    pango_layout_set_text(layout, text_.c_str(), -1);
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