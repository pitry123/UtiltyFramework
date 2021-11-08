#pragma once
#include <utils/video.hpp>

#include <Common.hpp>
#include <Utils.hpp>
#include <Imaging.hpp>

namespace Video
{
	using VideoState = core::video::video_state;	
	using InterlaceMode = core::video::interlace_mode;
	using Framerate = core::video::framerate;
	using VideoDataType = core::video::video_data_type;

	class Frame : public Imaging::ImageTemplate<core::video::frame_interface>
	{
	public:
		static constexpr uint64_t UNDEFINED_TIME = (std::numeric_limits<uint64_t>::max)();

		Frame()
		{
			// Empty Frame
		}

		Frame(core::video::frame_interface* frame) : 
			Imaging::ImageTemplate<core::video::frame_interface>(frame)
		{
		}		

		uint64_t DTS()
		{
			ThrowOnEmpty("Video::Frame");

			core::video::display_params display_params = {};
			if (m_core_object->query_display_params(display_params) == false)
				return UNDEFINED_TIME;

			return display_params.dts;
		}

		uint64_t PTS()
		{
			ThrowOnEmpty("Video::Frame");

			core::video::display_params display_params = {};
			if (m_core_object->query_display_params(display_params) == false)
				return UNDEFINED_TIME;

			return display_params.pts;
		}

		uint64_t Duration()
		{
			ThrowOnEmpty("Video::Frame");

			core::video::display_params display_params = {};
			if (m_core_object->query_display_params(display_params) == false)
				return UNDEFINED_TIME;

			return display_params.duration;
		}

		Video::Framerate Framerate()
		{
			ThrowOnEmpty("Video::Frame");

			core::video::video_params video_params = {};
			if (m_core_object->query_video_params(video_params) == false)
				return {};

			return video_params.framerate;
		}

		Video::InterlaceMode InterlaceMode()
		{
			ThrowOnEmpty("Video::Frame");

			core::video::video_params video_params = {};
			if (m_core_object->query_video_params(video_params) == false)
				return core::video::interlace_mode::UNDEFINED_INTERLACE_MODE;

			return video_params.interlace_mode;
		}		
	};

	class VideoSource;
	
	class VideoSourceFactory : public Common::CoreObjectWrapper<core::video::video_source_factory_interface>
	{
	public:
		VideoSourceFactory();
		VideoSourceFactory(core::video::video_source_factory_interface* factory);

		Video::VideoSource Create() const;

		template<typename T, typename... Args>
		static Video::VideoSourceFactory Get(Args&&... args)
		{
			return Video::VideoSourceFactory(utils::make_ref_count_ptr<T>(std::forward<Args>(args)...));
		}
	};

	template <typename T>
	class VideoControllerTemplate : public Common::CoreObjectWrapper<T>
	{
	private:
		utils::video::smart_video_controller_base<T>* smart_controller_base() const
		{
            return static_cast<utils::video::smart_video_controller_base<T>*>(static_cast<core::video::video_controller_interface*>(this->m_core_object));
		}		

	public:
		using ErrorSignal = Utils::SignalAdapter<
			std::function<void(int)>,
			utils::video::smart_video_controller_base<T>,
			int>;

		VideoControllerTemplate()
		{
			// Empty VideoControllerTemplate
		}

		VideoControllerTemplate(std::nullptr_t)
		{
			// Empty VideoControllerTemplate
		}

		VideoControllerTemplate(utils::video::smart_video_controller_base<T>* controller) :
			Common::CoreObjectWrapper<T>(controller)
		{
		}

		virtual ~VideoControllerTemplate() = default;

		Video::VideoState State()
		{
            this->ThrowOnEmpty("Video::VideoSource");
            return this->m_core_object->state();
		}

		void Start()
		{
            this->ThrowOnEmpty("Video::VideoSource");
            this->m_core_object->start();
		}

		void Stop()
		{
            this->ThrowOnEmpty("Video::VideoSource");
            this->m_core_object->stop();
		}

		void Pause()
		{
            this->ThrowOnEmpty("Video::VideoSource");
            this->m_core_object->pause();
		}

		ErrorSignal OnError()
		{
            this->ThrowOnEmpty("Video::VideoSource");
            return ErrorSignal(this->smart_controller_base()->on_error);
		}
	};

	class VideoController : public VideoControllerTemplate<core::video::video_controller_interface>
	{
	private:
		utils::video::smart_video_controller* smart_controller() const
		{
			return static_cast<utils::video::smart_video_controller*>(static_cast<core::video::video_controller_interface*>(m_core_object));
		}

	public:
		VideoController()
		{
			// Empty VideoController
		}

		VideoController(std::nullptr_t)
		{
			// Empty VideoController
		}

		VideoController(core::video::video_controller_interface* controller) :
			VideoControllerTemplate<core::video::video_controller_interface>(
				utils::make_ref_count_ptr<utils::video::smart_video_controller>(controller))
		{
		}

		virtual void UnderlyingObject(core::video::video_controller_interface** obj) const override
		{
			ThrowOnEmpty("Video::VideoController");

			if (obj == nullptr)
				throw std::invalid_argument("obj");

			(*obj) = smart_controller()->controller();
			if ((*obj) != nullptr)
				(*obj)->add_ref();
		}

		virtual explicit operator core::video::video_controller_interface*() const override
		{
			if (Empty() == true)
				return nullptr;

			return smart_controller()->controller();
		}
	};	

    class VideoSource : public VideoControllerTemplate<core::video::video_source_interface>
	{	
    private:
        utils::video::smart_video_source* smart_source() const
        {
            return static_cast<utils::video::smart_video_source*>(static_cast<core::video::video_source_interface*>(m_core_object));
        }

	public:
		using FrameSignal = Utils::SignalAdapter<
			std::function<void(const Video::Frame&)>,
			utils::video::smart_video_source,
			core::video::frame_interface*>;				
		
		VideoSource()
		{
			// Empty VideoSource
		}

		VideoSource(std::nullptr_t)
		{
			// Empty VideoSource
		}
		
		VideoSource(core::video::video_source_interface* source) : 
			VideoControllerTemplate<core::video::video_source_interface>(
				utils::make_ref_count_ptr< utils::video::smart_video_source>(source))
		{			
		}

		VideoSource(core::video::video_source_factory_interface* factory)
		{
			if (factory == nullptr)
				throw std::runtime_error("Failed to create VideoSource");

			utils::ref_count_ptr<core::video::video_source_interface> wrapped_source;
			if (factory->create(&wrapped_source) == false)
				throw std::runtime_error("Failed to create VideoSource");

			m_core_object = utils::make_ref_count_ptr<utils::video::smart_video_source>(wrapped_source);
		}

		VideoSource(const Video::VideoSourceFactory& factory) :
			VideoSource(static_cast<core::video::video_source_factory_interface*>(factory))
		{
		}
		
		FrameSignal OnFrame()
		{
			ThrowOnEmpty("Video::VideoSource");
            return FrameSignal(smart_source()->on_frame);
		}
				
		virtual void UnderlyingObject(core::video::video_source_interface** obj) const override
        {
            ThrowOnEmpty("Video::VideoSource");

			if (obj == nullptr)
				throw std::invalid_argument("obj");

			(*obj) = smart_source()->source();
			if ((*obj) != nullptr)
				(*obj)->add_ref();
        }

		virtual explicit operator core::video::video_source_interface*() const override
		{
			if (Empty() == true)
				return nullptr;

			return smart_source()->source();
		}				
	};

	class VideoSink : public VideoControllerTemplate<core::video::video_sink_interface>
	{
	private:
		utils::video::smart_video_sink* smart_sink() const
		{
			return static_cast<utils::video::smart_video_sink*>(static_cast<core::video::video_sink_interface*>(m_core_object));
		}

	public:
		VideoSink()
		{
			// Empty VideoSink
		}

		VideoSink(std::nullptr_t)
		{
			// Empty VideoSink
		}

		VideoSink(core::video::video_sink_interface* sink) :
			VideoControllerTemplate<core::video::video_sink_interface>(
				utils::make_ref_count_ptr<utils::video::smart_video_sink>(sink))
		{
		}

		bool SetFrame(const Video::Frame& frame)
		{
			ThrowOnEmpty("Video::VideoSink");
			return m_core_object->set_frame(static_cast<core::video::frame_interface*>(frame));
		}

		virtual void UnderlyingObject(core::video::video_sink_interface** obj) const override
		{
			ThrowOnEmpty("Video::VideoSink");

			if (obj == nullptr)
				throw std::invalid_argument("obj");

			(*obj) = smart_sink()->sink();
			if ((*obj) != nullptr)
				(*obj)->add_ref();
		}

		virtual explicit operator core::video::video_sink_interface*() const override
		{
			if (Empty() == true)
				return nullptr;

			return smart_sink()->sink();
		}
	};

	class VideoPublisher : public VideoControllerTemplate<core::video::video_publisher_interface>
	{
	private:
		utils::video::smart_video_publisher* smart_publisher() const
		{
			return static_cast<utils::video::smart_video_publisher*>(static_cast<core::video::video_publisher_interface*>(m_core_object));
		}

	public:
		VideoPublisher()
		{
			// Empty VideoPublisher
		}

		VideoPublisher(std::nullptr_t)
		{
			// Empty VideoPublisher
		}

		VideoPublisher(core::video::video_publisher_interface* publisher) :
			VideoControllerTemplate<core::video::video_publisher_interface>(
				utils::make_ref_count_ptr<utils::video::smart_video_publisher>(publisher))
		{
		}

		virtual void UnderlyingObject(core::video::video_publisher_interface** obj) const override
		{
			ThrowOnEmpty("Video::VideoPublisher");

			if (obj == nullptr)
				throw std::invalid_argument("obj");

			(*obj) = smart_publisher()->publisher();
			if ((*obj) != nullptr)
				(*obj)->add_ref();
		}

		virtual explicit operator core::video::video_publisher_interface*() const override
		{
			if (Empty() == true)
				return nullptr;

			return smart_publisher()->publisher();
		}
	};

	inline VideoSourceFactory::VideoSourceFactory()
	{
		// Empty Factory
	}

	inline VideoSourceFactory::VideoSourceFactory(core::video::video_source_factory_interface* factory) :
		Common::CoreObjectWrapper<core::video::video_source_factory_interface>(factory)
	{
	}

	inline VideoSource VideoSourceFactory::Create() const
	{
		ThrowOnEmpty("Video::VideoSourceFactory");

		utils::ref_count_ptr<core::video::video_source_interface> source;
		if (m_core_object->create(&source) == false)
			return Video::VideoSource(); // Empty Source

		return Video::VideoSource(source);
	}

	// Helpers / Trasnlators
	
	class VideoSourceFactoryBase :
		public utils::ref_count_base<core::video::video_source_factory_interface>
	{
	public:
		virtual ~VideoSourceFactoryBase() = default;	
		virtual Video::VideoSource Create() const = 0;		

	private:
		virtual bool create(core::video::video_source_interface** source) const override
		{
			Video::VideoSource instance = Create();
			if (instance.Empty() == true)
				return false;

			instance.UnderlyingObject(source);
			return true;
		}
	};
}
