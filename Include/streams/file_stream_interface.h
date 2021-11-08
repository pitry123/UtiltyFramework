/// @file	streams/file_stream_interface.h.
/// @brief	Declares the file stream class
#pragma once
#include <core/stream_interface.h>

namespace streams
{
	/// @class	file_stream_interface
	/// @brief	File streams access files for read and write by using the generic core::stream_interface API
	/// @date	15/05/2018
	class DLL_EXPORT file_stream_interface : public core::stream_interface
	{
	public:
		/// @enum	access_mode
		/// @brief	Values that represent available access modes
		enum access_mode
		{
			read = 1,
			write = 2,
            read_write = 3,
            append = 4
		};

		/// @fn	virtual file_stream_interface::~file_stream_interface() = default;
		/// @brief	Destructor
		/// @date	15/05/2018
		virtual ~file_stream_interface() = default;

		/// @fn	static bool file_stream_interface::create(const char* file_path, streams::file_stream_interface::access_mode mode, core::stream_interface** stream);
		/// @brief	Static factory: Creates a new file stream instance
		/// @date	15/05/2018
		/// @param 		   	file_path	Full pathname of the file.
		/// @param 		   	mode	 	The desired access mode.
		/// @param [out]	stream   	An address of a pointer to core::stream_interface
		/// @return	True if it succeeds, false if it fails.
		static bool create(const char* file_path,
                           streams::file_stream_interface::access_mode mode,
			               core::stream_interface** stream);
	};
}