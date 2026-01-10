#include "manager.h"

#include <iostream>

#include "backend/utils/time.h"
#include "trace/trace.h"

#include "surface.h"

namespace telemetry {
namespace consts {
    constexpr int default_worker_count = 4;
}

struct SurfaceWrapper {
    Surface surface;

    SurfaceWrapper() = default;
    ~SurfaceWrapper() = default;

    void notify_ready() {
        std::lock_guard<std::mutex> lock(mutex_);
        ready_ = true;
        cv_.notify_all();
    }

    void await_ready() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (ready_) {
            return;
        }
        cv_.wait(lock, [this]() { return ready_; });
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool ready_ = false;

    SurfaceWrapper(const SurfaceWrapper&) = delete;
    SurfaceWrapper& operator=(const SurfaceWrapper&) = delete;
    SurfaceWrapper(SurfaceWrapper&&) = delete;
    SurfaceWrapper& operator=(SurfaceWrapper&&) = delete;
};

Manager::Manager() {
    // Constructor implementation (if needed)
    log.info("Manager created");
};

Manager::~Manager() {
    // Destructor implementation (if needed)
    log.info("Manager destroyed");
};

bool Manager::init(float offset, const char* track_path, const char* custom_data_path, const char* layout_path, int worker_count) {
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

    if (worker_count <= 0) {
        log.info("Worker count not configured or incorrect, using auto configuration");
        worker_count = std::thread::hardware_concurrency() * 3 / 4;
        if (worker_count <= 0) {
            log.info("Failed to detect hardware concurrency, using default worker count");
            worker_count = consts::default_worker_count;
        }
    }

    log.info("Starting {} worker threads for drawing", worker_count);
    for (int i = 0; i < worker_count; ++i) {
        workers.emplace_back([this, i]() {
            std::function<void()> task;
            log.info("Worker-{} started", i);
            while (draw_queue_.pop(task)) {
                log.debug("Worker-{} thread executing drawing task", i);
                task();
            }
            log.info("Worker-{} exitting", i);
        });
    }

    TRACE_EVENT_END(EV_MANAGER_INIT);
    return true;
}

bool Manager::deinit() {
    TRACE_EVENT_BEGIN(EV_MANAGER_DEINIT);

    log.info("Stopping worker threads");
    draw_queue_.close();

    for(auto& worker : workers) {
        if(worker.joinable()) {
            worker.join();
        }
    }
    workers.clear();
    log.info("All worker threads stopped");

    log.info("Manager deinitialized");
    TRACE_EVENT_END(EV_MANAGER_DEINIT);
    return true;
}

bool Manager::get_overlay_dimensions(size_t* width, size_t* height, size_t* stride) const {
    if (layout_ == nullptr) {
        log.error("get_overlay_dimensions: layout not initialized");
        return false;
    }
    *width = layout_->get_width();
    *height = layout_->get_height();
    *stride = cairo_format_stride_for_width(format_, *width);
    return true;
}

bool Manager::get_overlay_format(cairo_format_t* format) const {
    *format = format_;
    return true;
}

bool Manager::draw(time::microseconds_t timestamp, cairo_surface_t* surface) {
    TRACE_EVENT_BEGIN(EV_MANAGER_DRAW);

    if (surface == nullptr) {
        log.error("draw: no cairo surface provided");
        TRACE_EVENT_END(EV_MANAGER_DRAW);
        return false;
    }

    std::deque<std::shared_ptr<SurfaceWrapper>> results;

    auto schedule_drawing = [this, &results](std::function<void(Surface&)> draw_func) {
        auto wrapper = std::make_shared<SurfaceWrapper>();
        results.push_back(wrapper);
        draw_queue_.push([draw_func, wrapper]() {
            draw_func(wrapper->surface);
            wrapper->notify_ready();
        });
    };

    log.debug("Drawing overlay at time {} us", timestamp);
    layout_->draw(timestamp, schedule_drawing);

    cairo_t *cr = cairo_create(surface);

    TRACE_EVENT_BEGIN(EV_MANAGER_CLEAR_SURFACE);
    cairo_save(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_restore(cr);
    TRACE_EVENT_END(EV_MANAGER_CLEAR_SURFACE);

    for (auto& result : results) {
        result->await_ready();

        Surface& surface = result->surface;

        TRACE_EVENT_BEGIN(EV_MANAGER_DRAW_CACHE);
        cairo_set_source_surface(cr, surface.surface, surface.x, surface.y);
        cairo_paint(cr);
        TRACE_EVENT_END(EV_MANAGER_DRAW_CACHE);
    }

    cairo_surface_flush(surface);
    cairo_destroy(cr);

    TRACE_EVENT_END(EV_MANAGER_DRAW);
    return true;
}

} // namespace telemetry