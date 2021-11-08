#include "files_handler_impl.h"

files::files_handler_impl::files_handler_impl(double flush_interval)
{
	m_timer_token = m_io_thread.register_timer(flush_interval, [this]()
	{
		flush();
	});
}

files::files_handler_impl::~files_handler_impl()
{
	m_io_thread.unregister_timer(m_timer_token);
}

bool 
files::files_handler_impl::subscribe_file(core::files::file_access_interface* file)
{
	return m_subscribed_files.use<bool>([&](files_container& files)
	{
		auto it = std::find_if(files.begin(), files.end(),
			[file](const utils::ref_count_ptr<core::files::file_access_interface>& current_file) -> bool
		{
			return (file == current_file);
		});

		if (it == files.end())
			files.push_back(file);

		return true;
	});
}

bool 
files::files_handler_impl::unsubscribe_file(core::files::file_access_interface* file)
{
	return m_subscribed_files.use<bool>([&](files_container& files)
	{
		auto it = std::find_if(files.begin(), files.end(),
			[file](const utils::ref_count_ptr<core::files::file_access_interface>& current_file) -> bool
		{
			return (file == current_file);
		});

		if (it == files.end())
			return false;

		files.erase(it);
		return true;
	});
}

void 
files::files_handler_impl::flush(core::files::file_access_interface* file)
{
	utils::ref_count_ptr<core::files::file_access_interface> local_file = file;
	m_io_thread.begin_invoke([local_file]()
	{
		local_file->flush();
	}, nullptr, true);
}

void 
files::files_handler_impl::flush()
{
	m_subscribed_files.use([this](files_container& files)
	{
		for (auto& file : files)
		{
			flush(file);
		}
	});
}

bool 
files::files_handler::create(double flush_interval, core::files::files_handler_interface** handler)
{
	if (flush_interval <= 0.0)
		return false;

	utils::ref_count_ptr<core::files::files_handler_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<files::files_handler_impl>(flush_interval);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*handler = instance;
	return true;
}