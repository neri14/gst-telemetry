/* SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 Przemyslaw Rokosz <neri@n3r1.com>
 */

#ifndef _GST_TELEMETRY_OVERLAY_H_
#define _GST_TELEMETRY_OVERLAY_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define GST_TYPE_TELEMETRY_OVERLAY   (gst_telemetry_overlay_get_type())
#define GST_TELEMETRY_OVERLAY(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TELEMETRY_OVERLAY,GstTelemetryOverlay))
#define GST_TELEMETRY_OVERLAY_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TELEMETRY_OVERLAY,GstTelemetryOverlayClass))
#define GST_IS_TELEMETRY_OVERLAY(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TELEMETRY_OVERLAY))
#define GST_IS_TELEMETRY_OVERLAY_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TELEMETRY_OVERLAY))

typedef struct _GstTelemetryOverlay GstTelemetryOverlay;
typedef struct _GstTelemetryOverlayClass GstTelemetryOverlayClass;

struct _GstTelemetryOverlay
{
  GstVideoFilter base_telemetryoverlay;

};

struct _GstTelemetryOverlayClass
{
  GstVideoFilterClass base_telemetryoverlay_class;
};

GType gst_telemetry_overlay_get_type (void);

G_END_DECLS

#endif
