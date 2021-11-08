/// @file	database/memory_database.h.
/// @brief	Declares the memory database classes
#pragma once
#include <core/database.h>

namespace database
{
	/// @class	memory_row
	/// @brief	Memory rows are DB entries which can shared in memory. They are generally shared between elements of the same process.
	/// @date	15/05/2018
	class DLL_EXPORT memory_row : public core::database::row_interface
	{
	public:
		/// @fn	virtual memory_row::~memory_row() = default;
		/// @brief	Destructor
		/// @date	15/05/2018
		virtual ~memory_row() = default;

		/// Static factory: Creates a new memory row instance
		/// @date	15/05/2018
		/// @param 			key   	The row's key (ID).
		/// @param 			size  	The row's data size.
		/// @param [in]		info  	The information.
		/// @param [in]		parser	If non-null, the parser of the object.
		/// @param [in]		parent	The row's parent key (ID).
		/// @param [out]	row   	An address of a pointer to
		/// 	core::database::row_interface.
		/// @return	True if it succeeds, false if it fails.
		static bool create(const core::database::key& key, size_t size, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser, core::database::table_interface* parent, core::database::row_interface** row);
	};

	/// @class	memory_table
	/// @brief	A container of memory rows
	/// @date	15/05/2018
	class DLL_EXPORT memory_table : public core::database::table_interface
	{
	public:
		/// @fn	virtual memory_table::~memory_table() = default;
		/// @brief	Destructor
		/// @date	15/05/2018
		virtual ~memory_table() = default;

		/// Static factory: Creates a new memory table instance
		/// @date	15/05/2018
		/// @param 		   	key		   	The table's key (ID).
		/// @param [in]	parent	   	The table's parent key (ID).
		/// @param 		   	name	   	The name.
		/// @param 		   	description	The description.
		/// @param [out]   	table	   	An address of a pointer to
		/// 	core::database::table_interface.
		/// @return	True if it succeeds, false if it fails.
		static bool create(const core::database::key& key, core::database::dataset_interface* parent, const char* name, const char* description, core::database::table_interface** table);

	};


	/// @class	memory_dataset
	/// @brief	A container of memory tables
	/// @date	15/05/2018
	class DLL_EXPORT memory_dataset : public core::database::dataset_interface
	{
	public:
		/// @fn	virtual memory_dataset::~memory_dataset() = default;
		/// @brief	Destructor
		/// @date	15/05/2018
		virtual ~memory_dataset() = default;

		/// @fn	static bool memory_dataset::create(const core::database::key& key, core::database::dataset_interface** set);
		/// @brief	Static factory: Creates a new memory dataset instance
		/// @date	15/05/2018
		/// @param 		   	key	The dataset key (ID).
		/// @param [out]	set	An address of a pointer to core::database::dataset_interface.
		/// @return	True if it succeeds, false if it fails.
		static bool create(const core::database::key& key, core::database::dataset_interface** set);
	};
}