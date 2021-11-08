#ifndef __MY_GST_SHM_BUFFER_POOL_H__
#define __MY_GST_SHM_BUFFER_POOL_H__

// gstreamer includes
#ifdef __GNUC__
    // Avoid tons of warnings with root code
    #pragma GCC system_header
#endif

#include <gst/gst.h>
#include <gst/gstinfo.h>
#include <gst/video/video.h>
#include <gst/gstbuffer.h>
#include <gst/video/gstvideometa.h>

G_BEGIN_DECLS

/**
* GST_SHM_BUFFER_POOL_OPTION_VIDEO_META:
*
* An option that can be activated on bufferpool to request video metadata
* on buffers from the pool.
*/
#define GST_SHM_BUFFER_POOL_OPTION_VIDEO_META "GstShmBufferPoolOptionVideoMeta"

/**
* GST_SHM_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT:
*
* A bufferpool option to enable extra padding. When a bufferpool supports this
* option, gst_buffer_pool_config_set_video_alignment() can be called.
*
* When this option is enabled on the bufferpool,
* #GST_SHM_BUFFER_POOL_OPTION_VIDEO_META should also be enabled.
*/
#define GST_SHM_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT "GstShmBufferPoolOptionVideoAlignment"

/* setting a bufferpool config */

//GST_EXPORT
void             gst_shm_buffer_pool_config_set_video_alignment(GstStructure *config, GstVideoAlignment *align);

//GST_EXPORT
gboolean         gst_shm_buffer_pool_config_get_video_alignment(GstStructure *config, GstVideoAlignment *align);

/* video bufferpool */
typedef struct _GstShmBufferPool GstShmBufferPool;
typedef struct _GstShmBufferPoolClass GstShmBufferPoolClass;
typedef struct _GstShmBufferPoolPrivate GstShmBufferPoolPrivate;

#define GST_TYPE_SHM_BUFFER_POOL      (gst_shm_buffer_pool_get_type())
#define GST_IS_SHM_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_SHM_BUFFER_POOL))
#define GST_SHM_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_SHM_BUFFER_POOL, GstShmBufferPool))
#define GST_SHM_BUFFER_POOL_CAST(obj) ((GstShmBufferPool*)(obj))

struct _GstShmBufferPool
{
	GstBufferPool bufferpool;
	GstShmBufferPoolPrivate* priv;
};

struct _GstShmBufferPoolClass
{
	GstBufferPoolClass parent_class;
};

GType           gst_shm_buffer_pool_get_type(void);
GstBufferPool*  gst_shm_buffer_pool_new(const gchar* session_name, guint minimum_pool_size, guint minimum_buffer_size, GstCaps* caps);
void			gst_shm_buffer_pool_publish(GstBufferPool* pool, GstBuffer* buffer);

#ifdef G_DEFINE_AUTOPTR_CLEANUP_FUNC
G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstShmBufferPool, gst_object_unref)
#endif

G_END_DECLS

#endif /* __MY_GST_SHM_BUFFER_POOL_H__ */
