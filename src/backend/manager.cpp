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

} // namespace telemetry