#pragma once
#include <utils/ref_count_base.hpp>
#include <utils/dispatcher.hpp>
#include <stdarg.h>

#ifdef _WIN32
#include <wtypes.h>
#else
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <ostream>
#include <cstdio>
#endif

namespace utils
{
	class console_synchronizer final : public utils::dispatcher
	{
	public:
		enum class color_code
		{
#ifdef _WIN32
			RED = 4 | FOREGROUND_INTENSITY,
			GREEN = 2 | FOREGROUND_INTENSITY,
			YELLOW = 14 | FOREGROUND_INTENSITY,
			BLUE = 1 | FOREGROUND_INTENSITY,
			MAGENTA = 5 | FOREGROUND_INTENSITY,
			CYAN = 3 | FOREGROUND_INTENSITY,
			WHITE = 15 | FOREGROUND_INTENSITY
#else
			RED = 31,
			GREEN = 32,
			YELLOW = 33,
			BLUE = 34,
			MAGENTA = 35,
			CYAN = 36,
			WHITE = 37
#endif
		};

	private:
#ifndef _WIN32
		class ColorModifier
		{
		private:
			color_code m_code;
		public:
			ColorModifier(color_code code) : m_code(code) {}
			friend std::ostream&
				operator<<(std::ostream& os, const ColorModifier& mod)
			{
				return os << "\033[" << static_cast<int>(mod.m_code) << ";1m";
			}
		};
#endif
		console_synchronizer(const console_synchronizer& other) = delete;       // non construction-copyable
		console_synchronizer& operator=(const console_synchronizer&) = delete;	// non copyable				
		console_synchronizer(const console_synchronizer&& other) = delete;      // non construction-movable
		console_synchronizer& operator=(const console_synchronizer&&) = delete;	// non movable		

		void set_color(color_code color)
		{
#ifdef _WIN32
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
#else
			ColorModifier mod(color);
			std::cout << mod;
#endif
		}

	public:
		console_synchronizer() :
			utils::dispatcher("Console Printer")
		{
		}

		~console_synchronizer()
		{
			sync();
		}

		void colored_print(color_code color, char const* const str)
		{
			std::string text = str;
			begin_invoke([this, color, text]() -> void
			{
				set_color(color);
				printf("%s", text.c_str());
			});
		}

		void colored_printf(color_code color, char const* const format, ...)
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
			
			colored_print(color, buffer);
		}		
	};	
}