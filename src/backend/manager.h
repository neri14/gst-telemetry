#ifndef MANAGER_H
#define MANAGER_H

#include <cairo.h>
#include <stdint.h>
#include "backend/utils/logging/logger.h"
#include "backend/utils/time.h"
#include "backend/track/track.h"
#include "backend/overlay/layout.h"

namespace telemetry {

class Manager {
public:
    Manager();
    ~Manager();

    bool init(float offset, const char* track_path, const char* custom_data_path, const char* layout_path);
    bool deinit();

    cairo_surface_t* draw(time::microseconds_t timestamp);

private:
    utils::logging::Logger log{"manager"};

    std::shared_ptr<track::Track> track_;
    std::shared_ptr<overlay::Layout> layout_;

    cairo_surface_t *surface_ = nullptr;
};

} // namespace telemetry

#endif // MANAGER_H