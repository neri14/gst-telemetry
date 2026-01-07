#include "api/backend_c_api.h"
#include "manager.h"

extern "C" {

ManagerHandle* manager_new(void) {
    return reinterpret_cast<ManagerHandle*>(new telemetry::Manager());
}

void manager_free(ManagerHandle* handle) {
    delete reinterpret_cast<telemetry::Manager*>(handle);
}

int manager_init(ManagerHandle* handle, float offset, char* track, char* custom_data, char* layout) {
    bool ok = reinterpret_cast<telemetry::Manager*>(handle)->init(offset, track, custom_data, layout);
    return ok ? 0 : -1;
}

int manager_deinit(ManagerHandle* handle) {
    bool ok = reinterpret_cast<telemetry::Manager*>(handle)->deinit();
    return ok ? 0 : -1;
}

int manager_draw(ManagerHandle* handle, int64_t timestamp, Canvas** out_canvases, int* out_count) {
    auto surface_list = reinterpret_cast<telemetry::Manager*>(handle)->draw(timestamp);
    if (!surface_list) {
        return -1;
    }

    *out_canvases = new Canvas[surface_list->size()];
    *out_count = static_cast<int>(surface_list->size());
    Canvas* canvases = *out_canvases;

    for (size_t i = 0; i < surface_list->size(); ++i) {
        int x = std::get<0>((*surface_list)[i]);
        int y = std::get<1>((*surface_list)[i]);
        cairo_surface_t* surface = std::get<2>((*surface_list)[i]);

        unsigned int width = cairo_image_surface_get_width(surface);
        unsigned int height = cairo_image_surface_get_height(surface);
        unsigned char* data = cairo_image_surface_get_data(surface);
        unsigned long data_size = cairo_image_surface_get_stride(surface) * height;

        canvases[i].x = x;
        canvases[i].y = y;
        canvases[i].width = width;
        canvases[i].height = height;
        canvases[i].data = data;
        canvases[i].data_size = data_size;
    }
    return 0;
}

void manager_free_canvases(Canvas* canvases) {
    delete[] canvases;
}

} // extern "C"
