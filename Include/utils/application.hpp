#include <core/framework.h>
#include <core/application.h>
#include <utils/ref_count_base.hpp>
#include <cstdarg>

namespace utils
{
	namespace application
	{
		class smart_cli_callback : public  utils::ref_count_base<core::application::cli_callback_interface>
		{
		public:
			using callback_func = std::function<void(const char *cmd, size_t size, const char* params[])>;		

			smart_cli_callback(const callback_func& func) :m_func(func)
			{
			}

			void on_cli_input(const char *cmd, size_t size, const char* params[]) override
			{
				if (m_func != nullptr)
					m_func(cmd, size, params);
			}

		private:
			callback_func m_func;
		};
	}

	inline void color_print(bool is_force, bool is_sync, core::console::colors color, char const* const format, ...)
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
}