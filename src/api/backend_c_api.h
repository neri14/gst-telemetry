#ifndef BACKEND_C_API_H
#define BACKEND_C_API_H

#include <cairo.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ManagerHandle ManagerHandle;

ManagerHandle* manager_new(void);
void manager_free(ManagerHandle* handle);
int manager_init(ManagerHandle* handle, float offset, char* track, char* custom_data, char* layout);
int manager_deinit(ManagerHandle* handle);

uint64_t manager_get_overlay_raw_size(ManagerHandle* handle);
cairo_surface_t* manager_draw(ManagerHandle* handle, int64_t timestamp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BACKEND_C_API_H
