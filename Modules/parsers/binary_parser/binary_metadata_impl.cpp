#include <core/parser.h>

#include <utils/thread_safe_object.hpp>
#include <streams/memory_stream_interface.h>
#include <core/buffer_interface.h>
#include <utils/buffer_allocator.hpp>
#include <utils/types.hpp>
#include <parsers/binary_parser.h>
#include "../nlohmann/fifo_json.hpp"
#include <utils/parser.hpp>
#include <utils/ref_count_object_pool.hpp>
#include <utils/strings.hpp>
#include <vector>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <iostream>

namespace parsers
{
	using namespace core::types;
	using namespace core::parsers;
	using namespace utils::types;
	/// A binary metadata - hold the metadata that describes the parsed object. it does not manage the data itself
	/// it holds a vector of nodes each describes the relevant node including its name, size, offset from start.
	/// If it describe a complex data or and ARRAY it will also hold a metadata to that describe the struct of array
	/// @date	23/09/2018
	class binary_metadata_impl : public utils::ref_count_base<parsers::binary_metadata>
	{
	private:
		/// A binary node - hold the description on a specific node of the data
		/// @date	03/10/2018
		class binary_node : public ref_count_base<binary_node_interface>
		{
			friend class binary_metadata_impl;

		private:
			std::string m_name;
			size_t m_offset;
			size_t m_size;
			type_enum m_type;
			bool m_big_endian;
			utils::parsers::simple_options m_options;
			std::string m_def_string;
			utils::ref_count_ptr<binary_metadata_interface> m_nested_parser;
			utils::ref_count_ptr <enum_data_interface>  m_enum;
			std::map<double,utils::ref_count_ptr<binary_metadata_interface>> m_choice_parser;
			size_t m_count;
			
		public:
			//general puprose
			binary_node(const char* name, size_t offset, size_t size,
				type_enum type, bool big_endian, binary_metadata_interface* nested_parser = nullptr, enum_data_interface* enum_type = nullptr,size_t count = 1) :
				m_name(name),
				m_offset(offset),
				m_size(size),
				m_type(type),
				m_big_endian(big_endian),
				m_nested_parser(nested_parser),
				m_enum(enum_type),
				m_count(count)
			{
			}

			//with options
			binary_node(const char* name, size_t offset, size_t size,
				type_enum type, bool big_endian,const simple_options_data& options, enum_data_interface* enum_type = nullptr,size_t count = 1) :
				binary_node(name,offset,size,type,big_endian,nullptr, enum_type,count)
				
			{
				m_options = options;
			}

			//for string
			binary_node(const char* name, size_t offset, size_t size,
				type_enum type, bool big_endian, const char* default_string, size_t count = 1) :
				binary_node(name, offset, size, type, big_endian, nullptr, nullptr, count)

			{
				m_def_string = default_string;
			}

			const char* name() const override
			{
				return m_name.c_str();
			}

			size_t offset() const override
			{
				return m_offset;
			}

			size_t size() const override
			{
				return m_size;
			}

			type_enum type() const override
			{
				return m_type;
			}

			bool big_endian() const override
			{
				return m_big_endian;
			}
			size_t count() const override
			{
				return m_count;
			}
			simple_options_data options() const override 
			{
				simple_options_data raw_options;
				raw_options =  m_options;
				return raw_options;
			}

			const char* string_default() const override
			{
				return m_def_string.c_str();
			}

			bool query_enum(enum_data_interface** enum_type) const override
			{
				if (enum_type == nullptr)
					return false;

				if (m_enum == nullptr)
					return false;

				*enum_type = m_enum;
				(*enum_type)->add_ref();
				return true;
			}
			bool nested(binary_metadata_interface** nested_parser) const override
			{
				if (nested_parser == nullptr)
					return false;

				if (m_nested_parser == nullptr)
					return false;

				*nested_parser = m_nested_parser;
				(*nested_parser)->add_ref();
				return true;
			}

			virtual bool read(void* data, size_t data_size, void* buffer, size_t buffer_size) const override
			{
				if (data == nullptr)
					return false;

				if (data_size > size())
					return false;

				if (data_size > buffer_size)
					return false;

				MEMCPY(data, data_size, buffer, size());

				if (m_big_endian != utils::types::is_big_endian())
				{
					//if data stream is mismatched to the platform endian convert
					uint8_t* data_change = (uint8_t*)data;
					size_t convert_data_size;
					if (data_size % 2 != 0)
					{
						convert_data_size = data_size - 1;
					}
					else
					{
						convert_data_size = data_size;
					}

					for (size_t i = 0; i < convert_data_size; i += 2)
					{
						utils::types::endian_swap(*((uint16_t*)&data_change[i]));
					}
				}

				return true;
			}

			virtual bool write(const void* data, size_t data_size, void* buffer, size_t buffer_size) const override
			{
				if (data == nullptr)
					return false;

				if (data_size > size())
					return false;

				if (data_size > buffer_size)
					return false;

				if (m_big_endian != utils::types::is_big_endian())
				{
					//if data stream is mismatched to the platform endian convert
					size_t convert_data_size;
					uint8_t* data_change = (uint8_t*)data;
					if (data_size % 2 != 0)
					{
						convert_data_size = data_size - 1;
					}
					else
					{
						convert_data_size = data_size;
					}

					for (size_t i = 0; i < convert_data_size; i += 2)
					{
						utils::types::endian_swap(*((uint16_t*)&data_change[i]));
					}
				}

				MEMCPY(buffer, buffer_size, data, data_size);
				return true;
			}
		};
		class bit_binary_node : public binary_node
		{
		protected:
			size_t m_num_of_bits;
			size_t m_bit_offset;
			template <typename T>
			uint64_t mask_bit_data(const void* data, uint64_t& exist_data)  const
			{
				uint64_t new_data;
				const T* temp = reinterpret_cast<const T*>(data);
				exist_data = exist_data & ~mask();
				new_data = static_cast<uint64_t>((*temp) << bit_offset());
				new_data = new_data | exist_data;

				return new_data;
			}
			template <typename T>
			bool check_data(const void* data) const
			{
				const T* temp = reinterpret_cast<const T*>(data);
				if (*temp > static_cast<T>((std::pow(2, num_of_bits()) - 1)))
					return false;

				return true;
			}
		public:
			bit_binary_node(const char* name, size_t size, size_t offset, size_t bit_offset,
				bool big_endian, size_t num_of_bits) :
				binary_node(name, offset, size, type_enum::BITMAP, big_endian)
			{
				m_num_of_bits = num_of_bits;
				m_bit_offset = bit_offset;
			}
			size_t num_of_bits() const
			{
				return m_num_of_bits;
			}

			size_t bit_offset() const
			{
				return m_bit_offset;
			}
			uint64_t mask() const
			{
				uint64_t bitmask = 0;
				bitmask = static_cast<uint64_t>(std::pow(2, m_num_of_bits)) - 1;
				bitmask <<= m_bit_offset;

				return bitmask;
			}

			virtual bool read(void* data, size_t data_size, void* buffer, size_t buffer_size) const override
			{
				if (data == nullptr)
					return false;

				if (data_size > size())
					return false;

				if (buffer_size > sizeof(uint64_t))
					return false;

				uint64_t exist_data = 0;
#ifdef WIN32
				memcpy_s(&exist_data, sizeof(uint64_t), buffer, buffer_size);
#else
				std::memcpy(&exist_data, buffer, buffer_size);
#endif

				exist_data &= mask();
				exist_data >>= bit_offset();

#ifdef WIN32
				memcpy_s(data, data_size, &exist_data, data_size);
#else
				std::memcpy(data, &exist_data, buffer_size);
#endif
				if (m_big_endian != utils::types::is_big_endian())
				{
					//if data stream is mismatched to the platform endian convert
					size_t convert_data_size;
					uint8_t* data_change = (uint8_t*)data;
					if (data_size % 2 != 0)
					{
						convert_data_size = data_size - 1;
					}
					else
					{
						convert_data_size = data_size;
					}

					for (size_t i = 0; i < convert_data_size; i += 2)
					{
						utils::types::endian_swap(*((uint16_t*)&data_change[i]));
					}
				}

				return true;




			}

			virtual bool write(const void* data, size_t data_size, void* buffer, size_t buffer_size) const override
			{
				if (data == nullptr)
					return false;

				if (data_size > size())
					return false;

				if (buffer_size > sizeof(uint64_t))
					return false;

				if (m_big_endian != utils::types::is_big_endian())
				{
					//if data stream is mismatched to the platform endian convert
					size_t convert_data_size;
					uint8_t* data_change = (uint8_t*)data;
					if (data_size % 2 != 0)
					{
						convert_data_size = data_size - 1;
					}
					else
					{
						convert_data_size = data_size;
					}

					for (size_t i = 0; i < convert_data_size; i += 2)
					{
						utils::types::endian_swap(*((uint16_t*)&data_change[i]));
					}
				}

				uint64_t exist_data = 0;
				uint64_t new_data = 0;
#ifdef WIN32
				memcpy_s(&exist_data, sizeof(uint64_t), buffer, buffer_size);
#else
				std::memcpy(&exist_data, buffer, buffer_size);
#endif


				//check if the bits are in range and then 
				// mask it with the rest of the bits
				if (data_size == sizeof(uint8_t))
				{
					if (false == check_data<uint8_t>(data))
						return false;
					new_data = mask_bit_data<uint8_t>(data, exist_data);
				}
				else if (data_size == sizeof(uint16_t))
				{
					if (false == check_data<uint16_t>(data))
						return false;
					new_data = mask_bit_data<uint16_t>(data, exist_data);
				}
				else if (data_size == sizeof(uint32_t))
				{
					if (false == check_data<uint32_t>(data))
						return false;
					new_data = mask_bit_data<uint32_t>(data, exist_data);
				}
				else if (data_size == sizeof(uint64_t))
				{
					if (false == check_data<uint64_t>(data))
						return false;
					new_data = mask_bit_data<uint64_t>(data, exist_data);
				}
				else
					return false;
#ifdef WIN32
				memcpy_s(buffer, buffer_size, &new_data, buffer_size);
#else
				std::memcpy(buffer, &new_data, buffer_size);
#endif
				return true;
			}

		};
		bool m_big_endian;
		mutable std::string m_json_str;
		using parser_node_vector =
			std::vector<utils::ref_count_ptr<binary_node_interface>>;
		utils::thread_safe_object<parser_node_vector> m_parser_metadata;
		utils::ref_count_ptr<binary_metadata_store_interface> m_store;
		std::atomic<size_t> m_offset;
		std::string m_name;
		utils::ref_count_ptr<binary_parser_creator_interface> m_parser_creator;
		utils::ref_count_ptr<utils::ref_count_object_pool<binary_parser_interface>> m_parsers_pool;
		/// Adds a bit node to metadata the assumption of this private functions
		/// that all nodes of this metadata are bit_binary_nodes
		/// @date	30/09/2019
		/// @param	name	   	The name.
		/// @param	num_of_bits	Number of bits.
		/// @param	data_size  	Size of the whole block is bytes.
		/// @return	A size_t.
		size_t add_bit_node(const char* name, size_t num_of_bits, size_t data_size, binary_node_interface* prev_bit_node)
		{
			size_t bit_count = 0;
			bit_binary_node* bit_node = nullptr;
			size_t offset = 0;
			size_t size = 0;
			size_t size_to_increase = 0;
			size_t bit_remaining = 0;
			size_t bit_offset = 0;
			if (prev_bit_node != nullptr)
			{
				bit_node = (bit_binary_node*)static_cast<binary_node_interface*>(prev_bit_node);
				bit_count = bit_node->bit_offset() + bit_node->num_of_bits();
				offset = bit_node->offset();
				size = bit_node->size();

				bit_offset = bit_node->bit_offset() + bit_node->num_of_bits();
				bit_remaining = ((bit_node->size() * 8) - bit_count);
				if (bit_remaining == 0)
				{
					size = data_size;
					size_to_increase = data_size;
					bit_offset = 0;
					offset = m_offset;
				}
				else if (bit_remaining < num_of_bits)
				{//the size of the hosting metadata is not large enough need to increase by one byte
					size_to_increase = (num_of_bits - bit_remaining) / 8 + 1;
					size += size_to_increase;
				}
			}
			else
			{
				size = data_size;
				size_to_increase = data_size;
				offset = m_offset;
				bit_remaining = size * 8;
			}

			m_parser_metadata.use<void>([&](parser_node_vector& parser_data)
			{
				utils::ref_count_ptr<binary_node_interface> new_node = utils::make_ref_count_ptr<bit_binary_node>(name, size, offset, bit_offset, m_big_endian, num_of_bits);
				parser_data.emplace_back(new_node);
			});

			m_offset += size_to_increase;

			return size_to_increase;
		}

		bool schema_to_json(const binary_metadata_interface* metadata, unordered_json& json) const
		{
			bool retval = true;
			char type_name[256];
			json["schema_name"] = metadata->namely();
			json["big_endian"] = m_big_endian;
			unordered_json array_json;
			for (size_t i = 0; i < metadata->node_count(); i++)
			{
				utils::ref_count_ptr<binary_node_interface> node;
				if (metadata->query_node_by_index(i, &node))
				{
					array_json[i]["name"] = node->name();
					array_json[i]["size"] = node->size();
					
					if (utils::types::get_type_name(node->type(), type_name))
						array_json[i]["type"] = type_name;
					else
						return false;
					if (utils::types::is_simple_type(node->type()) || 
						node->type() == type_enum::ENUM)
					{
						utils::parsers::simple_options options(node->options());
						if (options.has_min())
							array_json[i]["options"]["minval"] = options.to_string(options.raw_minval(), node->type());
						if (options.has_max())
							array_json[i]["options"]["maxval"] = options.to_string(options.raw_maxval(), node->type());
						if (options.has_default())
							array_json[i]["options"]["defval"] = options.to_string(options.raw_defval(), node->type());
						if (node->type() == type_enum::ENUM)
						{
							utils::ref_count_ptr<enum_data_interface> enum_data;
							if (node->query_enum(&enum_data))
							{
								array_json[i]["enum"] = enum_data->name();
							}

						}
					}
					else if (node->type() == STRING)
					{
						if (node->string_default()[0] != '\0')
						{
							array_json[i]["options"]["defval"] = node->string_default();
						}
					}
					else if (node->type() == ARRAY)
					{
						utils::ref_count_ptr<binary_metadata_interface> nested_metadata;
						utils::ref_count_ptr<binary_node_interface> internal_node;
						if (node->nested(&nested_metadata))
						{
							array_json[i]["length"] = node->count();

							if (nested_metadata->query_node_by_index(0, &internal_node))
							{
								if (utils::types::get_type_name(internal_node->type(), type_name))
									array_json[i]["array_type"] = type_name;
								else
									return false;

								array_json[i]["type_size"] = internal_node->size();
								if (internal_node->type() == COMPLEX)
								{
									utils::ref_count_ptr<binary_metadata_interface> array_metadata;
									internal_node->nested(&array_metadata);
									
									schema_to_json(array_metadata, array_json[i]["data"]);
								}
								else if (utils::types::is_simple_type(internal_node->type()) ||
									internal_node->type() == type_enum::ENUM)
								{
									utils::parsers::simple_options options(internal_node->options());
									if(options.has_min())
										array_json[i]["options"]["minval"] = options.to_string(options.raw_minval(), internal_node->type());
									if (options.has_max())
										array_json[i]["options"]["maxval"] = options.to_string(options.raw_maxval(), internal_node->type());
									if (options.has_default())
										array_json[i]["options"]["defval"] = options.to_string(options.raw_defval(), internal_node->type());
									
									utils::ref_count_ptr<enum_data_interface> enum_data;
									if (node->query_enum(&enum_data))
									{
										array_json[i]["enum"] = enum_data->name();
									}
								}
								else if (node->type() == STRING)
								{
									if (node->string_default()[0] != '\0')
									{
										array_json[i]["options"]["defval"] = node->string_default();
									}
								}
								
							}
						}
					}
					else if (node->type() == COMPLEX)
					{
						utils::ref_count_ptr<binary_metadata_interface> nested_metadata;
						if (node->nested(&nested_metadata))
						{
							retval = schema_to_json(nested_metadata, array_json[i]["data"]);
							if (false == retval)
								return retval;
						}
						else
							return false;
					}
					else if (node->type() == type_enum::BITMAP)
					{
						type_enum type;
						type = utils::types::get_type_by_size(node->size());
						if (utils::types::get_type_name(type, type_name))
						{
							bit_binary_node* bit_node = (bit_binary_node*)static_cast<binary_node_interface*>(node);
							std::stringstream bit_type;
							bit_type << type_name << ":" << bit_node->num_of_bits();
							array_json[i]["type"] = bit_type.str();
						}
					}
				}
			}
			json["data"] = array_json;
			return retval;
		}

		bool json_to_schema(unordered_json &json)
		{
			size_t size;
			type_enum type;
			std::string node_name;
			std::string schema_name = json["schema_name"].get<std::string>();
			std::string type_name;

			//Ignore the name and always create a full parser
			/*if (false == namely(schema_name.c_str()))
			{
				utils::ref_count_ptr<binary_metadata_interface> exit_metadata;
				if (m_store->query_parser_metadata(schema_name, &exit_metadata))
				{
				  
				}
			}*/

			m_big_endian = json["big_endian"].get<bool>();
			unordered_json array_json = json["data"];
			utils::parsers::simple_options options;
			for (unordered_json::iterator it = array_json.begin(); it != array_json.end(); ++it)
			{
				unordered_json obj = *it;
				size = obj["size"].get<size_t>();
				type_name = obj["type"].get<std::string>();
				type = utils::types::get_type(type_name.c_str());
				node_name = obj["name"].get<std::string>();
				if (type == type_enum::STRING)
				{
					std::string default_string;
					if(obj["options"]["defval"] != nullptr)
						default_string = obj["options"]["defval"].get<std::string>();

					if (false == put_string(node_name.c_str(), size, default_string.c_str()))
						return false;
				}
				else if (type == type_enum::COMPLEX)
				{
					unordered_json internal_obj = obj["data"];
					utils::ref_count_ptr<parsers::binary_metadata_impl> internal_metdata = utils::make_ref_count_ptr<parsers::binary_metadata_impl>
						(m_store,m_big_endian,m_parser_creator);
					internal_metdata->json_to_schema(internal_obj);
					if (!nest_metadata(node_name.c_str(), internal_metdata))
						return false;
				}
				else if (type == type_enum::ARRAY)
				{
					size_t len = obj["length"].get<size_t>();
					type_enum array_type = utils::types::get_type(obj["array_type"].get<std::string>().c_str());
					size_t type_size = obj["type_size"].get<size_t>();
					if (array_type == type_enum::COMPLEX)
					{
						unordered_json internal_obj = obj["data"];
						utils::ref_count_ptr<binary_metadata_impl> array_metadata = 
							utils::make_ref_count_ptr<binary_metadata_impl>
							(m_store,m_big_endian,m_parser_creator);
						
						array_metadata->json_to_schema(internal_obj);
						
						array(node_name.c_str(), type_size, array_type,len, options, array_metadata);

					}
					else if(is_simple_type(array_type) || 
						array_type == type_enum::ENUM)
					{
						if (obj["options"]["defval"] != nullptr)
							options.defval(obj["options"]["defval"].get<std::string>().c_str(), array_type);
						if (obj["options"]["minval"] != nullptr)
							options.minval(obj["options"]["minval"].get<std::string>().c_str(), array_type);
						if (obj["options"]["maxval"] != nullptr)
							options.maxval(obj["options"]["maxval"].get<std::string>().c_str(), array_type);

						unordered_json enum_json_data = obj["enum"];
						if (enum_json_data != nullptr)
						{
							utils::ref_count_ptr<enum_data_interface> enum_data;

							std::string enum_name = enum_json_data.get<std::string>();
							if (m_store != nullptr &&
								m_store->query_enum(enum_name.c_str(), &enum_data))
							{
								array(node_name.c_str(), len, type_size, options, enum_data);
								continue;
							}

						}
						array(node_name.c_str(), type_size, array_type, len, static_cast<simple_options_data>(options), nullptr);
					}
					else if(array_type == type_enum::STRING)
					{
						std::string def_string = obj["options"]["defval"].get<std::string>();
						array(node_name.c_str(), type_size, array_type, len,def_string.c_str());
					}
				
				}
				else if (type == type_enum::BITMAP)
				{
					size_t num_of_bits, num_of_bytes;
					if (utils::types::get_bitmap_sizes(type_name.c_str(), num_of_bits, num_of_bytes))
					{
						if (false == put_bits(node_name.c_str(), num_of_bits, num_of_bytes))
							return false;
					}
				}
				else
				{
					if(obj["options"]["defval"] != nullptr)
						options.defval(obj["options"]["defval"].get<std::string>().c_str(), type);
					if (obj["options"]["minval"] != nullptr)
						options.minval(obj["options"]["minval"].get<std::string>().c_str(), type);
					if (obj["options"]["maxval"] != nullptr)
						options.maxval(obj["options"]["maxval"].get<std::string>().c_str(), type);
					if (type == type_enum::ENUM)
					{
						utils::ref_count_ptr<enum_data_interface> enum_data;
						unordered_json enum_json_data = obj["enum"];
						if (enum_json_data != nullptr)
						{
							std::string enum_name = enum_json_data.get<std::string>();
							if (m_store != nullptr && 
								m_store->query_enum(enum_name.c_str(), &enum_data))
								put_enum(node_name.c_str(), size, enum_data, options);
							else
								put(node_name.c_str(), size, type, options);
						}
						else
							put(node_name.c_str(), size, type, options);

					}
					else if (!put(node_name.c_str(), size, type, options))
						return false;
				}
			}
			return true;
		}
	public:
		binary_metadata_impl(binary_metadata_store_interface* store, bool big_endian, binary_parser_creator_interface* parser_creator) :
			m_big_endian(big_endian),
			m_store(store),
			m_offset(0),
			m_parser_creator(parser_creator)
		{
			m_parsers_pool = utils::make_ref_count_ptr<utils::ref_count_object_pool<binary_parser_interface>>
                (static_cast<size_t>(2), utils::ref_count_object_pool<binary_parser_interface>::growing_mode::doubling,
					[&](binary_parser_interface** parser) {
				return m_parser_creator->create_parser(this, parser);
			});
		}

		binary_metadata_impl(bool big_endian = false, binary_parser_creator_interface* parser_creator = nullptr) :
			binary_metadata_impl(nullptr,big_endian,parser_creator)
		{
			
		}
		

		binary_metadata_impl(binary_metadata_store_interface* store, const char* json_metadata, binary_parser_creator_interface* parser_creator) :
			binary_metadata_impl(store,utils::types::is_big_endian(),parser_creator)
		{
			from_json(json_metadata);
		}

		/// Queries a node from the metadata according to the name
		/// @date	03/10/2018
		/// @param 		   	name	The name.
		/// @param [out]	node	If non-null, the node.
		/// @return	True if it succeeds, false if it fails.
		bool query_node(const char* name, binary_node_interface** node) const override
		{
			return m_parser_metadata.use<bool>([&](const parser_node_vector& parser_data)
			{
					for (size_t i = 0; i < parser_data.size(); i++)
					{
						if (std::strcmp(name, parser_data[i]->name()) == 0)
						{
							*node = parser_data[i];
							(*node)->add_ref();
							return true;
						}

					}
				return false;
			});
		}

		/// Queries a node according to the index
		/// @date	03/10/2018
		/// @param 		   	index	Zero-based index of the.
		/// @param [out]	node 	If non-null, the node.
		/// @return	True if it succeeds, false if it fails.
		bool query_node_by_index(size_t index, binary_node_interface** node) const override
		{
            return m_parser_metadata.use<bool>([&](const parser_node_vector& parser_data)
			{
				if (index >= parser_data.size())
					return false;

				*node = parser_data[index];
				if (nullptr == (*node))
					return false;

				(*node)->add_ref();
				return true;
			});
		}
				
		/// number of nodes in the metadata
		/// @date	03/10/2018
		/// @return	A size_t.
		size_t node_count() const override
		{
			return m_parser_metadata.use<size_t>([&](const parser_node_vector& parser_data)
			{
				return parser_data.size();
			});
		}

		/// Gets the size of the whole parser
		/// @date	03/10/2018
		/// @return	A size_t.
		size_t size() const override
		{
			return m_offset.load();
		}

		/// Gets the name of the metadata
		/// this is optional and might be empty
		/// @date	04/10/2018
		/// @return	Null if it fails, else a pointer to a const char.
		const char* namely() const override
		{
			if (m_name.empty())
				return "";

			return m_name.c_str();
		}

		bool create_parser(binary_parser_interface** parser) const override
		{
			if (parser == nullptr)
				return false;
			
			if (m_parsers_pool == nullptr)
				return m_parser_creator->create_parser(this, parser);
			
			if (m_parsers_pool->get_item(parser))
			{
				(*parser)->nullify();
				return true;
			}

			return false;
		}

		bool get_index_by_name(const char* name, size_t& index) const override
		{
			
			return m_parser_metadata.use<bool>([&](const parser_node_vector& parser_data)
			{
				for (size_t i = 0; i < parser_data.size(); i++)
				{
					if (std::strcmp(name, parser_data[i]->name()) != 0)
						continue;

					index = i;
					return true;
				}
				return false;
			});
		}
		/// Determines if the parser is big endian
		/// @date	03/10/2018
		/// @return	True if it succeeds, false if it fails.
		bool big_endian() const override
		{
			return m_big_endian;
		}

		const char* to_json(bool compact) const override
		{
			int indent = -1;

			if (false == compact)
				indent = 4;
			unordered_json json;
			
			if (schema_to_json(this, json))
			{
				m_json_str = json.dump(indent);

				return m_json_str.c_str();
			}

			return nullptr;
		}

		bool from_json(const char *json_string) override
		{
			unordered_json json;
			json = unordered_json::parse(json_string);

			return json_to_schema(json);
			
			
		}
		
		/// set a name to the parser
		/// @date	04/10/2018
		/// @param	name	The name.
		bool namely(const char* name) override
		{
			if (name == nullptr)
				return false;
			m_name = name;
			
			//if the name is "" just ignore it
			if (m_name.empty())
				return true;

			if (m_store != nullptr)
			{
				utils::ref_count_ptr<binary_metadata_interface> metadata;
				if (m_store->query_parser_metadata(name, &metadata))
				{
					//metadata with that name already exist
					return false;
				}
				else
				{
					//Store the metadata in the store
					m_store->add_parser_metadata(name, this);
				}
			}

			return true;


		}

		/// Queries the store of the metadata parser if exist
		/// @date	27/12/2018
		/// @param [in,out]	store	If non-null, the store.
		/// @return	True if it succeeds, false if it fails.
		bool query_store(binary_metadata_store_interface **store)
		{
			if (store == nullptr)
				return false;

			if (m_store == nullptr)
				return false;
			*store = m_store;
			(*store)->add_ref();
			return true;
		}

		/// set Big endian to true or false.
		/// @date	03/10/2018
		/// @param	big_endian	True to big endian.
		/// @return	A reference to a binary_metadata_impl.
		void set_big_endian(bool big_endian) override
		{
			m_big_endian = big_endian;
		}

		/// set Big endian to true or false.
		/// @date	03/10/2018
		/// @param	big_endian	True to big endian.
		/// @return	A reference to a binary_metadata_impl.
		binary_metadata_impl& big_endian(bool big_endian) 
		{
			set_big_endian(big_endian);
			return *this;
		}

		/// Puts a new node into the metadata parser
		/// @date	03/10/2018
		/// @param	name	The name.
		/// @param	size	The size.
		/// @param	type	(Optional) The type.
		/// @return	A reference to a binary_metadata_impl.
		bool put(const char* name, size_t size, type_enum type, const simple_options_data& options) override
		{
			if (name == nullptr)
				return false;

			m_parser_metadata.use<void>([&](parser_node_vector& parser_data)
			{
				std::string name_str(name);
				if (name_str.empty())
					name_str = std::to_string(parser_data.size());

				utils::ref_count_ptr<binary_node_interface> data =
					utils::make_ref_count_ptr<binary_node>(name_str.c_str(), m_offset, size, type, m_big_endian,options,nullptr);
				parser_data.emplace_back(data);
				m_offset += size;
			});
			return true;
		}

		bool put_enum(const char* name, size_t size, enum_data_interface* enum_type, const simple_options_data& options) override
		{
			if (name == nullptr)
				return false;

			m_parser_metadata.use<void>([&](parser_node_vector& parser_data)
			{
				std::string name_str(name);
				if (name_str.empty())
					name_str = std::to_string(parser_data.size());

				utils::ref_count_ptr<binary_node_interface> data =
					utils::make_ref_count_ptr<binary_node>(name_str.c_str(), m_offset, size, type_enum::ENUM, m_big_endian, options,enum_type);
				parser_data.emplace_back(data);
				m_offset += size;
			});
			return true;
		}


		bool put_bits(const char* name, size_t num_of_bits, size_t data_size) override
		{
			if (name == nullptr)
				return false;
			if (num_of_bits == 0)
				return false;
			if (data_size > sizeof(uint64_t))
				return false;

			std::string name_str(name);
			utils::ref_count_ptr<binary_node_interface> prev_node =
				m_parser_metadata.use<utils::ref_count_ptr<binary_node_interface>>
				([&](parser_node_vector& parser_data)
			{
				if (name_str.empty())
					name_str = std::to_string(parser_data.size());

				utils::ref_count_ptr<binary_node_interface> node;
				if (parser_data.size() > 0)
				{
					node = parser_data[parser_data.size() - 1];
					if (node->type() == type_enum::BITMAP &&
						parser_data.size() < sizeof(uint64_t))
					{
						return node;
					}
				}

				return utils::ref_count_ptr<binary_node_interface>();

			});

			add_bit_node(name_str.c_str(), num_of_bits, data_size, prev_node);

			return true;
		}
			
		bool put_string(const char* name,size_t max_size, const char* default_string) override
		{
			if (name == nullptr)
				return false;

			m_parser_metadata.use<void>([&](parser_node_vector& parser_data)
			{
				std::string name_str(name);
				if (name_str.empty())
					name_str = std::to_string(parser_data.size());

				utils::ref_count_ptr<binary_node_interface> data =
                    utils::make_ref_count_ptr<binary_node>(name_str.c_str(), m_offset, max_size, type_enum::STRING, m_big_endian, default_string, static_cast<size_t>(1));
				parser_data.emplace_back(data);
				m_offset += max_size;
			});
			return true;
		}

        bool nest_metadata(const char *name, binary_metadata_interface* nested_metadata) override
		{
			if (name == nullptr)
				return false;

			m_parser_metadata.use<void>([&](parser_node_vector& parser_data)
			{
				std::string name_str(name);
				if (name_str.empty())
					name_str = std::to_string(parser_data.size());

				parser_data.emplace_back(utils::make_ref_count_ptr<binary_node>
					(name_str.c_str(), m_offset, nested_metadata->size(), type_enum::COMPLEX, nested_metadata->big_endian(), nested_metadata));
				m_offset += nested_metadata->size();
			});

			return true;
		}

		/// Nests a complex metadata parser into the metadata
		/// @date	03/10/2018
		/// @param 		   	name		   	The name.
		/// @param [in]	nested_metadata	If non-null, the nested metadata.
		bool nest_metadata(const char *name, const char* metadata_name) override
		{
			if (m_store == nullptr)
				return false;
			utils::ref_count_ptr<binary_metadata_interface> metadata;
			if (false == m_store->query_parser_metadata(metadata_name, &metadata))
				return false;

			return nest_metadata(name, metadata);
		}

		/// Put an array type node into the metadata parser (simple or complex)
		/// @date	03/10/2018
		/// @param 		   	name		   	The name.
		/// @param 		   	size		   	The size.
		/// @param 		   	type		   	The type.
		/// @param 		   	num_of_elements	Number of elements.
		/// @param [in]	nested_parser  	(Optional) If non-null, the nested
		/// 	parser.
		/// @return	A reference to a binary_metadata_impl.
		bool array(const char *name, size_t size, type_enum type, size_t num_of_elements,simple_options_data options, binary_metadata_interface *nested_parser) override
		{
			if (name == nullptr)
				return false;
			utils::ref_count_ptr<binary_node> node;
			m_parser_metadata.use<void>([&](parser_node_vector& parser_data)
			{
				std::string name_str(name);
				if (name_str.empty())
					name_str = std::to_string(parser_data.size());

				node = utils::make_ref_count_ptr<binary_node>(name_str.c_str(), m_offset, size, ARRAY, m_big_endian, options,nullptr,num_of_elements);
				parser_data.emplace_back(node);
				m_offset += size*num_of_elements;
			});

			utils::ref_count_ptr<binary_metadata_impl> metadata = utils::make_ref_count_ptr<binary_metadata_impl>(m_store,big_endian(),m_parser_creator);
		
			std::stringstream str;
			str << name << "["<< num_of_elements <<"]";
			if (type == COMPLEX)
			{
				metadata->nest(str.str().c_str(), nested_parser);
			}
			else
			{
				metadata->put(str.str().c_str(), size, type, options);
			}
			node->m_nested_parser = metadata;

			return true;
		}

		//array of strings - need to be exported to the interface
		bool array(const char *name, size_t size, type_enum type, size_t num_of_elements, const char* default_string) 
		{
			if (name == nullptr)
				return false;
			utils::ref_count_ptr<binary_node> node;
			m_parser_metadata.use<void>([&](parser_node_vector& parser_data)
			{
				std::string name_str(name);
				if (name_str.empty())
					name_str = std::to_string(parser_data.size());

				node = utils::make_ref_count_ptr<binary_node>(name_str.c_str(), m_offset, size, ARRAY, m_big_endian, default_string,num_of_elements);
				parser_data.emplace_back(node);
				m_offset += size * num_of_elements;
			});

			utils::ref_count_ptr<binary_metadata_impl> metadata = utils::make_ref_count_ptr<binary_metadata_impl>(m_store, big_endian(), m_parser_creator);

			std::stringstream str;
			str << name << "[" << num_of_elements << "]";
			metadata->put_string(str.str().c_str(), size,default_string);
			node->m_nested_parser = metadata;
			return true;
		}

		/// Put an array type node into the metadata parser (complex only)
		/// @date	27/12/2018
		/// @param	name		   	The name of field.
		/// @param	num_of_elements	Number of elements.
		/// @param	metadata_name  	Name of the metadata to get from the store.
		/// @return	True if it succeeds, false if it fails to get the metadata from the store.
		bool array(const char *name, size_t num_of_elements, simple_options_data options, const char* metadata_name) override
		{
			if (m_store == nullptr)
				return false;
			utils::ref_count_ptr<binary_metadata_interface> metadata;
			if (false == m_store->query_parser_metadata(metadata_name, &metadata))
				return false;

			return array(name, metadata->size(), COMPLEX, num_of_elements, options ,metadata);
		}
		
		bool array(const char *name, size_t num_of_elements,size_t size, simple_options_data options, enum_data_interface* enum_type) override
		{
			if (name == nullptr)
				return false;
			
			if (enum_type == nullptr)
				return false;
			utils::ref_count_ptr<binary_node> node;
			m_parser_metadata.use<void>([&](parser_node_vector& parser_data)
			{
				node =	utils::make_ref_count_ptr<binary_node>(name, m_offset, size, ARRAY, m_big_endian, options,enum_type,num_of_elements);
				parser_data.emplace_back(node);
				m_offset += size*num_of_elements;
			});

			
			if (node != nullptr)
			{
				utils::ref_count_ptr<binary_metadata_impl> metadata = utils::make_ref_count_ptr<binary_metadata_impl>(m_store, big_endian(), m_parser_creator);
				std::stringstream str;
				str << name << "[" << num_of_elements <<"]";
				metadata->put_enum(str.str().c_str(), size, enum_type, options);
				node->m_nested_parser = metadata;
			}
			else
				return false;

			return true;
		}
		binary_metadata_impl& nest(const char *name, binary_metadata_interface *nested_metadata)
		{
			nest_metadata(name, nested_metadata);
			return *this;
		}
		/// Put a plain buffer type node into the metadata parser
		/// @date	03/10/2018
		/// @param	name	The name.
		/// @param	size	The size.
		/// @return	A reference to a binary_metadata_impl.
		binary_metadata_impl& buffer(const char *name, size_t size)
		{
			put(name, size, type_enum::BUFFER, utils::parsers::simple_options());
			return *this;
		}
	};

}
bool parsers::binary_metadata::create(bool big_endian, core::parsers::binary_metadata_store_interface* store, core::parsers::binary_parser_creator_interface* creator, core::parsers::binary_metadata_builder_interface** metadata)
{
	utils::ref_count_ptr<core::parsers::binary_metadata_store_interface> internal_store;
	if (store == nullptr)
	{//use the global store
		parsers::binary_metadata_store::instance(&internal_store);
	}
	else
		internal_store = store;

	utils::ref_count_ptr<core::parsers::binary_metadata_builder_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<parsers::binary_metadata_impl>(internal_store, big_endian, creator);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*metadata = instance;
	(*metadata)->add_ref();
	return true;
}
	
bool parsers::binary_metadata::create(const char* metadata_json, core::parsers::binary_metadata_store_interface* store , core::parsers::binary_parser_creator_interface* creator, core::parsers::binary_metadata_builder_interface** metadata)
{

	utils::ref_count_ptr<core::parsers::binary_metadata_store_interface> internal_store;
	if (store == nullptr)
	{//use the global store
		parsers::binary_metadata_store::instance(&internal_store);
	}
	else
		internal_store = store;

	utils::ref_count_ptr<core::parsers::binary_metadata_builder_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<parsers::binary_metadata_impl>(internal_store, metadata_json,creator);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*metadata = instance;
	(*metadata)->add_ref();
	return true;



}

	



