#pragma once
#include <core/ref_count_interface.h>
#include <core/types.h>
#include <core/buffer_interface.h>
#include <cstddef>
#include <limits>

namespace core
{
	namespace parsers
	{
		static constexpr uint8_t VAL_SIZE = 8;
		static constexpr uint8_t VAL_UNDEFINED[VAL_SIZE] = { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff };
		static constexpr size_t MAX_NAME = 256;

		class binary_metadata_interface;

		struct simple_options_data
		{
			bool has_min = false;
			uint8_t minval[VAL_SIZE];
			bool has_max = false;
			uint8_t maxval[VAL_SIZE];
			bool has_def = false;
			uint8_t defval[VAL_SIZE];
		};

		struct enum_data_item
		{
			char name[MAX_NAME];
			int64_t value;
		};
		
		enum json_details_level
		{
			JSON_ENUM_VALUES, //in enums print the enum value
			JSON_ENUM_LABLES, //in enums print the enum name
			JSON_ENUM_FULL,//print both lables and number
		};
		/// define enumeration description interface 
		/// @date	03/01/2020
		class DLL_EXPORT enum_data_interface : public core::ref_count_interface
		{
		public:
			virtual ~enum_data_interface() = default;

			/// Gets the name of the enumeration
			/// @date	03/01/2020
			/// @return	Null if it fails, else a pointer to a const char.
			virtual const char* name() const = 0;

			/// Adds an enumeration item 
			/// @date	03/01/2020
			/// @param	name	The name of the item.
			/// @param	val 	The value of the item.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_item(const char* name, int64_t val) const = 0;

			/// retrieve enumeration item by index
			/// @date	03/01/2020
			/// @param 		   	index			Zero-based index of the requested enumeration item.
			/// @param [out]	enum_data_item	The enum data item.
			/// @return	True if it succeeds, false if it fails.
			virtual bool item_by_index(size_t index,core::parsers::enum_data_item& enum_data_item) const = 0;

			///  retrieve enumeration item by its name
			/// @date	03/01/2020
			/// @param 		   	name		  	The name of the requested enumeration item .
			/// @param [out]	enum_data_item	The enum data item.
			/// @return	True if it succeeds, false if it fails.
			virtual bool item_by_name(const char* name, core::parsers::enum_data_item& enum_data_item) const = 0;

			/// retrieve enumeration item by its value
			/// @date	03/01/2020
			/// @param 		   	val			  	The value of the requested enumeration item.
			/// @param [in,out]	enum_data_item	The enum data item.
			/// @return	True if it succeeds, false if it fails.
			virtual bool item_by_val(int64_t val, core::parsers::enum_data_item& enum_data_item) const = 0;

			/// Get the Value by name of the enumeration item
			/// @date	03/01/2020
			/// @param 		   	name	The name the requested value.
			/// @param [out]	val 	The value returned.
			/// @return	True if it succeeds, false if it fails.
			virtual bool val_by_name(const char* name, int64_t &val) const = 0;

			/// Get the name of the enumeration item by its value
			/// @date	03/01/2020
			/// @param	val	The value.
			/// @return	Null if it fails, else a pointer to a const char of the name.
			virtual const char* name_by_val(int64_t val) const = 0;

			/// Gets the size representing the number of enumeration items in the enum
			/// @date	03/01/2020
			/// @return	the number of the enumeration items.
			virtual size_t size() const = 0;

			/// Converts the enumration into a JSON object
			/// @date	03/01/2020
			/// @param	compact	True to generated a compact JSON, false to generate a readable JSON.
			/// @return	Compact as a const char*.
			virtual const char* to_json(bool compact) const = 0;

			/// Initializes this enumeration object from  JSON
			/// @date	03/01/2020
			/// @param	json	The JSON.
			/// @return	True if it succeeds, false if it fails.
			virtual bool from_json(const char* json) const = 0;
		};

		class binary_metadata_store_interface;

		class DLL_EXPORT binary_node_interface : public core::ref_count_interface
		{
		public:
			virtual ~binary_node_interface() = default;

			/// Gets the name
			/// @date	11/08/2019
			/// @return	Null if it fails, else a pointer to a const char.
			virtual const char* name() const = 0;

			/// Gets the offset from the start
			/// @date	11/08/2019
			/// @return	A size_t.
			virtual size_t offset() const = 0;

			/// Gets the size of the node
			/// @date	11/08/2019
			/// @return	A size_t.
			virtual size_t size() const = 0;

			/// Gets the type
			/// @date	11/08/2019
			/// @return	A types::type_enum.
			virtual types::type_enum type() const = 0;

			/// Determines if it is a big endian
			/// @date	11/08/2019
			/// @return	True if it succeeds, false if it fails.
			virtual bool big_endian() const = 0;

			/// Gets the count of this node if it is an array
			/// @date	10/06/2020
			/// @return	the count of elements in array or 1 if not part of array.
			virtual size_t count() const = 0;
			
			/// Gets the minval, max val and default of a simple type
			/// @date	11/08/2019
			/// @return	A double.
			virtual core::parsers::simple_options_data options() const = 0;

			/// Gets the minval, max val and default of a simple type
			/// @date	11/08/2019
			/// @return	A double.
			virtual const char* string_default() const = 0;

			/// Queries an enum if the node is enum type. this might be null if the enum data was not set
			/// on creating the metadata parser
			/// @date	28/12/2019
			/// @param 		   	name		 	The name.
			/// @param [out]	enum_instance	If non-null, the enum instance.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_enum(core::parsers::enum_data_interface **enum_instance) const = 0;

			/// get the given nested parser
			/// @date	11/08/2019
			/// @param [out]	nested_parser	If non-null, the nested parser.
			/// @return	True if it succeeds, false if it fails.
			virtual bool nested(binary_metadata_interface **nested_parser) const = 0;

			/// Reads the relevant data from buffer to data according to the metadat of the node
			/// @date	19/10/2019
			/// @param [out]	data	   	If non-null, the data to write to.
			/// @param 		   	data_size  	Size of the data.
			/// @param [in]	buffer	   	If non-null, the buffer to read from.
			/// @param 		   	buffer_size	Size of the buffer.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read(void* data, size_t data_size, void* buffer, size_t buffer_size) const = 0;

			/// Writes data into buffer according to the node metadata
			/// @date	19/10/2019
			/// @param 		   	data	   	The data.
			/// @param 		   	data_size  	Size of the data.
			/// @param [in,out]	buffer	   	If non-null, the buffer.
			/// @param 		   	buffer_size	Size of the buffer.
			/// @return	True if it succeeds, false if it fails.
			virtual bool write(const void* data, size_t data_size, void* buffer, size_t buffer_size) const = 0;

		};
		
		class binary_parser_interface;

		class DLL_EXPORT binary_metadata_interface : public ref_count_interface
		{
		public:
			virtual ~binary_metadata_interface() = default;

			/// Queries a node from the metadata according to the name
			/// @date	03/10/2018
			/// @param 			name	The name.
			/// @param [out]	node	If non-null, the node.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_node(const char* name, core::parsers::binary_node_interface** node) const = 0;

			/// Queries a node according to the index
			/// @date	03/10/2018
			/// @param 		   	index	Zero-based index of the.
			/// @param [out]	node 	If non-null, the node.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_node_by_index(size_t index, core::parsers::binary_node_interface** node) const = 0;
			
			/// number of nodes in the metadata
			/// @date	03/10/2018
			/// @return	A size_t.
			virtual size_t node_count() const = 0;

			/// Gets the size of the whole parsed type
			/// @date	03/10/2018
			/// @return	A size_t.
			virtual size_t size() const = 0;

			/// Determines if the parsed type is big endian
			/// @date	03/10/2018
			/// @return	True if it succeeds, false if it fails.
			virtual bool big_endian() const = 0;
			
			/// Gets the name of the metadata 
			/// this is optional and might be empty, it is used to simplify the  reuse of a nested 
			/// parser in the same host parser
			/// @date	04/10/2018
			/// @return	Null if it fails, else a pointer to a const char.
			virtual const char* namely() const = 0;

			/// Gets index by name - return the index of the node by its name
			/// @date	11/09/2019
			/// @param	name	The name.
			/// @return	The index by name.
			virtual bool get_index_by_name(const char* name, size_t& index) const = 0;

			/// Creates a parser from the metadata
			/// @date	12/09/2019
			/// @param [in,out]	parser	If non-null, the parser.
			/// @return	True if it succeeds, false if it fails.
			virtual bool create_parser(core::parsers::binary_parser_interface** parser) const = 0;

			/// Converts this metadata object to JSON
			/// @date	23/12/2018
			/// @param 			compact	True to compact false for readable JSON.
			/// @return	The given data converted to a bool.
			virtual const char* to_json(bool compact) const = 0;
		};

		class DLL_EXPORT  binary_metadata_builder_interface : public binary_metadata_interface
		{
		public:

			/// set the name of the metadata object
			/// @date	09/07/2019
			/// @param	name	The name.
			/// @return	True if it succeeds, false if it fails.
			virtual bool namely(const char* name) = 0;

			/// Puts named field of a simple type (primitive) 
			/// @date	11/07/2019
			/// @param	name	The name of the field.
			/// @param	size	The size of the field.
			/// @param	type	The type of the field.
			/// @return	True if it succeeds, false if it fails.
			virtual bool put(const char* name, size_t size, core::types::type_enum type, const core::parsers::simple_options_data& options) = 0;

			/// Puts named field of a enumaration type
			/// @date	27/12/2019
			/// @param	name	 	The name.
			/// @param	size	 	The size.
			/// @param	type	 	The type.
			/// @param	enum_type	interface describes the type of the enum.
			/// @param	options  	Options for controlling the operation.
			/// @return	True if it succeeds, false if it fails.
			virtual bool put_enum(const char* name, size_t size,core::parsers::enum_data_interface* enum_type, const core::parsers::simple_options_data& options) = 0;

			/// Puts the named field of bits
			/// @date	19/10/2019
			/// @param	name	   	The name.
			/// @param	num_of_bits	Number of bits.
			/// @param	data_size  	Size of the data.
			/// @return	True if it succeeds, false if it fails.
			virtual bool put_bits(const char* name, size_t num_of_bits, size_t data_size) = 0;

			/// Puts a string metadata
			/// @date	26/02/2020
			/// @param	name   	The name.
			/// @param	options	Options for controlling the operation.
			/// @return	True if it succeeds, false if it fails.
			virtual bool put_string(const char* name, size_t max_size, const char* default_string) = 0;

			/// Nest metadata allows adding (nesting) a complex meta object (struct, array) as a field in its parent field
			/// @date	11/07/2019
			/// @param 		   	name		   	The name of the nested field.
			/// @param [in]	nested_metadata	If non-null, the nested metadata object.
			/// @return	True if it succeeds, false if it fails.
			virtual bool nest_metadata(const char *name, core::parsers::binary_metadata_interface* nested_metadata) = 0;

			/// Nest metadata allows adding (nesting) a complex meta object (struct, array) as a field in its parent field
			/// allow adding already existing meta object by name, this improve performance by using already exist meta object in many objects
			/// @date	11/07/2019
			/// @param	name		 	The name.
			/// @param	metadata_name	Name of the metadata.
			/// @return	True if it succeeds, false if it fails.
			virtual bool nest_metadata(const char *name, const char* metadata_name) = 0;

			/// add an array of field to the meta object
			/// @date	11/07/2019
			/// @param 		   	name		   	The name of the field.
			/// @param 		   	size		   	The size of an element.
			/// @param 		   	type		   	The type of an element.
			/// @param 		   	num_of_elements	Number of elements.
			/// @param [in]	nested_parser  	If non-null, the nested parser.
			/// @return	True if it succeeds, false if it fails.
			virtual bool array(const char *name, size_t size, core::types::type_enum type, size_t num_of_elements, simple_options_data options, core::parsers::binary_metadata_interface *nested_parser) = 0;

			/// add an array of field to the meta object by name
			/// @date	11/07/2019
			/// @param	name		   	The name.
			/// @param	num_of_elements	Number of elements.
			/// @param	metadata_name  	Name of the metadata.
			/// @return	True if it succeeds, false if it fails.
			virtual bool array(const char *name, size_t num_of_elements, simple_options_data options, const char* metadata_name) = 0;

			/// add an array of field to the meta object by name
			/// @date	05/03/2020
			/// @param	name		   	The name.
			/// @param	num_of_elements	Number of elements.
			/// @param	options		   	Options for controlling the operation.
			/// @param	enum_name	   	Name of the enum.
			/// @return	True if it succeeds, false if it fails.
			virtual bool array(const char *name, size_t num_of_elements, size_t size, simple_options_data options, core::parsers::enum_data_interface* enum_type) = 0;

			/// Sets big endian define the meta object as big/little endian
			/// @date	11/07/2019
			/// @param	big_endian	True to big endian.
			virtual void set_big_endian(bool big_endian) = 0;

			/// Initializes this object from the given from JSON
			/// @date	03/10/2018
			/// @param	json	The JSON.
			/// @return	True if it succeeds, false if it fails.
			virtual bool from_json(const char *json_string) = 0;
		};

		class DLL_EXPORT binary_parser_interface : public ref_count_interface
		{
		public:
			/// Queries a the parser metadata
			/// @date	20/11/2018
			/// @param [in,out]	parser_metadata	If non-null, the parser metadata.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_metadata(const binary_metadata_interface** parser_metadata) const = 0;
			/// Parses provided buffer according the metadata
			/// @date	03/10/2018
			/// @param	data	The data.
			/// @param	size	The data size.
			/// @return	True if it succeeds, false if it fails.
			virtual bool parse(const void* data, size_t size) = 0;
						
			/// Parses the given buffer
			/// @date	27/12/2018
			/// @param [in]	buffer	If non-null, the buffer.
			/// @return	True if it succeeds, false if it fails.
			virtual bool parse(core::safe_buffer_interface *buffer) = 0;

			/// Parse from string buffer
			/// @date	23/12/2018
			/// @param	data	The data.
			/// @return	True if it succeeds, false if it fails.
			virtual bool parse_from_string(const char* data) = 0;

			/// Queries a buffer of the parser
			/// @date	27/02/2019
			/// @param [in,out]	buffer	If non-null, the buffer.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_buffer(core::safe_buffer_interface** buffer) const = 0;

			/// Gets the size of the parser
			/// @date	21/07/2019
			/// @return	A size_t.
			virtual size_t buffer_size() const = 0;

			/// Converts this object to a JSON
			/// @date	23/12/2018
			/// @param 			compact	True to compact false for readable JSON.
			/// @return	The given data converted to a bool.
			virtual const char* to_json(json_details_level details_level, bool compact)  = 0;

			/// Check and get JSON - Check if json has errors and if so update the no_error flag to true 
 			/// and json erroneous fields include <error message string> 
			/// @date	05/08/2020
			/// @param 		   	compact   	True to compact.
			/// @param [in,out]	has_errors	True if has errors, false if not.
			/// @return	Null if it fails, else a pointer to a const char.
			virtual const char* check_and_get_json(json_details_level details_level, bool compact, bool& no_errors) = 0;

			/// Initializes this object from the given from JSON
			/// the function will allow creating an object from json if fields are missing or there are extra fields
			/// @date	03/10/2018
			/// @param	json_string	   	The JSON.
			/// @param	validate_object	True to validate the object fields.
			/// @return	True if it succeeds, false if it fails.
			virtual bool from_json(const char *json_string) = 0;

			/// Reads a simple data (primitive) from the buffer
			/// @date	03/10/2018
			/// @param 		   	index				   	Zero-based index of the.
			/// @param [out]	data				   If non-null, the data read from the buffer
			/// @param 		   	number_of_bytes_to_read	Number of bytes to reads (size of the object).
			/// @param [out]	type				   	The type.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_simple(size_t index, void* data, size_t number_of_bytes_to_read, types::type_enum& type) const = 0;

			/// Reads a simple data (primitive) from the buffer
			/// @date	03/10/2018
			/// @param 		   	name				   	The name of the data to read.
			/// @param [out]	data				   	If non-null, the data read from the buffer.
			/// @param 		   	number_of_bytes_to_read	Number of bytes to reads (size of the object).
			/// @param [out]	type				   	The type of the data.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_simple(const char* name, void* data, size_t number_of_bytes_to_read, types::type_enum& type) const = 0;

			/// Reads enum by index, the function reads the enum according to the field name.
			/// if enum metadata does not exist, it return and enum item with only the val and empty string 
				/// @date	28/12/2019
			/// @param 		   	name	 	The name.
			/// @param [in,out]	enum_item	The enum item.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_enum(const char* name, core::parsers::enum_data_item& enum_item) const = 0;

			/// Reads enum by index, the function reads the enum according to the field index.
			/// if enum metadata does not exist, it return and enum item with only the val and empty string 
			/// @date	28/12/2019
			/// @param 		   	index	 	Zero-based index of the.
			/// @param [in,out]	enum_item	The enum item.
			/// @return	True if it succeeds, false if it fails (not an enum).
			virtual bool read_enum_by_index(size_t index, core::parsers::enum_data_item& enum_item) const = 0;
			/// Reads data from the buffer
			/// @date	03/10/2018
			/// @param [out]	data				   	If non-null, the data read from the buffer.
			/// @param 		   	number_of_bytes_to_read	Number of bytes to reads (size of the object).
			/// @param [in]	node				   	If non-null, the node that describes the data to read.
			/// @param [out]	type				   	The type.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_by_node(void* data, size_t number_of_bytes_to_read, const core::parsers::binary_node_interface *node, core::types::type_enum& type) const = 0;

			/// Reads a complex data from the parser - complex data is usually a struct or a more complex data that requires a parser of its own.
			/// @date	03/10/2018
			/// @param 		   	name  	The name.
			/// @param [out]	parser	If non-null, the parser.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_complex(const char* name, binary_parser_interface** parser) const = 0;

			/// Reads a complex data from the parser - complex data is usually a struct or a more complex data that requires a parser of its own.
			/// @date	03/10/2018
			/// @param 		   	index 	Zero-based index of the.
			/// @param [out]	parser	If non-null, the parser.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_complex(size_t index, binary_parser_interface** parser) const = 0;

			/// Reads at, reads an element from an array - good only for simple arrays
			/// @date	03/10/2018
			/// @param 		   	index				   	Zero-based index of the of the node that start the array.
			/// @param [out]	data				   	If non-null, the data.
			/// @param 		   	number_of_bytes_to_read	Number of bytes to reads.
			/// @param [out]	type				   	The type.
			/// @param 		   	array_index			   	Zero-based index of the element to read in the array.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_at(size_t index, void* data, size_t number_of_bytes_to_read, core::types::type_enum &type, size_t array_index) const = 0;

			/// Reads at, reads an element from an array - good only for simple
			/// arrays
			/// @date	03/10/2018
			/// @param 			name				   	The name.
			/// @param [out]	data				   	If non-null, the data.
			/// @param 			number_of_bytes_to_read	Number of bytes to reads.
			/// @param [out]	type				   	The type.
			/// @param 			array_index			   	Zero-based index of the element
			/// 	to read in the array.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_at(const char* name, void* data, size_t number_of_bytes_to_read, core::types::type_enum &type, size_t array_index) const = 0;

			/// Reads at, reads an element from an array - good only for complex objects
			/// @date	03/10/2018
			/// @param 		   	name  	The name.
			/// @param [in,out]	parser	If non-null, the parser.
			/// @param 		   	index 	Zero-based index of the.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_complex_at(const char* name, binary_parser_interface** parser, size_t array_index) const = 0;

			/// Reads at, reads an element from an array - good only for complex objects
			/// @date	03/10/2018
			/// @param 		   	index	   	Zero-based index of the.
			/// @param [out]	parser	   	If non-null, the parser.
			/// @param 		   	array_index	Zero-based index of the element
			/// 	to read in the array.
			/// @return	True if it succeeds, false if it fails.
			virtual bool read_complex_at(size_t index, binary_parser_interface** parser, size_t array_index) const = 0;

			/// Writes data to the buffer
			/// @date	03/10/2018
			/// @param 		   	name						The name.
			/// @param [in]	data						If non-null, the data.
			/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
			/// @return	True if it succeeds, false if it fails.
			virtual bool write(const char* name, const void* data, size_t number_of_bytes_to_write, core::types::type_enum type) = 0;

			/// Writes data to the buffer based on the size
			/// @date	03/10/2018
			/// @param	index						Zero-based index of the.
			/// @param	data						The data.
			/// @param	number_of_bytes_to_write	Number of bytes to writes.
			/// @return	True if it succeeds, false if it fails.
			virtual bool write(size_t index, const void* data, size_t number_of_bytes_to_write, core::types::type_enum type) = 0;
						
			/// Writes string to the buffer
			/// @date	03/10/2018
			/// @param 		   	name						The name.
			/// @param [in]	data						If non-null, the data.
			/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
			/// @return	True if it succeeds, false if it fails.
			virtual bool write_string(const char* name, const char* data) = 0;

			/// Writes string to the buffer
			/// @date	03/10/2018
			/// @param 		   	name						The name.
			/// @param [in]	data						If non-null, the data.
			/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
			/// @return	True if it succeeds, false if it fails.
			virtual bool write_string(size_t index, const char* data) = 0;

			/// Writes at a specific index to and array
			/// @date	03/10/2018
			/// @param 		   	name						The name.
			/// @param [in]	data						If non-null, the data.
			/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
			/// @param 		   	index						Zero-based index of the.
			/// @return	True if it succeeds, false if it fails.
			virtual bool write_at(const char* name, const void* data, size_t number_of_bytes_to_write, size_t index) = 0;

			/// Writes at a specific index to and array
			/// @date	03/10/2018
			/// @param          index						The index.
			/// @param          [in]	data                If non-null, the data.
			/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
			/// @param 		   	index						Zero-based index of the.
			/// @return	True if it succeeds, false if it fails.
			virtual bool write_at(size_t index, const void* data, size_t number_of_bytes_to_write, size_t array_index) = 0;

			/// Checks this object if all data is valid
			/// @date	19/08/2019
			/// @return	True if it succeeds, false if it fails.
			virtual bool validate() const = 0;

			/// Validates the data of the field of the given name
			/// @date	19/08/2019
			/// @param	name	The name.
			/// @return	True if it succeeds, false if it fails.
			virtual bool validate(const char* name) const = 0;

			/// Validates the data of the field of the given index
			/// @date	21/10/2019
			/// @param	index	Zero-based index of the fields
			/// @return	True if it succeeds, false if it fails.
			virtual bool validate(size_t index) const = 0;

			/// Sets a default value to a specific field in the parser
			/// @date	21/08/2019
			/// @param	name	The name of the filed.
			virtual void set_default_value(const char* name) = 0;

			/// Sets the parser fields to a default value
			/// @date	21/08/2019
			virtual void set_default_values() = 0;

			/// Nullifies this object
			/// @date	06/08/2020
			virtual void nullify() = 0;

			virtual size_t offset() = 0;


		};

		class DLL_EXPORT binary_metadata_store_interface : public ref_count_interface
		{
		public:
			/// store a named parser metadata  
			/// @date	27/12/2018
			/// @param 		   	name		   	The name of the metadata parser.
			/// @param [in]	parser_metadata	If non-null, the parser metadata.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_parser_metadata(const char* name, core::parsers::binary_metadata_interface *parser_metadata) = 0;

			/// Queries parser metadata from the store
			/// @date	27/12/2018
			/// @param 		   	name		   	The name of the parser metadata.
			/// @param [out]	parser_metadata	If non-null, the parser metadata.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_parser_metadata(const char* name, core::parsers::binary_metadata_interface** parser_metadata) = 0;

			/// Adds an enum information including list of name and code of the enum
			/// @date	18/12/2019
			/// @param 		   	name			The name.
			/// @param [in]	enum_instace	If non-null, the enum instance.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_enum(const char* name, core::parsers::enum_data_interface* enum_instance) = 0;

			/// Queries enum information
			/// @date	18/12/2019
			/// @param 		   	name		 	The name.
			/// @param [out]	enum_instance	If non-null, return the enum instance.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_enum(const char* name, core::parsers::enum_data_interface** enum_instance) = 0;
		};

		class DLL_EXPORT binary_parser_creator_interface : public ref_count_interface
		{
		public:
			virtual bool create_parser(const core::parsers::binary_metadata_interface* metadata, core::parsers::binary_parser_interface** parser) const = 0;
			
		};
	}
}
