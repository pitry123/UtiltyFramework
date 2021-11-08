#pragma once
#include "common/gstreamer_base_video_source.hpp"
#include <video/sources/gstreamer_test_source.h>

namespace video
{
	namespace sources
	{		
		class gstreamer_test_source_impl :
			public video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_test_source>>
		{
		private:		
			uint32_t m_width;
			uint32_t m_height;
			core::imaging::pixel_format m_format;
			core::video::framerate m_framerate;

		protected:
			virtual std::string pipeline_description() override;

		public:
			gstreamer_test_source_impl(uint32_t width, uint32_t height, core::imaging::pixel_format format, const core::video::framerate& framerate, core::imaging::image_algorithm_interface* algo);
			virtual ~gstreamer_test_source_impl();
		};
	}
}

