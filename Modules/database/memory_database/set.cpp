#include "set.h"

inline bool database::memory_dataset_impl::create_table(const core::database::key& table_key, const char* name, const char* description, core::database::table_interface** table)
{
	return database::memory_table::create(table_key, this,name,description, table);
}

database::memory_dataset_impl::memory_dataset_impl(const core::database::key& key) :
	utils::database::dataset_base<database::memory_dataset>(key)
{	
}

bool database::memory_dataset::create(const core::database::key& key, core::database::dataset_interface** table)
{
	if (table == nullptr)
		return false;

	utils::ref_count_ptr<core::database::dataset_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<memory_dataset_impl>(key);
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
