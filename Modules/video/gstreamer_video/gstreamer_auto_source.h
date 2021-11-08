#pragma once
#include "common/gstreamer_base_video_source.hpp"
#include <video/sources/gstreamer_auto_source.h>

namespace video
{
	namespace sources
	{		
		class gstreamer_auto_source_impl :
			public video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_auto_source>>
		{
		private:
			bool m_sync;
			uint32_t m_width;
			uint32_t m_height;
			core::imaging::pixel_format m_format;

		protected:
			virtual std::string pipeline_description() override;

		public:
			gstreamer_auto_source_impl(bool sync, uint32_t width, uint32_t height, core::imaging::pixel_format format, core::imaging::image_algorithm_interface* algo);
			virtual ~gstreamer_auto_source_impl();
		};
	}
}

