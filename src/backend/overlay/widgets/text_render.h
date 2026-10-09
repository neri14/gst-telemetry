#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include "backend/utils/color.h"
#include "backend/utils/text_align.h"
#include <string>

extern "C" {
    #include <cairo.h>
}

namespace telemetry {
namespace overlay {

struct TextSize {
    int width;
    int height;
};

// Measures the pixel size `text` would occupy if rendered with `font` (a Pango
// font description string, e.g. "Arial 12"), without drawing anything. `cr` is
// only used to create a throwaway Pango layout for measurement - nothing is
// painted into it.
TextSize measure_text(cairo_t* cr, const std::string& text, const std::string& font);

// Renders `text` into the [margin, width-margin] x [margin, height-margin] box
// of the target surface behind `cr`, with an optional stroked border. Shared by
// StringWidget (single label per widget) and ChartWidget (multiple extreme-value
// labels drawn into one shared cache).
void draw_text(cairo_t* cr, int width, int height, int margin,
                const std::string& text, const std::string& font,
                ETextAlign align, rgb color,
                double border_width, rgb border_color);

} // namespace overlay
} // namespace telemetry

#endif // TEXT_RENDER_H
