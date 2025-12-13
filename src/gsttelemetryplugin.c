/* SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 Przemyslaw Rokosz <neri@n3r1.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#define PACKAGE "undefined"
#define VERSION "undefined"
#endif

#include <gst/gst.h>
#include "gsttelemetry.h"

static gboolean
plugin_init (GstPlugin * plugin)
{
  gboolean result = TRUE;
  result = result && gst_element_register (
    plugin, "telemetry", GST_RANK_NONE, GST_TYPE_TELEMETRY);
  return result;
}

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    telemetry,
    "Video telemetry plugin",
    plugin_init,
    VERSION,
    "MIT",
    PACKAGE,
    "https://github.com/neri14/gst-telemetry"
)
