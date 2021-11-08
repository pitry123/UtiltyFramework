#pragma once
#include <core/application.h>
#include <utils/console_synchronizer.hpp>
#include <utils/application.hpp>

#include <Common.hpp>
#include <Core.hpp>

#include <vector>
#include <memory>
#include <Files.hpp>
#include <Strings.hpp>

namespace Application
{
	/// A runnable class modern API class that wrap a core runnable interface 
	/// Builder is maintaining a list of runnable interface init, start and stop them
	/// @date	09/11/2018
	class Runnable : public Common::CoreObjectWrapper<core::application::runnable_interface>
	{
	public:
		Runnable()
		{
			// Empty runnable
		}

		Runnable(core::application::runnable_interface* core_runnable) :
			Common::CoreObjectWrapper<core::application::runnable_interface>(core_runnable)
		{
		}

		template<typename T, typename... Args>
		static Runnable Create(Args&&... args)
		{
			utils::ref_count_ptr<core::application::runnable_interface> instance = utils::make_ref_count_ptr<T>(std::forward<Args>(args)...);
			return Runnable(instance);
		}

		void Init()
		{
			ThrowOnEmpty("Application::Runnable");
			m_core_object->init();
		}

		void Start()
		{
			ThrowOnEmpty("Application::Runnable");
			m_core_object->start();
		}

		void Started()
		{
			ThrowOnEmpty("Application::Runnable");
			m_core_object->started();
		}

		void Stop()
		{
			ThrowOnEmpty("Application::Runnable");
			m_core_object->stop();
		}

		void Stopped()
		{
			ThrowOnEmpty("Application::Runnable");
			m_core_object->stopped();
		}
	};

	/// A builder class that is in cahrge of building the system configuration and dispatchers
	/// @date	09/11/2018
	class Builder
	{
	private:
		std::vector<Application::Runnable> m_runnables;

	protected:
		void AddRunnable(const Application::Runnable& runnable)
		{
			m_runnables.emplace_back(runnable);
		}

		void AddRunnable(core::application::runnable_interface* runnable)
		{
			m_runnables.emplace_back(Application::Runnable(runnable));
		}

		template<typename T, typename... Args>
		void AddRunnable(Args&&... args)
		{
			utils::ref_count_ptr<core::application::runnable_interface> runnable = utils::make_ref_count_ptr<T>(std::forward<Args>(args)...);
			AddRunnable(runnable);
		}

		virtual void BuildEnvironment() = 0;
		virtual void BuildDispatchers() = 0;
		
		virtual void OnInitialized()	{ /* Do Nothing */ }
		virtual void OnStarting()		{ /* Do Nothing */ }
		virtual void OnStarted()		{ /* Do Nothing */ };
		virtual void OnStopping()		{ /* Do Nothing */ };
		virtual void OnStopped()		{ /* Do Nothing */ };	

	private:
		/// Initializes all runnable on the runnable list
		/// @date	09/11/2018
		void Init()
		{
			for (auto& runnable : m_runnables)
				runnable.Init();

			OnInitialized();
		}

		/// Starts all runnable - this indicate to the runnable that all other runnable are initiated
		/// @date	09/11/2018
		void Start()
		{
			OnStarting();

			for (auto& runnable : m_runnables)
				runnable.Start();
		}

		/// Started indicate to all runnable that start function was called to all runnables
		/// @date	09/11/2018
		void Started()
		{
			for (auto& runnable : m_runnables)
				runnable.Started();

			OnStarted();
		}

		/// Stops all runnables
		/// @date	09/11/2018
		void Stop()
		{
			OnStopping();

			for (size_t i = 0; i < m_runnables.size(); i++)
				m_runnables[m_runnables.size() - (i + 1)].Stop();			
		}

		/// Indicate to all runnables that stop was called
		/// @date	09/11/2018
		void Stopped()
		{
			for (size_t i = 0; i < m_runnables.size(); i++)
				m_runnables[m_runnables.size() - (i + 1)].Stopped();

			OnStopped();
		}

	public:
		Builder()
		{
		}

		virtual ~Builder()
		{
			Stop();
			Stopped();

			for (size_t i = 0; i < m_runnables.size(); i++)
				m_runnables[m_runnables.size() - (i + 1)] = nullptr;
		}

		/// Main function of builder this function build and start the application
		/// @date	09/11/2018
		void Build()
		{
			BuildEnvironment();
			BuildDispatchers();

			Init();
			Start();
			Started();
		}
	};

	/// A main application.
	/// MainApp wrap all runtime environment issues create and start the builder
	/// It wrap the interface to main_app which is implemented in the core module
	/// It manage the provide CLI interface and command line arguments management
	/// It manage the application version (based on CMake Build)
	/// main() should call BuildAndRun function to start the application
	/// @date	09/11/2018
	class MainApp : public Common::CoreObjectWrapper<core::application::application_interface>
	{
	public:
		using CliCommandCallbackFunc = utils::application::smart_cli_callback::callback_func;

		MainApp()
		{
			core::application::application_interface::instance(&m_core_object);
			ThrowOnEmpty("Application::MainApp");
		}

		MainApp(int argc, const char* argv[]) :
			MainApp()
		{
			if (m_core_object->parse_cmd_line(argc, argv) == false)
				throw std::runtime_error("Failed to parse command line arguments");
		}

		MainApp(core::application::application_interface* main_app) :
			Common::CoreObjectWrapper<core::application::application_interface>(main_app)
		{
		}

		virtual ~MainApp() = default;

		/// Builds and run 
		/// Build and Run the application, should be called from main 
		///  This is a blocking function that manage the main application loop
		/// see variadic template BuildAndRun() that also mange the creation of the Builder Class
		/// @date	09/11/2018
		/// @param [in,out]	builder	The builder.
		/// @return	An int.
		int BuildAndRun(Builder& builder)
		{
			ThrowOnEmpty("Application::MainApp");

			PrintBanner();
			PrintEzFrameworkVersion();
			PrintApplicationVersion();
			AddCLICmds();
			
			builder.Build();
			return MainLoop();
		}

		/// Builds and run
		/// Build and Run the application, should be called from main
		/// This is the recomended function to be called as the startup call from main()
		/// @date	09/11/2018
		/// @tparam	T   	Generic type parameter should be the concrete class of the application Builder.
		/// @tparam	Args	Type of the arguments to pass to the Builder Class.
		/// @param	args	Variable arguments providing [in] The arguments.
		/// @return	An int.
		template<typename T, typename... Args>
		int BuildAndRun(Args&&... args)
		{
			T builder(std::forward<Args>(args)...);
			return BuildAndRun(builder);
		}

		void Exit(int errorCode = 0)
		{
			ThrowOnEmpty("Empty MainApp");
			m_core_object->exit(errorCode);
		}

		/// Factory settings - return the path to the location where the configuration file of the factory setting is located
		/// @date	09/11/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @return	A const std::string.
		const std::string FactorySettings()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty MainApp");

			return m_core_object->factory_path();
		}

		/// Developer settings
		/// return the path to the location where the configuration file of the Developer setting is located
		/// NOT IN USE!
		/// @date	09/11/2018
		/// @return	A const std::string.
		const std::string DeveloperSettings()
		{
			return "";
		}

		/// User settings
		/// return the path to the location where the configuration file of the User setting is located
		/// @date	09/11/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @return	A const std::string.
		const std::string UserSettings()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty MainApp");

			return m_core_object->user_path();
		}

		/// Database path return the path to the location where the
		/// dataset file of the DebugEnvironment is located
		/// @date	09/11/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @return	A const std::string.
		const std::string DataBasePath()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty MainApp");

			return m_core_object->database_path();
		}

		/// Parameter strore initialize path to INI file that set the paths to all other files
		/// @date	09/11/2018
		/// @return	A const std::string.
		const std::string ParamStroreINIPath()
		{
			ThrowOnEmpty("Application::MainApp");
			return m_core_object->param_store_ini_path();
		}

		/// return the Application name
		/// @date	09/11/2018
		/// @return	A const std::string.
		const std::string  AppName()
		{
			ThrowOnEmpty("Application::MainApp");
			return m_core_object->app_name();
		}

		/// return the Execution path
		/// @date	09/11/2018
		/// @return	A const std::string.
		const std::string  ExecutionPath()
		{
			ThrowOnEmpty("Application::MainApp");
			return m_core_object->execution_path();
		}

		/// return the Application group name, this is a free text use that alloes grouping several application configuration paths into one place
		/// @date	09/11/2018
		/// @return	A const std::string.
		const std::string  AppGroupName()
		{
			ThrowOnEmpty("Application::MainApp");
			return m_core_object->app_group_name();
		}

		/// Versions of the application
		/// @date	09/11/2018
		/// @param [out]	ver	The version.
		/// @return	True if it succeeds, false if it fails.
		bool Version(Core::Framework::VersionStruct& ver)
		{
#ifdef APP_VERSION_MAJOR
			ver.major = APP_VERSION_MAJOR;
			ver.minor = APP_VERSION_MINOR;
			ver.patch = APP_VERSION_PATCH;
			ver.build = APP_VERSION_BUILD;
			return true;
#else
			(void)ver;
			return false;
#endif // APP_VERSION_MAJOR
		}

		/// Gets the version in a string format
		/// @date	09/11/2018
		/// @return	A std::string.
		std::string Version()
		{
			Core::Framework::VersionStruct version;
			if (Version(version) == false)
				return "";

			std::stringstream stream;
			stream << version;

#ifdef APP_VERSION_BUILD
				stream << " (build: " << version.build << ")";
#endif // APP_VERSION_BUILD

			return stream.str();
		}

		bool Verbose() 
		{
			ThrowOnEmpty("MainApp");
			return m_core_object->verbose();
		}
				
		/// Adds a CLI command to be used on runtime
		/// @date	09/11/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	cmd		   	The command line.
		/// @param	description	The description.
		/// @param	func	   	The function.
		/// @return	True if it succeeds, false if it fails.
		bool AddCLICmd(std::string cmd, std::string description, const CliCommandCallbackFunc& func)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty MainApp");

			utils::ref_count_ptr<core::application::cli_callback_interface> cmdData = utils::make_ref_count_ptr<utils::application::smart_cli_callback>(func);
			return m_core_object->add_cli_cmd(cmd.c_str(), description.c_str(), cmdData);
		}

		bool AddCustomInfo(const std::string& info)
		{
			ThrowOnEmpty("MainApp");
			return m_core_object->add_custom_info(info.c_str());
		}
		
		const char* GetCustomInfoByIndex(size_t index)
		{
			ThrowOnEmpty("MainApp");
			return m_core_object->get_custom_info_by_index(index);
		}

		Logging::Logger Log(const char* name)
		{
			utils::ref_count_ptr <core::logging::logger> log;
			ThrowOnEmpty("MainApp");
			if (false == m_core_object->query_logger(name, &log))
				return Logging::Logger();

			return Logging::Logger(log);
		}

		Logging::Logger Log(size_t index)
		{
			utils::ref_count_ptr <core::logging::logger> log;
			ThrowOnEmpty("MainApp");
			if (false == m_core_object->query_logger_by_index(index, &log))
				return Logging::Logger();

			return Logging::Logger(log);
		}

		size_t LoggrsCount()
		{
			ThrowOnEmpty("MainApp");
			return m_core_object->loggers_count();
		}

	protected:

		/// Print ezframework version
		/// @date	09/11/2018
		virtual void PrintEzFrameworkVersion()
		{
			Core::Console::ColorPrint(true, true, Core::Console::Colors::GREEN, "ezFramework Version: %s\n", Core::Framework::Version());
		}

		/// Print application version
		/// @date	09/11/2018
		virtual void  PrintApplicationVersion()
		{
			Core::Console::ColorPrint(true, true, Core::Console::Colors::GREEN, "%s Version: %s\n", AppName().c_str(), Version().c_str());
		}

		/// Adds CLI commands - default commands
		/// @date	09/11/2018
		virtual void AddCLICmds()
		{
			ThrowOnEmpty("Application::MainApp");

			AddCLICmd("version", "print all versions", [&](const char* cmd, size_t size, const char*params[]) 
			{
				(void)cmd;
				(void)size;
				(void)params;

				PrintApplicationVersion();
				PrintEzFrameworkVersion();
			});
		}

		/// Print banner
		/// @date	09/11/2018
		virtual void PrintBanner()
		{
			printf("\n%s\n", "         ______                                           _    \n         |  ___|                                         | |   \n  ___ ___| |_ _ __ __ _ _ __ ___   _____      _____  _ __| | __\n / _ \\_  /  _| '__/ _` | '_ ` _ \\ / _ \\ \\ /\\ / / _ \\| '__| |/ /\n|  __// /| | | | | (_| | | | | | |  __/\\ V  V / (_) | |  |   < \n \\___/___\\_| |_|  \\__,_|_| |_| |_|\\___| \\_/\\_/ \\___/|_|  |_|\\_\\\n                                                               \n                                                               ");
		}		

	private:
		int MainLoop()
		{
			return m_core_object->main_loop();
		}

	};

	// ModernAPI helpers/translators

	/// @class	RunnableBase
	/// @brief	Modern API implementation for runnable_interface
	/// @date	12/08/2019
	class RunnableBasePure : public core::application::runnable_interface
	{
	public:
		/// @brief	Default Destructor
		virtual ~RunnableBasePure() = default;

		virtual void Init() = 0;
		virtual void Start() = 0;
		virtual void Started() = 0;
		virtual void Stop() = 0;
		virtual void Stopped() = 0;

	private:
		// --------------------------------------------------------------------
		// Implement abstract API of base class to call the 'modern' API
		// --------------------------------------------------------------------
		virtual void init() override
		{
			Init();
		}

		virtual void start() override
		{
			Start();
		}

		virtual void started() override
		{
			Started();
		}

		virtual void stop() override
		{
			Stop();
		}

		virtual void stopped() override
		{
			Stopped();
		}		
	};

	class RunnableBase : public utils::ref_count_base<RunnableBasePure>
	{
	public:
		virtual ~RunnableBase() = default;

		virtual void Init() override { /* Do Nothing */ }
		virtual void Start() override { /* Do Nothing */ }
		virtual void Started() override { /* Do Nothing */ }
		virtual void Stop() override { /* Do Nothing */ }
		virtual void Stopped() override { /* Do Nothing */ }
	};
}