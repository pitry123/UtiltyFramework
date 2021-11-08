/// @file	core/stream_interface.h.
/// @brief	Declares the stream interface class
#pragma once
#include <cstdint>
#include <cstddef>
#include <core/ref_count_interface.h>

namespace core
{
	/// @enum	stream_status
	/// @brief	Values that represent stream status
	enum stream_status
	{
		/* success */
		status_no_error = 0,                // Operation succeeded without any warning

		/* errors */
		status_feature_unsupported = -1,    // Unsupported feature
		status_write_failed = -101,			// Failure in open stream in WRITE mode
		status_read_failed = -102,			// Failure in open stream in READ mode
		status_close_failed = -103,			// Failure while closing stream
		status_open_failed = -104,			// Failure while opening stream
	};

	/// @class	stream_interface
	/// @brief	An interface defining a stream.
	/// @date	14/05/2018
	class DLL_EXPORT stream_interface : 
		public core::ref_count_interface
	{
	public:
		/// @enum	relative_position
		/// @brief	Values that represent relative position options
		enum class relative_position
		{
			begin = 0,
			current = 1,
			end = 2,
		};

		/// @fn	stream_interface::~stream_interface() = default;
		/// @brief	Destructor
		/// @date	14/05/2018
		~stream_interface() = default;

        /// @fn	virtual core::stream_status stream_interface::flush() = 0;
        /// @brief	Flushes this object
        /// @date	14/05/2018
        /// @return	The core::stream_status.
        virtual core::stream_status flush() = 0;

		/// @fn	virtual core::stream_status stream_interface::reset(bool wipe_data) = 0;
		/// @brief	Resets the stream position
		/// @date	14/05/2018
		/// @param	wipe_data	True to clear all data.
		/// @return	The core::stream_status.
		virtual core::stream_status reset(bool wipe_data) = 0;

		/// @fn	virtual core::stream_status stream_interface::get_position(uint64_t& position) = 0;
		/// @brief	Gets the current position
		/// @date	14/05/2018
		/// @param [out]	position	The result position.
		/// @return	The core::stream_status.
		virtual core::stream_status get_position(uint64_t& position) = 0;

		/// @fn	virtual core::stream_status stream_interface::set_position(int64_t offset, core::stream_interface::relative_position relative_to) = 0;
		/// @brief	Sets the stream position
		/// @date	14/05/2018
		/// @param	offset	   	The stream offset.
		/// @param	relative_to	The offset's relativeness.
		/// @return	The core::stream_status.
		virtual core::stream_status set_position(int64_t offset, core::stream_interface::relative_position relative_to) = 0;

		/// @fn	virtual core::stream_status stream_interface::set_position(int64_t offset, core::stream_interface::relative_position relative_to, uint64_t& position) = 0;
		/// @brief	Sets the stream position
		/// @date	14/05/2018
		/// @param 		   	offset	   	The stream offset.
		/// @param 		   	relative_to	The offset's relativeness.
		/// @param [out]	position   	The new position.
		/// @return	The core::stream_status.
		virtual core::stream_status set_position(int64_t offset, core::stream_interface::relative_position relative_to, uint64_t& position) = 0;

		/// @fn	virtual core::stream_status stream_interface::read_bytes(void* data, size_t number_of_bytes_to_read, size_t& number_of_bytes_read) = 0;
		/// @brief	Reads bytes from stream
		/// @date	14/05/2018
		/// @param [in]		data				   	Buffer for writing the read bytes.
		/// @param 		   	number_of_bytes_to_read	Number of bytes to read.
		/// @param [out]	number_of_bytes_read   	Number of bytes read.
		/// @return	The core::stream_status.
		virtual core::stream_status read_bytes(void* data, size_t number_of_bytes_to_read, size_t& number_of_bytes_read) = 0;

		/// @fn	virtual core::stream_status stream_interface::write_bytes(const void* data, size_t number_of_bytes_to_write, size_t& number_of_bytes_written) = 0;
		/// @brief	Writes bytes to stream
		/// @date	14/05/2018
		/// @param 		   	data						The data.
		/// @param 		   	number_of_bytes_to_write	Number of bytes to write.
		/// @param [out]	number_of_bytes_written 	Number of bytes written.
		/// @return	The core::stream_status.
		virtual core::stream_status write_bytes(const void* data, size_t number_of_bytes_to_write, size_t& number_of_bytes_written) = 0;

		/// @fn	template <typename T> inline core::stream_status stream_interface::write_object(const T& obj)
		/// @brief	Writes an object to stream
		/// @date	14/05/2018
		/// @tparam	T	Generic type parameter.
		/// @param	obj	The object instance.
		/// @return	The core::stream_status.
		template <typename T>
		inline core::stream_status write_object(const T& obj)
		{
			size_t bytes_written;
			core::stream_status retval = write_bytes((void*)&obj, sizeof(T), bytes_written);
			if (retval != core::stream_status::status_no_error)
				return retval;

			if (bytes_written != sizeof(T))
				return core::stream_status::status_write_failed;

			return core::stream_status::status_no_error;
		}

		/// @fn	template <typename T> inline core::stream_status stream_interface::read_object(T& obj)
		/// @brief	Reads an object from stream
		/// @date	14/05/2018
		/// @tparam	T	Generic type parameter.
		/// @param [out]	obj	The object to write to.
		/// @return	The core::stream_status.
		template <typename T>
		inline core::stream_status read_object(T& obj)
		{
			size_t bytes_read;
			core::stream_status retval = read_bytes((void*)&obj, sizeof(T), bytes_read);
			if (retval != core::stream_status::status_no_error)
				return retval;

			if (bytes_read != sizeof(T))
				return core::stream_status::status_read_failed;

			return core::stream_status::status_no_error;
		}
	};
}

