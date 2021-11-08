#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include "gstanytimeset.h"

#define PACKAGE "gstanytime"

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "anytimeset", GST_RANK_NONE,
	  GST_TYPE_ANYTIME_SET))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    anytime,
    "atime Streaming features",
    plugin_init, "1.0.0", GST_LICENSE_UNKNOWN, PACKAGE, "Elbit Systems")
