#pragma once
#include <utils/scope_guard.hpp>

// gstreamer includes
#ifdef __GNUC__
    // Avoid tons of warnings with root code
    #pragma GCC system_header
#endif

#include <gst/gst.h>

namespace video
{
	namespace helpers
	{
		class gstreamer_helpers
		{
		private:
			gstreamer_helpers() {}; // Non Constructible

		public:
			static bool has_element(const char* name)
			{
				if (name == nullptr)
					return false;

				GstElement* factory = gst_element_factory_make(name, nullptr);
				utils::scope_guard factory_releaser([&]()
				{
					if (factory != nullptr)
						gst_object_unref(factory);
				});

				return (factory != nullptr);
			}
		};
	}
}