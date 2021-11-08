/// @file	core/video.h.
/// @brief	Declares the video interface classes
#pragma once
#include <core/imaging.h>

namespace core
{
	namespace video
	{
#pragma pack(1)
		/// @struct	display_params
		/// @brief	A video frame's display parameters.
		/// @date	14/05/2018
		struct DLL_EXPORT display_params
		{
			/// @brief	The decoding timestamp
			uint64_t dts;
			/// @brief	The presentation timestamp
			uint64_t pts;
			/// @brief	The frame's duration
			uint64_t duration;
			/// @brief	User timestamp (0 if not available)
			uint64_t timestamp;
			/// @brief	The frame's ID
			uint64_t frame_id;
		};

		/// @enum	interlace_mode
		/// @brief	Values that represent video interlace modes
		enum interlace_mode
		{
			PROGRESSIVE,
			INTERLEAVED,
			MIXED,
			FIELDS,
			UNDEFINED_INTERLACE_MODE
		};

		/// @struct	framerate
		/// @brief	A video framerate represented as fraction of 2 native numbers
		/// @date	14/05/2018
		struct DLL_EXPORT framerate
		{
			/// @brief	The numerator
			uint32_t numerator;
			/// @brief	The denominator
			uint32_t denominator;
		};

		/// @enum	video_data_type
		/// @brief	Values that represent video data types
		enum video_data_type
		{
			RAW,
			H264,
			UNDEFINED_VIDEO_DATA_TYPE
		};

		/// @struct	video_params
		/// @brief	A video stream parameters.
		/// @date	14/05/2018
		struct DLL_EXPORT video_params
		{
			/// @brief	The video interlace mode
			core::video::interlace_mode interlace_mode; //interlace_mode t:int
			/// @brief	The video framerate
			core::video::framerate framerate;	//t:framerate
			/// @brief	The video data type
			core::video::video_data_type data_type;	//t:video_data_type
		};
#pragma pack()

		/// @class	frame_interface
		/// @brief	An interface defining a video frame.
		/// @date	14/05/2018
		class DLL_EXPORT frame_interface :
			public core::imaging::image_interface
		{
		public:
			/// @fn	virtual frame_interface::~frame_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~frame_interface() = default;

			/// @fn	virtual bool frame_interface::query_display_params(core::video::display_params& display_params) const = 0;
			/// @brief	Queries the display parameters
			/// @date	14/05/2018
			/// @param	[out]	display_params	The result display parameters.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_display_params(core::video::display_params& display_params) const = 0;

			/// @fn	virtual bool frame_interface::query_video_params(core::video::video_params& video_params) const = 0;
			/// @brief	Queries the video parameters
			/// @date	14/05/2018
			/// @param	[out]	video_params	The result video parameters.			
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_video_params(core::video::video_params& video_params) const = 0;
		};

		/// @enum	video_state
		/// @brief	Values that represent video source states
		enum video_state
		{
			STOPPED,
			PAUSED,
			PLAYING
		};

		/// @class	frame_callback
		/// @brief	An interface defining a frame callback which can be subscribed to a video source.
		/// @date	14/05/2018
		class DLL_EXPORT frame_callback :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual frame_callback::~frame_callback() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~frame_callback() = default;

			/// @fn	virtual void frame_callback::on_frame(core::video::frame_interface* frame) = 0;
			/// @brief	Handles new frame signals
			/// @date	14/05/2018
			/// @param [in]		frame	The new frame.
			virtual void on_frame(core::video::frame_interface* frame) = 0;
		};

		/// @class	video_error_callback
		/// @brief	An interface defining a video error callback which can be subscribed to a video source.
		/// @date	14/05/2018
		class DLL_EXPORT video_error_callback :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual video_error_callback::~video_error_callback() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~video_error_callback() = default;

			/// @fn	virtual void video_error_callback::on_error(int error_code) = 0;
			/// @brief	Handles error signals
			/// @date	14/05/2018
			/// @param	error_code	The error code.
			virtual void on_error(int error_code) = 0;
		};

		/// @class	video_controller_interface
		/// @brief	An interface defining a video controller which controls a video state
		/// @date	14/05/2018
		class DLL_EXPORT video_controller_interface :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual video_controller_interface::~video_controller_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~video_controller_interface() = default;

			/// @fn	virtual core::video::video_state state() = 0;
			/// @brief	Get the cuurent video state
			/// @date	14/05/2018
			virtual core::video::video_state state() = 0;

			/// @fn	virtual void video_controller_interface::start() = 0;
			/// @brief	Starts video
			/// @date	14/05/2018
			virtual void start() = 0;

			/// @fn	virtual void video_controller_interface::stop() = 0;
			/// @brief	Stops video
			/// @date	14/05/2018
			virtual void stop() = 0;

			/// @fn	virtual void video_controller_interface::pause() = 0;
			/// @brief	Pauses video
			/// @date	14/05/2018
			virtual void pause() = 0;

			/// @fn	virtual bool video_controller_interface::add_error_callback(core::video::video_error_callback* callback) = 0;
			/// @brief	Subscribes an error callback
			/// @date	14/05/2018
			/// @param [in]		callback	The callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_error_callback(core::video::video_error_callback* callback) = 0;

			/// @fn	virtual bool video_controller_interface::remove_error_callback(core::video::video_error_callback* callback) = 0;
			/// @brief	Unsubscribe an error callback
			/// @date	14/05/2018
			/// @param [in]		callback	The callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool remove_error_callback(core::video::video_error_callback* callback) = 0;
		};

		/// @class	video_source_interface
		/// @brief	An interface defining a video source.
		/// @date	14/05/2018
		class DLL_EXPORT video_source_interface :
			public core::video::video_controller_interface
		{
		public:
			/// @fn	virtual video_source_interface::~video_source_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~video_source_interface() = default;

			/// @fn	virtual bool video_source_interface::add_frame_callback(core::video::frame_callback* callback) = 0;
			/// @brief	Subscribes a frames callback
			/// @date	14/05/2018
			/// @param [in]		callback	The callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_frame_callback(core::video::frame_callback* callback) = 0;

			/// @fn	virtual bool video_source_interface::remove_frame_callback(core::video::frame_callback* callback) = 0;
			/// @brief	Unsubscribe a frames callback
			/// @date	14/05/2018
			/// @param [in]		callback	The callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool remove_frame_callback(core::video::frame_callback* callback) = 0;
		};

		/// @class	video_sink_interface
		/// @brief	An interface defining a video sink.
		/// 		Sinks can manipulate frames being set to them.
		/// 		Example of a common sink would be a video encoder.
		/// @date	14/05/2018
		class DLL_EXPORT video_sink_interface :
			public core::video::video_controller_interface
		{
		public:
			/// @fn	virtual video_sink_interface::~video_sink_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~video_sink_interface() = default;

			/// @fn	virtual bool video_sink_interface::set_frame(core::video::frame_interface* frame) = 0;
			/// @brief	Sets a frame to the sink
			/// @date	14/05/2018
			/// @param [in]		frame	The frame.
			/// @return	True if it succeeds, false if it fails.
			virtual bool set_frame(core::video::frame_interface* frame) = 0;
		};

		/// @class	video_source_factory_interface
		/// @brief	An interface defining a video source factory.
		/// @date	14/05/2018
		class DLL_EXPORT video_source_factory_interface :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual video_source_factory_interface::~video_source_factory_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~video_source_factory_interface() = default;

			/// @fn	virtual bool video_source_factory_interface::create(core::video::video_source_interface** source) const = 0;
			/// @brief	Creates a new video source
			/// @date	14/05/2018
			/// @param [out]	source	An address to a source pointer.
			/// @return	True if it succeeds, false if it fails.
			virtual bool create(core::video::video_source_interface** source) const = 0;
		};

		/// @class	video_publisher_interface
		/// @brief	An interface defining a video publisher.
		/// @date	14/05/2018
		class DLL_EXPORT video_publisher_interface :
			public core::video::video_controller_interface
		{
		public:
			/// @fn	virtual video_publisher_interface::~video_publisher_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~video_publisher_interface() = default;
		};
	}
}
