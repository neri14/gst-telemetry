/* SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 Przemyslaw Rokosz <neri@n3r1.com>
 */

/**
 * SECTION:element-gsttelemetryoverlay
 *
 * The telemetryoverlay element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! telemetryoverlay ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gsttelemetryoverlay.h"

GST_DEBUG_CATEGORY_STATIC (gst_telemetry_overlay_debug_category);
#define GST_CAT_DEFAULT gst_telemetry_overlay_debug_category

/* prototypes */


static void gst_telemetry_overlay_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_telemetry_overlay_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_telemetry_overlay_dispose (GObject * object);
static void gst_telemetry_overlay_finalize (GObject * object);

static GstCaps *gst_telemetry_overlay_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter);
static GstCaps *gst_telemetry_overlay_fixate_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * othercaps);
static gboolean gst_telemetry_overlay_accept_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps);
static gboolean gst_telemetry_overlay_set_caps (GstBaseTransform * trans,
    GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_telemetry_overlay_query (GstBaseTransform * trans,
    GstPadDirection direction, GstQuery * query);
static gboolean gst_telemetry_overlay_decide_allocation (GstBaseTransform * trans,
    GstQuery * query);
static gboolean gst_telemetry_overlay_filter_meta (GstBaseTransform * trans,
    GstQuery * query, GType api, const GstStructure * params);
static gboolean gst_telemetry_overlay_propose_allocation (GstBaseTransform * trans,
    GstQuery * decide_query, GstQuery * query);
static gboolean gst_telemetry_overlay_transform_size (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, gsize size, GstCaps * othercaps,
    gsize * othersize);
static gboolean gst_telemetry_overlay_get_unit_size (GstBaseTransform * trans,
    GstCaps * caps, gsize * size);
static gboolean gst_telemetry_overlay_start (GstBaseTransform * trans);
static gboolean gst_telemetry_overlay_stop (GstBaseTransform * trans);
static gboolean gst_telemetry_overlay_sink_event (GstBaseTransform * trans,
    GstEvent * event);
static gboolean gst_telemetry_overlay_src_event (GstBaseTransform * trans,
    GstEvent * event);
static GstFlowReturn gst_telemetry_overlay_prepare_output_buffer (GstBaseTransform *
    trans, GstBuffer * input, GstBuffer ** outbuf);
static gboolean gst_telemetry_overlay_copy_metadata (GstBaseTransform * trans,
    GstBuffer * input, GstBuffer * outbuf);
static gboolean gst_telemetry_overlay_transform_meta (GstBaseTransform * trans,
    GstBuffer * outbuf, GstMeta * meta, GstBuffer * inbuf);
static void gst_telemetry_overlay_before_transform (GstBaseTransform * trans,
    GstBuffer * buffer);
static GstFlowReturn gst_telemetry_overlay_transform (GstBaseTransform * trans,
    GstBuffer * inbuf, GstBuffer * outbuf);
static GstFlowReturn gst_telemetry_overlay_transform_ip (GstBaseTransform * trans,
    GstBuffer * buf);

enum
{
  PROP_0
};

/* pad templates */

static GstStaticPadTemplate gst_telemetry_overlay_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/unknown")
    );

static GstStaticPadTemplate gst_telemetry_overlay_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/unknown")
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTelemetryOverlay, gst_telemetry_overlay, GST_TYPE_BASE_TRANSFORM,
  GST_DEBUG_CATEGORY_INIT (gst_telemetry_overlay_debug_category, "telemetryoverlay", 0,
  "debug category for telemetryoverlay element"));

static void
gst_telemetry_overlay_class_init (GstTelemetryOverlayClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_telemetry_overlay_src_template);
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_telemetry_overlay_sink_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "Telemetry Overlay", "Mixer/Video/Text", "Overlay telemetry data on video frames",
      "Przemyslaw Rokosz <neri@n3r1.com>");

  gobject_class->set_property = gst_telemetry_overlay_set_property;
  gobject_class->get_property = gst_telemetry_overlay_get_property;
  gobject_class->dispose = gst_telemetry_overlay_dispose;
  gobject_class->finalize = gst_telemetry_overlay_finalize;
  base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_transform_caps);
  base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_fixate_caps);
  base_transform_class->accept_caps = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_accept_caps);
  base_transform_class->set_caps = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_set_caps);
  base_transform_class->query = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_query);
  base_transform_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_decide_allocation);
  base_transform_class->filter_meta = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_filter_meta);
  base_transform_class->propose_allocation = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_propose_allocation);
  base_transform_class->transform_size = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_transform_size);
  base_transform_class->get_unit_size = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_get_unit_size);
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_stop);
  base_transform_class->sink_event = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_sink_event);
  base_transform_class->src_event = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_src_event);
  base_transform_class->prepare_output_buffer = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_prepare_output_buffer);
  base_transform_class->copy_metadata = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_copy_metadata);
  base_transform_class->transform_meta = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_transform_meta);
  base_transform_class->before_transform = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_before_transform);
  base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_transform);
  base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_telemetry_overlay_transform_ip);

}

static void
gst_telemetry_overlay_init (GstTelemetryOverlay *telemetryoverlay)
{
}

void
gst_telemetry_overlay_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (object);

  GST_DEBUG_OBJECT (telemetryoverlay, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_telemetry_overlay_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (object);

  GST_DEBUG_OBJECT (telemetryoverlay, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_telemetry_overlay_dispose (GObject * object)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (object);

  GST_DEBUG_OBJECT (telemetryoverlay, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_telemetry_overlay_parent_class)->dispose (object);
}

void
gst_telemetry_overlay_finalize (GObject * object)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (object);

  GST_DEBUG_OBJECT (telemetryoverlay, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_telemetry_overlay_parent_class)->finalize (object);
}

static GstCaps *
gst_telemetry_overlay_transform_caps (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps, GstCaps * filter)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);
  GstCaps *othercaps;

  GST_DEBUG_OBJECT (telemetryoverlay, "transform_caps");

  othercaps = gst_caps_copy (caps);

  /* Copy other caps and modify as appropriate */
  /* This works for the simplest cases, where the transform modifies one
   * or more fields in the caps structure.  It does not work correctly
   * if passthrough caps are preferred. */
  if (direction == GST_PAD_SRC) {
    /* transform caps going upstream */
  } else {
    /* transform caps going downstream */
  }

  if (filter) {
    GstCaps *intersect;

    intersect = gst_caps_intersect (othercaps, filter);
    gst_caps_unref (othercaps);

    return intersect;
  } else {
    return othercaps;
  }
}

static GstCaps *
gst_telemetry_overlay_fixate_caps (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps, GstCaps * othercaps)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "fixate_caps");

  return NULL;
}

static gboolean
gst_telemetry_overlay_accept_caps (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "accept_caps");

  return TRUE;
}

static gboolean
gst_telemetry_overlay_set_caps (GstBaseTransform * trans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "set_caps");

  return TRUE;
}

static gboolean
gst_telemetry_overlay_query (GstBaseTransform * trans, GstPadDirection direction,
    GstQuery * query)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "query");

  return TRUE;
}

/* decide allocation query for output buffers */
static gboolean
gst_telemetry_overlay_decide_allocation (GstBaseTransform * trans, GstQuery * query)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "decide_allocation");

  return TRUE;
}

static gboolean
gst_telemetry_overlay_filter_meta (GstBaseTransform * trans, GstQuery * query, GType api,
    const GstStructure * params)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "filter_meta");

  return TRUE;
}

/* propose allocation query parameters for input buffers */
static gboolean
gst_telemetry_overlay_propose_allocation (GstBaseTransform * trans,
    GstQuery * decide_query, GstQuery * query)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "propose_allocation");

  return TRUE;
}

/* transform size */
static gboolean
gst_telemetry_overlay_transform_size (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps, gsize size, GstCaps * othercaps, gsize * othersize)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "transform_size");

  return TRUE;
}

static gboolean
gst_telemetry_overlay_get_unit_size (GstBaseTransform * trans, GstCaps * caps,
    gsize * size)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "get_unit_size");

  return TRUE;
}

/* states */
static gboolean
gst_telemetry_overlay_start (GstBaseTransform * trans)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "start");

  return TRUE;
}

static gboolean
gst_telemetry_overlay_stop (GstBaseTransform * trans)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "stop");

  return TRUE;
}

/* sink and src pad event handlers */
static gboolean
gst_telemetry_overlay_sink_event (GstBaseTransform * trans, GstEvent * event)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "sink_event");

  return GST_BASE_TRANSFORM_CLASS (gst_telemetry_overlay_parent_class)->sink_event (
      trans, event);
}

static gboolean
gst_telemetry_overlay_src_event (GstBaseTransform * trans, GstEvent * event)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "src_event");

  return GST_BASE_TRANSFORM_CLASS (gst_telemetry_overlay_parent_class)->src_event (
      trans, event);
}

static GstFlowReturn
gst_telemetry_overlay_prepare_output_buffer (GstBaseTransform * trans, GstBuffer * input,
    GstBuffer ** outbuf)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "prepare_output_buffer");

  return GST_FLOW_OK;
}

/* metadata */
static gboolean
gst_telemetry_overlay_copy_metadata (GstBaseTransform * trans, GstBuffer * input,
    GstBuffer * outbuf)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "copy_metadata");

  return TRUE;
}

static gboolean
gst_telemetry_overlay_transform_meta (GstBaseTransform * trans, GstBuffer * outbuf,
    GstMeta * meta, GstBuffer * inbuf)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "transform_meta");

  return TRUE;
}

static void
gst_telemetry_overlay_before_transform (GstBaseTransform * trans, GstBuffer * buffer)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "before_transform");

}

/* transform */
static GstFlowReturn
gst_telemetry_overlay_transform (GstBaseTransform * trans, GstBuffer * inbuf,
    GstBuffer * outbuf)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "transform");

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_telemetry_overlay_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
  GstTelemetryOverlay *telemetryoverlay = GST_TELEMETRY_OVERLAY (trans);

  GST_DEBUG_OBJECT (telemetryoverlay, "transform_ip");

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "telemetryoverlay", GST_RANK_NONE,
      GST_TYPE_TELEMETRY_OVERLAY);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    telemetryoverlay,
    "FIXME plugin description",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

