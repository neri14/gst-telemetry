#ifndef SURFACE_H
#define SURFACE_H

#include <cairo.h>

namespace telemetry {

struct Surface {
    cairo_surface_t* surface = nullptr;
    int x = 0;
    int y = 0;
};

} // namespace telemetry

#endif // SURFACE_H
