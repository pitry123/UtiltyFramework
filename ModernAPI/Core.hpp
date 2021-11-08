#pragma once
#include <core/framework.h>
#include <core/guid.h>
#include <Strings.hpp>
#include <Logging.hpp>

#include <cstdarg>

#define LOG_QUICK(name, severity) ::Core::Framework::CreateLogger(name, severity).CreateStream(severity)

namespace Core
{
	class Framework
	{
	public:
		using VersionStruct = core::framework::version_struct;

		static inline const char* Version()
		{
			return core::framework::version();
		}

		static inline void Version(Core::Framework::VersionStruct& ver)
		{
			core::framework::version(ver);
		}

		/// Creates a new Logger
		/// @date	05/08/2018
		/// @exception	std::invalid_argument	Thrown when log name is null
		/// @exception	std::runtime_error   	Raised when a runtime error	condition occurs.
		/// @param	name  	The logger's name.
		/// @param	filter	Specifies the severity level filter.
		/// @return	A Logger.		
		static inline Logging::Logger CreateLogger(const char* name, Logging::Severity filter = Logging::Severity::TRACE)
		{
			if (name == nullptr)
				throw std::invalid_argument("name");

			utils::ref_count_ptr<core::logging::logger> instance;
			if (core::framework::create_looger(name, filter, &instance) == false)
				throw std::runtime_error("Failed to create default logger");

			return Logging::Logger(instance);
		}
	};

	class Console
	{
	public:
		using Colors = core::console::colors;

		static inline void ColorPrint(bool force, bool sync, Core::Console::Colors color, char const* const format, ...)
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

			core::console::color_print(force, sync, color, buffer);
		}

		static inline void ColorPrint(Core::Console::Colors color, char const* const format, ...)
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

			core::console::color_print(color, buffer);
		}
		
		static inline void ColorFilter(Colors color, bool enabled)
		{
			core::console::color_filter(color, enabled);
		}	
	};	

	using GUID = core::guid;
	class GUIDGenerator 
	{
	
	public:
		static GUID Generate()
		{
			utils::ref_count_ptr<core::guid_generator> guid_generator;
			
			if (false == core::guid_generator::instance(&guid_generator))
				throw std::runtime_error("guid instance");

			return guid_generator->generate();
		}
	};
}