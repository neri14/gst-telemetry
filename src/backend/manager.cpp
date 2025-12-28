#include "manager.h"

#include <iostream>

namespace telemetry {

Manager::Manager() {
    // Constructor implementation (if needed)
    log.info("Manager created");
};

Manager::~Manager() {
    // Destructor implementation (if needed)
    log.info("Manager destroyed");
};

bool Manager::init(float offset, const char* track, const char* layout) {
    // Initialization code using the offset
    log.info("Manager initialization started");

    bool ok = true;
    if (track == nullptr) {
        log.warning("Track file path not provided");
        ok = false;
    }
    if (layout == nullptr) {
        log.warning("Layout file path not provided");
        ok = false;
    }

    if (!ok) {
        log.error("Manager initialization failed");
        return false;
    }

    log.info("Offset: {}", offset);
    log.info("Track: {}", track);
    log.info("Layout: {}", layout);

    offset_ = offset;
    track_ = std::string(track);
    layout_ = std::string(layout);

    return true;
}

bool Manager::deinit() {
    // Deinitialization code
    log.info("Manager deinitialized");
    return true;
}

bool Manager::draw(long timestamp, cairo_surface_t* surface) {
    if (surface == nullptr) {
        log.error("draw: no cairo surface provided");
        return false;
    }

    log.debug("Drawing overlay at time {} us", timestamp);

    /** TEST CAIRO DRAWING **/
    cairo_t *cr = cairo_create(surface);
    // Clear to transparent
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    // Draw your overlay content
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.8); // semi-transparent red
    cairo_rectangle(cr, 10, 10, 100, 100);
    cairo_fill(cr);
    // Draw text
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24);
    cairo_move_to(cr, 10, 150);
    cairo_show_text(cr, "Telemetry");
    // Flush and destroy
    cairo_surface_flush(surface);
    cairo_destroy(cr);
    /** TEST CAIRO DRAWING **/

    return true;
}

} // namespace telemetry