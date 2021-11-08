#pragma once
#include "common/gstreamer_base_appsrc_pipeline.hpp"
#include "common/gstreamer_utils.hpp"
#include <video/sources/gstreamer_h264_player.h>

#define MAX_PENDING_H264_FRAMES 256

namespace video
{
	namespace sources
	{
		class gstreamer_h264_player_impl :
			public video::sources::gstreamer_base_appsrc_pipeline<video::sources::gstreamer_h264_player>
		{
		private:
			utils::ref_count_ptr<core::video::video_source_interface> m_h264_source;
			utils::ref_count_ptr<core::video::frame_callback> m_callback;
			bool m_live;			
			core::imaging::pixel_format m_output_format;

		protected:
			virtual std::string pipeline_description() override
			{
				bool intel_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("mfxdecode");
				bool omx_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("omxh264dec");
				bool d3d11_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("d3d11h264dec");
				bool nvidia_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("nvh264sldec");				

				std::stringstream description_builder;

				// parser and decoder
				description_builder <<
                    "appsrc name=appsrc" << index() << " ! queue ! video/x-h264 ! h264parse !";

				if (intel_hardware_accelerated)
				{
					description_builder <<
						"mfxdecode name=mfxdecode" << index();

					if (m_live == true)
						description_builder << " live_mode=true";

					description_builder << " ! ";
				}
				else if (omx_hardware_accelerated)
				{
					description_builder <<
						"omxh264dec name=omxh264dec" << index() << " ! video/x-raw ! ";
				}
				else if (d3d11_hardware_accelerated)
				{
					description_builder <<
						"d3d11h264dec name=d3d11h264dec" << index() << " ! video/x-raw ! ";
				}
				else if (nvidia_hardware_accelerated)
				{
					description_builder <<
						"nvh264sldec name=nvh264sldec" << index() << " ! video/x-raw ! ";
				}				
				else
				{
					description_builder <<
						"avdec_h264 name=avdec_h264" << index() << " ! ";
				}

				if (m_output_format != core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT)
				{
					if (intel_hardware_accelerated == true && m_output_format == core::imaging::pixel_format::BGRA)
					{
						// mfxh264dec supports hardware conversion to BGRA
						description_builder << "video/x-raw, format=" << m_output_format << " ! ";
					}
					else
					{
						description_builder <<
							"videoconvert name=videoconvert" << index() << " ! video/x-raw, format=" << m_output_format << " ! ";
					}
				}

				// appsink
				description_builder <<
					"queue ! appsink name=appsink" << index();

				if (m_live == true)
					description_builder << " sync=false";

				return description_builder.str();
			}
			
			virtual std::string appsrc_name() override
			{
				std::stringstream app_src_name_builder;
				app_src_name_builder << "appsrc" << index();

				return app_src_name_builder.str();
			}

			virtual bool set_caps(GstElement* appsrc, core::video::frame_interface* frame) override
			{
				core::imaging::image_params image_params;
				if (frame->query_image_params(image_params) == false)
					return false;			

				core::video::video_params video_params;
				if (frame->query_video_params(video_params) == false)
					return false;

				int width = static_cast<int>(image_params.width);
				int height = static_cast<int>(image_params.height);
				int fractionNumerator = static_cast<int>(video_params.framerate.numerator);
				int fractionDenominator = static_cast<int>(video_params.framerate.denominator);

				g_object_set(G_OBJECT(appsrc), "caps",
					gst_caps_new_simple("video/x-h264",
						"stream-format", G_TYPE_STRING, "byte-stream",
						"alignment", G_TYPE_STRING, "au",
						"width", G_TYPE_INT, width,
						"height", G_TYPE_INT, height,
						"framerate", GST_TYPE_FRACTION, fractionNumerator, fractionDenominator,
						//"interlace-mode", G_TYPE_STRING, "progressive",						
						"parsed", G_TYPE_BOOLEAN, TRUE,						
						nullptr), nullptr);

				return true;
			}
	
			virtual bool on_start() override
			{
				if (video::sources::gstreamer_base_appsrc_pipeline<video::sources::gstreamer_h264_player>::on_start() == false)
					return false;

				return m_h264_source->add_frame_callback(m_callback);
			}

			virtual void on_stop() override
			{
				m_h264_source->remove_frame_callback(m_callback);
				video::sources::gstreamer_base_appsrc_pipeline<video::sources::gstreamer_h264_player>::on_stop();
			}

		public:
			gstreamer_h264_player_impl(core::video::video_source_interface* h264_source, bool live, core::imaging::pixel_format output_format, bool async) :
				video::sources::gstreamer_base_appsrc_pipeline<video::sources::gstreamer_h264_player>(async == true ? MAX_PENDING_H264_FRAMES : 1, async),
				m_h264_source(h264_source),
				m_live(live),
				m_output_format(output_format)
			{				
				if (h264_source == nullptr)
					throw std::invalid_argument("h264_source");

				m_callback = utils::make_ref_count_ptr<utils::video::smart_frame_callback>([this](core::video::frame_interface* frame)
				{
					core::video::video_params video_params;
					if (frame->query_video_params(video_params) == false || video_params.data_type != core::video::video_data_type::H264)
						return;

					if (this->async() == true)
					{
						this->set_frame(frame);
					}
					else
					{
						while (state() == core::video::video_state::PLAYING && this->set_frame(frame) == false)
						{
							std::this_thread::yield();
						}
					}
				});
			}

			virtual ~gstreamer_h264_player_impl() override
			{
				stop();
			}
		};
	}
}

