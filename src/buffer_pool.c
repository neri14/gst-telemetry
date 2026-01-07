#include "buffer_pool.h"
#include <stdlib.h>

static BufferPool* _allocate_new_buffer_pool(gsize buffer_size);
static Buffer* _allocate_new_buffer(gsize buffer_size);

BufferPoolManager* buffer_pool_manager_create() {
    BufferPoolManager* mngr = malloc(sizeof(BufferPoolManager));
    if (mngr == NULL) {
        return NULL; //failed to allocate memory
    }
    mngr->pools = NULL;
    return mngr;
}

void buffer_pool_manager_destroy(BufferPoolManager* mngr) {
    if (mngr == NULL) {
        return; // NULL manager provided
    }

    BufferPool* pool = mngr->pools;
    while (pool != NULL) {
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
        BufferPool* next_pool = pool->next;
        free(pool);
        pool = next_pool;
    }

    free(mngr);
}

GstBuffer* buffer_pool_manager_acquire_buffer(BufferPoolManager* mngr, gsize buffer_size) {
    if (mngr == NULL) {
        return NULL; // NULL manager provided
    }
    if (mngr->pools == NULL) {
        mngr->pools = _allocate_new_buffer_pool(buffer_size);
    }

    BufferPool* pool = mngr->pools;
    while (pool != NULL && pool->buffer_size != buffer_size) {
        if (pool->next == NULL) {
            pool->next = _allocate_new_buffer_pool(buffer_size);
        }
        pool = pool->next;
    }

    if (pool == NULL) {
        return NULL;
    }

    if (pool->buffers == NULL) {
        Buffer* buffer = _allocate_new_buffer(buffer_size);
        pool->buffers = buffer;
    }

    Buffer* buffer = pool->buffers;
    while(buffer != NULL && !gst_buffer_is_writable(buffer->gst_buffer)) {
        if (buffer->next == NULL) {
            buffer->next = _allocate_new_buffer(buffer_size);
        }
        buffer = buffer->next;
    }

    if (buffer == NULL) {
        return NULL;
    }

    if (!gst_buffer_is_writable(buffer->gst_buffer)) {
        return NULL; //should not happen
    }
    return buffer->gst_buffer;
}

size_t buffer_pool_manager_count(BufferPoolManager* mngr) {
    if (mngr == NULL) {
        return 0;
    }

    size_t count = 0;
    BufferPool* pool = mngr->pools;
    while (pool != NULL) {
        Buffer* buffer = pool->buffers;
        while (buffer != NULL) {
            ++count;
            buffer = buffer->next;
        }
        pool = pool->next;
    }
    return count;
}

size_t buffer_pool_manager_count_in_use(BufferPoolManager* mngr) {
    if (mngr == NULL) {
        return 0;
    }

    size_t count = 0;
    BufferPool* pool = mngr->pools;
    while (pool != NULL) {
        Buffer* buffer = pool->buffers;
        while (buffer != NULL) {
            if (!gst_buffer_is_writable(buffer->gst_buffer)) {
                ++count;
            }
            buffer = buffer->next;
        }
        pool = pool->next;
    }
    return count;
}

static BufferPool* _allocate_new_buffer_pool(gsize buffer_size) {
    BufferPool* pool = malloc(sizeof(BufferPool));
    if (pool == NULL) {
        return NULL;
    }
    pool->buffer_size = buffer_size;
    pool->buffers = NULL;
    pool->next = NULL;
    return pool;
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
