#include "manager.h"

#include <iostream>

#include "backend/utils/time.h"

namespace telemetry {

Manager::Manager() {
    // Constructor implementation (if needed)
    log.info("Manager created");
};

Manager::~Manager() {
    // Destructor implementation (if needed)
    log.info("Manager destroyed");
};

bool Manager::init(float offset, const char* track_path, const char* custom_data_path, const char* layout_path) {
    // Initialization code using the offset
    log.info("Manager initialization started");

    bool ok = true;
    if (track_path == nullptr) {
        log.warning("Track file path not provided");
        ok = false;
    }
    if (layout_path == nullptr) {
        log.warning("Layout file path not provided");
        ok = false;
    }
    if (!ok) {
        log.error("Insufficient parameters provided for initialization");
        return false;
    }

    time::microseconds_t offset_us = time::s_to_us(static_cast<time::seconds_t>(offset));
    log.info("Offset: {}s", offset);
    log.info("Offset: {}us", offset_us);
    log.info("Track path: {}", track_path);
    log.info("Layout path: {}", layout_path);

    track_ = std::make_shared<track::Track>(offset_us);
    ok = track_->load(track_path);
    if (!ok) {
        log.error("Failed to load track from path: {}", track_path);
        return false;
    }
    log.info("Track loaded successfully");

    if (custom_data_path != nullptr) {
        ok = track_->load_custom_data(custom_data_path);
        if (!ok) {
            log.error("Failed to load custom data from path: {}", custom_data_path);
            return false;
        }
        log.info("Custom data loaded successfully");
    }

    layout_ = std::make_shared<overlay::Layout>(track_);
    ok = layout_->load(layout_path);
    if (!ok) {
        log.error("Failed to load layout from path: {}", layout_path);
        return false;
    }
    log.info("Layout loaded successfully");

    log.info("Manager initialized successfully");
    return true;
}

bool Manager::deinit() {
    // Deinitialization code
    log.info("Manager deinitialized");
    return true;
}

std::shared_ptr<surface_list_t> Manager::draw(time::microseconds_t timestamp) {
    //TODO add tracing later
    log.debug("Drawing overlay at time {} us", timestamp);

    auto surface_list = std::make_shared<surface_list_t>();
    layout_->draw(timestamp, *surface_list);


    // /** TEST CAIRO DRAWING **/
    // cairo_t *cr = cairo_create(surface);
    // // Clear to transparent
    // cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    // cairo_paint(cr);
    // cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    // // Draw your overlay content
    // cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.8); // semi-transparent red
    // cairo_rectangle(cr, 10, 10, 100, 100);
    // cairo_fill(cr);
    // // Draw text
    // cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    // cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_surface_countNORMAL, CAIRO_FONT_WEIGHT_BOLD);
    // cairo_set_font_size(cr, 24);
    // cairo_move_to(cr, 10, 150);

    // // auto val = track_->get("point.timer", timestamp);
    // auto val = track_->get("time_elapsed", timestamp);
    // cairo_show_text(cr, val ? val.as_string().c_str() : "unknown");
    // // cairo_show_text(cr, "Telemetry");

    // // Flush and destroy
    // cairo_surface_flush(surface);
    // cairo_destroy(cr);
    // /** TEST CAIRO DRAWING **/

    return surface_list;
}

} // namespace telemetry