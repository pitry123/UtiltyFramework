#pragma once
#include <parsers/binary_parser.h>
#include <utils/thread_safe_object.hpp>
#include <utils/types.hpp>
#include <unordered_map>

namespace parsers
{
	
	class binary_parser_impl :
		public utils::ref_count_base<binary_parser>
	{
	private:
		utils::ref_count_ptr<const core::parsers::binary_metadata_interface> m_metadata;
		utils::ref_count_ptr<core::safe_buffer_interface> m_buffer;
		size_t m_offset;
		std::string m_json_str;

		using parser_map =
			std::unordered_map<std::string, utils::ref_count_ptr<binary_parser_impl>>;
		utils::thread_safe_object<parser_map> m_parsers;

		bool parse_nested();
		bool is_big_endian() const;
		bool build_nested_parser(core::parsers::binary_node_interface* node);
		/// Writes data to the buffer.
		/// @date	03/10/2018
		/// @param 	   	data						The data.
		/// @param 	   	number_of_bytes_to_write	Number of bytes to writes.
		/// @param [in]	node						If non-null, the node.
		/// @return	True if it succeeds, false if it fails.
		bool write_by_node(const void* data, size_t number_of_bytes_to_write, const core::parsers::binary_node_interface* node);
		bool write_to_buffer(void* data, size_t number_of_bytes_to_read, size_t offset);

		bool write_bits_by_node(const void* data, size_t number_of_bytes_to_write, const core::parsers::binary_node_interface* node);
		bool read_complex(size_t index, binary_parser_impl** parser) const;
		bool read_complex(const char* name, binary_parser_impl** parser) const;
		bool read_complex_at(size_t index, binary_parser_impl** parser, size_t array_index) const;
		bool bitmap_to_json(core::parsers::binary_node_interface* node, unordered_json& json) const;
		bool enum_val_to_json(int64_t val, const core::parsers::binary_node_interface* node, unordered_json& json, core::parsers::json_details_level details_level) const;
		bool enum_node_to_json(const core::parsers::binary_node_interface* node, unordered_json& json, core::parsers::json_details_level details_level) const;
		bool enum_to_json_verbosed(const core::parsers::binary_node_interface* node, unordered_json& json) const;
		bool enum_val_from_json(const unordered_json& enum_json, const core::parsers::binary_node_interface* node, int64_t& val);
		bool array_to_json(const core::parsers::binary_node_interface* node, unordered_json& json, size_t index, core::parsers::json_details_level details_level) const;
		bool json(unordered_json& json, core::parsers::json_details_level details_level) const;
		void write_simple_default(const core::parsers::binary_node_interface* node);
		void write_string_default(const core::parsers::binary_node_interface* node);
		template <typename T>
		bool val_from_json(const unordered_json& json, const char* name, size_t index)
		{
			if (index > m_metadata->node_count())
				return false;

			if (true == json.is_array())
			{
				if (false == json[index].is_number() &&
					false == json[index].is_boolean())
					return false;
				
				if (false == write<T>(index, json[index].get<T>()))
					return false;
			}
			else
			{
				if (name == nullptr)
					return false;

				auto it = json.find(name);
				if (it != json.end())
				{
					if ((true == it->is_number() ||
						true == it->is_boolean()))
					{
						if (false == write<T>(index, it->get<T>()))
							return false;
					}
				}
			}

			return true;
		}
		int64_t reinterpret_enum(size_t size, int64_t val) const;
		bool validate(const core::parsers::binary_node_interface* node) const;
		bool bitmap_from_json(const unordered_json& json, core::parsers::binary_node_interface* node, size_t& index);
		bool array_from_json(const unordered_json& json, size_t i, utils::ref_count_ptr<core::parsers::binary_node_interface>& node);
		bool from_json(const unordered_json& json);

		bool read_enum_by_node(int64_t& val, size_t size, const core::parsers::binary_node_interface* node, core::types::type_enum& type) const;
	public:
		binary_parser_impl(const core::parsers::binary_metadata_interface* metadata, bool reset = false);

		/// Constructor for creating child parser only used internally in the
		/// parser
		/// @date	04/10/2018
		/// @param [in]	metadata	If non-null, the metadata.
		/// @param [in]	buffer  	If non-null, the buffer.
		/// @param 	   	offset  	The offset.
		binary_parser_impl(core::parsers::binary_metadata_interface* metadata, core::safe_buffer_interface* buffer, size_t offset = 0);

		/// Queries a the parser metadata
		/// @date	20/11/2018
		/// @param [in,out]	parser_metadata	If non-null, the parser metadata.
		/// @return	True if it succeeds, false if it fails.
		bool query_metadata(const core::parsers::binary_metadata_interface** parser_metadata) const override;

		/// Parses provided buffer according the metadata
		/// @date	03/10/2018
		/// @param	data	The data.
		/// @param	size	The data size.
		/// @return	True if it succeeds, false if it fails.
		bool parse(const void* data, size_t size) override;

		/// Parses provided buffer according the metadata
		/// @date	03/10/2018
		/// @param	data	The data.
		/// @param	size	The data size.
		/// @return	True if it succeeds, false if it fails.
		bool parse(core::safe_buffer_interface *buffer) override;
		
		/// Parse from string - parse an hex string into a buffer and call parse
		/// @date	03/10/2018
		/// @param	data	The data.
		/// @return	True if it succeeds, false if it fails.
		bool parse_from_string(const char* data) override;

		/// Queries the buffer of the parser
		/// @date	27/02/2019
		/// @param [out]	buffer	If non-null, the buffer.
		/// @return	True if it succeeds, false if it fails.
		virtual bool query_buffer(core::safe_buffer_interface** buffer) const override;

		size_t buffer_size() const override;
		
		const char* to_json(core::parsers::json_details_level details_level, bool compact) override;

		const char* check_and_get_json(core::parsers::json_details_level details_level, bool compact, bool& no_errors) override;

		bool from_json(const char *json) override;

		bool read_simple(size_t index, void* data, size_t number_of_bytes_to_read, core::types::type_enum& type) const override;

		bool read_simple(const char* name, void* data, size_t number_of_bytes_to_read, core::types::type_enum& type) const override;

		bool read_enum(const char* name, core::parsers::enum_data_item& enum_item) const override;
		bool read_enum_by_index(size_t index, core::parsers::enum_data_item& enum_item) const override;

		/// Reads a simple data (primitive) from the buffer
		/// @date	03/10/2018
		/// @exception	std::invalid_argument	Thrown when an invalid argument
		/// 	error condition occurs.
		/// @tparam	T	Generic type parameter.
		/// @param	index	Zero-based index of the data from the start
		/// @return	The simple data.
		template <typename T>
		T read_simple(size_t index) const
		{
			static_assert(std::is_pointer<T>::value == false, "T cannot be of pointer type");
			static_assert(std::is_array<T>::value == false, "T cannot be of array type");

			T data;
			core::types::type_enum type;
			if (false == read_simple(index, (uint8_t*)&data, sizeof(T), type))
				throw std::runtime_error("binary_parser_impl::read_simple");
			
			return data;
		}

		template <typename T>
		T read_simple(const char* name) const
		{
			static_assert(std::is_pointer<T>::value == false, "T cannot be of pointer type");
			static_assert(std::is_array<T>::value == false, "T cannot be of array type");

			T data;
			core::types::type_enum type;
			read_simple(name, (uint8_t*)&data, sizeof(T), type);

			if (utils::types::get_type<T>() != type)
				throw std::invalid_argument("Read type mismatch");
		}

		/// Reads data from the buffer
		/// @date	03/10/2018
		/// @param [out]	data				   	If non-null, the data read from the buffer.
		/// @param 		   	number_of_bytes_to_read	Number of bytes to reads (size of the object).
		/// @param [in]	node				   	If non-null, the node that describes the data to read.
		/// @param [out]	type				   	The type.
		/// @return	True if it succeeds, false if it fails.
		bool read_by_node(void* data, size_t number_of_bytes_to_read, const core::parsers::binary_node_interface *node, core::types::type_enum& type) const override;
		bool read_from_buffer(void* data, size_t number_of_bytes_to_read, size_t offset, bool big_endian) const;

		bool read_bits_by_node(void* data, size_t number_of_bytes_to_read, const core::parsers::binary_node_interface *node) const;
		/// Reads a string from the parser
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	index	Zero-based index of the field.
		/// @return	The string.
		bool read_string_by_index(size_t index, char* str, size_t length) const;

		/// Reads a string from the parser
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name of the field.
		/// @return	The string.
		bool read_string(const char* name, char* str, size_t length) const;

		/// Reads a complex data from the parser - complex data is usually a struct or a more complex data that requires a parser of its own.
		/// @date	03/10/2018
		/// @param 		   	name  	The name.
		/// @param [out]	parser	If non-null, the parser.
		/// @return	True if it succeeds, false if it fails.
		bool read_complex(const char* name, binary_parser_interface** parser) const override;

		/// Reads a complex data from the parser - complex data is usually a struct or a more complex data that requires a parser of its own.
		/// @date	03/10/2018
		/// @param 		   	index 	Zero-based index of the.
		/// @param [out]	parser	If non-null, the parser.
		/// @return	True if it succeeds, false if it fails.
		bool read_complex(size_t index, binary_parser_interface** parser) const override;

		/// Reads array attributes, read an array attributes
		/// @date	03/10/2018
		/// @param 		   	name		   	The name.
		/// @param [out]	element_size   	Size of a single element in the array.
		/// @param [out]	num_of_elements	Number of elements.
		/// @param [out]	type		   	The type of a single element.
		/// @return	True if it succeeds, false if it fails.
		bool read_array_attributes(const char* name, size_t& element_size, size_t& num_of_elements, core::types::type_enum& type) const;

		/// Reads array attributes, read an array attributes
		/// @date	03/10/2018
		/// @param 		   	index		   	Zero-based index of the.
		/// @param [out]	element_size   	Size of a single element in the array.
		/// @param [out]	num_of_elements	Number of elements.
		/// @param [out]	type		   	The type of a single element.
		/// @return	True if it succeeds, false if it fails.
		bool read_array_attributes(size_t index, size_t &element_size, size_t &num_of_elements, core::types::type_enum &type) const;

		/// Reads at, reads an element from an array - good only for simple arrays
		/// @date	03/10/2018
		/// @param 		   	index				   	Zero-based index of the of the node that start the array.
		/// @param [out]	data				   	If non-null, the data.
		/// @param 		   	number_of_bytes_to_read	Number of bytes to reads.
		/// @param [out]	type				   	The type.
		/// @param 		   	array_index			   	Zero-based index of the element to read in the array.
		/// @return	True if it succeeds, false if it fails.
		bool read_at(size_t index, void* data, size_t number_of_bytes_to_read, core::types::type_enum &type, size_t array_index) const override;

		/// Reads at, reads an element from an array - good only for simple arrays
		/// @date	03/10/2018
		/// @exception	std::invalid_argument	Thrown when an invalid argument
		/// 	error condition occurs.
		/// @tparam	T	Generic type parameter.
		/// @param	name 	The name of the arras.
		/// @param	index	Zero-based index of the of the element to read in the array.
		/// @return	at.
		template <typename T>
		T read_at(const char* name, size_t index) const
		{
			static_assert(std::is_pointer<T>::value == false, "T cannot be of pointer type");
			static_assert(std::is_array<T>::value == false, "T cannot be of array type");

			T data;
			core::types::type_enum type;
			read_at(name, &data, sizeof(T), type, index);

			if (utils::types::get_type<T>() != type)
				throw std::invalid_argument("type mismatch");
		}

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
		bool read_at(const char* name, void* data, size_t number_of_bytes_to_read, core::types::type_enum &type, size_t array_index) const override;

		/// Reads at, reads an element from an array - good only for complex objects
		/// @date	03/10/2018
		/// @param 		   	name  	The name.
		/// @param [in,out]	parser	If non-null, the parser.
		/// @param 		   	index 	Zero-based index of the.
		/// @return	True if it succeeds, false if it fails.
		bool read_complex_at(const char* name, binary_parser_interface** parser, size_t array_index) const override;

		/// Reads at, reads an element from an array - good only for complex objects
		/// @date	03/10/2018
		/// @param 		   	index	   	Zero-based index of the.
		/// @param [out]	parser	   	If non-null, the parser.
		/// @param 		   	array_index	Zero-based index of the element
		/// 	to read in the array.
		/// @return	True if it succeeds, false if it fails.
		bool read_complex_at(size_t index, binary_parser_interface** parser, size_t array_index) const override;

		/// Writes a simple data to the buffer
		/// @date	03/10/2018
		/// @tparam	T	Generic type parameter.
		/// @param	name	The name.
		/// @param	data	The data.
		/// @return	True if it succeeds, false if it fails.
		template <typename T>
		bool write(const char* name, const T &data)
		{
			static_assert(std::is_pointer<T>::value == false, "T cannot be of pointer type");
			static_assert(std::is_array<T>::value == false, "T cannot be of array type");

			core::types::type_enum type = utils::types::get_type<T>();
			return write(name, (uint8_t*)(&data), sizeof(T), type);
		}

		/// Writes data to the buffer
		/// @date	03/10/2018
		/// @param 		   	name						The name.
		/// @param [in]	data						If non-null, the data.
		/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
		/// @return	True if it succeeds, false if it fails.
		bool write(const char* name, const void* data, size_t number_of_bytes_to_write, core::types::type_enum type) override;

		/// Writes string to the buffer
		/// @date	03/10/2018
		/// @param 		   	name						The name.
		/// @param [in]	data						If non-null, the data.
		/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
		/// @return	True if it succeeds, false if it fails.
		bool write_string(const char* name, const char* data) override;

		/// Writes string to the buffer
		/// @date	03/10/2018
		/// @param 		   	name						The name.
		/// @param [in]	data						If non-null, the data.
		/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
		/// @return	True if it succeeds, false if it fails.
		bool write_string(size_t index, const char* data) override;

		/// Writes data to the buffer
		/// @date	03/10/2018
		/// @tparam	T	Generic type parameter.
		/// @param	index	Zero-based index of the.
		/// @param	data 	The data.
		/// @return	True if it succeeds, false if it fails.
		template <typename T>
		bool write(size_t index, const T &data)
		{
			static_assert(std::is_pointer<T>::value == false, "T cannot be of pointer type");
			static_assert(std::is_array<T>::value == false, "T cannot be of array type");

			core::types::type_enum type = utils::types::get_type<T>();
			return write(index, (const uint8_t*)&data, sizeof(T), type);
		}

		/// Writes
		/// @date	03/10/2018
		/// @param	index						Zero-based index of the.
		/// @param	data						The data.
		/// @param	number_of_bytes_to_write	Number of bytes to writes.
		/// @return	True if it succeeds, false if it fails.
		bool write(size_t index, const void* data, size_t number_of_bytes_to_write, core::types::type_enum type) override;

		/// Writes at a specific index to and array
		/// @date	03/10/2018
		/// @param 		   	name						The name.
		/// @param [in]	data						If non-null, the data.
		/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
		/// @param 		   	index						Zero-based index of the.
		/// @return	True if it succeeds, false if it fails.
		bool write_at(const char* name, const void* data, size_t number_of_bytes_to_write, size_t index) override;
		/// Writes at a specific index to and array
		/// @date	03/10/2018
		/// @param          index						The index.
		/// @param          [in]	data                If non-null, the data.
		/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
		/// @param 		   	index						Zero-based index of the.
		/// @return	True if it succeeds, false if it fails.
		bool write_at(size_t index, const void* data, size_t number_of_bytes_to_write, size_t array_index) override;

		/// Checks this object if all data is valid
		/// @date	19/08/2019
		/// @return	True if it succeeds, false if it fails.
		bool validate() const override;

		/// Validates the data of the field of the given name
		/// @date	19/08/2019
		/// @param	name	The name.
		/// @return	True if it succeeds, false if it fails.
		bool validate(const char* name) const override;

		/// Validates the data of the field of the given index
		/// @date	21/10/2019
		/// @param	index	Zero-based index of the fields
		/// @return	True if it succeeds, false if it fails.
		bool validate(size_t index) const override;
		/// Sets a default value to a specific field in the parser
		/// @date	21/08/2019
		/// @param	name	The name.
		void set_default_value(const char* name) override;

		/// Sets the parser fields to a default value
		/// @date	21/08/2019
		void set_default_values() override;

		void nullify() override;

		size_t offset() override;
	};
}