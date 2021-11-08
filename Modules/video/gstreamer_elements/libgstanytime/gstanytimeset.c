#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstanytimeset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <libgstatime/gstatimemeta.h>

GST_DEBUG_CATEGORY_STATIC (anytimeset_debug);
#define GST_CAT_DEFAULT (anytimeset_debug)

static GstStaticPadTemplate sink_template_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_template_factory =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

G_DEFINE_TYPE (GstAnytimeSet, gst_anytime_set, GST_TYPE_BASE_TRANSFORM);

static GstFlowReturn handle_transform_ip(GstBaseTransform *trans, GstBuffer *buf);

static void gst_anytime_set_class_init (GstAnytimeSetClass* klass)
{
  GstElementClass* element_class = GST_ELEMENT_CLASS(klass);
  GstBaseTransformClass* transform_class = GST_BASE_TRANSFORM_CLASS(klass);

  /* register pads */
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template_factory));

  gst_element_class_set_static_metadata (element_class,
      "atime NTP timestamps extension", "Effect/ANY",
      "Add absolute timestamps in a playback "
      "session", "David Agami <david.agami@elbitsystems.com>");      
      
  GST_DEBUG_CATEGORY_INIT (anytimeset_debug, "anytimeset",
      0, "atime timestamp extension");  

  transform_class->passthrough_on_same_caps = FALSE;
  transform_class->transform_ip = handle_transform_ip;
}

static void gst_anytime_set_init(GstAnytimeSet* self)
{
}

static gboolean handle_buffer(GstAnytimeSet* self, GstBuffer * buf)
{
	guint8 flags;

	GstClockTime timestamp = 0;
	struct timespec tms;

#if defined(_WIN32) && defined(_MSC_VER)
	/* The C11 way */
	if (!timespec_get(&tms, TIME_UTC))
	{
		timestamp = 0;
	}
#else
	/* POSIX.1-2008 way */
	if (clock_gettime(CLOCK_REALTIME, &tms))
	{
		timestamp = 0;
	}
#endif
	else
	{
		/* seconds, multiplied with 1 million */
		int64_t micros = tms.tv_sec * 1000000;
		/* Add full microseconds */
		micros += tms.tv_nsec / 1000;
		/* round up if necessary */
		if (tms.tv_nsec % 1000 >= 500) {
			++micros;
		}

		timestamp = (GstClockTime)micros;
	}

	flags = 0;

	/* convert timestamp */

	GST_DEBUG_OBJECT(self, "timestamp: %" G_GUINT64_FORMAT, timestamp);
	GST_DEBUG_OBJECT(self, "PTS: %" G_GUINT64_FORMAT, buf->pts);

	/* convert back to NTP time. upper 32 bits should contain the seconds
	 * and the lower 32 bits, the fractions of a second.
	 * Thus timestamp = time * denom/num
	 * */
	 /*timestamp = gst_util_uint64_scale (timestamp, GST_SECOND,
		   (G_GINT64_CONSTANT (1) << 32));

	 GST_DEBUG_OBJECT (self, "converted timestamp: %" G_GUINT64_FORMAT, timestamp);  */

	if (gst_buffer_is_writable(buf))
		gst_buffer_add_atime_meta(buf, timestamp);

	GST_DEBUG_OBJECT(self, "Added meta");

	/* C */
	if (flags & (1 << 7))
		GST_BUFFER_FLAG_UNSET(buf, GST_BUFFER_FLAG_DELTA_UNIT);
	else
		GST_BUFFER_FLAG_SET(buf, GST_BUFFER_FLAG_DELTA_UNIT);

	/* E */
	/* if (flags & (1 << 6));  TODO */

	/* D */
	if (flags & (1 << 5))
		GST_BUFFER_FLAG_SET(buf, GST_BUFFER_FLAG_DISCONT);
	else
		GST_BUFFER_FLAG_UNSET(buf, GST_BUFFER_FLAG_DISCONT);

	GST_DEBUG_OBJECT(self, "PTS: %" G_GUINT64_FORMAT, buf->pts);
	return TRUE;
}

static GstFlowReturn handle_transform_ip(GstBaseTransform *trans, GstBuffer *buf)
{
	GstAnytimeSet* self = GST_ANYTIME_SET(trans);

	if (!handle_buffer(self, buf)) {
		gst_buffer_unref(buf);
		return GST_FLOW_ERROR;
	}

	return GST_FLOW_OK;
}