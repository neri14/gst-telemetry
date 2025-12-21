/**
 * SECTION:element-gsttelemetry
 *
 * Overlays telemetry data from provided telemetry file onto video stream.
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include "gsttelemetry.h"

GST_DEBUG_CATEGORY_STATIC (gst_telemetry_debug_category);
#define GST_CAT_DEFAULT gst_telemetry_debug_category

/* prototypes */

static void gst_telemetry_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_telemetry_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_telemetry_dispose (GObject * object);
static void gst_telemetry_finalize (GObject * object);

static gboolean gst_telemetry_start (GstBaseTransform * trans);
static gboolean gst_telemetry_stop (GstBaseTransform * trans);
static gboolean gst_telemetry_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);
static GstFlowReturn gst_telemetry_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe);
static GstFlowReturn gst_telemetry_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * frame);

enum
{
  PROP_0,
  PROP_OFFSET,
  PROP_TRACK,
  PROP_LAYOUT,
};

/* pad templates */

/* FIXME: add/remove formats you can handle */
#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("{ I420, Y444, Y42B, UYVY, RGBA }")

/* FIXME: add/remove formats you can handle */
#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE("{ I420, Y444, Y42B, UYVY, RGBA }")


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTelemetry, gst_telemetry, GST_TYPE_VIDEO_FILTER,
  GST_DEBUG_CATEGORY_INIT (gst_telemetry_debug_category, "telemetry", 0,
  "debug category for telemetry element"));

static void
gst_telemetry_class_init (GstTelemetryClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "Telemetry Overlay", "Mixer/Video/Text", "Overlay telemetry data on video frames",
      "Przemyslaw Rokosz <neri@n3r1.com>");

  gobject_class->set_property = gst_telemetry_set_property;
  gobject_class->get_property = gst_telemetry_get_property;
  gobject_class->dispose = gst_telemetry_dispose;
  gobject_class->finalize = gst_telemetry_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_telemetry_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_telemetry_stop);
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_telemetry_set_info);
  video_filter_class->transform_frame = GST_DEBUG_FUNCPTR (gst_telemetry_transform_frame);
  video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (gst_telemetry_transform_frame_ip);

  g_object_class_install_property (gobject_class, PROP_OFFSET,
      g_param_spec_float ("offset", "Offset",
        "Telemetry offset in seconds (time of telemtry start relative to video start)",
        -G_MAXFLOAT, G_MAXFLOAT, 0.0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_TRACK,
      g_param_spec_string ("track", "Track",
        "Path to GPS track file (GPX format)",
        NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_LAYOUT,
      g_param_spec_string ("layout", "Layout",
        "Path to overlay layout XML file",
        NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void
gst_telemetry_init (GstTelemetry *telemetry)
{
  telemetry->offset = 0.0;
  telemetry->layout = NULL;
  telemetry->track = NULL;

  telemetry->manager = manager_new ();
}

void
gst_telemetry_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstTelemetry *telemetry = GST_TELEMETRY (object);

  GST_DEBUG_OBJECT (telemetry, "set_property");

  switch (property_id) {
    case PROP_OFFSET:
      GST_OBJECT_LOCK (telemetry);
      telemetry->offset = g_value_get_float (value);
      GST_OBJECT_UNLOCK (telemetry);
      break;
    case PROP_TRACK:
      GST_OBJECT_LOCK (telemetry);
      g_free(telemetry->track);
      telemetry->track = g_value_dup_string (value);
      GST_OBJECT_UNLOCK (telemetry);
      break;
    case PROP_LAYOUT:
      GST_OBJECT_LOCK (telemetry);
      g_free(telemetry->layout);
      telemetry->layout = g_value_dup_string (value);
      GST_OBJECT_UNLOCK (telemetry);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_telemetry_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstTelemetry *telemetry = GST_TELEMETRY (object);

  GST_DEBUG_OBJECT (telemetry, "get_property");

  switch (property_id) {
    case PROP_OFFSET:
      GST_OBJECT_LOCK (telemetry);
      g_value_set_float (value, telemetry->offset);
      GST_OBJECT_UNLOCK (telemetry);
      break;
    case PROP_TRACK:
      GST_OBJECT_LOCK (telemetry);
      g_value_set_string (value, telemetry->track);
      GST_OBJECT_UNLOCK (telemetry);
      break;
    case PROP_LAYOUT:
      GST_OBJECT_LOCK (telemetry);
      g_value_set_string (value, telemetry->layout);
      GST_OBJECT_UNLOCK (telemetry);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_telemetry_dispose (GObject * object)
{
  GstTelemetry *telemetry = GST_TELEMETRY (object);

  GST_DEBUG_OBJECT (telemetry, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_telemetry_parent_class)->dispose (object);
}

void
gst_telemetry_finalize (GObject * object)
{
  GstTelemetry *telemetry = GST_TELEMETRY (object);

  GST_DEBUG_OBJECT (telemetry, "finalize");

  /* clean up object here */
  manager_free (telemetry->manager);

  G_OBJECT_CLASS (gst_telemetry_parent_class)->finalize (object);
}

static gboolean
gst_telemetry_start (GstBaseTransform * trans)
{
  GstTelemetry *telemetry = GST_TELEMETRY (trans);

  GST_DEBUG_OBJECT (telemetry, "start");

  int ret = 0;

  GST_OBJECT_LOCK (telemetry);
  ret = manager_init (telemetry->manager, telemetry->offset, telemetry->track, telemetry->layout);
  GST_OBJECT_UNLOCK (telemetry);

  if (ret != 0) {
    GST_ERROR_OBJECT (telemetry, "Failed to initialize telemetry manager");
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_telemetry_stop (GstBaseTransform * trans)
{
  GstTelemetry *telemetry = GST_TELEMETRY (trans);

  GST_DEBUG_OBJECT (telemetry, "stop");

  int ret = 0;

  GST_OBJECT_LOCK (telemetry);
  ret = manager_deinit (telemetry->manager);
  GST_OBJECT_UNLOCK (telemetry);

  if (ret != 0) {
    GST_ERROR_OBJECT (telemetry, "Failed to deinitialize telemetry manager");
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_telemetry_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstTelemetry *telemetry = GST_TELEMETRY (filter);

  GST_DEBUG_OBJECT (telemetry, "set_info");

  return TRUE;
}

/* transform */
static GstFlowReturn
gst_telemetry_transform_frame (GstVideoFilter * filter, GstVideoFrame * inframe,
    GstVideoFrame * outframe)
{
  GstTelemetry *telemetry = GST_TELEMETRY (filter);

  GST_DEBUG_OBJECT (telemetry, "transform_frame");

  gst_video_frame_copy(outframe, inframe);//FIXME for now do nothing with the frame

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_telemetry_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  GstTelemetry *telemetry = GST_TELEMETRY (filter);

  GST_DEBUG_OBJECT (telemetry, "transform_frame_ip");

  return GST_FLOW_OK;
}
