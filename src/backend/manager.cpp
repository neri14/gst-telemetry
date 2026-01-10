#include "manager.h"

#include <iostream>

#include "backend/utils/time.h"
#include "trace/trace.h"

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
    TRACE_EVENT_BEGIN(EV_MANAGER_INIT);

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
        TRACE_EVENT_END(EV_MANAGER_INIT);
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
        TRACE_EVENT_END(EV_MANAGER_INIT);
        return false;
    }
    log.info("Track loaded successfully");

    if (custom_data_path != nullptr) {
        ok = track_->load_custom_data(custom_data_path);
        if (!ok) {
            log.error("Failed to load custom data from path: {}", custom_data_path);
            TRACE_EVENT_END(EV_MANAGER_INIT);
            return false;
        }
        log.info("Custom data loaded successfully");
    }

    layout_ = std::make_shared<overlay::Layout>(track_);
    ok = layout_->load(layout_path);
    if (!ok) {
        log.error("Failed to load layout from path: {}", layout_path);
        TRACE_EVENT_END(EV_MANAGER_INIT);
        return false;
    }
    log.info("Layout loaded successfully");

    surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, layout_->get_width(), layout_->get_height());

    TRACE_EVENT_END(EV_MANAGER_INIT);
    return true;
}

bool Manager::deinit() {
    TRACE_EVENT_BEGIN(EV_MANAGER_DEINIT);
    
    cairo_surface_destroy(surface_);
    surface_ = nullptr;

    log.info("Manager deinitialized");
    TRACE_EVENT_END(EV_MANAGER_DEINIT);
    return true;
}

cairo_surface_t* Manager::draw(time::microseconds_t timestamp) {
    TRACE_EVENT_BEGIN(EV_MANAGER_DRAW);

    if (surface_ == nullptr) {
        log.error("draw: no cairo surface available");
        TRACE_EVENT_END(EV_MANAGER_DRAW);
        return nullptr;
    }

    cairo_t *cr = cairo_create(surface_);

    TRACE_EVENT_BEGIN(EV_MANAGER_CLEAR_SURFACE);
    cairo_save(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_restore(cr);
    TRACE_EVENT_END(EV_MANAGER_CLEAR_SURFACE);

    log.debug("Drawing overlay at time {} us", timestamp);
    layout_->draw(timestamp, cr);

    cairo_surface_flush(surface_);
    cairo_destroy(cr);

    TRACE_EVENT_END(EV_MANAGER_DRAW);
    return surface_;
}

} // namespace telemetry