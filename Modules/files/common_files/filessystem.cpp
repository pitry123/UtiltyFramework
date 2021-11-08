#include <files/filessystem.h>
#include <boost/filesystem.hpp>


bool files::filessystem::copy_file(const char* src_file, const char* dest_file, bool overwrite_existing)
{
	try
	{
		boost::filesystem::path app_path(dest_file);
		boost::filesystem::path dir(app_path.parent_path().string().c_str());
		if (false == boost::filesystem::exists(dir))
		{
			boost::filesystem::create_directories(dir);

		}

		boost::filesystem::copy_option option = overwrite_existing ? boost::filesystem::copy_option::overwrite_if_exists :
																	  boost::filesystem::copy_option::none;

		boost::filesystem::copy_file(src_file, dest_file, option);
		return true;
	}
	catch (...)
	{
		return false;
	}
}
#if _WIN32
bool files::filessystem::is_file_exist(const char* name) {
	FILE *file;
	if (fopen_s(&file, name, "r") == 0) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}
#else
bool files::filessystem::is_file_exist(const char* name) {
	if (FILE *file = fopen(name, "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}
#endif