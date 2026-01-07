#ifndef MANAGER_H
#define MANAGER_H

#include <cairo.h>
#include <stdint.h>
#include <memory>
#include "backend/utils/logging/logger.h"
#include "backend/utils/time.h"
#include "backend/track/track.h"
#include "backend/overlay/layout.h"

namespace telemetry {

using surface_list_t = overlay::surface_list_t;

class Manager {
public:
    Manager();
    ~Manager();

    bool init(float offset, const char* track_path, const char* custom_data_path, const char* layout_path);
    bool deinit();

    std::shared_ptr<surface_list_t> draw(time::microseconds_t timestamp);

private:
    utils::logging::Logger log{"manager"};

    std::shared_ptr<track::Track> track_;
    std::shared_ptr<overlay::Layout> layout_;
};

} // namespace telemetry

#endif // MANAGER_H