/// @file	.\Include\core\files.h.
/// @brief	Declares the files interfaces
#pragma once
#include <core/ref_count_interface.h>

namespace core
{
    namespace files
    {
        /// @class  file_access_interface
        /// 		
        /// @brief  An interface for specific file implementations.
        ///         Defines flush method that will implement file writing logic 
        ///         and format. Will be used by files handler to perform FS writing 
        ///         in low priority thread
        ///         
        /// @date   29/12/2018
        class DLL_EXPORT file_access_interface : public core::ref_count_interface
        {
        public:
            /// @brief  Default destructor
            virtual ~file_access_interface() = default;

            /// @brief  File flush interface for file writing logic and format
            /// 		
            /// @return True if it succeeds, false if it fails.
            virtual bool flush() = 0;
        };

        /// @class  files_handler_interface
        /// 		
        /// @brief  An interface for files handler thread.
        ///         Defines subscription interface for file accessors. The 
        ///         implementation of this interface will be responsible for all 
        ///         file accessors flushing process which will be done in low 
        ///         priority
        ///         
        /// @date   29/12/2017
        class DLL_EXPORT files_handler_interface : public core::ref_count_interface
        {
        public:
            /// @brief  Default destructor
            virtual ~files_handler_interface() = default;

            /// @brief  File subscription
            ///
            /// @param [in,out] file    If non-null, the file.
            ///
            /// @return True if it succeeds, false if it fails.
            virtual bool subscribe_file(core::files::file_access_interface* file) = 0;

            /// @brief  Unsubscribe a file
            ///
            /// @param [in,out] file    If non-null, the file.
            ///
            /// @return True if it succeeds, false if it fails.
            virtual bool unsubscribe_file(core::files::file_access_interface* file) = 0;

			/// @brief	Flushes all subscribed files
			virtual void flush() = 0;

			/// @brief	Flushes a specific file. Note that the input file doesn't has to
			/// 		be subscribed.
			///
			/// @param [in,out]	file	If non-null, the file.
			virtual void flush(core::files::file_access_interface* file) = 0;
        };
    }
}