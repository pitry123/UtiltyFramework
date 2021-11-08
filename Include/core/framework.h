/// @file	core/framework.h.
/// @brief	Declares the framework's core services
#pragma once
#include <core/logging.h>
#include <cstdint>

namespace core
{
	class DLL_EXPORT framework
	{
	public:
		class DLL_EXPORT logger_hook_interface : public core::ref_count_interface
		{
		public:
			virtual ~logger_hook_interface() = default;
			virtual void on_logger_created(core::logging::logger* logger) = 0;
		};

		struct version_struct
		{
			uint32_t major;
			uint32_t minor;
			uint32_t patch;
			uint32_t build;

			bool operator<(const version_struct& other) const
			{
				if (major != other.major)
					return (major < other.major);

				if (minor != other.minor)
					return (minor < other.minor);

				if (patch != other.patch)
					return (patch < other.patch);				

				if (build != other.build)
					return (build < other.build);

				return false;
			}
		};		

		static const char* version();
		static void version(core::framework::version_struct& ver);

		static bool add_logger_hook(core::framework::logger_hook_interface* logger_hook);
		static bool remove_logger_hook(core::framework::logger_hook_interface* logger_hook);
		static bool create_looger(const char* name, core::logging::severity filter, core::logging::logger** logger);
	};

	inline bool operator==(const core::framework::version_struct& lhs, const core::framework::version_struct& rhs)
	{
		return ((lhs.major == rhs.major) &&
			(lhs.minor == rhs.minor) &&
			(lhs.patch == rhs.patch) &&
			(lhs.build == rhs.build));
	}
	
	inline bool operator!=(const core::framework::version_struct& lhs, const core::framework::version_struct& rhs)
	{
		return !(lhs == rhs);
	}

	class DLL_EXPORT console
	{
	public:
		enum colors
		{
			UNDEFINED_COLOR = 0x0,
			RED		= 0x01,
			GREEN	= 0x02,
			YELLOW	= 0x04,
			BLUE	= 0x08,
			MAGENTA = 0x10,
			CYAN	= 0x20,
			WHITE	= 0x40
		};

		static void color_print(core::console::colors color, const char* str);
		static void color_print(bool force, bool sync, core::console::colors color, const char* str);
		static void color_filter(core::console::colors color, bool enabled);
	};	
}