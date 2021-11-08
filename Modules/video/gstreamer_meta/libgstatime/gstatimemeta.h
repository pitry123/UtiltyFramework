#ifndef _GST_META_ATIME_H_
#define _GST_META_ATIME_H_

// gstreamer includes
#ifdef __GNUC__
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif

#include <gst/gst.h>
#include <gst/gstmeta.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

G_BEGIN_DECLS

typedef struct __ATimeMeta ATimeMeta;

struct __ATimeMeta {  
  GstMeta meta;  
  GstClockTime absoluteTime; 
};

//registering out metadata API definition	
GST_PLUGIN_EXPORT GType atime_meta_api_get_type(void);
#define ATIME_META_API_TYPE (atime_meta_api_get_type())

//finds and returns the metadata with our new API.
#define gst_buffer_get_atime_meta(b) \
  ((ATimeMeta*)gst_buffer_get_meta((b),ATIME_META_API_TYPE))

//removes the metadata of our new API.  
#define gst_buffer_del_atime_meta(b) (gst_buffer_remove_meta((b), gst_buffer_get_atime_meta(b)))  
  
GST_PLUGIN_EXPORT const GstMetaInfo* atime_meta_get_info (void);
#define ATIME_META_INFO (atime_meta_get_info())

GST_PLUGIN_EXPORT ATimeMeta* gst_buffer_add_atime_meta (GstBuffer      *buffer,
GstClockTime absoluteTime);

G_END_DECLS

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_GST_META_ATIME_H_
