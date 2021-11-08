#pragma once

#include <files/files_handler.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/thread_safe_object.hpp>
#include <utils/dispatcher.hpp>

#include <vector>
#include <algorithm>


namespace files
{
	/// @class	files_handler_impl
	///
	/// @brief	The files handler implementation.
	///
	/// @date	16/05/2018
	class files_handler_impl : public utils::ref_count_base<files::files_handler>
	{
	private:
		using files_container = std::vector<utils::ref_count_ptr<core::files::file_access_interface>>;
		utils::thread_safe_object<files_container> m_subscribed_files;
		utils::dispatcher m_io_thread;
		int m_timer_token;

	public:

		/// @brief	Constructor
		///
		/// @param	flush_interval	The flush interval.
		files_handler_impl(double flush_interval);

		/// @brief	Destructor
		virtual ~files_handler_impl();

		/// @brief	File subscription
		///
		/// @param [in,out]	file	If non-null, the file.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool subscribe_file(core::files::file_access_interface* file) override;

		/// @brief	Unsubscribe a file
		///
		/// @param [in,out]	file	If non-null, the file.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool unsubscribe_file(core::files::file_access_interface* file) override;

		/// @brief	Flushes all subscribed files
		virtual void flush() override;

		/// @brief	Flushes a specific file. Note that the input file doesn't has to
		///			be subscribed.
		///
		/// @param [in,out]	file	If non-null, the file.
		virtual void flush(core::files::file_access_interface* file) override;
	};
}