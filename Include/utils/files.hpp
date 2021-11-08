#pragma once

#include <core/files.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <fstream>
#include <stdexcept>
#include <mutex>


namespace utils
{
	namespace files
	{
		/// @class  file_access_base
		///
		/// @brief  Base class for all file access interface implementations
		///         Role:
		///         Manages the common resources for all types of file access 
		///         interface
		///         Responsibility:
		///         To create and manage file stream within the file access 
		///         specific type
		///         
		/// @tparam T   Generic type parameter.
		///
		/// @date   01/01/2018
		template <typename T>
		class file_access_base : public utils::ref_count_base<T>
		{
		protected:
			mutable std::mutex      m_file_mutex;
			mutable std::fstream    m_file;
			std::string             m_file_path;

		public:
			/// @brief  Constructor
			///
			/// @exception  std::runtime_error  Raised when a file creation fails
			///
			/// @param  file_path   Full pathname of the file.
			/// @param  mode        The file mode.
			file_access_base(const char* file_path)
				: m_file_path(file_path)
			{
			}

			/// @brief  Destructor
			virtual ~file_access_base() = default;

			/// @brief	Getter for full file pathname
			///
			/// @return	Full file pathname.
			const char* get_name() const { return m_file_path.c_str(); }
		};
	}
}