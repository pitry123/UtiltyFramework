#include "gstshmmeta.h"
#include "gstshmcommon.hpp"

GType gst_meta_shm_buffer_api_get_type(void)
{
	static volatile GType type;
	static const gchar *tags[] = { NULL };

	if (g_once_init_enter(&type)) {
		GType _type = gst_meta_api_type_register("GstMetaSharedBufferAPI", tags);
		g_once_init_leave(&type, _type);
	}
	return type;
}

gboolean gst_meta_shm_buffer_init(GstMeta *meta, gpointer pool_item, GstBuffer *buffer)
{
	GstMetaShmBuffer* buffer_meta = (GstMetaShmBuffer*)meta;

	if (pool_item != NULL)
		static_cast<gst_pool_item*>(pool_item)->add_ref();

	buffer_meta->pool_item = pool_item;
	return TRUE;
}

gboolean gst_meta_shm_buffer_transform(
	GstBuffer *dest_buf,
	GstMeta *src_meta,
	GstBuffer *src_buf,
	GQuark type,
	gpointer data) 
{
	GstMetaShmBuffer* dest_meta = GST_META_SHM_BUFFER_ADD(dest_buf, NULL);

	GstMetaShmBuffer* src_meta_buffer = (GstMetaShmBuffer*)src_meta;
	GstMetaShmBuffer* dest_meta_buffer = (GstMetaShmBuffer*)dest_meta;

	if (dest_meta_buffer->pool_item != NULL)
		static_cast<gst_pool_item*> (dest_meta_buffer->pool_item)->release();
	
	dest_meta_buffer->pool_item = src_meta_buffer->pool_item;
	if (dest_meta_buffer->pool_item != NULL)
		static_cast<gst_pool_item*>(dest_meta_buffer->pool_item)->add_ref();

	return TRUE;
}

void gst_meta_shm_buffer_free(GstMeta* meta, GstBuffer *buffer) 
{
	GstMetaShmBuffer* meta_buffer = (GstMetaShmBuffer*)meta;
	if (meta_buffer->pool_item != NULL)
		static_cast<gst_pool_item*>(meta_buffer->pool_item)->release();
}

const GstMetaInfo* gst_meta_shm_buffer_get_info(void)
{
	static const GstMetaInfo *meta_info = NULL;

	if (g_once_init_enter(&meta_info)) 
	{
		const GstMetaInfo *meta =
			gst_meta_register(gst_meta_shm_buffer_api_get_type(), "GstMetaShmBuffer",
				sizeof(GstMetaShmBuffer), (GstMetaInitFunction)gst_meta_shm_buffer_init,
				(GstMetaFreeFunction)gst_meta_shm_buffer_free, (GstMetaTransformFunction)gst_meta_shm_buffer_transform);
		g_once_init_leave(&meta_info, meta);
	}

	return meta_info;
}