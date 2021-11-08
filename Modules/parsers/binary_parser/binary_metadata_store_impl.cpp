#include <utils/thread_safe_object.hpp>
#include <unordered_map>
#include "binary_metadata_store_impl.h"
#include <parsers/binary_parser.h>
/// get or create the binary store as a make it singletone
/// @date	31/12/2018
/// @param [in,out]	metadata_store	If non-null, the metadata store.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_metadata_store::instance(core::parsers::binary_metadata_store_interface** metadata_store)
{
	if (metadata_store == nullptr)
		return false;

	static std::once_flag flag;

	static utils::ref_count_ptr< core::parsers::binary_metadata_store_interface> instance;
	try
	{
		std::call_once(flag, [&]()
		{
			instance = utils::make_ref_count_ptr<parsers::binary_metadata_store_impl>();
		});
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*metadata_store = instance;
	return true;
}

bool parsers::binary_metadata_store::create(core::parsers::binary_metadata_store_interface** metadata_store)
{
	if (metadata_store == nullptr)
		return false;

	

	utils::ref_count_ptr< core::parsers::binary_metadata_store_interface> instance;
	try
	{
		
		instance = utils::make_ref_count_ptr<parsers::binary_metadata_store_impl>();
		
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*metadata_store = instance;
	return true;
}

parsers::binary_metadata_store_impl::binary_metadata_store_impl()
{

}

bool parsers::binary_metadata_store_impl::add_parser_metadata(const char* name, core::parsers::binary_metadata_interface *parser_metadata)
{
    return m_parser_metadata.use<bool>([&](metadata_parser_map& parser_data)
	{
		auto it = parser_data.find(name);
		if (it == parser_data.end())
		{
			parser_data.emplace(name, parser_metadata);
			return true;
		}
		else
			return false;
	});
}

bool parsers::binary_metadata_store_impl::query_parser_metadata(const char* name, core::parsers::binary_metadata_interface **parser_metadata)
{
    return m_parser_metadata.use<bool>([&](const metadata_parser_map& parser_data)
	{
		auto it = parser_data.find(name);
		if (it == parser_data.end())
			return false;

		(*parser_metadata) = it->second;
		if (nullptr == (*parser_metadata))
			return false;

		(*parser_metadata)->add_ref();
		return true;
	});
}

bool parsers::binary_metadata_store_impl::add_enum(const char* name, core::parsers::enum_data_interface* enum_instance)
{
    return m_enums_metadata.use<bool>([&](metadata_enum_map& enum_data)
	{
		auto it = enum_data.find(name);
		if (it == enum_data.end())
		{
			enum_data.emplace(name, enum_instance);
			return true;
		}
		else
			return false;
	});
	return false;
}

bool parsers::binary_metadata_store_impl::query_enum(const char* name, core::parsers::enum_data_interface **enum_instance)
{
    return m_enums_metadata.use<bool>([&](const metadata_enum_map& enum_data)
	{
		auto it = enum_data.find(name);
		if (it == enum_data.end())
			return false;

		(*enum_instance) = it->second;
		if (nullptr == (*enum_instance))
			return false;

		(*enum_instance)->add_ref();
		return true;
	});
}




