#ifndef BACKEND_C_API_H
#define BACKEND_C_API_H

#include <cairo.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ManagerHandle ManagerHandle;

ManagerHandle* manager_new(void);
void manager_free(ManagerHandle* handle);
int manager_init(ManagerHandle* handle, float offset, char* track, char* custom_data, char* layout, int worker_count);
int manager_deinit(ManagerHandle* handle);

int manager_get_overlay_dimensions(ManagerHandle* handle, size_t* width, size_t* height, size_t* stride);
int manager_get_overlay_format(ManagerHandle* handle, cairo_format_t* format);

int manager_draw(ManagerHandle* handle, int64_t timestamp, cairo_surface_t* surface);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BACKEND_C_API_H
