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
#include <cairo.h>

#include "gsttelemetry.h"
#include "trace/trace.h"


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
static GstCaps * gst_telemetry_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter);
static gboolean gst_telemetry_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);
static GstFlowReturn gst_telemetry_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * frame);

enum
{
  PROP_0,
  PROP_OFFSET,
  PROP_TRACK,
  PROP_CUSTOM_DATA,
  PROP_LAYOUT,
};

/* pad templates */

#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("{ I420, Y444, Y42B, UYVY, RGBA }") "; " \
    GST_VIDEO_CAPS_MAKE_WITH_FEATURES("memory:GLMemory, meta:GstVideoOverlayComposition", "RGBA") "; " \
    GST_VIDEO_CAPS_MAKE_WITH_FEATURES("memory:GLMemory", "RGBA")

#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE("{ I420, Y444, Y42B, UYVY, RGBA }") "; " \
    GST_VIDEO_CAPS_MAKE_WITH_FEATURES("memory:GLMemory, meta:GstVideoOverlayComposition", "RGBA") "; " \
    GST_VIDEO_CAPS_MAKE_WITH_FEATURES("memory:GLMemory", "RGBA")

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
  base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_telemetry_transform_caps);
  base_transform_class->passthrough_on_same_caps = FALSE;

  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_telemetry_set_info);
  video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (gst_telemetry_transform_frame_ip);

  g_object_class_install_property (gobject_class, PROP_OFFSET,
      g_param_spec_float ("offset", "Offset",
        "Telemetry offset in seconds (time of telemtry start relative to video start)",
        -G_MAXFLOAT, G_MAXFLOAT, 0.0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_TRACK,
      g_param_spec_string ("track", "Track",
        "Path to GPS track file (GPX format)",
        NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CUSTOM_DATA,
      g_param_spec_string ("custom-data", "Custom Data",
        "Path to custom data file (XML format)",
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
  telemetry->initial_timestamp = GST_CLOCK_TIME_NONE;
  telemetry->layout = NULL;
  telemetry->track = NULL;
  telemetry->custom_data = NULL;
  telemetry->gl_mode = FALSE;

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
    case PROP_CUSTOM_DATA:
      GST_OBJECT_LOCK (telemetry);
      g_free(telemetry->custom_data);
      telemetry->custom_data = g_value_dup_string (value);
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
    case PROP_CUSTOM_DATA:
      GST_OBJECT_LOCK (telemetry);
      g_value_set_string (value, telemetry->custom_data);
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
  TRACE_INIT();
  TRACE_EVENT_BEGIN(EV_GST_START);

  gst_base_transform_set_passthrough (trans, FALSE);
  gst_base_transform_set_in_place (trans, TRUE);

  int ret = 0;

  GST_OBJECT_LOCK (telemetry);
  ret = manager_init (telemetry->manager, telemetry->offset, telemetry->track, telemetry->custom_data, telemetry->layout);
  GST_OBJECT_UNLOCK (telemetry);

  TRACE_EVENT_END(EV_GST_START);
  if (ret != 0) {
    GST_ERROR_OBJECT (telemetry, "Failed to initialize telemetry manager");
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_telemetry_stop (GstBaseTransform * trans)
{
  TRACE_EVENT_BEGIN(EV_GST_STOP);
  GstTelemetry *telemetry = GST_TELEMETRY (trans);

  GST_DEBUG_OBJECT (telemetry, "stop");

  int ret = 0;

  GST_OBJECT_LOCK (telemetry);
  ret = manager_deinit (telemetry->manager);
  GST_OBJECT_UNLOCK (telemetry);

  TRACE_EVENT_END(EV_GST_STOP);
  TRACE_DEINIT();

  if (ret != 0) {
    GST_ERROR_OBJECT (telemetry, "Failed to deinitialize telemetry manager");
    return FALSE;
  }
  return TRUE;
}

static GstCaps *
gst_telemetry_transform_caps (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps, GstCaps * filter)
{
  GstTelemetry *telemetry = GST_TELEMETRY (trans);
  GstCaps *result, *tmp;
  guint i;
  
  GST_DEBUG_OBJECT (telemetry, "transform_caps direction: %s", 
                    direction == GST_PAD_SINK ? "sink->src" : "src->sink");

  result = gst_caps_new_empty ();
  
  for (i = 0; i < gst_caps_get_size (caps); i++) {
    GstStructure *structure = gst_caps_get_structure (caps, i);
    GstCapsFeatures *features = gst_caps_get_features (caps, i);
    GstCapsFeatures *new_features;
    
    // Add the original caps
    tmp = gst_caps_new_full (gst_structure_copy (structure), NULL);
    if (features) {
      new_features = gst_caps_features_copy (features);
      gst_caps_set_features (tmp, 0, new_features);
    }
    result = gst_caps_merge (result, tmp);
    
    // If transforming from sink to src (downstream), also advertise that we can ADD overlay composition
    if (direction == GST_PAD_SINK) {
      // Check if it's a GL memory caps
      if (features && gst_caps_features_contains (features, "memory:GLMemory")) {
        // If it doesn't already have the overlay composition meta, add a variant that does
        if (!gst_caps_features_contains (features, "meta:GstVideoOverlayComposition")) {
          tmp = gst_caps_new_full (gst_structure_copy (structure), NULL);
          new_features = gst_caps_features_copy (features);
          gst_caps_features_add (new_features, "meta:GstVideoOverlayComposition");
          gst_caps_set_features (tmp, 0, new_features);
          result = gst_caps_merge (result, tmp);
        }
      }
    }
    // If transforming from src to sink (upstream), also allow accepting without overlay composition
    else if (direction == GST_PAD_SRC) {
      // Check if it's requesting GL memory with overlay composition
      if (features && 
          gst_caps_features_contains (features, "memory:GLMemory") &&
          gst_caps_features_contains (features, "meta:GstVideoOverlayComposition")) {
        // Also advertise that we can accept without the meta
        tmp = gst_caps_new_full (gst_structure_copy (structure), NULL);
        new_features = gst_caps_features_copy (features);
        gst_caps_features_remove (new_features, "meta:GstVideoOverlayComposition");
        gst_caps_set_features (tmp, 0, new_features);
        result = gst_caps_merge (result, tmp);
      }
    }
  }
  
  // Apply the filter if provided
  if (filter) {
    tmp = gst_caps_intersect_full (result, filter, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (result);
    result = tmp;
  }
  
  gchar *result_str = gst_caps_to_string (result);
  GST_DEBUG_OBJECT (telemetry, "transformed caps: %s", result_str);
  g_free (result_str);
  
  return result;
}

static gboolean
gst_telemetry_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstTelemetry *telemetry = GST_TELEMETRY (filter);
  GST_DEBUG_OBJECT (telemetry, "set_info");

  // Debug print input and output caps
  gchar *incaps_str = gst_caps_to_string (incaps);
  gchar *outcaps_str = gst_caps_to_string (outcaps);
  
  GST_INFO_OBJECT (telemetry, "Input caps: %s", incaps_str);
  GST_INFO_OBJECT (telemetry, "Output caps: %s", outcaps_str);
  
  g_free (incaps_str);
  g_free (outcaps_str);

  GstCapsFeatures *features = gst_caps_get_features (incaps, 0);
  if (features && gst_caps_features_contains (features, "memory:GLMemory")) {
    telemetry->gl_mode = TRUE;
    GST_INFO_OBJECT (telemetry, "GL pipeline detected, operating in GL mode");
  } else {
    telemetry->gl_mode = FALSE;
    GST_INFO_OBJECT (telemetry, "GL pipeline not detected, operating in CPU mode");
  }

  return TRUE;
}

/* transform */
static GstFlowReturn
gst_telemetry_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  TRACE_EVENT_BEGIN(EV_GST_TRANSFORM_FRAME);

  GstTelemetry *telemetry = GST_TELEMETRY (filter);
  GST_DEBUG_OBJECT (telemetry, "transform_frame_ip");

  // Calculate timestamp relative to initial frame
  gint64 timestamp = GST_TIME_AS_USECONDS(GST_BUFFER_PTS(frame->buffer));
  if (telemetry->initial_timestamp == GST_CLOCK_TIME_NONE) {
    telemetry->initial_timestamp = timestamp;
  }
  timestamp -= telemetry->initial_timestamp;

  // Call manager to draw telemetry onto Cairo surface
  GST_DEBUG_OBJECT (telemetry, "drawing telemetry for timestamp: %ld us", timestamp);
  cairo_surface_t *surface = manager_draw(telemetry->manager, timestamp);

  if (surface == NULL) {
    GST_ERROR_OBJECT (telemetry, "Failed to draw telemetry overlay");
    TRACE_EVENT_END(EV_GST_TRANSFORM_FRAME);
    return GST_FLOW_ERROR;
  }

  TRACE_EVENT_BEGIN(EV_GST_PREPARE_BUFFER);
  // Get Cairo surface data and stride
  guint8 *cairo_data = cairo_image_surface_get_data(surface);
  gint cairo_stride = cairo_image_surface_get_stride(surface);
  gint width = cairo_image_surface_get_width(surface);
  gint height = cairo_image_surface_get_height(surface);

  // Create GstBuffer from Cairo surface data
  GstBuffer *overlay_buffer = gst_buffer_new_allocate(NULL, cairo_stride * height, NULL);
  gst_buffer_fill(overlay_buffer, 0, cairo_data, cairo_stride * height);

  // Add video meta
  gst_buffer_add_video_meta(overlay_buffer, GST_VIDEO_FRAME_FLAG_NONE,
                            GST_VIDEO_OVERLAY_COMPOSITION_FORMAT_RGB, width, height);

  TRACE_EVENT_END(EV_GST_PREPARE_BUFFER);

  TRACE_EVENT_BEGIN(EV_GST_PREPARE_COMPOSITION);
  // Create overlay rectangle
  GstVideoOverlayRectangle *rect = gst_video_overlay_rectangle_new_raw(
      overlay_buffer,
      0, 0,  // x, y position on video
      width, height,  // render width, height
      GST_VIDEO_OVERLAY_FORMAT_FLAG_NONE);

  // Create composition
  GstVideoOverlayComposition *comp = gst_video_overlay_composition_new(rect);

  TRACE_EVENT_END(EV_GST_PREPARE_COMPOSITION);

  if (telemetry->gl_mode) {
    // In GL mode, attach as metadata
    GST_DEBUG_OBJECT (telemetry, "Attaching overlay as metadata for GL pipeline");
    gst_buffer_add_video_overlay_composition_meta(frame->buffer, comp);
  } else {
    TRACE_EVENT_BEGIN(EV_GST_BLEND_OVERLAY);
    // In CPU mode, blend directly
    GST_DEBUG_OBJECT (telemetry, "Blending overlay directly in CPU pipeline");
    gst_video_overlay_composition_blend(comp, frame);
    TRACE_EVENT_END(EV_GST_BLEND_OVERLAY);
  }

  //BEGIN ugly hack - hold on to the old composition reference until next frame is drawn;
  //    combination of GPU processing and GST_GL_WINDOW=surfaceless sometimes causes
  //    reference to composition to be lost too early resulting in gloverlaycompositor
  //    to draw old overlay resulting in overlay being choppy (freezing for few seconds)
  static GstVideoOverlayComposition *old_comp = NULL;
  if (old_comp) {
      gst_video_overlay_composition_unref(old_comp);
      old_comp = NULL;
  }
  old_comp = gst_video_overlay_composition_ref(comp);
  //END ugly hack

  TRACE_EVENT_BEGIN(EV_GST_CLEANUP_RESOURCES);
  // Cleanup
  gst_video_overlay_composition_unref(comp);
  gst_video_overlay_rectangle_unref(rect);
  gst_buffer_unref(overlay_buffer);

  TRACE_EVENT_END(EV_GST_CLEANUP_RESOURCES);

  TRACE_EVENT_END(EV_GST_TRANSFORM_FRAME);
  return GST_FLOW_OK;
}
