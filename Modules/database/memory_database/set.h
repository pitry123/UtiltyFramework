#pragma once
#include <database/memory_database.h>
#include <utils/database.hpp>

namespace database
{	
	class memory_dataset_impl :
		public utils::database::dataset_base<database::memory_dataset>
	{
	protected:
		virtual bool create_table(const core::database::key& table_key, const char* name, const char* description, core::database::table_interface** table) override;

	public:
		memory_dataset_impl(const core::database::key& key);
	};
}
