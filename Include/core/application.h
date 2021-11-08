#pragma once
#include <core/os.h>
#include <core/ref_count_interface.h>
#include <core/logging.h>

#include <cstddef>

namespace core
{
	namespace application
	{
		class DLL_EXPORT runnable_interface : public virtual core::ref_count_interface
		{
		public:
			virtual ~runnable_interface() = default;

			virtual void init() = 0;
			virtual void start() = 0;
			virtual void started() = 0;
			virtual void stop() = 0;
			virtual void stopped() = 0;
		};

		/// A CLI callback interface.
		/// @date	21/08/2018
		class DLL_EXPORT cli_callback_interface : public ref_count_interface
		{
		public:
			/// Destructor
			/// @date	21/08/2018
			virtual ~cli_callback_interface() = default;

			/// Executes the data changed action
			/// @date	21/08/2018
			/// @param	key   	The key.
			/// @param	size  	The size.
			/// @param	buffer	The buffer.
			virtual void on_cli_input(const char* cmd, size_t size, const char* params[]) = 0;
		};

		/// A application interface.
		/// @date	23/08/2018
		class DLL_EXPORT application_interface : public ref_count_interface
		{
			public:
				virtual ~application_interface() = default;

				/// Starts the application CLI
				/// @date	23/08/2018
				/// @return	An int.
				virtual int main_loop() = 0;

				/// Exits the main loop of the application 
				/// @date	12/05/2020
				/// @param	error_code	The error code to exit with.
				virtual void exit(int error_code) = 0;

				/// Adds a CLI command
				/// @date	23/08/2018
				/// @param 		   	cmd		   	The command.
				/// @param 		   	description	The description.
				/// @param [in]		callback   	If non-null, the callback.
				/// @return	True if it succeeds, false if it fails.
				virtual bool  add_cli_cmd(const char* cmd, const char* description, core::application::cli_callback_interface* callback) = 0;

				/// Parse command line
				/// @date	23/08/2018
				/// @param	argc	The argc.
				/// @param	argv	The argv.
				/// @return	True if it succeeds, false if it fails.
				virtual bool parse_cmd_line(int argc, const char* argv[]) = 0;

				/// Application name
				/// @date	23/08/2018
				/// @return	the name of the application
				virtual const char* app_name() const = 0;

				/// Execution path
				/// @date	23/08/2018
				/// @return	the execution path.
				virtual const char* execution_path() const = 0;
								
				/// Logs the level
				/// @date	23/08/2018
				/// @return	A core::logging::severity the log level of the application
				virtual core::logging::severity log_level() const = 0;

				/// Sets log level
				/// @date	23/08/2018
				/// @param	log_level	The log level.
				virtual void set_log_level(core::logging::severity log_level) = 0;

				/// Parameter store initialize path
				/// @date	23/08/2018
				/// @return the path to the INI file
				virtual const char* param_store_ini_path() const = 0;

				/// Factory path
				/// @date	23/08/2018
				/// @return	path to the factory path
				virtual const char* factory_path() const = 0;

				/// User path
				/// @date	23/08/2018
				/// @return	path to the user path
				virtual const char* user_path() const  = 0;

				/// Database path - path to the Debug environment dataset
				/// @date	20/11/2018
				/// @return	Null if it fails, else a pointer to a const char.
				virtual const char* database_path() const = 0;

				/// Project name
				/// @date	23/08/2018
				/// @return	return the project name - it is a group where a set of applications belong to
				virtual const char* app_group_name() const = 0;

				/// Add any info to be use by the application for example packages version
				/// @date	23/08/2018
				/// @param  const char* information data that need to be save by the application
				/// @return	True if it succeeds, false if it fails.
				virtual bool  add_custom_info(const char* info) = 0;				
			
				/// Get custom info data per index
				/// @date	23/08/2018
				/// @param  index of the vector
				/// @return	const char* the info, nullptr if N/A
				virtual const char* get_custom_info_by_index(size_t index) const = 0;

				/// Queries a logger from the logger list
				/// @date	17/01/2020
				/// @param 		   	name  	The name.
				/// @param [in,out]	logger	If non-null, the logger.
				/// @return	True if it succeeds, false if it fails.
				virtual bool query_logger(const char* name, core::logging::logger** logger) = 0;

				/// Queries a logger from the logger list by index
				/// @date	17/01/2020
				/// @param 		   	name  	The name.
				/// @param [in,out]	logger	If non-null, the logger.
				/// @return	True if it succeeds, false if it fails.
				virtual bool query_logger_by_index(size_t index, core::logging::logger** logger) = 0;

				/// Queries a logger from the logger list by index
				/// @date	17/01/2020
				/// @param 		   	name  	The name.
				/// @param [in,out]	logger	If non-null, the logger.
				/// @return	True if it succeeds, false if it fails.
				virtual size_t loggers_count() = 0;

				/// get Verbose status 
				/// @date	27/01/2020
				/// @return	True if it succeeds, false if it fails.
				virtual bool verbose() const = 0;

				/// Instances the given application
				/// @date	23/08/2018
				/// @param [in,out]	application	If non-null, the application.
				/// @return	True if it succeeds, false if it fails.
				static bool instance(application_interface** application);
		};
	}
}
