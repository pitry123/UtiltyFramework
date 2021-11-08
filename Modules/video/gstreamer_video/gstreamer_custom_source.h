#pragma once
#include "common/gstreamer_base_video_source.hpp"
#include <video/sources/gstreamer_custom_source.h>

namespace video
{
	namespace sources
	{		
		class gstreamer_custom_source_impl :
			public video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_custom_source>>
		{
		private:				
			std::string m_pipeline;
			std::string m_appsink_name;

		protected:
			virtual std::string appsink_name() const override;
			virtual std::string pipeline_description() override;			

		public:
			gstreamer_custom_source_impl(const char* pipeline, const char* appsink_name, core::imaging::image_algorithm_interface* algo);
			virtual ~gstreamer_custom_source_impl();
		};
	}
}

