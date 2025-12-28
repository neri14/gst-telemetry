#ifndef _GST_TELEMETRY_H_
#define _GST_TELEMETRY_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

#include "api/backend_c_api.h"

G_BEGIN_DECLS

#define GST_TYPE_TELEMETRY   (gst_telemetry_get_type())
#define GST_TELEMETRY(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TELEMETRY,GstTelemetry))
#define GST_TELEMETRY_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TELEMETRY,GstTelemetryClass))
#define GST_IS_TELEMETRY(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TELEMETRY))
#define GST_IS_TELEMETRY_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TELEMETRY))

typedef struct _GstTelemetry GstTelemetry;
typedef struct _GstTelemetryClass GstTelemetryClass;

struct _GstTelemetry
{
  GstVideoFilter base_telemetry;

  ManagerHandle *manager;
  long initial_timestamp;

  float offset;
  char *track;
  char *layout;

  gboolean gl_mode;
};

struct _GstTelemetryClass
{
  GstVideoFilterClass base_telemetry_class;
};

GType gst_telemetry_get_type (void);

G_END_DECLS

#endif
