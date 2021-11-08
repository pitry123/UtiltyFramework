#include <core/framework.h>
#include <logging/boost_logger.h>
#include <utils/console_synchronizer.hpp>
#include <utils/strings.hpp>
#include <utils/callback_handler.hpp>

#include <mutex>
#include <string>
#include <sstream>

static utils::console_synchronizer* CONSOLE_PRINTER()
{
    static std::mutex MUTEX;

	// Some threading implementations does not play nice when unloading a dll/so.
	// It might be that the printing thread will be destroyed before destructing INSTANCE.
	// This behavior results in a live-lock as we're unable to
	// synchronize (join) the printing thread.
	// Since INSTANCE is kind of a singleton, we don't mind
	// defining it as a raw pointer and not clearing it
	// on application's shutdown.
    static utils::console_synchronizer* INSTANCE = nullptr;

    std::lock_guard<std::mutex> locker(MUTEX);
    if (INSTANCE == nullptr)
        INSTANCE = utils::make_ref_count_ptr<utils::console_synchronizer>().detach();

    return INSTANCE;
}

static utils::callback_handler<core::framework::logger_hook_interface> m_logger_hooks;

const char* core::framework::version()
{
	static std::once_flag flag;
	static std::string version_str;

	std::call_once(flag, [&]()
	{
		core::framework::version_struct ver_struct;
		version(ver_struct);

		std::stringstream stream;
		stream << ver_struct;
		if (ver_struct.build != EZ_FRAMEWORK_VERSION_BUILD_UNKNOWN)
			stream << " (build: " << ver_struct.build << ")";

		version_str = stream.str();
	});

	return version_str.c_str();
}

void core::framework::version(core::framework::version_struct& ver)
{
	ver.major = EZ_FRAMEWORK_VERSION_MAJOR;
	ver.minor = EZ_FRAMEWORK_VERSION_MINOR;
	ver.patch = EZ_FRAMEWORK_VERSION_PATCH;
	ver.build = EZ_FRAMEWORK_VERSION_BUILD;
}

bool core::framework::add_logger_hook(core::framework::logger_hook_interface* logger_hook)
{
	if (logger_hook == nullptr)
		return false;

	return m_logger_hooks.add_callback(logger_hook);
}

bool core::framework::remove_logger_hook(core::framework::logger_hook_interface* logger_hook)
{
	if (logger_hook == nullptr)
		return false;

	return m_logger_hooks.remove_callback(logger_hook);
}

bool core::framework::create_looger(const char* name, core::logging::severity filter, core::logging::logger** logger)
{
	bool retval = false;
	retval = ::logging::boost_logger::create(name, filter, logger);
	if (false == retval)
		return retval;

	m_logger_hooks.raise_callbacks([&](core::framework::logger_hook_interface* hook)
	{
		hook->on_logger_created(*logger);
	});
	
	return retval;
}

static bool has_color_filter = true;
static core::console::colors filtered_color = core::console::colors::UNDEFINED_COLOR;

void core::console::color_filter(core::console::colors color, bool enabled)
{
	if (has_color_filter != enabled)
	{
		//Reset the filter
		filtered_color = core::console::colors::UNDEFINED_COLOR;
		has_color_filter = enabled;
	}
	if(color != core::console::colors::UNDEFINED_COLOR)
		filtered_color = static_cast<core::console::colors>(filtered_color | color); 
	else
		filtered_color = color;
}

void core::console::color_print(bool force, bool sync, core::console::colors color, const char* str)
{
	if (str == nullptr)
		return;
	if (false == force)
	{
		if (false == has_color_filter)
		{
			if (filtered_color == core::console::colors::UNDEFINED_COLOR ||
				(filtered_color & color) != 0)
				return;
		}
		else if (filtered_color != core::console::colors::UNDEFINED_COLOR)
		{
			if ((filtered_color & color) == 0)
				return;
		}
	}

	utils::console_synchronizer::color_code code = utils::console_synchronizer::color_code::WHITE;

	switch (color)
	{
	case core::console::RED:
		code = utils::console_synchronizer::color_code::RED;
		break;
	case core::console::GREEN:
		code = utils::console_synchronizer::color_code::GREEN;
		break;
	case core::console::YELLOW:
		code = utils::console_synchronizer::color_code::YELLOW;
		break;
	case core::console::BLUE:
		code = utils::console_synchronizer::color_code::BLUE;
		break;
	case core::console::MAGENTA:
		code = utils::console_synchronizer::color_code::MAGENTA;
		break;
	case core::console::CYAN:
		code = utils::console_synchronizer::color_code::CYAN;
		break;
	case core::console::WHITE:
	default:
		// Default to WHITE
		break;
	}

    CONSOLE_PRINTER()->colored_print(code, str);
	if (sync)
        CONSOLE_PRINTER()->sync();
}

void core::console::color_print(core::console::colors color, const char* str)
{
	color_print(false, false, color, str);
}

