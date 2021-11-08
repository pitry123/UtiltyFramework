/// @file	core/geographic.h.
/// @brief	Declares the geographic interface classes
#pragma once
#include <core/ref_count_interface.h>

namespace core
{
	namespace geographic
	{
		/// @struct	vector3d
		/// @brief	A geographic 3d vector. Allows representation of location, orientation, etc...
		/// @date	14/05/2018
		struct vector3d
		{
			double x;
			double y;
			double z;
		};

		/// @struct	fov
		/// @brief	A camera field of view structure.
		/// @date	14/05/2018
		struct fov
		{
			double horizontal;
			double vertical;
		};

		/// @struct	telemetry
		/// @brief	A telemetry structure.
		/// @date	14/05/2018
		struct telemetry
		{
            core::geographic::vector3d location;			// lat : long : height
			core::geographic::vector3d location_error;		// lat : long : height
            core::geographic::vector3d orientation ;		// yaw : pitch : roll
			core::geographic::vector3d orientation_error;	// yaw : pitch : roll
            core::geographic::fov fov;
		};

		/// @class	telemetry_callback_interface
		/// @brief	An interface defining a telemetry callback which can be subscribed to a telemetry server.
		/// @date	14/05/2018
		class telemetry_callback_interface :
			public core::ref_count_interface
		{
		public:
			/// @fn	telemetry_callback_interface::~telemetry_callback_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			~telemetry_callback_interface() = default;

			/// @fn	virtual void telemetry_callback_interface::on_data(const core::telemetry& telemetry) = 0;
			/// @brief	Handles telemetry data signals
			/// @date	14/05/2018
			/// @param	telemetry	The telemetry.
            virtual void on_data(const core::geographic::telemetry& telemetry) = 0;
		};

		/// @class	telemetry_server_interface
		/// @brief	An interface defining a telemetry server.
		/// 		Telemetry servers are usually connected to a camera and report its geographic parameters
		/// @date	14/05/2018
		class telemetry_server_interface :
			public core::ref_count_interface
		{
		public:
			/// @fn	telemetry_server_interface::~telemetry_server_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			~telemetry_server_interface() = default;

			/// @fn	virtual bool telemetry_server_interface::add_callback(core::telemetry_callback_interface* callback) = 0;
			/// @brief	Subscribes a telemetry callback
			/// @date	14/05/2018
			/// @param [in]		callback	The callback.
			/// @return	True if it succeeds, false if it fails.
            virtual bool add_callback(core::geographic::telemetry_callback_interface* callback) = 0;

			/// @fn	virtual bool telemetry_server_interface::remove_callback(core::telemetry_callback_interface* callback) = 0;
			/// @brief	Unsubscribes a telemetry callback
			/// @date	14/05/2018
			/// @param [in]		callback	The callback.
			/// @return	True if it succeeds, false if it fails.
            virtual bool remove_callback(core::geographic::telemetry_callback_interface* callback) = 0;
		};
	}
}
