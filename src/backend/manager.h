#ifndef MANAGER_H
#define MANAGER_H

#include <cairo.h>
#include <stdint.h>
#include "backend/utils/logging/logger.h"
#include "backend/track/track.h"

namespace telemetry {

class Manager {
public:
    Manager();
    ~Manager();

    bool init(float offset, const char* track_path, const char* layout_path);
    bool deinit();

    bool draw(int64_t timestamp, cairo_surface_t* surface);

private:
    utils::logging::Logger log{"manager"};

    int64_t offset_;
    std::shared_ptr<track::Track> track_;
    // Layout layout_;
};

} // namespace telemetry

#endif // MANAGER_H