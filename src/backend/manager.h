#ifndef MANAGER_H
#define MANAGER_H

#include <cairo.h>
#include "backend/utils/logging/logger.h"

namespace telemetry {

class Manager {
public:
    Manager();
    ~Manager();

    bool init(float offset, const char* track, const char* layout);
    bool deinit();

    bool draw(long timestamp, cairo_surface_t* surface);

private:
    utils::logging::Logger log{"manager"};

    float offset_;
    std::string track_;
    std::string layout_;
};

} // namespace telemetry

#endif // MANAGER_H