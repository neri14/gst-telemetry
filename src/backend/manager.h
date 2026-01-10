#ifndef MANAGER_H
#define MANAGER_H

#include <thread>
#include <queue>
#include <functional>
#include <cairo.h>
#include <stdint.h>
#include "backend/utils/logging/logger.h"
#include "backend/utils/time.h"
#include "backend/utils/blocking_queue.h"
#include "backend/track/track.h"
#include "backend/overlay/layout.h"

namespace telemetry {

class Manager {
public:
    Manager();
    ~Manager();

    bool init(float offset, const char* track_path,
        const char* custom_data_path, const char* layout_path,
        int worker_count);
    bool deinit();

    uint64_t get_overlay_raw_size() const;
    cairo_surface_t* draw(time::microseconds_t timestamp);

private:
    mutable utils::logging::Logger log{"manager"};

    std::shared_ptr<track::Track> track_;
    std::shared_ptr<overlay::Layout> layout_;

    cairo_surface_t *surface_ = nullptr;

    BlockingQueue<std::function<void()>> draw_queue_;
    std::vector<std::jthread> workers;
};

} // namespace telemetry

#endif // MANAGER_H