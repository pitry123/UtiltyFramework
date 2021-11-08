#pragma once
#include <core/os.h>

namespace files
{
	class DLL_EXPORT filessystem
	{
	public:
		static bool copy_file(const char* src_file, const char* dest_file, bool overwrite_existing);
		static bool is_file_exist(const char* name);
	};
}