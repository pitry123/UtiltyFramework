/// @file	core/database.h.
/// @brief	Declares the database interfaces and types
#pragma once
#include <core/disposable_interface.h>
#include <core/parser.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>
#include <algorithm>
#include <functional>

namespace core
{
	namespace database
	{
		static constexpr size_t MAX_KEY_SIZE = 16;
		static constexpr size_t UNBOUNDED_ROW_SIZE = (std::numeric_limits<size_t>::max)();
		static constexpr size_t MAX_NAME = 256;
		static constexpr size_t MAX_DESCRIPTION = 256;
		/// @struct	key
		/// @brief	A database entry key.
		/// @date	14/05/2018
		struct key
		{
			size_t length;
			uint8_t data[MAX_KEY_SIZE];

			/// @fn	size_t operator<(const core::database::key& other) const
			/// @brief	operator <
			/// @date	14/05/2018
			/// @param	other	The key to compare with.
			/// @return	True if (this < other), otherwise False.
			bool operator<(const key& other) const
			{
				static_assert(std::is_pod<key>::value == true, "key structure was changed and it's not POD anymore. Please fix!");

				size_t min_length = (std::min)(length, other.length);
				if (min_length > sizeof(decltype(other.data)))
					min_length = sizeof(decltype(other.data));

				int result = std::memcmp(data, other.data, min_length);
				if (result == 0)
					return (min_length < other.length);

				return (result < 0);
			}
		
			bool undefined() const
			{
				return length == 0;
			}
		};

		/// @struct	key_hash
		/// @brief	A key hash for hash-tables (e.g. std::unordered_map)
		/// @date	14/05/2018
		struct key_hash
		{
			/// @fn	size_t operator()(const core::database::key& key) const
			/// @brief	Function call operator
			/// @date	14/05/2018
			/// @param	key	The key for calculating the hash code.
			/// @return	The resulted hash code.
			size_t operator()(const core::database::key& key) const
			{
				static_assert((sizeof(decltype(key.data)) % sizeof(uint32_t)) == 0, "key size must be a multiplication of sizeof(uint32_t). Please fix!");

                size_t length = (std::min)(key.length, sizeof(decltype(key.data)));
				uint8_t data[sizeof(decltype(key.data))] = {};
				std::memcpy(data, key.data, length);

				size_t retval = 0;
                for (size_t i = 0; i < sizeof(decltype(key.data)); i += sizeof(uint32_t))
				{
					retval ^= std::hash<uint32_t>()(*(static_cast<uint32_t*>(static_cast<void*>(data + i))));
				}

				return retval;
			}
		};

		/// @fn	inline bool operator==(const key& lhs, const key& rhs)
		/// @brief	Equality operator
		/// @date	14/05/2018
		/// @param	lhs	The first instance to compare.
		/// @param	rhs	The second instance to compare.
		/// @return	True if the parameters are considered equivalent.
		inline bool operator==(const key& lhs, const key& rhs)
		{			
			if (lhs.length != rhs.length)
				return false;

			size_t length = (std::min)(lhs.length, sizeof(decltype(lhs.data)));			
            return (memcmp(lhs.data, rhs.data, length) == 0);
		}

		/// @fn	inline bool operator!=(const key& lhs, const key& rhs)
		/// @brief	Inequality operator
		/// @date	14/05/2018
		/// @param	lhs	The first instance to compare.
		/// @param	rhs	The second instance to compare.
		/// @return	True if the parameters are not considered equivalent.
		inline bool operator!=(const key& lhs, const key& rhs)
		{
			return !(lhs == rhs);
		}

		/// optional additional Information about the row.
		/// @date	19/11/2018
		struct row_info
		{
			core::types::type_enum type;
			char name[MAX_NAME];
			char description[MAX_DESCRIPTION];
			char type_name[MAX_NAME];
		};

		// Forward declations...
		class row_interface;
		class table_interface;
		class dataset_interface;
		/// @class	row_callback_interface
		/// @brief	A row update callback interface.
		/// @date	14/05/2018
		class DLL_EXPORT row_callback_interface :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual row_callback_interface::~row_callback_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~row_callback_interface() = default;

			/// Handles row's data was changed signals
			/// @date	14/05/2018
			/// @param [in]	row   		the row.
			/// @param 		   	size  	the data size.
			/// @param 		   	buffer	the data as buffer.
			virtual void on_data_changed(core::database::row_interface* row, size_t size, const void* buffer) = 0;
		};
		
		/// @class	row_interface
		/// @brief	An interface defining a data row
		/// @date	14/05/2018
		class DLL_EXPORT row_interface :
			public core::disposable_interface
		{
		public:
			/// @fn	virtual row_interface::~row_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~row_interface() = default;

			/// @fn	virtual core::database::key row_interface::key() const = 0;
			/// @brief	Gets the row's key (ID)
			/// @date	14/05/2018
			/// @return	A core::database::key.
			virtual core::database::key key() const = 0;			

			/// @fn	virtual size_t row_interface::data_size() const = 0;
			/// @brief	Row's data size
			/// @date	14/05/2018
			/// @return	A size_t.
			virtual size_t data_size() const = 0;

			/// @fn	virtual uint8_t row_interface::write_priority() const = 0;
			/// @brief	Gets the current write-priority
			/// 		This is part of a write filter mechanism. See function 'write_bytes' below 
			/// @date	14/05/2018
			/// @return	An uint8_t.
			virtual uint8_t write_priority() const = 0;

			/// @fn	virtual bool row_interface::query_parent(core::database::table_interface** parent) const = 0;
			/// @brief	Queries parent table
			/// @date	03/09/2018
			/// @param [out]	parent	If non-null, the parent.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_parent(core::database::table_interface** parent) const = 0;

			/// @fn	virtual bool row_interface::read_bytes(void* buffer) const = 0;
			/// @brief	Reads the whole row's data and writes it into a buffer
			/// @date	14/05/2018
			/// @param [in]		buffer	the buffer to write to.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_bytes(void* buffer) const = 0;

			/// @fn	virtual bool row_interface::read_bytes(void* buffer, size_t size) const = 0;
			/// @brief	Reads bytes from row's data and writes them into a buffer
			/// @date	14/05/2018
			/// @param [in]		buffer	the buffer to write to.
			/// @param 		   	size  	Number of bytes to read.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_bytes(void* buffer, size_t size) const = 0;

			/// @fn	virtual bool row_interface::write_bytes(const void* buffer, size_t size, bool force_report, uint8_t priority) = 0;
			/// @brief	Updates row's data
			/// @date	14/05/2018
			/// @param	buffer			The data buffer.
			/// @param	size			The data size.
			/// @param	force_report	True to force report even if supplied data did not change anything.
			/// @param	priority		The write-priority. Update is ignored if value < 'write_priority'
			/// @return	True if it succeeds, false if it fails.
			virtual bool write_bytes(const void* buffer, size_t size, bool force_report, uint8_t priority) = 0;

			/// Check and write bytes - check the data before writing to it
			/// @date	05/09/2019
			/// @param	buffer			The buffer.
			/// @param	size			The size.
			/// @param	force_report	True to force report.
			/// @param	priority		The priority.
			/// @return	True if it succeeds, false if it fails.
			virtual bool check_and_write_bytes(const void* buffer, size_t size, bool force_report, uint8_t priority) = 0;

			/// @fn	virtual bool row_interface::set_write_priority(uint8_t priority) = 0;
			/// @brief	Sets the value of 'write_priority'
			/// @date	14/05/2018
			/// @param	priority	The priority.
			/// @return	True if it succeeds, false if it fails.
			virtual bool set_write_priority(uint8_t priority) = 0;

			/// @fn	virtual bool row_interface::subscribe_callback(core::database::row_callback_interface* callback) = 0;
			/// @brief	Subscribes a data change callback
			/// @date	14/05/2018
			/// @param [in]	callback	the callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool subscribe_callback(core::database::row_callback_interface* callback) = 0;

			/// @fn	virtual bool row_interface::unsubscribe_callback(core::database::row_callback_interface* callback) = 0;
			/// @brief	unsubscribe a data change callback
			/// @date	14/05/2018
			/// @param [in]	callback	the callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool unsubscribe_callback(core::database::row_callback_interface* callback) = 0;

			/// Queries parser metadata get the structure metadata if exist. 
			/// metadata might be nullptr
			/// @date	20/11/2018
			/// @param [out]	parser_metadata	If non-null, the parser metadata.
			virtual bool query_parser_metadata(core::parsers::binary_metadata_interface** parser_metadata) const = 0;

			/// Gets the information of the row
			/// @date	20/11/2018
			/// @return	A reference to a const row_info.
			virtual const row_info& info() const = 0;

		};

		/// @class	table_callback_interface
		/// @brief	A table update callback interface.
		/// @date	14/05/2018
		class DLL_EXPORT table_callback_interface :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual table_callback_interface::~table_callback_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~table_callback_interface() = default;

			/// @fn	virtual void table_callback_interface::on_row_added(core::database::row_interface* row) = 0;
			/// @brief	Handles row added signals
			/// @date	14/05/2018
			/// @param [in]	the row.
			virtual void on_row_added(core::database::row_interface* row) = 0;

			/// @fn	virtual void table_callback_interface::on_row_removed(core::database::row_interface* row) = 0;
			/// @brief	Handles row removed signals
			/// @date	14/05/2018
			/// @param [in]	the row.
			virtual void on_row_removed(core::database::row_interface* row) = 0;
		};

		/// @class	table_interface
		/// @brief	An interface defining a data table.
		/// @date	14/05/2018
		class DLL_EXPORT table_interface :
			public core::disposable_interface
		{
		public:
			/// @fn	virtual table_interface::~table_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~table_interface() = default;

			/// @fn	virtual core::database::key table_interface::key() const = 0;
			/// @brief	Gets the table's key (ID)
			/// @date	14/05/2018
			/// @return	A core::database::key.
			virtual core::database::key key() const = 0;			

			/// @fn	virtual size_t table_interface::size() const = 0;
			/// @brief	Gets the rows count
			/// @date	14/05/2018
			/// @return	A size_t.
			virtual size_t size() const = 0;

			/// Gets the name of the table
			/// @date	10/11/2018
			/// @return	Null if empty, else a pointer to a const char.
			virtual const char* name() const = 0;

			/// Gets the description of the table
			/// @date	10/11/2018
			/// @return	Null if empty, else a pointer to a const char.
			virtual const char* description() const = 0;
			/// @fn	virtual bool table_interface::query_parent(core::database::dataset_interface** parent) const = 0;
			/// @brief	Queries parent dataset
			/// @date	03/09/2018
			/// @param [out]	parent	If non-null, the parent.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_parent(core::database::dataset_interface** parent) const = 0;

			/// @fn	virtual bool table_interface::add_row(const core::database::key& key, size_t data_size) = 0;
			/// @brief	Adds a new row
			/// @date	14/05/2018
			/// @param	key		 	The row's key.
			/// @param	data_size	Size of the row's data.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_row(const core::database::key& key, size_t data_size) = 0;

			/// Adds a new row
			/// @date	14/05/2018
			/// @param 		   	key			 	The row's key.
			/// @param 		   	data_size	 	Size of the row's data.
			/// @param [in]	    row_info_data	Information about the row
			/// 	information.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_row(const core::database::key& key, size_t data_size, const row_info& info, parsers::binary_metadata_interface* parser) = 0;

			/// @fn	virtual bool table_interface::remove_row(const core::database::key& key, core::database::row_interface** removed_row) = 0;
			/// @brief	Removes a row
			/// @date	14/05/2018
			/// @param 		   	key		   	The row's key.
			/// @param [out]	removed_row	If non-null, the removed row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool remove_row(const core::database::key& key, core::database::row_interface** removed_row) = 0;

			/// @fn	virtual bool table_interface::query_row(const core::database::key& key, core::database::row_interface** row) const = 0;
			/// @brief	Queries row by key
			/// @date	14/05/2018
			/// @param 		   	key	The row's key.
			/// @param [out]	row	If non-null and found, the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_row(const core::database::key& key, core::database::row_interface** row) const = 0;

			/// @fn	virtual bool table_interface::query_row_by_index(size_t index, core::database::row_interface** row) const = 0;
			/// @brief	Queries row by index
			/// @date	14/05/2018
			/// @param 		   	index	Zero-based index of the row.
			/// @param [out]	row  	If non-null, the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_row_by_index(size_t index, core::database::row_interface** row) const = 0;

			/// @fn	virtual bool table_interface::query_row_by_index(size_t index, core::database::row_interface** row) const = 0;
			/// @brief	Queries row by name
			/// @date	14/05/2018
			/// @param 		   	index	Zero-based index of the row.
			/// @param [out]	row  	If non-null, the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_row_by_name(const char* name, core::database::row_interface** row) const = 0;

			/// @fn	virtual bool table_interface::subscribe_callback(core::database::table_callback_interface* callback) = 0;
			/// @brief	Subscribes updates callback
			/// @date	14/05/2018
			/// @param [in]	callback	If non-null, the callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool subscribe_callback(core::database::table_callback_interface* callback) = 0;

			/// @fn	virtual bool table_interface::unsubscribe_callback(core::database::table_callback_interface* callback) = 0;
			/// @brief	unsubscribe updates callback
			/// @date	14/05/2018
			/// @param [in]	callback	If non-null, the callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool unsubscribe_callback(core::database::table_callback_interface* callback) = 0;

			virtual bool subscribe_data_callback(core::database::row_callback_interface* callback) = 0;
			virtual bool unsubscribe_data_callback(core::database::row_callback_interface* callback) = 0;
		};

		/// @class	dataset_callback_interface
		/// @brief	A dataset update callback interface.
		/// @date	14/05/2018
		class DLL_EXPORT dataset_callback_interface :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual dataset_callback_interface::~dataset_callback_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~dataset_callback_interface() = default;

			/// @fn	virtual void dataset_callback_interface::on_table_added(core::database::table_interface* table) = 0;
			/// @brief	Handles table added signals
			/// @date	14/05/2018
			/// @param [in]		table	the table.
			virtual void on_table_added(core::database::table_interface* table) = 0;

			/// @fn	virtual void dataset_callback_interface::on_table_removed(core::database::table_interface* table) = 0;
			/// @brief	Handles table removed signals
			/// @date	14/05/2018
			/// @param [in]		table	the table.
			virtual void on_table_removed(core::database::table_interface* table) = 0;
		};

		/// @class	dataset_interface
		/// @brief	An interface defining a set of tables
		/// @date	14/05/2018
		class DLL_EXPORT dataset_interface :
			public core::disposable_interface
		{
		public:
			/// @fn	virtual dataset_interface::~dataset_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~dataset_interface() = default;

			/// @fn	virtual core::database::key dataset_interface::key() const = 0;
			/// @brief	Gets the dataset key
			/// @date	14/05/2018
			/// @return	A core::database::key.
			virtual core::database::key key() const = 0;

			/// @fn	virtual size_t dataset_interface::size() const = 0;
			/// @brief	Gets the tables count
			/// @date	14/05/2018
			/// @return	A size_t.
			virtual size_t size() const = 0;

			/// @fn	virtual bool dataset_interface::add_table(const core::database::key& key) = 0;
			/// @brief	Adds a table
			/// @date	14/05/2018
			/// @param	key	The table's key.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_table(const core::database::key& key) = 0;

			/// @fn	virtual bool dataset_interface::add_table(const core::database::key& key) = 0;
			/// @brief	Adds a table
			/// @date	14/05/2018
			/// @param	key	The table's key.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_table(const core::database::key& key, const char* name, const char* description) = 0;

			/// @fn	virtual bool dataset_interface::remove_table(const core::database::key& key, core::database::table_interface** removed_table) = 0;
			/// @brief	Removes a table
			/// @date	14/05/2018
			/// @param 		   	key			 	The table's key.
			/// @param [out]	removed_table	If non-null, the removed table.
			/// @return	True if it succeeds, false if it fails.
			virtual bool remove_table(const core::database::key& key, core::database::table_interface** removed_table) = 0;

			/// @fn	virtual bool dataset_interface::query_table(const core::database::key& key, core::database::table_interface** table) const = 0;
			/// @brief	Queries table by key
			/// @date	14/05/2018
			/// @param 		   	key  	The table's key.
			/// @param [out]	table	If non-null, the table.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_table(const core::database::key& key, core::database::table_interface** table) const = 0;

			/// @fn	virtual bool dataset_interface::query_table_by_index(size_t index, core::database::table_interface** table) const = 0;
			/// @brief	Queries table by index
			/// @date	14/05/2018
			/// @param 		   	index	Zero-based index of the table.
			/// @param [out]	table	If non-null, the table.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_table_by_index(size_t index, core::database::table_interface** table) const = 0;

			/// Queries table by name
			/// @date	14/05/2018
			/// @param 			name 	Zero-based index of the table.
			/// @param [out]	table	If non-null, the table.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_table_by_name(const char* name, core::database::table_interface** table) const = 0;

			/// @fn	virtual bool dataset_interface::subscribe_callback(core::database::dataset_callback_interface* callback) = 0;
			/// @brief	Subscribes updates callback
			/// @date	14/05/2018
			/// @param [in]		callback	the callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool subscribe_callback(core::database::dataset_callback_interface* callback) = 0;

			/// @fn	virtual bool dataset_interface::unsubscribe_callback(core::database::dataset_callback_interface* callback) = 0;
			/// @brief	Unsubscribe updates callback
			/// @date	14/05/2018
			/// @param [in]		callback	the callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool unsubscribe_callback(core::database::dataset_callback_interface* callback) = 0;
		};
	}
}
