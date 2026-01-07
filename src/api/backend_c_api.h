#ifndef BACKEND_C_API_H
#define BACKEND_C_API_H

#include <cairo.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Canvas {
    int x;
    int y;
    unsigned int width;
    unsigned int height;

    unsigned char* data;
    unsigned long data_size;
} Canvas;

typedef struct ManagerHandle ManagerHandle;

ManagerHandle* manager_new(void);
void manager_free(ManagerHandle* handle);
int manager_init(ManagerHandle* handle, float offset, char* track, char* custom_data, char* layout);
int manager_deinit(ManagerHandle* handle);

int manager_draw(ManagerHandle* handle, int64_t timestamp, Canvas** out_canvases, int* out_count);
void manager_free_canvases(Canvas* canvases);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BACKEND_C_API_H
