#pragma once
#include "common/gstreamer_base_video_source.hpp"
#include <video/sources/gstreamer_file_source.h>

namespace video
{
	namespace sources
	{		
		class gstreamer_file_source_impl :
			public video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_file_source>>
		{
		private:	
			std::string m_file_path;		

		protected:
			virtual std::string pipeline_description() override;

		public:
			gstreamer_file_source_impl(const char* uri, core::imaging::image_algorithm_interface* algo);
			virtual ~gstreamer_file_source_impl();
		};
	}
}

