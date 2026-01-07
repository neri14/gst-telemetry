#ifndef GST_TELEMETRY_BUFFER_POOL_H
#define GST_TELEMETRY_BUFFER_POOL_H

#include <gst/gst.h>

/* basic usage:
 *   // in plugin initialization
 *   BufferPoolManager* mngr = buffer_pool_manager_create()
 * 
 *   // whenever GstBuffer* of size SIZE is needed
 *   buffer_pool_manager_acquire_buffer(mngr, SIZE);
 * 
 *   // after using acquired GstBuffers*
 *   //NOT YET IMPLEMENTED
 *   buffer_pool_manager_cleanup(mngr);
 * 
 *   // in plugin deinitialization
 *   buffer_pool_manager_destroy(mngr);
 * 
 * 
 * NOT YET IMPLEMENTED:
 * buffer_pool_manager_gc removes pools that are not in use
 *   i.e. remove all pools in which all GstBuffers are writable
 *     i.e.2. no other ref to that buffer exists
 * 
 * BufferPoolManager can be used without calling buffer_pool_manager_gc
 *   which will be faster (no checking for unused pools
 *   and deallocating/allocating buffers that are used every other frame)
 *   but more memory expensive (lingering unused buffers)
 */

typedef struct Buffer {
    GstBuffer* gst_buffer;

    struct Buffer* next;
} Buffer;

typedef struct BufferPool {
    gsize buffer_size;
    struct Buffer* buffers;

    struct BufferPool* next;
} BufferPool;

typedef struct BufferPoolManager {
    struct BufferPool* pools;
} BufferPoolManager;


// allocate BufferPoolManager and return pointer
BufferPoolManager* buffer_pool_manager_create();

// free given BufferPoolManager and all underlying structure
void buffer_pool_manager_destroy(BufferPoolManager* pool);

// find/create writable buffer of given size
// IMPORTANT: returned GstBuffer is writable (i.e. only pool manager holds its reference)
// unref'ing it will cause undefined behavior
GstBuffer* buffer_pool_manager_acquire_buffer(BufferPoolManager* mngr, gsize buffer_size);

// get number of allocated buffers (use only for debug)
size_t buffer_pool_manager_count(BufferPoolManager* mngr);

// get number of allocated buffers in use (use only for debug)
size_t buffer_pool_manager_count_in_use(BufferPoolManager* mngr);

// NOT YET IMPLEMENTED
// remove buffer pools holding buffers of unused size
// void buffer_pool_manager_gc()

#endif // GST_TELEMETRY_BUFFER_POOL_H