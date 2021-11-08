#include "gstshmsink.h"
#include "gstshmbufferpool.h"

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4244) // conversion
#	pragma warning(disable: 4996) // deprecated
#endif

#define GLIB_VERSION_RULE() GLIB_CHECK_VERSION(2, 58, 0)

struct _GstShmSinkPrivate
{
	GstCaps *caps;
	gboolean emit_signals;

	GCond cond;
	GMutex mutex;
	GQueue* queue;
	GstCaps* last_caps;
	gboolean flushing;
	gboolean started;
	gboolean is_eos;
	gchar* session_name;
	guint pool_size;
	guint shared_buffer_size;
	GstBufferPool* pool;

	GstShmSinkCallbacks callbacks;
	gpointer user_data;
	GDestroyNotify notify;
};

GST_DEBUG_CATEGORY_STATIC(shm_sink_debug);
#define GST_CAT_DEFAULT shm_sink_debug

enum
{
	/* signals */
	SIGNAL_EOS,
	SIGNAL_NEW_SAMPLE,

	LAST_SIGNAL
};

#define DEFAULT_PROP_EOS					TRUE
#define DEFAULT_PROP_EMIT_SIGNALS			FALSE
#define DEFAULT_PROP_SESSION_NAME			NULL
#define DEFAULT_PROP_POOL_SIZE				20
#define DEFAULT_PROP_SHARED_BUFFER_SIZE		1024*1024*10 /* 10 MBs */

enum
{
	PROP_0,
	PROP_CAPS,
	PROP_EOS,
	PROP_EMIT_SIGNALS,
	PROP_SESSION_NAME,
	PROP_POOL_SIZE,
	PROP_SHARED_BUFFER_SIZE,
	PROP_LAST
};

static GstStaticPadTemplate gst_shm_sink_template =
GST_STATIC_PAD_TEMPLATE("sink",
	GST_PAD_SINK,
	GST_PAD_ALWAYS,
	GST_STATIC_CAPS_ANY);

static void gst_shm_sink_uri_handler_init(gpointer g_iface, gpointer iface_data);
static void gst_shm_sink_dispose(GObject * object);
static void gst_shm_sink_finalize(GObject * object);

static void gst_shm_sink_set_property(
	GObject* object,
	guint prop_id,
	const GValue* value,
	GParamSpec* pspec);

static void gst_shm_sink_get_property(
	GObject* object, 
	guint prop_id,
	GValue* value,
	GParamSpec* pspec);

static gboolean gst_shm_sink_start(GstBaseSink* psink);
static gboolean gst_shm_sink_stop(GstBaseSink* psink);
static gboolean gst_shm_sink_event(GstBaseSink* sink, GstEvent* event);
static gboolean gst_shm_sink_query(GstBaseSink* bsink, GstQuery* query);
static GstFlowReturn gst_shm_sink_render_common(GstBaseSink* psink, GstBuffer* buffer);
static GstFlowReturn gst_shm_sink_render(GstBaseSink * psink,	GstBuffer* buffer);
static GstFlowReturn gst_shm_sink_render_list(GstBaseSink * psink, GstBufferList* list);
static gboolean gst_shm_sink_setcaps(GstBaseSink* sink, GstCaps* caps);
static GstCaps *gst_shm_sink_getcaps(GstBaseSink* psink, GstCaps* filter);

static gboolean gst_shm_sink_set_session_name(GstShmSink* sink, const gchar* name);
static gchar* gst_shm_sink_get_session_name(GstShmSink* psink);
static gboolean gst_shm_sink_set_pool_size(GstShmSink* sink, guint size);
static guint gst_shm_sink_get_pool_size(GstShmSink* psink);
static gboolean gst_shm_sink_set_shared_buffer_size(GstShmSink* sink, guint size);
static guint gst_shm_sink_get_shared_buffer_size(GstShmSink* psink);


gboolean gst_shm_sink_propose_allocation(GstBaseSink* sink, GstQuery* query);

static guint gst_shm_sink_signals[LAST_SIGNAL] = { 0 };

#if GLIB_VERSION_RULE()
#define gst_shm_sink_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE(GstShmSink, gst_shm_sink, GST_TYPE_BASE_SINK,
    G_ADD_PRIVATE(GstShmSink)
    GST_DEBUG_CATEGORY_INIT(shm_sink_debug, "shmsink", 0, "Shared memory sink for Framework 2.0"))
#else
#define gst_shm_sink_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE(GstShmSink, gst_shm_sink, GST_TYPE_BASE_SINK,
    G_IMPLEMENT_INTERFACE(GST_TYPE_URI_HANDLER,
        gst_shm_sink_uri_handler_init))
#endif

// Implementaion starts here...

static void gst_shm_sink_class_init(GstShmSinkClass* klass)
{
	GObjectClass* gobject_class = (GObjectClass*)klass;
	GstElementClass* element_class = (GstElementClass*)klass;
	GstBaseSinkClass* basesink_class = (GstBaseSinkClass*)klass;

#if !GLIB_VERSION_RULE()
	GST_DEBUG_CATEGORY_INIT(shm_sink_debug, "shmsink", 0, "Shared memory sink for Framework 2.0");
#endif

	gobject_class->dispose = gst_shm_sink_dispose;
	gobject_class->finalize = gst_shm_sink_finalize;

	gobject_class->set_property = gst_shm_sink_set_property;
	gobject_class->get_property = gst_shm_sink_get_property;

	g_object_class_install_property(gobject_class, PROP_CAPS,
		g_param_spec_boxed("caps", "Caps",
			"The allowed caps for the sink pad", GST_TYPE_CAPS,
			G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_EOS,
		g_param_spec_boolean("eos", "EOS",
			"Check if the sink is EOS or not started", DEFAULT_PROP_EOS,
			G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_EMIT_SIGNALS,
		g_param_spec_boolean("emit-signals", "Emit signals",
			"Emit EOS and new-sample signals",
			DEFAULT_PROP_EMIT_SIGNALS,
			G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_SESSION_NAME,
		g_param_spec_string("session-name", "Session Name",
			"The name of the shared memory session",
			DEFAULT_PROP_SESSION_NAME,
			G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_POOL_SIZE,
		g_param_spec_uint("pool-size", "Pool Size",
			"Number of shared memory buffers to allocate",
			0, 100, DEFAULT_PROP_POOL_SIZE,
			G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class, PROP_SHARED_BUFFER_SIZE,
		g_param_spec_uint("shared-buffer-size", "Shared Buffer Size",
			"The minimum allocation size of each buffer of the shared memory bufferpool",
			0, G_MAXUINT, DEFAULT_PROP_POOL_SIZE,
			G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	
	/**
	* GstShmSink::eos:
	* @shmsink: the shmsink element that emitted the signal
	*
	* Signal that the end-of-stream has been reached. This signal is emitted from
	* the streaming thread.
	*/
	gst_shm_sink_signals[SIGNAL_EOS] =
		g_signal_new("eos", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(GstShmSinkClass, eos),
			NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);
	
	/**
	* GstShmSink::new-sample:
	* @shmsink: the shmsink element that emited the signal
	*
	* Signal that a new sample is available.
	*
	* This signal is emitted from the streaming thread and only when the
	* "emit-signals" property is %TRUE.
	*
	* The new sample can be retrieved with the "pull-sample" action
	* signal or gst_shm_sink_pull_sample() either from this signal callback
	* or from any other thread.
	*
	* Note that this signal is only emitted when the "emit-signals" property is
	* set to %TRUE, which it is not by default for performance reasons.
	*/
	gst_shm_sink_signals[SIGNAL_NEW_SAMPLE] =
		g_signal_new("new-sample", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(GstShmSinkClass, new_sample),
			NULL, NULL, NULL, GST_TYPE_FLOW_RETURN, 0, G_TYPE_NONE);
		
	gst_element_class_set_static_metadata(element_class, "ElbitShmSink",
		"Generic/Sink", "Publishes raw buffers to applications hosting the shared_memory_video_source (Elbit's Framework 2.0)",
		"Dudi Agami <David.Agami@elbitsystems.com>");

	gst_element_class_add_static_pad_template(element_class,
		&gst_shm_sink_template);

	basesink_class->start = gst_shm_sink_start;
	basesink_class->stop = gst_shm_sink_stop;
	basesink_class->event = gst_shm_sink_event;
	basesink_class->render = gst_shm_sink_render;
	basesink_class->render_list = gst_shm_sink_render_list;
	basesink_class->get_caps = gst_shm_sink_getcaps;
	basesink_class->set_caps = gst_shm_sink_setcaps;
	basesink_class->query = gst_shm_sink_query;
	basesink_class->propose_allocation = gst_shm_sink_propose_allocation;

#if !GLIB_VERSION_RULE()
    g_type_class_add_private(klass, sizeof(GstShmSinkPrivate));
#endif
}

#if GLIB_VERSION_RULE()
#define GST_SHM_SINK_GET_PRIVATE(obj)  \
    (GstShmSinkPrivate*)(gst_shm_sink_get_instance_private(obj))
#else
#define GST_SHM_SINK_GET_PRIVATE(obj)  \
    G_TYPE_INSTANCE_GET_PRIVATE(obj, GST_TYPE_SHM_SINK,	GstShmSinkPrivate)
#endif

static void gst_shm_sink_init(GstShmSink* shmsink)
{
	GstShmSinkPrivate *priv;

    priv = shmsink->priv = GST_SHM_SINK_GET_PRIVATE(shmsink);

	g_mutex_init(&priv->mutex);
	g_cond_init(&priv->cond);
	priv->queue = g_queue_new();

	priv->is_eos = DEFAULT_PROP_EOS;
	priv->emit_signals = DEFAULT_PROP_EMIT_SIGNALS;
	priv->session_name = DEFAULT_PROP_SESSION_NAME;
	priv->pool_size = DEFAULT_PROP_POOL_SIZE;
	priv->shared_buffer_size = DEFAULT_PROP_SHARED_BUFFER_SIZE;
	priv->pool = NULL;
}

static void gst_shm_sink_dispose(GObject * obj)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(obj);
	GstShmSinkPrivate *priv = shmsink->priv;
	GstMiniObject *queue_obj;

	GST_OBJECT_LOCK(shmsink);
	if (priv->caps) {
		gst_caps_unref(priv->caps);
		priv->caps = NULL;
	}
	if (priv->notify) {
		priv->notify(priv->user_data);
	}
	priv->user_data = NULL;
	priv->notify = NULL;

	GST_OBJECT_UNLOCK(shmsink);

	g_mutex_lock(&priv->mutex);
	while ((queue_obj = g_queue_pop_head(priv->queue)))
		gst_mini_object_unref(queue_obj);
	gst_caps_replace(&priv->last_caps, NULL);
	
	if (priv->session_name != NULL)
	{
		g_free(priv->session_name);
		priv->session_name = NULL;
	}

	g_mutex_unlock(&priv->mutex);

	G_OBJECT_CLASS(parent_class)->dispose(obj);
}

static void gst_shm_sink_finalize(GObject * obj)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(obj);
	GstShmSinkPrivate *priv = shmsink->priv;

	g_mutex_clear(&priv->mutex);
	g_cond_clear(&priv->cond);
	g_queue_free(priv->queue);

	if (priv->pool != NULL)
	{
		gst_object_unref(priv->pool);
		priv->pool = NULL;
	}

	G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void gst_shm_sink_set_property(GObject * object, guint prop_id,
	const GValue * value, GParamSpec * pspec)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(object);

	switch (prop_id) {
	case PROP_CAPS:
		gst_shm_sink_set_caps(shmsink, gst_value_get_caps(value));
		break;
	case PROP_EMIT_SIGNALS:
		gst_shm_sink_set_emit_signals(shmsink, g_value_get_boolean(value));
		break;
	case PROP_SESSION_NAME:
		gst_shm_sink_set_session_name(shmsink, g_value_get_string(value));
		break;
	case PROP_POOL_SIZE:
		gst_shm_sink_set_pool_size(shmsink, g_value_get_uint(value));
		break;
	case PROP_SHARED_BUFFER_SIZE:
		gst_shm_sink_set_shared_buffer_size(shmsink, g_value_get_uint(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void gst_shm_sink_get_property(GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(object);

	switch (prop_id) 
	{
	case PROP_CAPS:
	{
		GstCaps *caps;

		caps = gst_shm_sink_get_caps(shmsink);
		gst_value_set_caps(value, caps);
		if (caps)
			gst_caps_unref(caps);
		break;
	}
	case PROP_EOS:
		g_value_set_boolean(value, gst_shm_sink_is_eos(shmsink));
		break;
	case PROP_EMIT_SIGNALS:
		g_value_set_boolean(value, gst_shm_sink_get_emit_signals(shmsink));
		break;
	case PROP_SESSION_NAME:
		g_value_set_string(value, gst_shm_sink_get_session_name(shmsink));
		break;
	case PROP_POOL_SIZE:
		g_value_set_uint(value, gst_shm_sink_get_pool_size(shmsink));
		break;
	case PROP_SHARED_BUFFER_SIZE:
		g_value_set_uint(value, gst_shm_sink_get_shared_buffer_size(shmsink));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void gst_shm_sink_flush_unlocked(GstShmSink * shmsink)
{
	GstMiniObject *obj;
	GstShmSinkPrivate *priv = shmsink->priv;

	GST_DEBUG_OBJECT(shmsink, "flush stop shmsink");
	priv->is_eos = FALSE;
	while ((obj = g_queue_pop_head(priv->queue)))
		gst_mini_object_unref(obj);

	g_cond_signal(&priv->cond);
}

static gboolean gst_shm_sink_start(GstBaseSink * psink)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(psink);
	GstShmSinkPrivate *priv = shmsink->priv;

	g_mutex_lock(&priv->mutex);
	GST_DEBUG_OBJECT(shmsink, "starting");
	priv->flushing = FALSE;
	priv->started = TRUE;
	/*if (priv->session_name == NULL)
	{
		GST_ERROR_OBJECT(psink, "Session Name is not defined");
		return FALSE;
	}*/
	g_mutex_unlock(&priv->mutex);

	return TRUE;
}

static gboolean gst_shm_sink_stop(GstBaseSink * psink)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(psink);
	GstShmSinkPrivate *priv = shmsink->priv;

	g_mutex_lock(&priv->mutex);
	GST_DEBUG_OBJECT(shmsink, "stopping");
	priv->flushing = TRUE;
	priv->started = FALSE;
	gst_shm_sink_flush_unlocked(shmsink);
	gst_caps_replace(&priv->last_caps, NULL);
	g_mutex_unlock(&priv->mutex);

	return TRUE;
}

static gboolean gst_shm_sink_setcaps(GstBaseSink* sink, GstCaps* caps)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate *priv = shmsink->priv;

	g_mutex_lock(&priv->mutex);
	GST_DEBUG_OBJECT(shmsink, "receiving CAPS");
	g_queue_push_tail(priv->queue, gst_event_new_caps(caps));
	g_mutex_unlock(&priv->mutex);

	return TRUE;
}

static gboolean gst_shm_sink_event(GstBaseSink * sink, GstEvent * event)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate *priv = shmsink->priv;

	switch (event->type) 
	{
	case GST_EVENT_SEGMENT:
		g_mutex_lock(&priv->mutex);
		GST_DEBUG_OBJECT(shmsink, "receiving SEGMENT");
		g_queue_push_tail(priv->queue, gst_event_ref(event));
		g_mutex_unlock(&priv->mutex);
		break;
	case GST_EVENT_EOS: 
	{
		gboolean emit = TRUE;

		g_mutex_lock(&priv->mutex);
		GST_DEBUG_OBJECT(shmsink, "receiving EOS");
		priv->is_eos = TRUE;
		g_cond_signal(&priv->cond);
		g_mutex_unlock(&priv->mutex);

		g_mutex_lock(&priv->mutex);

		if (priv->flushing)
			emit = FALSE;

		g_mutex_unlock(&priv->mutex);

		if (emit) 
		{
			/* emit EOS now */
			if (priv->callbacks.eos)
				priv->callbacks.eos(shmsink, priv->user_data);
			else
				g_signal_emit(shmsink, gst_shm_sink_signals[SIGNAL_EOS], 0);
		}

		break;
	}
	case GST_EVENT_FLUSH_START:
		/* we don't have to do anything here, the base class will call unlock
		* which will make sure we exit the _render method */
		GST_DEBUG_OBJECT(shmsink, "received FLUSH_START");
		break;
	case GST_EVENT_FLUSH_STOP:
		g_mutex_lock(&priv->mutex);
		GST_DEBUG_OBJECT(shmsink, "received FLUSH_STOP");
		gst_shm_sink_flush_unlocked(shmsink);
		g_mutex_unlock(&priv->mutex);
		break;
	default:
		break;
	}

	return GST_BASE_SINK_CLASS(parent_class)->event(sink, event);
}

static GstMiniObject* dequeue_buffer(GstShmSink * shmsink)
{
	GstShmSinkPrivate *priv = shmsink->priv;
	GstMiniObject *obj;

	do
	{
		obj = g_queue_pop_head(priv->queue);

		if (GST_IS_EVENT(obj))
		{
			GstEvent *event = GST_EVENT_CAST(obj);

			switch (GST_EVENT_TYPE(obj)) 
			{
			case GST_EVENT_CAPS:
			{
				GstCaps *caps;

				gst_event_parse_caps(event, &caps);
				GST_DEBUG_OBJECT(shmsink, "activating caps %" GST_PTR_FORMAT, caps);
				gst_caps_replace(&priv->last_caps, caps);
				break;
			}
			default:
				break;
			}

			gst_mini_object_unref(obj);
		}
	} while (TRUE);

	return obj;
}

static GstFlowReturn gst_shm_sink_render_common(GstBaseSink * psink, GstBuffer* buffer)
{
	GstFlowReturn ret;
	GstShmSink *shmsink = GST_SHM_SINK_CAST(psink);
	GstShmSinkPrivate *priv = shmsink->priv;
	gboolean emit;

	g_mutex_lock(&priv->mutex);
	if (priv->flushing)
		goto flushing;

	/* queue holding caps event might have been FLUSHed,
	* but caps state still present in pad caps */
	if (G_UNLIKELY(!priv->last_caps &&
		gst_pad_has_current_caps(GST_BASE_SINK_PAD(psink)))) {
		priv->last_caps = gst_pad_get_current_caps(GST_BASE_SINK_PAD(psink));
		GST_DEBUG_OBJECT(shmsink, "activating pad caps %" GST_PTR_FORMAT,
			priv->last_caps);
	}

	gst_shm_buffer_pool_publish(priv->pool, buffer);

	g_cond_signal(&priv->cond);
	emit = priv->emit_signals;
	g_mutex_unlock(&priv->mutex);

	if (priv->callbacks.new_sample)
	{
		ret = priv->callbacks.new_sample(shmsink, priv->user_data);
	}
	else
	{
		ret = GST_FLOW_OK;
		if (emit)
		{
			g_signal_emit(shmsink, gst_shm_sink_signals[SIGNAL_NEW_SAMPLE], 0, &ret);
		}
	}

	return ret;

flushing:
	{
		GST_DEBUG_OBJECT(shmsink, "we are flushing");
		g_mutex_unlock(&priv->mutex);
		return GST_FLOW_FLUSHING;
	}
//stopping:
//	{
//		return ret;
//	}
}

static GstFlowReturn gst_shm_sink_render(GstBaseSink * psink, GstBuffer * buffer)
{
	GstShmSink *shmsink = GST_SHM_SINK_CAST(psink);
	GstShmSinkPrivate *priv = shmsink->priv;

	g_mutex_lock(&priv->mutex);
	if (priv->flushing)
	{
		g_mutex_unlock(&priv->mutex);
		return GST_FLOW_FLUSHING;
	}
	g_mutex_unlock(&priv->mutex);
	return gst_shm_sink_render_common(psink, buffer);
}

static GstFlowReturn gst_shm_sink_render_list(GstBaseSink * sink, GstBufferList * list)
{
	GstFlowReturn flow;
	GstBuffer *buffer;
	guint i, len;

	/* ElbitShmSink doesn't support buffer lists, extract individual buffers
	* then and push them one-by-one */
	GST_INFO_OBJECT(sink, "chaining each group in list as a merged buffer");

	len = gst_buffer_list_length(list);

	flow = GST_FLOW_OK;
	for (i = 0; i < len; i++) {
		buffer = gst_buffer_list_get(list, i);
		flow = gst_shm_sink_render(sink, buffer);
		if (flow != GST_FLOW_OK)
			break;
	}

	return flow;
}

static GstCaps* gst_shm_sink_getcaps(GstBaseSink * psink, GstCaps * filter)
{
	GstCaps *caps;
	GstShmSink *shmsink = GST_SHM_SINK_CAST(psink);
	GstShmSinkPrivate *priv = shmsink->priv;

	GST_OBJECT_LOCK(shmsink);
	if ((caps = priv->caps)) {
		if (filter)
			caps = gst_caps_intersect_full(filter, caps, GST_CAPS_INTERSECT_FIRST);
		else
			gst_caps_ref(caps);
	}
	GST_DEBUG_OBJECT(shmsink, "got caps %" GST_PTR_FORMAT, caps);
	GST_OBJECT_UNLOCK(shmsink);

	return caps;
}

static gboolean gst_shm_sink_set_session_name(GstShmSink* sink, const gchar* name)
{
	gboolean result;
	GstShmSink *shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate *priv = shmsink->priv;

	g_return_val_if_fail(GST_IS_SHM_SINK(sink), FALSE);

	GST_OBJECT_LOCK(shmsink);

	if (name != NULL)
	{	
		if (priv->started)
		{
			GST_WARNING_OBJECT(sink, "Session Name can be set only before playback started");
			result = FALSE;
		}
		else
		{
			if (priv->session_name != NULL)
				g_free(priv->session_name);

			priv->session_name = g_strdup(name);
			result = TRUE;			
		}		
	}
	else 
	{
		GST_WARNING_OBJECT(sink, "Session Name cannot be NULL");
		result = FALSE;
	}

	GST_OBJECT_UNLOCK(shmsink);
	return result;
}

static gchar* gst_shm_sink_get_session_name(GstShmSink* sink)
{
	gchar* session_name = NULL;
	GstShmSink* shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate *priv = shmsink->priv;

	GST_OBJECT_LOCK(shmsink);
	session_name = priv->session_name;
	GST_OBJECT_UNLOCK(shmsink);

	return session_name;
}

static gboolean gst_shm_sink_set_pool_size(GstShmSink* sink, guint size)
{
	gboolean result;
	GstShmSink *shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate *priv = shmsink->priv;

	g_return_val_if_fail(GST_IS_SHM_SINK(sink), FALSE);

	GST_OBJECT_LOCK(shmsink);
	if (priv->started)
	{
		GST_WARNING_OBJECT(sink, "Pool size can be set only before playback started");
		result = FALSE;
	}
	else
	{
		priv->pool_size = size;
		result = TRUE;
	}	

	GST_OBJECT_UNLOCK(shmsink);
	return result;
}

static guint gst_shm_sink_get_pool_size(GstShmSink* sink)
{
	guint pool_size;;
	GstShmSink* shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate *priv = shmsink->priv;

	GST_OBJECT_LOCK(shmsink);
	pool_size = priv->pool_size;
	GST_OBJECT_UNLOCK(shmsink);

	return pool_size;
}

static gboolean gst_shm_sink_set_shared_buffer_size(GstShmSink* sink, guint size)
{
	gboolean result;
	GstShmSink *shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate *priv = shmsink->priv;

	g_return_val_if_fail(GST_IS_SHM_SINK(sink), FALSE);

	GST_OBJECT_LOCK(shmsink);
	if (priv->started)
	{
		GST_WARNING_OBJECT(sink, "Pool size can be set only before playback started");
		result = FALSE;
	}
	else
	{
		priv->shared_buffer_size = size;
		result = TRUE;
	}

	GST_OBJECT_UNLOCK(shmsink);
	return result;
}

static guint gst_shm_sink_get_shared_buffer_size(GstShmSink* sink)
{
	guint shared_buffer_size;;
	GstShmSink* shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate *priv = shmsink->priv;

	GST_OBJECT_LOCK(shmsink);
	shared_buffer_size = priv->shared_buffer_size;
	GST_OBJECT_UNLOCK(shmsink);

	return shared_buffer_size;
}

static gboolean gst_shm_sink_query(GstBaseSink * bsink, GstQuery * query)
{
	gboolean ret;

	GstQueryType query_type = GST_QUERY_TYPE(query);
	switch (query_type)
	{
	case GST_QUERY_SEEKING: {
		GstFormat fmt;

		/* we don't supporting seeking */
		gst_query_parse_seeking(query, &fmt, NULL, NULL, NULL);
		gst_query_set_seeking(query, fmt, FALSE, 0, -1);
		ret = TRUE;
		break;
	}
	default:
		ret = GST_BASE_SINK_CLASS(parent_class)->query(bsink, query);
		break;
	}

	return ret;
}

/* external API */

/**
* gst_shm_sink_set_caps:
* @shmsink: a #GstShmSink
* @caps: caps to set
*
* Set the capabilities on the shmsink element.  This function takes
* a copy of the caps structure. After calling this method, the sink will only
* accept caps that match @caps. If @caps is non-fixed, or incomplete,
* you must check the caps on the samples to get the actual used caps.
*/
void gst_shm_sink_set_caps(GstShmSink * shmsink, const GstCaps * caps)
{
	GstCaps *old;
	GstShmSinkPrivate *priv;

	g_return_if_fail(GST_IS_SHM_SINK(shmsink));

	priv = shmsink->priv;

	GST_OBJECT_LOCK(shmsink);
	GST_DEBUG_OBJECT(shmsink, "setting caps to %" GST_PTR_FORMAT, caps);
	if ((old = priv->caps) != caps) {
		if (caps)
			priv->caps = gst_caps_copy(caps);
		else
			priv->caps = NULL;
		if (old)
			gst_caps_unref(old);
	}
	GST_OBJECT_UNLOCK(shmsink);
}

/**
* gst_shm_sink_get_caps:
* @shmsink: a #GstShmSink
*
* Get the configured caps on @shmsink.
*
* Returns: the #GstCaps accepted by the sink. gst_caps_unref() after usage.
*/
GstCaps* gst_shm_sink_get_caps(GstShmSink * shmsink)
{
	GstCaps *caps;
	GstShmSinkPrivate *priv;

	g_return_val_if_fail(GST_IS_SHM_SINK(shmsink), NULL);

	priv = shmsink->priv;

	GST_OBJECT_LOCK(shmsink);
	if ((caps = priv->caps))
		gst_caps_ref(caps);
	GST_DEBUG_OBJECT(shmsink, "getting caps of %" GST_PTR_FORMAT, caps);
	GST_OBJECT_UNLOCK(shmsink);

	return caps;
}

/**
* gst_shm_sink_is_eos:
* @shmsink: a #GstShmSink
*
* Check if @shmsink is EOS, which is when no more samples can be pulled because
* an EOS event was received.
*
* This function also returns %TRUE when the shmsink is not in the PAUSED or
* PLAYING state.
*
* Returns: %TRUE if no more samples can be pulled and the shmsink is EOS.
*/
gboolean gst_shm_sink_is_eos(GstShmSink * shmsink)
{
	gboolean ret;
	GstShmSinkPrivate *priv;

	g_return_val_if_fail(GST_IS_SHM_SINK(shmsink), FALSE);

	priv = shmsink->priv;

	g_mutex_lock(&priv->mutex);
	if (!priv->started)
		goto not_started;

	if (priv->is_eos)
	{
		GST_DEBUG_OBJECT(shmsink, "we are EOS");
		ret = TRUE;
	}
	else 
	{
		GST_DEBUG_OBJECT(shmsink, "we are not yet EOS");
		ret = FALSE;
	}
	g_mutex_unlock(&priv->mutex);

	return ret;

not_started:
	{
		GST_DEBUG_OBJECT(shmsink, "we are stopped, return TRUE");
		g_mutex_unlock(&priv->mutex);
		return TRUE;
	}
}

/**
* gst_shm_sink_set_emit_signals:
* @shmsink: a #GstShmSink
* @emit: the new state
*
* Make shmsink emit the "new-preroll" and "new-sample" signals. This option is
* by default disabled because signal emission is expensive and unneeded when
* the application prefers to operate in pull mode.
*/
void gst_shm_sink_set_emit_signals(GstShmSink * shmsink, gboolean emit)
{
	GstShmSinkPrivate *priv;

	g_return_if_fail(GST_IS_SHM_SINK(shmsink));

	priv = shmsink->priv;

	g_mutex_lock(&priv->mutex);
	priv->emit_signals = emit;
	g_mutex_unlock(&priv->mutex);
}

/**
* gst_shm_sink_get_emit_signals:
* @shmsink: a #GstShmSink
*
* Check if shmsink will emit the "new-preroll" and "new-sample" signals.
*
* Returns: %TRUE if @shmsink is emiting the "new-preroll" and "new-sample"
* signals.
*/
gboolean
gst_shm_sink_get_emit_signals(GstShmSink * shmsink)
{
	gboolean result;
	GstShmSinkPrivate *priv;

	g_return_val_if_fail(GST_IS_SHM_SINK(shmsink), FALSE);

	priv = shmsink->priv;

	g_mutex_lock(&priv->mutex);
	result = priv->emit_signals;
	g_mutex_unlock(&priv->mutex);

	return result;
}

/**
* gst_shm_sink_set_callbacks: (skip)
* @shmsink: a #GstShmSink
* @callbacks: the callbacks
* @user_data: a user_data argument for the callbacks
* @notify: a destroy notify function
*
* Set callbacks which will be executed for each new preroll, new sample and eos.
* This is an alternative to using the signals, it has lower overhead and is thus
* less expensive, but also less flexible.
*
* If callbacks are installed, no signals will be emitted for performance
* reasons.
*/
void gst_shm_sink_set_callbacks(GstShmSink * shmsink,
	GstShmSinkCallbacks * callbacks, gpointer user_data, GDestroyNotify notify)
{
	GDestroyNotify old_notify;
	GstShmSinkPrivate *priv;

	g_return_if_fail(GST_IS_SHM_SINK(shmsink));
	g_return_if_fail(callbacks != NULL);

	priv = shmsink->priv;

	GST_OBJECT_LOCK(shmsink);
	old_notify = priv->notify;

	if (old_notify) {
		gpointer old_data;

		old_data = priv->user_data;

		priv->user_data = NULL;
		priv->notify = NULL;
		GST_OBJECT_UNLOCK(shmsink);

		old_notify(old_data);

		GST_OBJECT_LOCK(shmsink);
	}
	priv->callbacks = *callbacks;
	priv->user_data = user_data;
	priv->notify = notify;
	GST_OBJECT_UNLOCK(shmsink);
}

gboolean gst_shm_sink_propose_allocation(GstBaseSink* sink, GstQuery * query)
{
	GstCaps* caps = NULL;
	gboolean need_pool = FALSE;

	GstBufferPool *pool = NULL;
    //GstStructure *config = NULL;
	guint size = 0;
	guint min = 0;
	guint max = 0;

	GstShmSink* shmsink = GST_SHM_SINK_CAST(sink);
	GstShmSinkPrivate* priv = shmsink->priv;

	gboolean retval = FALSE;

	gst_query_parse_allocation(query, &caps, &need_pool);
	// Note that even is 'need_pool == FALSE', weh still need
	// to create a local pool as it is allocated on
	// the shared memory

	if (gst_query_get_n_allocation_pools(query) > 0)
	{
		/* we got configuration from our peer, parse them */
		gst_query_parse_nth_allocation_pool(query, 0, &pool, &size, &min, &max);
	}
	else
	{
		size = 0;
		min = max = 0;
	}

	g_mutex_lock(&priv->mutex);
	const gchar* session_name = priv->session_name;
	guint pool_size = MAX(max, priv->pool_size);
	guint shared_buffer_size = MAX(size, priv->shared_buffer_size);
	
	if (priv->pool == NULL)
		priv->pool = gst_shm_buffer_pool_new(session_name, pool_size, shared_buffer_size, caps);

	pool = priv->pool;
	g_mutex_unlock(&priv->mutex);		

	/*config = gst_buffer_pool_get_config(pool);
	gst_buffer_pool_config_add_option(config, GST_SHM_BUFFER_POOL_OPTION_VIDEO_META);	
	gst_buffer_pool_config_set_params(config, caps, shared_buffer_size, pool_size, pool_size);
	gst_buffer_pool_set_config(pool, config);*/

	gst_query_add_allocation_pool(query, pool, size, min, max);
	retval = TRUE;

	if (caps != NULL)
		gst_caps_unref(caps);

	return retval;

}

/*** GSTURIHANDLER INTERFACE *************************************************/

static GstURIType
gst_shm_sink_uri_get_type(GType type)
{
	return GST_URI_SINK;
}

static const gchar *const *
gst_shm_sink_uri_get_protocols(GType type)
{
	static const gchar *protocols[] = { "elbitshmsink", NULL };

	return protocols;
}

static gchar *
gst_shm_sink_uri_get_uri(GstURIHandler * handler)
{
	return g_strdup("elbitshmsink");
}

static gboolean
gst_shm_sink_uri_set_uri(GstURIHandler * handler, const gchar * uri,
	GError ** error)
{
	/* GstURIHandler checks the protocol for us */
	return TRUE;
}

static void
gst_shm_sink_uri_handler_init(gpointer g_iface, gpointer iface_data)
{
	GstURIHandlerInterface *iface = (GstURIHandlerInterface *)g_iface;

	iface->get_type = gst_shm_sink_uri_get_type;
	iface->get_protocols = gst_shm_sink_uri_get_protocols;
	iface->get_uri = gst_shm_sink_uri_get_uri;
	iface->set_uri = gst_shm_sink_uri_set_uri;

}

// Intel C++ does not properly keep warning state for function templates,
// so popping warning state at the end of translation unit leads to warnings in the middle.
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#	pragma warning(pop)
#endif