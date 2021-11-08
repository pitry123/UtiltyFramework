#ifndef _GST_SHM_SINK_H_
#define _GST_SHM_SINK_H_

// gstreamer includes
#ifdef __GNUC__
    // Avoid tons of warnings with root code
    #pragma GCC system_header
#endif

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>
#include <gst/gstbuffer.h>
#include <gst/gstbufferlist.h>
#include <gst/gstquery.h>

G_BEGIN_DECLS

#define GST_TYPE_SHM_SINK \
  (gst_shm_sink_get_type())
#define GST_SHM_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_SHM_SINK,GstShmSink))
#define GST_SHM_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_SHM_SINK,GstShmSinkClass))
#define GST_IS_SHM_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_SHM_SINK))
#define GST_IS_SHM_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_SHM_SINK))
#define GST_SHM_SINK_CAST(obj) \
  ((GstShmSink*)(obj))

typedef struct _GstShmSink GstShmSink;
typedef struct _GstShmSinkClass GstShmSinkClass;
typedef struct _GstShmSinkPrivate GstShmSinkPrivate;

typedef struct 
{
	void(*eos)						(GstShmSink *shmsink, gpointer user_data);
	GstFlowReturn(*new_sample)		(GstShmSink *shmsink, gpointer user_data);

	/*< private >*/
	gpointer	_gst_reserved[GST_PADDING];
} GstShmSinkCallbacks;

struct _GstShmSink
{
	GstBaseSink basesink;

	/*< private >*/
	GstShmSinkPrivate *priv;

	/*< private >*/
	gpointer     _gst_reserved[GST_PADDING];
};

struct _GstShmSinkClass
{
	GstBaseSinkClass basesink_class;

	/* signals */
	void(*eos)							(GstShmSink *shmsink);
	GstFlowReturn(*new_sample)			(GstShmSink *shmsink);

	/*< private >*/
	gpointer	_gst_reserved[GST_PADDING - 2];
};

GType gst_shm_sink_get_type(void);

void gst_shm_sink_set_caps(GstShmSink *shmsink, const GstCaps *caps);
GstCaps* gst_shm_sink_get_caps(GstShmSink *shmsink);

gboolean gst_shm_sink_is_eos(GstShmSink *shmsink);

void gst_shm_sink_set_emit_signals(GstShmSink *shmsink, gboolean emit);
gboolean gst_shm_sink_get_emit_signals(GstShmSink *shmsink);

void gst_shm_sink_set_callbacks(GstShmSink* shmsink,
	GstShmSinkCallbacks *callbacks,
	gpointer user_data,
	GDestroyNotify notify);

#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstShmSink, gst_object_unref)
#endif

G_END_DECLS

#endif /* _GST_SHM_SINK_H_ */
