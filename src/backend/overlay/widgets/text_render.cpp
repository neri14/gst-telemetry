#include "text_render.h"

#include "backend/utils/logging/logger.h"

extern "C" {
    #include <pango/pangocairo.h>
}

namespace telemetry {
namespace overlay {

namespace {
    utils::logging::Logger log{"text_render"};
}

TextSize measure_text(cairo_t* cr, const std::string& text, const std::string& font) {
    PangoLayout* layout = pango_cairo_create_layout(cr);

    PangoFontDescription* pfont = pango_font_description_from_string(font.c_str());
    pango_layout_set_font_description(layout, pfont);
    pango_font_description_free(pfont);

    pango_layout_set_text(layout, text.c_str(), -1);

    int w, h;
    pango_layout_get_pixel_size(layout, &w, &h);

    g_object_unref(layout);

    return TextSize{w, h};
}

void draw_text(cairo_t* cr, int width, int height, int margin,
               const std::string& text, const std::string& font,
               ETextAlign align, rgb color,
               double border_width, rgb border_color) {
    //setup
    PangoLayout* layout = pango_cairo_create_layout(cr);

    //  set font
    PangoFontDescription *pfont = pango_font_description_from_string(font.c_str());
    pango_layout_set_font_description(layout, pfont);
    pango_font_description_free(pfont);
    pfont = nullptr;

    //  set text and alignment
    pango_layout_set_text(layout, text.c_str(), -1);
    pango_layout_set_alignment(layout, to_pango_align(align));

    //  check if clipping may occur
    int w_in_margin = width - 2 * margin;
    int h_in_margin = height - 2 * margin;
    int w,h;
    pango_layout_get_pixel_size(layout, &w, &h);
    if (w > w_in_margin || h > h_in_margin) {
        log.warning("Text size ({}x{}) exceeds expected size ({}x{}), clipping may occur", w, h, w_in_margin, h_in_margin);
    }

    // set layout size to cache size
    pango_layout_set_width(layout, w_in_margin * PANGO_SCALE);
    pango_layout_set_height(layout, h_in_margin * PANGO_SCALE);

    // draw border
    cairo_move_to(cr, margin, margin);

    cairo_set_source_rgba(cr, border_color.r, border_color.g, border_color.b, border_color.a);
    cairo_set_line_width(cr, border_width*2);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

    pango_cairo_layout_path(cr, layout);
    cairo_stroke(cr);

    // draw background
    cairo_move_to(cr, margin, margin);

    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    pango_cairo_show_layout (cr, layout);

    // cleanup
    g_object_unref(layout);
}

} // namespace overlay
} // namespace telemetry
