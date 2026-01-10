#ifndef GST_TELEMETRY_BUFFER_POOL_H
#define GST_TELEMETRY_BUFFER_POOL_H

#include <gst/gst.h>


typedef struct Buffer {
    GstBuffer* gst_buffer;

    struct Buffer* next;
} Buffer;

typedef struct BufferPool {
    struct Buffer* buffers;
    gsize buffer_size;
} BufferPool;


BufferPool* buffer_pool_create(gsize buffer_size);
void buffer_pool_destroy(BufferPool* pool);

GstBuffer* buffer_pool_acquire(BufferPool* pool);

size_t buffer_pool_count(BufferPool* pool);
size_t buffer_pool_count_in_use(BufferPool* pool);

#endif // GST_TELEMETRY_BUFFER_POOL_H
