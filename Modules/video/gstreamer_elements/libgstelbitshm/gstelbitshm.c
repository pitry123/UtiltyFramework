#include <gst/gst.h>
#include "gstshmsink.h"

#ifdef MINGW_COMPILER
// This projects compiles both C and C++ code
// It seems that linker exports only C++ symbols with MinGW in this case
// Setting the flags to 'MSVC like' export definitions resolves the issue
// as it exports the above functions as C style
#if (!defined(GST_STATIC_COMPILATION))
#undef GST_PLUGIN_EXPORT
#define GST_PLUGIN_EXPORT __declspec(dllexport)
#ifdef GST_EXPORTS
#undef GST_EXPORT
#define GST_EXPORT __declspec(dllexport)
#else
#undef GST_EXPORT
#define GST_EXPORT __declspec(dllimport) extern
#endif // GST_EXPORTS
#endif // (!defined(GST_STATIC_COMPILATION))
#endif // MINGW_COMPILER

G_BEGIN_DECLS

#define PACKAGE "gstelbitshm"

static gboolean
plugin_init(GstPlugin * plugin)
{
	if (!gst_element_register(plugin, "elbitshmsink", GST_RANK_NONE,
		GST_TYPE_SHM_SINK))
		return FALSE;

	return TRUE;
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
    elbitshm,	
	"Element used to send buffers through a shared memory to Framework 2.0's shared_memory_source",
	plugin_init, "1.0.0", "Proprietary", PACKAGE, "Elbit Systems")

G_END_DECLS