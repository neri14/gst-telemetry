#include "buffer_pool.h"
#include <stdlib.h>


static Buffer* _allocate_new_buffer(gsize buffer_size);

BufferPool* buffer_pool_create(gsize buffer_size) {
    BufferPool* pool = malloc(sizeof(BufferPool));
    if (pool == NULL) {
        return NULL; //failed to allocate memory
    }
    pool->buffers = NULL;
    pool->buffer_size = buffer_size;
    return pool;
}

void buffer_pool_destroy(BufferPool* pool) {
    if (pool == NULL) {
        return; // NULL manager provided
    }

    Buffer* buffer = pool->buffers;
    while (buffer != NULL) {
        if (buffer->gst_buffer != NULL) {
            gst_buffer_unref(buffer->gst_buffer);
            buffer->gst_buffer = NULL;
        }
        Buffer* next_buffer = buffer->next;
        free(buffer);
        buffer = next_buffer;
    }
    pool->buffers = NULL;

    free(pool);
}

GstBuffer* buffer_pool_acquire(BufferPool* pool) {
    if (pool == NULL) {
        return NULL; // NULL manager provided
    }
    if (pool->buffers == NULL) {
        pool->buffers = _allocate_new_buffer(pool->buffer_size);
    }

    Buffer* buffer = pool->buffers;
    while(buffer != NULL && !gst_buffer_is_writable(buffer->gst_buffer)) {
        if (buffer->next == NULL) {
            buffer->next = _allocate_new_buffer(pool->buffer_size);
        }
        buffer = buffer->next;
    }

    if (buffer == NULL) {
        return NULL; //should not happen
    }

    if (!gst_buffer_is_writable(buffer->gst_buffer)) {
        return NULL; //should not happen
    }
    return buffer->gst_buffer;
}

size_t buffer_pool_count(BufferPool* pool) {
    if (pool == NULL) {
        return 0;
    }

    size_t count = 0;
    Buffer* buffer = pool->buffers;
    while (buffer != NULL) {
        ++count;
        buffer = buffer->next;
    }
    return count;
}

size_t buffer_pool_count_in_use(BufferPool* pool) {
    if (pool == NULL) {
        return 0;
    }

    size_t count = 0;
    Buffer* buffer = pool->buffers;
    while (buffer != NULL) {
        if (!gst_buffer_is_writable(buffer->gst_buffer)) {
            ++count;
        }
        buffer = buffer->next;
    }
    return count;
}


static Buffer* _allocate_new_buffer(gsize buffer_size) {
    Buffer* buffer = malloc(sizeof(Buffer));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->gst_buffer = gst_buffer_new_allocate(NULL, buffer_size, NULL);
    if (buffer->gst_buffer == NULL) {
        free(buffer);
        return NULL;
    }

    buffer->next = NULL;
    return buffer;
}
