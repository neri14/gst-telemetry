#ifndef TRACE_H
#define TRACE_H


typedef enum {
    EVT_BEGIN,
    EVT_END,
    EVT_INSTANT,
} trace_event_type_t;


typedef enum {
#define TRACE_CAT(id, name) id,
#include "trace_categories.h"
#undef TRACE_CAT
    TRACE_CAT_COUNT
} trace_category_t;

#ifdef __cplusplus
static_assert(TRACE_CAT_COUNT <= 256, "TRACE_CAT_COUNT exceeds 256");
#else
_Static_assert(TRACE_CAT_COUNT <= 256, "TRACE_CAT_COUNT exceeds 256");
#endif


typedef enum {
#define TRACE_EVENT_NAME(id, name) id,
#include "trace_event_names.h"
#undef TRACE_EVENT_NAME
    TRACE_EVENT_NAME_COUNT
} trace_event_names_t;

#ifdef __cplusplus
static_assert(TRACE_EVENT_NAME_COUNT <= 65536, "TRACE_EVENT_NAME_COUNT exceeds 65536");
#else
_Static_assert(TRACE_EVENT_NAME_COUNT <= 65536, "TRACE_EVENT_NAME_COUNT exceeds 65536");
#endif


#ifdef ENABLE_TRACING

#ifdef __cplusplus
extern "C" {
#endif

void trace_init();
void trace_deinit();
void trace_event(trace_event_type_t type, trace_category_t category, trace_event_names_t event);

#ifdef __cplusplus
} // extern "C"
#endif

#define TRACE_INIT() \
    trace_init();

#define TRACE_DEINIT() \
    trace_deinit();

#define TRACE_EVENT(type, category, event) \
    trace_event(type, category, event);

#define TRACE_EVENT_BEGIN(category, event) \
    TRACE_EVENT(EVT_BEGIN, category, event)

#define TRACE_EVENT_END(category, event) \
    TRACE_EVENT(EVT_END, category, event)

#define TRACE_EVENT_INSTANT(category, event) \
    TRACE_EVENT(EVT_INSTANT, category, event)

#else // ENABLE_TRACING

#define TRACE_INIT() // empty

#define TRACE_DEINIT() // empty

#endif // ENABLE_TRACING


#endif // TRACE_H
