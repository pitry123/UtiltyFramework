#include <core/ref_count_interface.h>
#include <core/framework.h>
#include <core/application.h>
#include <core/application.h>
#include <utils/signal.hpp>
#include <utils/thread_safe_object.hpp>
#include <utils/application.hpp>
#include <utils/console_synchronizer.hpp>
#include <utils/strings.hpp>
#include <utils/logging.hpp>
#include <utils/types.hpp>
#include <boost/filesystem/string_file.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <exception>
#include <atomic>
#include <mutex>

namespace utils
{
	namespace application
	{
		static constexpr size_t MAX_PARAMS = 20;

		using namespace core::application;
		using namespace std;
		using namespace boost::program_options;

#ifdef _WIN32
		static constexpr char DEF_USER_PATH[] = "%PROGRAMDATA%/ElbitSystemsLtd/";
#else
		static constexpr char DEF_USER_PATH[] = "~/.config/ElbitSystemsLtd/";
#endif

		static inline void color_print(bool is_force, bool is_sync, core::console::colors color, char const* const format, ...)
		{
			const size_t BUFFER_SIZE = 1024;
			char buffer[BUFFER_SIZE];

			va_list args;
			va_start(args, format);

#ifdef _WIN32
			vsnprintf_s(buffer, BUFFER_SIZE, format, args);
#else
			vsnprintf(buffer, BUFFER_SIZE, format, args);
#endif
			va_end(args);

			core::console::color_print(is_force, is_sync, color, buffer);
		}

		static inline bool parse_boolean(const char* str, bool& val)
		{
			if (str == nullptr)
				return false;

			std::string bool_str(str);
			if (bool_str == "off" || bool_str == "false")
			{
				val = false;
			}
			else if (bool_str == "on" || bool_str == "true")
			{
				val = true;
			}
			else
			{
				return false;
			}

			return true;
		}

		class cli_cmd : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			utils::ref_count_ptr<cli_callback_interface> m_callback;
			std::string m_cmd;
			std::string m_description;
		public:
			cli_cmd(std::string cmd, std::string description, cli_callback_interface* callback) :
				m_callback(callback),
				m_cmd(cmd),
				m_description(description)
			{

			}

			const char* description()
			{
				return m_description.c_str();
			}

			const char* cmd()
			{
				return m_cmd.c_str();
			}

			utils::ref_count_ptr<cli_callback_interface> callback()
			{
				return m_callback;
			}
		};

		/// A main application class which manages the CLI and and main thread.
		///
		/// @date	22/08/2018
		class main_app : public  utils::ref_count_base<core::application::application_interface>
		{
		public:
			main_app() :
				m_log_level(core::logging::severity::INFO),
				m_verbose(false),
				m_logger_hook(utils::make_ref_count_ptr<utils::logging::smart_logger_hook>())
			{	
				m_cli_enalbed = true;
				m_error_code = 0;
				m_logger_hook->logger_added += [this](core::logging::logger* logger)
				{
					add_logger(logger);
				};								
				core::framework::add_logger_hook(m_logger_hook);

				add_cli_cmds();
			}

			virtual ~main_app()
			{
				core::framework::remove_logger_hook(m_logger_hook);
			}

			/// Starts a CLI 
			/// Initiates the application main loop
			/// @date	22/08/2018
			/// @return	An int.
			int main_loop() override
			{
				try
				{
					m_is_running = true;
					if (false == m_cli_enalbed)
						return run_no_cli();
					
					while (m_is_running)
					{
						std::string params;
						std::string cmd;
						std::string input;

						color_print(true, true, core::console::WHITE, "%s>", app_name());
						std::getline(cin, input);
						
						if(input == "")
							continue;
						
						std::vector<std::string> params_list = split_string_with_quote(input, " ");
						if (params_list.size() == 0)
							continue;

						cmd = params_list[0];
						params_list.erase(params_list.begin());
						
						utils::ref_count_ptr<cli_callback_interface> callback;
						callback = m_callbacks.use<utils::ref_count_ptr<cli_callback_interface>>([&](subscriptions_map& callbacks)
						{
							utils::ref_count_ptr<cli_callback_interface> callback;
							auto it = callbacks.find(cmd);
							if (it == callbacks.end())
							{
								return callback;
							}

							return it->second->callback();
						});

						if (callback == nullptr)
						{
							if (cmd.empty() == false)
								report_unsupported(cmd, 0, nullptr);

							continue;
						}

						if (params_list.size() == 0)
						{
							callback->on_cli_input(cmd.c_str(), 0, nullptr);
						}
						else
						{
							const char* params_list_str[MAX_PARAMS];

							for (size_t i = 0; i < MAX_PARAMS; i++)
							{
								if (i >= params_list.size())
									break;

								params_list_str[i] = params_list[i].c_str();
							}

							callback->on_cli_input(cmd.c_str(), params_list.size(), params_list_str);
						}
					}
				}
				catch (std::exception &e)
				{
					color_print(true, true, core::console::colors::RED, "cli exception: %s", e.what());
					return -1;
				}

				return m_error_code;
			}

			int run_no_cli()
			{
				try
				{
					if(false == m_verbose)
						core::console::color_filter(core::console::colors::UNDEFINED_COLOR, false);

					m_is_running = true;
					
					auto pred = [&]() -> bool
					{
						return !m_is_running;
					};

					std::unique_lock<std::mutex> locker(m_waitMutex);
					m_wait_handle.wait(locker, pred);

				}
				catch (const std::exception& e)
				{
					m_is_running = false;
					color_print(false, true, core::console::colors::YELLOW, "Exit - block_no_cli - %s\n",e.what());
				}
				return m_error_code;
			}

			void exit(int error_code) override
			{
				std::unique_lock<std::mutex> locker(m_waitMutex);
				m_error_code = error_code;
				m_is_running = false;
				m_wait_handle.notify_all();
				cin.putback('\n');

			}
			/// Parse command line
			///
			/// @date	22/08/2018
			///
			/// @param	argc	The argc.
			/// @param	argv	The argv.
			///
			/// @return	True if it succeeds, false if it fails.
			virtual bool parse_cmd_line(int argc, const char* argv[]) override
			{
				options_description cmdline_options;
				try
				{
					std::string loglevel;

					//get the APP name 
					boost::filesystem::path app_path(argv[0]);
					m_app_name = app_path.stem().string();
					m_app_path = app_path.parent_path().string();
					if (argc > 1)
					{
						options_description generic("Generic Options");
						generic.add_options()
							// First parameter describes option name/short name
							// The second is parameter to option
							// The third is description
							("help,h", "print usage message")
							("version,v", "infrastructure version")
							("loglevel,l", value<std::string>(&loglevel)->default_value(""), "level of logging")
						    ("verbose,b", "define whether the system should run as verbose")
							("cli,c",value<bool>(&m_cli_enalbed)->default_value(true) , "define whether application should allow cli");
						options_description config("Configuration");
						config.add_options()
							("groupapp,g", value<std::string>(&m_app_group_name), "app group name")
							("paramfile,i", value<std::string>(&m_param_store_ini_path), "param file ini path")
							("factory,f", value<std::string>(&m_factory_path), "path to factory configuration path")
							("user,u", value<std::string>(&m_user_path), "path to user configuration path")
							("database,s", value<std::string>(&m_database_path), "path to database schema file (Debug Environment file)");


						cmdline_options.add(generic).add(config);

						variables_map vm;
						store(parse_command_line(argc, argv, cmdline_options), vm);
						notify(vm);
						if (vm.count("help"))
						{
							std::cout << cmdline_options << "\n";
							return false;
						}
						if (vm.count("verbose"))
						{
							m_verbose = true;
						}
						else
						{
							m_verbose = false;
						}

						conflicting_options(vm, "user", "paramfile");
						conflicting_options(vm, "factory", "paramfile");
					}
					
					if (loglevel.empty() == false)
					{
						core::logging::severity severity_level;
						if (parse(loglevel.c_str(), severity_level) == false)
							throw std::runtime_error("Failed to parse logging severity");

						m_log_level = severity_level;
						set_logs_filter(m_log_level);
					}
					if (m_app_path.empty())
					{
						//indicate current path
						m_app_path = ".";
					}
					

					if (m_database_path.empty())
					{
						m_database_path = m_app_path + "/DE/";
					}
					else
					{
						auto_expand_environment_variables(m_database_path);
						//if the path is with no file finalize the path, otherwise ignore
						if(m_database_path.rfind(".xml") == std::string::npos)
							finalize_path(m_database_path);
					}

					if (m_user_path.empty())
					{
						m_user_path = DEF_USER_PATH;
						m_user_path += "/Configuration/";
						auto_expand_environment_variables(m_user_path);
					}
					else
					{
						auto_expand_environment_variables(m_user_path);
						finalize_path(m_user_path);
						
					}

					if (m_factory_path.empty())
					{
						m_factory_path = m_app_path + "/Configuration/";
					}
					else
					{
						auto_expand_environment_variables(m_factory_path);
						finalize_path(m_factory_path);
					}
					
				}
				catch (const std::exception& e)
				{
					color_print(true, true, core::console::colors::RED, "%s\n", e.what());
					color_print(true, true, core::console::colors::RED, "Number of Params:%d\n", argc);
					for (int i = 0; i < argc; i++)
					{
						color_print(true, true, core::console::colors::RED, "Param:[%d]: %s\n", i, argv[i]);
					}
					color_print(true, true, core::console::colors::RED, "Command line: %s\n", e.what());
					std::cout << cmdline_options << "\n";
					if (argc == 1)
						return true;
					return false;
				}

				return true;
			}

			/// Adds a CLI command
			///
			/// @date	22/08/2018
			///
			/// @exception	std::invalid_argument	Thrown when an invalid argument
			/// 	error condition occurs.
			///
			/// @param 		   	cmd		   	The command.
			/// @param 		   	description	The description.
			/// @param [in]	    callback   	If non-null, the callback.
			///
			/// @return	True if it succeeds, false if it fails.
			bool  add_cli_cmd(const char* cmd, const char* description, core::application::cli_callback_interface* callback) override
			{
				try
				{
					if (cmd == nullptr)
						throw std::invalid_argument("cmd");

					if (callback == nullptr)
						throw std::invalid_argument("callback");

					return m_callbacks.use<bool>([&](subscriptions_map& callbacks)
					{
						auto it = callbacks.find(cmd);
						if (it != callbacks.end())
						{
							// CLI command override
							callbacks.erase(it); 
						}
						
						callbacks.emplace(cmd, utils::make_ref_count_ptr<cli_cmd>(cmd, description, callback));
						return true;
					});
				}
				catch (...)
				{
					return false;
				}
			}

			/// Deletes the command described by cmd
			///
			/// @date	22/08/2018
			///
			/// @exception	std::invalid_argument	Thrown when an invalid argument
			/// 	error condition occurs.
			///
			/// @param	cmd	The command.
			///
			/// @return	True if it succeeds, false if it fails.
			bool delete_cmd(std::string cmd)
			{
				try
				{
					if (cmd.empty() == true)
						throw std::invalid_argument("cmd");

					return m_callbacks.use<bool>([&](subscriptions_map& callbacks)
					{
						auto it = callbacks.find(cmd);
						if (it == callbacks.end())
							return false;

						callbacks.erase(it);
						return true;
					});
				}
				catch (...)
				{
					return false;
				}
			}

			/// Application name
			///
			/// @date	22/08/2018
			///
			/// @return	Null if it fails, else a pointer to a const char.
			const char* app_name() const override
			{
				return m_app_name.c_str();
			}

			/// Execution path
			///
			/// @date	22/08/2018
			///
			/// @return	Null if it fails, else a pointer to a const char.
			const char* execution_path() const override
			{
				return m_app_path.c_str();
			}

			/// Logs level of the application
			///
			/// @date	22/08/2018
			///
			/// @return	A core::logging::severity.
			core::logging::severity log_level() const override
			{
				return m_log_level;
			}

			/// Logs level of the application
			///
			/// @date	22/08/2018
			///
			/// @return	A core::logging::severity.
			void set_log_level(core::logging::severity log_level) override
			{
				m_log_level = log_level;
			}

			/// Parameter store ini file path
			///
			/// @date	22/08/2018
			///
			/// @return	Null if it fails, else a pointer to a const char.
			const char* param_store_ini_path() const override
			{
				return m_param_store_ini_path.c_str();
			}

			/// Factory path
			///
			/// @date	22/08/2018
			///
			/// @return	Null if it fails, else a pointer to a const char.
			const char* factory_path() const override
			{
				return m_factory_path.c_str();
			}

			bool verbose() const override
			{
				return m_verbose;
			}

			/// User path
			///
			/// @date	22/08/2018
			///
			/// @return	Null if it fails, else a pointer to a const char.
			const char* user_path() const override
			{
				return m_user_path.c_str();
			}

			/// User path
			///
			/// @date	22/08/2018
			///
			/// @return	Null if it fails, else a pointer to a const char.
			const char* database_path() const override
			{
				return m_database_path.c_str();
			}

			/// Project name
			///
			/// @date	22/08/2018
			///
			/// @return	Null if it fails, else a pointer to a const char.
			const char* app_group_name() const override
			{
				return m_app_group_name.c_str();
			}

			bool add_custom_info(const char* info) override
			{
				return m_info_vector.template use<bool>([&](info_vector& vec) -> bool
				{
					if (info == nullptr)
						return false;

					vec.emplace_back(info);
					return true;
				});
			}

			const char* get_custom_info_by_index(size_t index) const override
			{
				return m_info_vector.template use<const char*>([&](const info_vector& vec) -> const char*
				{
					if (index >= vec.size())
						return nullptr;

					return vec[index].c_str();
				});
			}

			bool add_logger(core::logging::logger* logger) 
			{
				if (logger == nullptr)
					return false;

				return m_loggers.use<bool>([&](logger_vector& loggers)
				{
					for (auto it = loggers.begin(); it != loggers.end(); it++)
					{
						if (std::strcmp((*it)->name(), logger->name()) == 0)
						{
							if (m_verbose)
							{
								std::stringstream str;
								str << "duplicate log name created: " << logger->name()<<"\n";
								core::console::color_print(true, true, core::console::colors::YELLOW, str.str().c_str());
							}
							
						}

					}
					loggers.emplace_back(logger);
					return true;
				});
			}

			bool remove_logger(const char* name) 
			{
				std::vector<int> dror;
				
				return m_loggers.use<bool>([&](logger_vector& loggers)
				{
					auto it = loggers.begin();
					for (;it!= loggers.end();it++)
					{
						if(std::strcmp((*it)->name(), name) == 0)
							break;

					}

					if (it != loggers.end())
					{
						loggers.erase(it);
						return true;
					}
					return false;
				});
			}

			bool query_logger(const char* name, core::logging::logger** logger) override
			{
				return m_loggers.use<bool>([&](logger_vector& loggers)
				{
					auto it = loggers.begin();
					for (; it != loggers.end(); it++)
					{
						if (std::strcmp((*it)->name(), name) == 0)
							break;

					}

					if (it != loggers.end())
					{
						*logger = *it;
						(*logger)->add_ref();
						return true;
					}
					return false;

				});
			}

			bool query_logger_by_index(size_t index, core::logging::logger** logger) override
			{
				if (logger == nullptr)
					return false;

				return m_loggers.use<bool>([&](logger_vector& loggers)
				{
					if (index > loggers.size())
						return false;
					

					*logger = loggers[index];
					(*logger)->add_ref();
					return true;
				});
			}

			size_t loggers_count() override
			{
				return m_loggers.use<size_t>([&](logger_vector& loggers)
				{
					return loggers.size();
				});
			}

		private:
			std::string m_app_group_name;
			std::string m_user_path;
			std::string m_factory_path;
			std::string m_param_store_ini_path;
			std::string m_database_path;
			std::string m_app_path;
			std::string m_app_name;
			std::atomic<core::logging::severity> m_log_level;
			bool m_verbose;
			bool m_cli_enalbed;
			bool m_is_running;
			
			bool m_error_code;
			std::mutex m_waitMutex;
			std::condition_variable m_wait_handle;
			
			using subscriptions_map =
				std::unordered_map<std::string, utils::ref_count_ptr<cli_cmd>>;
			utils::thread_safe_object<subscriptions_map> m_callbacks;

			using info_vector = std::vector<std::string>;
			utils::thread_safe_object<info_vector> m_info_vector;

			using logger_vector =
				std::vector<utils::ref_count_ptr<core::logging::logger>>;
			
			utils::thread_safe_object<logger_vector> m_loggers;
			utils::ref_count_ptr<utils::logging::smart_logger_hook> m_logger_hook;

			void report_unsupported(std::string cmd, size_t size, const char *params[])
			{
				stringstream os;
				if (size == 0)
				{
					os << "Command: '" << cmd << "' unknown" << endl;
				}
				else
				{
					os << "Command: '" << cmd << "' unknown with params: ";

					size_t i = 0;
					while (i < size)
					{
						os << params[i];
						i++;
						if (i < size)
						{
							os << ",";
						}
					}

					os << endl;
				}

				color_print(true, true, core::console::colors::RED, os.str().c_str());
			}

			void report_invalid_arguments(std::string cmd, size_t size, const char *params[])
			{
				stringstream os;
				if (size == 0)
				{
					os << "Command: '" << cmd << "'";
				}
				else
				{
					os << "Command: " << cmd << "'" << " with params: ";

					size_t i = 0;
					while (i < size)
					{
						os << params[i];
						i++;
						if (i < size)
						{
							os << ",";
						}
					}
				}

				os << " has invalid/missing arguments" << endl;

				color_print(true, true, core::console::colors::RED, os.str().c_str());
			}

			/// Function used to check that 'opt1' and 'opt2' are not specified
			///			at the same time.
			///
			/// @date	22/08/2018
			///
			/// @exception	logic_error	Raised when a logic error condition occurs.
			///
			/// @param	vm  	variable map
			/// @param	opt1	The first option.
			/// @param	opt2	The second option.
			void conflicting_options(const variables_map& vm,
				const char* opt1, const char* opt2)
			{
				if (vm.count(opt1) && !vm[opt1].defaulted()
					&& vm.count(opt2) && !vm[opt2].defaulted())
					throw logic_error(string("Conflicting options '")
						+ opt1 + "' and '" + opt2 + "'.");
			}

			/// Function used to check that of 'for_what' is specified, then
			///			'required_option' is specified too.
			///
			/// @date	22/08/2018
			///
			/// @exception	logic_error	Raised when a logic error condition occurs.
			///
			/// @param	vm  	variable map
			/// @param	for_what	   	the dependent option.
			/// @param	required_option	The required option according to the for what.
			void option_dependency(const variables_map& vm,
				const char* for_what, const char* required_option)
			{
				if (vm.count(for_what) && !vm[for_what].defaulted())
					if (vm.count(required_option) == 0 || vm[required_option].defaulted())
						throw logic_error(string("Option '") + for_what
							+ "' requires option '" + required_option + "'.");
			}

			bool add_cli_cmd(std::string cmd, std::string description, const smart_cli_callback::callback_func &func)
			{
				utils::ref_count_ptr<smart_cli_callback> cmdData = utils::make_ref_count_ptr<smart_cli_callback>(func);
				return add_cli_cmd(cmd.c_str(), description.c_str(), cmdData);
			}

		
				
		void print_log_list()
			{
				size_t i = 0;
				while (i < loggers_count())
				{
					utils::ref_count_ptr<core::logging::logger> log;
					if (false == query_logger_by_index(i, &log))
					{
						color_print(true, true, core::console::colors::RED, "undefined error\n");
					}

					char str[256];
					if (parse(log->filter(), str))
						color_print(true, true, core::console::colors::WHITE, " Log[%d]: %s Filter: %s Enabled:%d \n",i, log->name(), str, log->enabled());
					else
						color_print(true, true, core::console::colors::WHITE, " Log[%d]: %s Filter: unknown\n",i, log->name());
					i++;
				}
			}
			void set_logs_enabled(bool enable)
			{
				size_t i = 0;
				while (i < loggers_count())
				{
					utils::ref_count_ptr<core::logging::logger> log;
					if (false == query_logger_by_index(i, &log))
					{
						color_print(true, true, core::console::colors::RED, "undefined error\n");
					}
					log->enabled(enable);
					i++;
				}
			}

			void set_logs_filter(core::logging::severity sevirity)
			{
				size_t i = 0;
				while (i < loggers_count())
				{
					utils::ref_count_ptr<core::logging::logger> log;
					if (false == query_logger_by_index(i, &log))
					{
						color_print(true, true, core::console::colors::RED, "undefined error\n");
					}
					log->filter(sevirity);
					i++;
				}
			}

			void add_cli_cmds()
			{
				add_cli_cmd("help", "shows this help", [this](const char* cmd, size_t size, const char* params[])
				{
					m_callbacks.use<void>([&](subscriptions_map& callbacks)
					{
						for (auto &callback : callbacks)
						{
							color_print(true, true, core::console::colors::WHITE, "%s: %s\n\n",callback.second->cmd(), callback.second->description());
						}
					
					});

				});

				add_cli_cmd("exit", "exit the application", [this](const char* cmd, size_t size, const char* params[])
				{
					exit(0);
				});

				add_cli_cmd("print", "<off|on> <color> stop/resume colored console print display/hide the colors separated by |", [this](const char* cmd, size_t size, const char* params[])
				{
					try
					{
						if (size == 0)
						{
							report_invalid_arguments(cmd, size, params);
						}
						else if (size == 1)
						{
							bool enabled = false;
							if (parse_boolean(params[0], enabled))
							{
								core::console::color_filter(core::console::colors::UNDEFINED_COLOR, enabled);
							}
							else
							{
								report_invalid_arguments(cmd, size, params);
							}
						}
						else
						{
							bool enabled = false;
							if (parse_boolean(params[0], enabled))
							{
								core::console::colors result_color = core::console::colors::UNDEFINED_COLOR;
								std::vector<std::string> colors_list = split_string(params[1], "|");
								core::console::colors color;

								for (auto& color_str : colors_list)
								{
									if (parse(color_str.c_str(), color))
									{
										result_color = static_cast<core::console::colors>(result_color | color);
									}
									else
									{
										report_invalid_arguments(cmd, size, params);
										return;
									}
								}
								core::console::color_filter(result_color, enabled);
							}
							else
							{
								report_invalid_arguments(cmd, size, params);
							}
						}
					}
					catch (...)
					{
						report_unsupported(cmd, size, params);
					}
				});
				add_cli_cmd("ezversion", "print infrastructure version", [this](const char *cmd, size_t size, const char* params[])
				{
					try
					{
						color_print(true, true, core::console::colors::WHITE, "ezFramework Version: %s\n", core::framework::version());
					}
					catch (...)
					{
						report_unsupported(cmd, size, params);
					}
				});
				
				add_cli_cmd("log", "log list - Print all logs in the system and its severity filter \n log get <log name> - print log severity filter\n log set <sevirity filter> [<log name>| <log index>] if log name not set it will assign to all logs- set new severity filter\n - set the same severity filter to all logs ",
					[&](const char* cmd, size_t size, const char*params[])
				{
					std::string param(params[0]);
					if (size == 1)
					{
						if (param == "list")
						{
							print_log_list();
						}
						else if (param == "disable")
						{
							set_logs_enabled(false);
						}
						else if(param == "enable")
						{
							set_logs_enabled(true);
						}
						else
						{
							report_invalid_arguments(cmd, size, params);
						}

					}
					else if (size == 2)
					{
						std::string cmd_params(params[0]);
						if (cmd_params == "set")
						{
							core::logging::severity sevirity;
							if (false == parse(params[1], sevirity))
							{

								report_invalid_arguments(cmd, size, params);
								return;
							}
							
							set_logs_filter(sevirity);
						}
						else if (cmd_params == "get")
						{
							utils::ref_count_ptr<core::logging::logger> log;
							if (false == query_logger(params[1], &log))
							{
								color_print(true, true, core::console::colors::RED, "undefined Log name\n");
								return;
							}
							else
							{
								char str[256];
								if (parse(log->filter(), str))
									color_print(true, true, core::console::colors::WHITE, " Log: %s Filter: %s Enabled:%d \n", log->name(), str, log->enabled());
							}

						}
					}
					else if (size == 3)
					{
						core::logging::severity sevirity;
						utils::ref_count_ptr<core::logging::logger> log;
						if (utils::types::is_numeric(params[1]))
						{
                            size_t logger_index = static_cast<size_t>(std::stoi(params[1]));
							if (false == query_logger_by_index(logger_index, &log))
							{
								color_print(true, true, core::console::colors::RED, "undefined Log index\n");
								return;
							}
						}
						else if (false == query_logger(params[1], &log))
						{
							color_print(true, true, core::console::colors::RED, "undefined Log name\n");
							return;
						}

						std::string cmd_params(params[0]);
						if (cmd_params == "set")
						{
							if (false == parse(params[2], sevirity))
							{
								std::string enabled_str(params[2]);
								if (enabled_str == "enable")
								{
									log->enabled(true);
								}
								else if (enabled_str == "disable")
								{
									log->enabled(false);
								}
								else
									report_invalid_arguments(cmd, size, params);
							}
							else
							{
								log->filter(sevirity);
							}

						}
						else
							report_invalid_arguments(cmd, size, params);

					}
					else
						report_invalid_arguments(cmd, size, params);
				});
			}

			void finalize_path(std::string& path)
			{
				if (path[path.length() - 1] != '/' &&
					path[path.length() - 1] != '\\')
					path += "/";
			}
		};
	}

}

static utils::ref_count_ptr<core::application::application_interface> the_app = utils::make_ref_count_ptr<utils::application::main_app>();

bool core::application::application_interface::instance(core::application::application_interface** application)
{
	if (application == nullptr)
		return false;

	static std::once_flag flag;

	
	if (the_app == nullptr)
	{
		try
		{
			std::call_once(flag, [&]()
			{
				the_app = utils::make_ref_count_ptr<utils::application::main_app>();
			});
		}
		catch (...)
		{
			return false;
		}
	}
	
	if (the_app == nullptr)
		return false;

	the_app->add_ref();
	*application = the_app;
	return true;
}

