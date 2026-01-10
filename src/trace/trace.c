#include "trace.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdatomic.h>

#ifdef ENABLE_TRACING

#ifndef TRACE_SIZE
#define TRACE_SIZE (32*1024*1024) // 32 MB
#endif // TRACE_SIZE

typedef struct Event {
    uint64_t timestamp;
    pid_t thread_id;
    uint8_t type;
    uint8_t category;
    uint16_t event;
} Event;

static atomic_uint_fast32_t trace_count = 0;
static Event *trace_buffer = NULL;
static uint32_t trace_buffer_size = 0;

const char *trace_catorgy_names[TRACE_CAT_COUNT] = {
#define TRACE_CAT(id, name) name,
#include "trace_categories.h"
#undef TRACE_CAT
};

void trace_init() {
    trace_buffer_size = TRACE_SIZE / sizeof(Event);
    trace_buffer = (Event *)malloc(trace_buffer_size * sizeof(Event));
    if (!trace_buffer) {
        fprintf(stderr, "Failed to allocate trace buffer\n");
        return;
    }

    printf("Tracing initialized with buffer size: %.2f MB (%u events)\n", (TRACE_SIZE / 1048576.0), trace_buffer_size);
    (void)trace_count;
}

void trace_deinit() {
    printf("Total trace events recorded: %u of %u max\n", (uint32_t)atomic_load(&trace_count), trace_buffer_size);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    pid_t pid = syscall(SYS_getpid);
    char filename[256];
    snprintf(filename, sizeof(filename), "trace_%ld_%d.json", (long)tv.tv_sec, pid);

    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "[\n");

        for (uint32_t i = 0; i < atomic_load(&trace_count); i++) {
            Event *e = &trace_buffer[i];
            fprintf(file, "{\"name\": \"%s\", \"ph\": \"%c\", \"ts\": %lu, \"pid\": %d, \"tid\": %d}",
                    trace_catorgy_names[e->category],
                    e->type == EVT_BEGIN ? 'B' : (e->type == EVT_END ? 'E' : 'i'),
                    e->timestamp,
                    pid,
                    e->thread_id);
            if (i + 1 < atomic_load(&trace_count)) {
                fprintf(file, ",\n");
            }
        }

        fprintf(file, "\n]\n");
        fclose(file);
        printf("Trace output written to %s\n", filename);
    } else {
        fprintf(stderr, "Failed to open trace output file\n");
    }


    free(trace_buffer);
    trace_buffer = NULL;
    trace_buffer_size = 0;
    printf("Tracing deinitialized\n");
}

void trace_event(trace_event_type_t type, trace_category_t category, trace_event_names_t event) {
    if (!trace_buffer) {
        return;
    }

    uint_fast32_t idx;
    do {
        idx = atomic_load(&trace_count);
        if (idx >= trace_buffer_size) {
            // Trace buffer full, drop event
            return;
        }
    } while (!atomic_compare_exchange_weak(&trace_count, &idx, idx + 1));

    struct timeval tv;
    gettimeofday(&tv, NULL);

    Event *e = &trace_buffer[idx];
    e->timestamp = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
    e->thread_id = syscall(SYS_gettid);
    e->type = (uint8_t)type;
    e->category = (uint8_t)category;
    e->event = (uint16_t)event;
}

#endif // ENABLE_TRACING
