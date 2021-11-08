#include "table.h"

inline bool database::memory_table_impl::create_row(const core::database::key& row_key, size_t data_size, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser, core::database::row_interface** row)
{
	return database::memory_row::create(row_key, data_size, info, parser, this, row);
}


database::memory_table_impl::memory_table_impl(
	const core::database::key& key,
	core::database::dataset_interface* parent,
	const char* name,
	const char* description) :
	utils::database::table_base<database::memory_table>(key, parent, name, description)
{
}

bool database::memory_table::create(const core::database::key& key, core::database::dataset_interface* parent, const char* name, const char* description, core::database::table_interface** table)
{
	if (table == nullptr)
		return false;

	utils::ref_count_ptr<core::database::table_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<memory_table_impl>(key, parent,name, description);
	}
	catch (...)
	{
		return false;
	}

    if (instance == nullptr)
        return false;

	*table = instance;
	(*table)->add_ref();
	return true;
}
