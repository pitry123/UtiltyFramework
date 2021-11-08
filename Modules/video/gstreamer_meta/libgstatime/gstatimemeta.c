#include "gstatimemeta.h"

GType atime_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] = { NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register ("ATimeApi", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

static gboolean
atime_meta_init (GstMeta * meta, gpointer params, GstBuffer * buffer)
{
  ATimeMeta *emeta = (ATimeMeta *) meta;

  emeta->absoluteTime = 0;

  return TRUE;
}
 
static gboolean
atime_meta_transform (GstBuffer * transbuf, GstMeta * meta,
    GstBuffer * buffer, GQuark type, gpointer data)
{
  ATimeMeta *emeta = (ATimeMeta *) meta;
  
  /* we always copy no matter what transform */
  gst_buffer_add_atime_meta (transbuf, emeta->absoluteTime);

  return TRUE;
}

static void
atime_meta_free (GstMeta * meta, GstBuffer * buffer)
{
  ATimeMeta* emeta = (ATimeMeta*)meta;
  (void)emeta;

  //if we had to release a pointer in the future...
}

const GstMetaInfo* atime_meta_get_info(void)
{
#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4090) // conditional expression is constant
#endif

	static const GstMetaInfo *meta_info = NULL;

	if (g_once_init_enter(&meta_info))
	{
		const GstMetaInfo *meta =
			gst_meta_register(ATIME_META_API_TYPE, "ATime",
				sizeof(ATimeMeta), (GstMetaInitFunction)atime_meta_init,
				(GstMetaFreeFunction)atime_meta_free, (GstMetaTransformFunction)atime_meta_transform);
		g_once_init_leave(&meta_info, meta);
	}

	return meta_info;

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
}

ATimeMeta *gst_buffer_add_atime_meta (GstBuffer   *buffer,
                    GstClockTime absoluteTime)
{
  ATimeMeta *meta;

  g_return_val_if_fail (GST_IS_BUFFER (buffer), NULL);

  meta = (ATimeMeta *) gst_buffer_add_meta (buffer,
      ATIME_META_INFO, NULL);

  if (meta != NULL)
	  meta->absoluteTime = absoluteTime;

  return meta;
}
