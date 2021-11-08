#ifndef __GST_SHARED_BUFFER_META_H__
#define __GST_SHARED_BUFFER_META_H__

// gstreamer includes
#ifdef __GNUC__
    // Avoid tons of warnings with root code
    #pragma GCC system_header
#endif

#include <gst/gst.h>
#include <gst/gstmeta.h>

G_BEGIN_DECLS

typedef struct _GstMetaShmBuffer GstMetaShmBuffer;

struct _GstMetaShmBuffer {
	GstMeta meta;
	gpointer pool_item;
};

GType gst_meta_shm_buffer_api_get_type(void);
const GstMetaInfo* gst_meta_shm_buffer_get_info(void);
#define GST_META_SHM_BUFFER_GET(buf) ((GstMetaShmBuffer*)gst_buffer_get_meta(buf,gst_meta_shm_buffer_api_get_type()))
#define GST_META_SHM_BUFFER_ADD(buf, shared) ((GstMetaShmBuffer*)gst_buffer_add_meta(buf,gst_meta_shm_buffer_get_info(),(gpointer)shared))

G_END_DECLS

#endif /* __GST_SHARED_BUFFER_META_H__ */
