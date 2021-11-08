/// @file	files/files_handler.h.
/// @brief	Declares the files handler class
#pragma once
#include <core/files.h>

namespace files
{
    /// @class  files_hander
    /// 		
    /// @brief  An async auto/manual files flush handler.
    ///         Flushing the subscribed files asynchronously in a low priority thread.
    ///         Same thread (context) is being used also when flush() and flush(core::files::file_access_interface*)
    ///         are literally called.
    ///         
    /// @date   11/03/2018
    class DLL_EXPORT files_handler : public core::files::files_handler_interface
    {
    public:
        /// @brief  Virtual default destructor
        virtual ~files_handler() = default;

		/// @brief	Static factory: Creates new files handler instance
		/// 		
		/// @param 		   	flush_interval	The desired flush interval in milliseconds.
		/// @param [out]	handler		  	An address of a pointer to core::files::files_handler_interface
		/// 				
		/// @return	True if it succeeds, false if it fails.
		static bool create(double flush_interval, core::files::files_handler_interface** handler);
    };
}