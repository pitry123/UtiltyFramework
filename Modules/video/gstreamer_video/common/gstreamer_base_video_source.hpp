#pragma once
#include "gstreamer_base_video_controller.hpp"

#include <utils/ref_count_base.hpp>
#include <utils/video.hpp>
#include <utils/strings.hpp>

#include <string>
#include <stdexcept>

#include <gst/app/gstappsink.h>
#include <libgstatime/gstatimemeta.h>

namespace video
{
	class gst_buffer : public utils::ref_count_base<core::buffer_interface>
	{
	private:
		class gst_buffer_guard
		{
		private:
			GstBuffer * m_buffer;
			GstMapInfo m_map;

		public:
			gst_buffer_guard(GstBuffer* buffer) :
				m_buffer(gst_buffer_ref(buffer))
			{
				gst_buffer_map(m_buffer, &m_map, static_cast<GstMapFlags>(GstMapFlags::GST_MAP_READ));
			}

			~gst_buffer_guard()
			{
				gst_buffer_unmap(m_buffer, &m_map);
				gst_buffer_unref(m_buffer);
			}

			GstBuffer* get() const
			{
				return m_buffer;
			}

			uint8_t* data() const
			{
				return static_cast<uint8_t*>(m_map.data);
			}
		};

		gst_buffer_guard m_buffer;

	public:
		gst_buffer(GstBuffer* buffer) : m_buffer(buffer)
		{
		}

		virtual size_t size() const override
		{
			return gst_buffer_get_size(m_buffer.get());
		}

		virtual uint8_t* data() override
		{
			return m_buffer.data();
		}
	};

	class gst_frame : public utils::ref_count_base<core::video::frame_interface>
	{
	private:
		class gst_sample_guard
		{
		private:
			GstSample * m_sample;

		public:
			gst_sample_guard(GstSample* sample) :
				m_sample(gst_sample_ref(sample))
			{
			}

			~gst_sample_guard()
			{
				gst_sample_unref(m_sample);
			}

			GstSample* get() const
			{
				return m_sample;
			}
		};
		
		gst_sample_guard m_sample;
		core::imaging::image_params m_image_params;
		core::video::display_params m_display_params;
		core::video::video_params m_video_params;
		utils::ref_count_ptr<core::buffer_interface> m_buffer;		

	public:
		gst_frame(GstSample* sample, const GstMetaInfo* rtp_timestamps_meta_info, core::imaging::image_algorithm_interface* algo) :
			m_sample(sample),
			m_image_params({}),
			m_display_params({}),
			m_video_params({})
		{
			GstCaps* caps = gst_sample_get_caps(m_sample.get());
			/*gchar* str = gst_caps_to_string(caps);
			printf("Frame Caps: %s\n", str);*/

			gst_caps_foreach(caps, [](
				GstCapsFeatures* features,
				GstStructure* structure,
				gpointer data) -> gboolean
			{
				static GQuark h264_id = g_quark_from_string("video/x-h264");

				gst_frame* frame = static_cast<gst_frame*>(data);

				GQuark quark = gst_structure_get_name_id(structure);
				if (quark == h264_id)
				{
					frame->m_video_params.data_type = core::video::video_data_type::H264;
					
					gboolean parsed = FALSE;
					gst_structure_get_boolean(structure, "parsed", &parsed);
					if (parsed == false)
						return TRUE;
				}
				else // assume raw
				{					
					frame->m_video_params.data_type = core::video::video_data_type::RAW;
				}				

				gint width = 0;
				if (gst_structure_get_int(structure, "width", &width) == FALSE)
					return FALSE;

				frame->m_image_params.width = static_cast<uint32_t>(width);

				gint height = 0;
				if (gst_structure_get_int(structure, "height", &height) == FALSE)
					return FALSE;

				frame->m_image_params.height = static_cast<uint32_t>(height);

				const gchar* format = gst_structure_get_string(structure, "format");
				if (format != nullptr)
				{
					if (std::strcmp(format, "BGRx") == 0)
						format = "BGRA";

					if (parse(format, frame->m_image_params.format) == false)
						frame->m_image_params.format = core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT;
				}
				else
				{
					frame->m_image_params.format = core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT;
				}

				const gchar* interlace_mode = gst_structure_get_string(structure, "interlace-mode");
				if (interlace_mode != nullptr)
				{
					if (strcmp(interlace_mode, "progressive") == 0)
						frame->m_video_params.interlace_mode = core::video::interlace_mode::PROGRESSIVE;
					else if (strcmp(interlace_mode, "interleaved") == 0)
						frame->m_video_params.interlace_mode = core::video::interlace_mode::INTERLEAVED;
					else if (strcmp(interlace_mode, "mixed") == 0)
						frame->m_video_params.interlace_mode = core::video::interlace_mode::MIXED;
					else if (strcmp(interlace_mode, "fields") == 0)
						frame->m_video_params.interlace_mode = core::video::interlace_mode::FIELDS;
					else
						return FALSE;
				}
				else
				{
					// Assume progressive
					frame->m_video_params.interlace_mode = core::video::interlace_mode::PROGRESSIVE;
				}

				gint numerator;
				gint denominator;
				if (gst_structure_get_fraction(structure, "framerate", &numerator, &denominator) == FALSE)
					return FALSE;				

				frame->m_video_params.framerate.numerator = static_cast<uint32_t>(numerator);
				frame->m_video_params.framerate.denominator = static_cast<uint32_t>(denominator);

				return TRUE;
			}, this);

			GstBuffer* buffer = gst_sample_get_buffer(m_sample.get());

			size_t buffer_size = gst_buffer_get_size(buffer);
			// buffer size is assumed to be smaller than uint32_t's max value
			if (buffer_size > (std::numeric_limits<uint32_t>::max)())
				throw std::runtime_error("Unexpected: Image buffer size is greater than 'std::numeric_limits<uint32_t>::max)()'");

			m_image_params.size = static_cast<uint32_t>(buffer_size);
			m_display_params.dts = buffer->dts;
			m_display_params.pts = buffer->pts;
			m_display_params.duration = buffer->duration;
            m_display_params.timestamp = 0;

            /*if (m_video_params.data_type == core::video::video_data_type::H264)
                m_display_params.frame_id = static_cast<uint64_t>(buffer->mini_object.flags);*/

			if (rtp_timestamps_meta_info != nullptr)
			{
				// Try to get timestamp		
				GstMeta* meta = gst_buffer_get_meta(buffer, rtp_timestamps_meta_info->api);
				if (meta != nullptr)
					m_display_params.timestamp = static_cast<uint64_t>(reinterpret_cast<ATimeMeta*>(meta)->absoluteTime);
			}

			m_buffer = utils::make_ref_count_ptr<gst_buffer>(buffer);

			if (algo == nullptr)
				return;

			utils::ref_count_ptr<core::imaging::image_interface> image;
			if (algo->apply(this, &image) == false)
				return;

			core::imaging::image_params image_params;
			if (image->query_image_params(image_params) == false)
				return;

			utils::ref_count_ptr<core::buffer_interface> image_buffer;
			if (image->query_buffer(&image_buffer) == false)
				return;

			m_image_params = image_params;
			m_buffer = image_buffer;
		}

		virtual bool query_image_params(core::imaging::image_params& image_params) const override
		{
			image_params = m_image_params;
			return true;
		}

		virtual bool query_buffer(core::buffer_interface** buffer) const override
		{
			if (buffer == nullptr)
				return false;

			if (m_buffer == nullptr)
				return false;

			*buffer = m_buffer;
			(*buffer)->add_ref();
			return true;
		}

		virtual bool query_display_params(core::video::display_params& display_params) const override
		{
			display_params = m_display_params;
			return true;
		}

		virtual bool query_video_params(core::video::video_params& video_params) const override
		{
			video_params = m_video_params;
			return true;
		}
	};

	namespace sources
	{
		template <typename T>
		class gstreamer_base_video_source :
			public utils::video::video_source_base<video::common::gstreamer_base_video_controller<utils::ref_count_base<T>>>
		{
		private:
			int m_index;

			// Usually we won't use such declarations as it's not RAII.
			// However, on this specific case and for simplicity reasons,
			// since this member is initialized and destructed solely in the working tread,
			// and since the working thread is an RAII managed object,
			// we can set the member as plain pointer
			bool m_async;
			GstElement* m_appsink;			

			// We try to get the RTP timestamps metadata info once.
			// This way we can tell if it's available in our buffers
			// and use it without the need to link 'gstrtpatime'
			std::once_flag m_rtp_timestamps_once_flag;
			const GstMetaInfo* m_rtp_timestamps_meta_info;	

			utils::ref_count_ptr<core::imaging::image_algorithm_interface> m_algo;

			void handle_new_sample()
			{
				if (this->callback_count() == 0)
					return;

				utils::ref_count_ptr<core::video::frame_interface> frame;
				if (query_frame(&frame) == false)
					return;

				this->raise_frame(frame, m_async);
			}

		protected:
			int& index()
			{
				return m_index;
			}

			bool async() const
			{
				return m_async;
			}

			virtual std::string appsink_name() const
			{
				std::stringstream stream;
				stream << "appsink" << m_index;
				return stream.str();
			}

			virtual void on_pipeline_constructed(GstElement* pipeline) override
			{				
				m_appsink = gst_bin_get_by_name(GST_BIN(pipeline), appsink_name().c_str());

				if (m_appsink == nullptr)
					throw std::runtime_error("Failed to find the appsink element");

				if (this->async() == true)
				{
					gst_app_sink_set_max_buffers(GST_APP_SINK(m_appsink), 20);
					gst_app_sink_set_drop(GST_APP_SINK(m_appsink), TRUE);
				}
				else
				{
					gst_app_sink_set_drop(GST_APP_SINK(m_appsink), FALSE);
				}

				// Register to signals emition...
				/*g_object_set(G_OBJECT(appsink), "emit-signals", TRUE, "sync", FALSE, NULL);
				g_signal_connect(appsink, "new-sample", G_CALLBACK(on_new_sample), this);*/

				// Register to callbacks API
				GstAppSinkCallbacks callbacks = {};

				callbacks.new_sample = [](GstAppSink* appsink, gpointer data) -> GstFlowReturn
				{
					static_cast<gstreamer_base_video_source*>(data)->handle_new_sample();
					return GstFlowReturn::GST_FLOW_OK;
				};

				gst_app_sink_set_callbacks(GST_APP_SINK(m_appsink), &callbacks, this, nullptr);
			}

			virtual void on_pipeline_completed() override
			{
				if (m_appsink == nullptr)
					return;

				GstAppSinkCallbacks callbacks = {};
				gst_app_sink_set_callbacks(GST_APP_SINK(m_appsink), &callbacks, nullptr, nullptr);
				gst_element_set_state(m_appsink, GstState::GST_STATE_NULL);

				this->sync();
			}

			virtual void on_pipeline_destroyed() override
			{
				if (m_appsink != nullptr)
					gst_object_unref(m_appsink);

				m_appsink = nullptr;
			}

			virtual void on_error(int error_code) override
			{
				this->raise_error(error_code);
			}

			virtual bool query_frame(core::video::frame_interface** frame)
			{
				if (frame == nullptr)
					return false;

				if (m_appsink == nullptr)
					return false;

				std::call_once(m_rtp_timestamps_once_flag, [&]()
				{
					m_rtp_timestamps_meta_info = gst_meta_get_info("ATime");
				});

				GstSample* sample = nullptr;
				utils::scope_guard sample_releaser([&sample]()
				{
					if (sample != nullptr)
						gst_sample_unref(sample);
				});

				/* get the sample from appsink */
				sample = gst_app_sink_pull_sample(GST_APP_SINK(m_appsink));
				utils::ref_count_ptr<core::video::frame_interface> gstframe = utils::make_ref_count_ptr<gst_frame>(sample, m_rtp_timestamps_meta_info, m_algo);

				*frame = gstframe;
				(*frame)->add_ref();
				return true;
			}

		public:
			gstreamer_base_video_source(core::imaging::image_algorithm_interface* algo = nullptr, 
				bool async = true, 
				size_t max_pending_frames = utils::video::video_source_base<video::common::gstreamer_base_video_controller<utils::ref_count_base<T>>>::DEFAULT_MAX_PENDING_FRAMES) :
				utils::video::video_source_base<video::common::gstreamer_base_video_controller<utils::ref_count_base<T>>>(max_pending_frames),
				m_async(async),
				m_appsink(nullptr),
				m_rtp_timestamps_meta_info(nullptr),
				m_algo(algo)
			{
				static std::atomic<int> INSTANCE_COUNTER{ 0 };
				m_index = ++INSTANCE_COUNTER;
			}

			virtual ~gstreamer_base_video_source()
			{
				this->stop();
			}
		};
	}
}

