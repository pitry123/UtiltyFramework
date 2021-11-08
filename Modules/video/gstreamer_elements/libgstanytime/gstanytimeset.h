#ifndef __GST_ANYTIME_SET_H__
#define __GST_ANYTIME_SET_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GST_TYPE_ANYTIME_SET \
  (gst_anytime_set_get_type())

#define GST_ANYTIME_SET(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ANYTIME_SET,GstAnytimeSet))

#define GST_ANYTIME_SET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ANYTIME_SET,GstAnytimeSetClass))

#define GST_IS_ANYTIME_SET(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ANYTIME_SET))

#define GST_IS_ANYTIME_SET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ANYTIME_SET))

	typedef struct _GstAnytimeSet GstAnytimeSet;
	typedef struct _GstAnytimeSetClass GstAnytimeSetClass;

	struct _GstAnytimeSet {
		GstBaseTransform element;
	};

	struct _GstAnytimeSetClass {
		GstBaseTransformClass parent_class;
	};

	GType gst_anytime_set_get_type(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GST_ANYTIME_SET_H__ */
