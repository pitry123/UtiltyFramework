#pragma once
#include <database/memory_database.h>
#include <utils/database.hpp>

namespace database
{	
	class memory_table_impl :
		public utils::database::table_base<database::memory_table>
	{
	protected:
		virtual bool create_row(const core::database::key& row_key, size_t data_size, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser, core::database::row_interface** row) override;

	public:
		memory_table_impl(
			const core::database::key& key,
			core::database::dataset_interface* parent,
			const char* name,
			const char* description);
	};
}
