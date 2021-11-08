#pragma once

#include <parsers/binary_parser.h>


#include <utils/types.hpp>
#include <core/parser.h>

#include <utils/parser.hpp>
#include <Buffers.hpp>

#include <string>
#include <vector>
#include <cfloat> 
using TypeEnum = core::types::type_enum;

namespace Parsers
{
	using SimpleOptions = utils::parsers::simple_options;
	using EnumDataItem = core::parsers::enum_data_item;
	using JsonDetailsLevel = core::parsers::json_details_level;
	class BinaryParser;
	
	/// A binary meta data is helper class of BinaryParser that hold the schema of a data structure.
	/// @date	10/10/2018
	class BinaryMetaData : public Common::CoreObjectWrapper<core::parsers::binary_metadata_interface>
    {
	public:
		/// A binary node is an internal class that describe a single node of data in the data structure.
		/// @date	10/10/2018
		class BinaryNode : public Common::CoreObjectWrapper<core::parsers::binary_node_interface>
		{		
		public:
			BinaryNode()
			{
				// Empty
			}

			BinaryNode(core::parsers::binary_node_interface *node) :
				Common::CoreObjectWrapper<core::parsers::binary_node_interface>(node)
			{
			}

			const char* Name() const
			{
				return m_core_object->name();
			}

			size_t Offset() const
			{
				return m_core_object->offset();
			}

			size_t Size() const
			{
				return m_core_object->size();
			}

			TypeEnum Type() const
			{
				return m_core_object->type();
			}

			bool BigEndian() const
			{
				return m_core_object->big_endian();
			}

			SimpleOptions Options() const
			{
				return m_core_object->options();
			}

			BinaryMetaData Nested() const
			{
				utils::ref_count_ptr<core::parsers::binary_metadata_interface> meta_data;
				m_core_object->nested(&meta_data);
				return BinaryMetaData(meta_data);
			}

		};

		BinaryMetaData()
		{
			// Empty 
		}

		BinaryMetaData(core::parsers::binary_metadata_interface *metadata):
			Common::CoreObjectWrapper<core::parsers::binary_metadata_interface>(metadata)
		{			
		}
		const char* Namely()
		{
			ThrowOnEmpty("BinaryMetaData");
			return m_core_object->namely();
		}
		BinaryNode QueryNode(const char* name) const
		{
			ThrowOnEmpty("BinaryMetaData");

			utils::ref_count_ptr<core::parsers::binary_node_interface> node;
			if (m_core_object->query_node(name, &node) == false)
			{
				throw std::runtime_error("query node failed");
			}
			return BinaryNode(node);
		}

		BinaryNode QueryNode(size_t index) const
		{
			ThrowOnEmpty("BinaryMetaData");

			utils::ref_count_ptr<core::parsers::binary_node_interface> node;
			if (m_core_object->query_node_by_index(index, &node) == false)
			{
				throw std::runtime_error("query node failed");
			}
			return BinaryNode(node);
		}
		
		size_t NodeCount() const
		{
			ThrowOnEmpty("BinaryMetaData");

			return m_core_object->node_count();
		}
		
		size_t Size() const
		{
			ThrowOnEmpty("BinaryMetaData");

			return m_core_object->size();
		}

		bool BigEndian() const
		{
			ThrowOnEmpty("BinaryMetaData");

			return m_core_object->big_endian();
		}

		BinaryParser CreateParser() const;

		const char* ToJson(bool compact = true)
		{
			ThrowOnEmpty("BinaryMetaData");

			return m_core_object->to_json(compact);
		}
	};

	class EnumData : public Common::CoreObjectWrapper<core::parsers::enum_data_interface>
	{
	public:
		EnumData() {}

		EnumData(core::parsers::enum_data_interface* enum_data) :
			CoreObjectWrapper<core::parsers::enum_data_interface>(enum_data) {}

	public:

		bool operator==(const EnumData& rhs) 
		{
			if (Empty() && rhs.Empty())
				return true;
			else if (Empty())
				return false;
			else if (rhs.Empty())
				return false;

			return Equals(rhs.Name());
		}

		bool Equals(const char* name)
		{
			ThrowOnEmpty("EnumData::Equals");
			return strcmp(m_core_object->name(), name) == 0;
		}

		size_t Size()
		{
			ThrowOnEmpty("EnumData::Size");
			return m_core_object->size();
		}

		const char* Name() const
		{
			ThrowOnEmpty("EnumData::Name");
			return m_core_object->name();
		};

		EnumData& AddNewItem(int64_t val, const char* name)
		{
			ThrowOnEmpty("EnumData::AddNewItem");
			if (false == m_core_object->add_item(name, val))
				throw std::runtime_error("add_item");
			return *this;

		};

		bool GetItemValueByName(const char* name, int64_t& val)
		{
			ThrowOnEmpty("EnumData::GetItemValueByName");
			return m_core_object->val_by_name(name, val);
		};

		const char* GetItemNameByValue(int64_t val)
		{
			ThrowOnEmpty("EnumData::GetItemNameByValue");
			return m_core_object->name_by_val(val);
		};

		bool ItemByName(const char* name, EnumDataItem& item)
		{
			ThrowOnEmpty("EnumData::ItemByName");
			return m_core_object->item_by_name(name, item);
		}

		bool ItemByVal(int64_t val, EnumDataItem& item)
		{
			ThrowOnEmpty("EnumData::ItemByName");
			return m_core_object->item_by_val(val, item);
		}

		bool ItemByIndex(size_t index, EnumDataItem& item)
		{
			ThrowOnEmpty("EnumData::ItemByName");
			return m_core_object->item_by_index(index, item);
		}
		const char* ToJson(bool compact = true) 
		{
			ThrowOnEmpty("EnumData::ToJson");
			return m_core_object->to_json(compact);
		};

		bool FromJson(const char* json) 
		{
			ThrowOnEmpty("EnumData::FromJson");
			return m_core_object->from_json(json);
		};
	};

	class EnumDataFactory : public Common::NonConstructible
	{
	public:

		static EnumData Create(const char* name)
		{
			utils::ref_count_ptr<core::parsers::enum_data_interface> instance;
			if (parsers::enum_data::create(name, &instance) == false)
				throw std::runtime_error("Failed to create EnumData");

			return EnumData(instance);
		}
	};
	
	/// A binary metadata store that allow automatic store of already created metadata.
	/// Add instace of a store class to the parser metadata and you can 
	/// @date	27/12/2018
	class BinaryMetadataStore : public  Common::CoreObjectWrapper<core::parsers::binary_metadata_store_interface>
	{
	public:
		BinaryMetadataStore()
		{
			parsers::binary_metadata_store::instance(&m_core_object);
		}

		BinaryMetadataStore(core::parsers::binary_metadata_store_interface *store):
			Common::CoreObjectWrapper<core::parsers::binary_metadata_store_interface>(store)
		{
			
		}
		/// Creates a new BinaryMetadataStore instance
		/// This can be used to generate another set of parsers
		/// @date	25/07/2019
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @return	A BinaryMetadataStore.
		static BinaryMetadataStore Create()
		{
			utils::ref_count_ptr<core::parsers::binary_metadata_store_interface> instance;
			if (parsers::binary_metadata_store::create(&instance) == false)
				throw std::runtime_error("Failed to create Binary store");

			return BinaryMetadataStore(instance);
		}

		BinaryMetaData Metadata(const char* name)
		{
			ThrowOnEmpty("BinaryMetadataStore");
			if (name == nullptr)
				throw std::invalid_argument("name");

			utils::ref_count_ptr<core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_parser_metadata(name, &metadata))
				return BinaryMetaData(); //Empty

			return BinaryMetaData(metadata);
		}

		template <typename T>
		BinaryMetaData Metadata()
		{
			std::string name = utils::types::get_type_name<T>();

			return Metadata(name.c_str());
		}

		bool SetMetadata(const char* name, BinaryMetaData& metadata)
		{
			ThrowOnEmpty("BinaryMetadataStore");
			
			if (name == nullptr)
				throw std::invalid_argument("name");

			if (metadata.Empty())
				throw std::invalid_argument("metadata");

			return m_core_object->add_parser_metadata(name, static_cast<core::parsers::binary_metadata_interface*>(metadata));
		}

		EnumData Enum(const char* name)
		{
			ThrowOnEmpty("EnumDataStore");
			if (name == nullptr)
				throw std::invalid_argument("name");
		
			utils::ref_count_ptr<core::parsers::enum_data_interface> enum_data;
			if (false == m_core_object->query_enum(name, &enum_data))
				return EnumData();
		
			return EnumData(enum_data);
		}

		template<typename T>
		EnumData Enum()
		{
			static_assert(std::is_enum<T>::value == true, "Enum require only enum types");
			std::string enumName;
			enumName = utils::types::get_type_name<T>();
			return Enum(enumName.c_str());
		}
		bool SetEnum(const char* name, EnumData& metadata)
		{
			ThrowOnEmpty("SetEnumDataStore");
		
			if (name == nullptr)
				throw std::invalid_argument("name");
		
			if (metadata.Empty())
				throw std::invalid_argument("metadata");
		
			return m_core_object->add_enum(name, static_cast<core::parsers::enum_data_interface*>(metadata));
		}
	};

	/// A binary meta data builder extend BinaryMetaData with functions that allow creating the metadata structure.
	/// @date	10/10/2018
	class BinaryMetaDataBuilder : public BinaryMetaData
	{
	private:
		core::parsers::binary_metadata_builder_interface *metadata()
		{
			return (core::parsers::binary_metadata_builder_interface *)static_cast<core::parsers::binary_metadata_interface*>(*this);
		}

	public:
		BinaryMetaDataBuilder()
		{
			//Empty
		}

		BinaryMetaDataBuilder(core::parsers::binary_metadata_builder_interface* instance):
			BinaryMetaData(instance)
		{
		}

		BinaryMetaDataBuilder(const BinaryMetaData& metadata) :
			BinaryMetaData(static_cast<core::parsers::binary_metadata_interface*>(metadata))
		{
		}

		static BinaryMetaDataBuilder Create(BinaryMetadataStore& store, bool bigEndian = false)
		{
			utils::ref_count_ptr<core::parsers::binary_metadata_builder_interface> instance;
			utils::ref_count_ptr<core::parsers::binary_parser_creator_interface> parser_creator;
			parser_creator = utils::make_ref_count_ptr<utils::parsers::binary_parser_creator>();
			if (parsers::binary_metadata::create(bigEndian, 
				static_cast<core::parsers::binary_metadata_store_interface*>(store),
				parser_creator,
				&instance) == false)
				throw std::runtime_error("Failed to create Binary Parser");

			return BinaryMetaDataBuilder(instance);
		}

		static BinaryMetaDataBuilder Create(bool bigEndian = false)
		{
			utils::ref_count_ptr<core::parsers::binary_metadata_builder_interface> instance;
			
			utils::ref_count_ptr<core::parsers::binary_parser_creator_interface> parser_creator;
			parser_creator = utils::make_ref_count_ptr<utils::parsers::binary_parser_creator>();

			if (parsers::binary_metadata::create(bigEndian, nullptr, parser_creator, &instance) == false)
				throw std::runtime_error("Failed to create Binary Parser");

			return BinaryMetaDataBuilder(instance);
		}

		static BinaryMetaDataBuilder Create(const char* metadataJson)
		{
			utils::ref_count_ptr<core::parsers::binary_metadata_builder_interface> instance;
			
			utils::ref_count_ptr<core::parsers::binary_parser_creator_interface> parser_creator;
			parser_creator = utils::make_ref_count_ptr<utils::parsers::binary_parser_creator>();

			if (parsers::binary_metadata::create(metadataJson,nullptr, parser_creator, &instance) == false)
				throw std::runtime_error("Failed to create Binary Parser");

			return BinaryMetaDataBuilder(instance);
		}
		
		static BinaryMetaDataBuilder Create(const char* metadataJson,BinaryMetadataStore& store)
		{
			utils::ref_count_ptr<core::parsers::binary_metadata_builder_interface> instance;

			utils::ref_count_ptr<core::parsers::binary_parser_creator_interface> parser_creator;
			parser_creator = utils::make_ref_count_ptr<utils::parsers::binary_parser_creator>();

			if (parsers::binary_metadata::create(metadataJson, 
				static_cast<core::parsers::binary_metadata_store_interface*>(store), parser_creator, &instance) == false)
				throw std::runtime_error("Failed to create Binary Parser");

			return BinaryMetaDataBuilder(instance);
		}

		bool FromJson(const char* json)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			return metadata()->from_json(json);
		}

		BinaryMetaDataBuilder& Namely(const char* name)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			if (false == metadata()->namely(name))
				throw std::runtime_error("metadata already exist");

			return *this;
		}
		
		BinaryMetaDataBuilder& Simple(const char* name, size_t size, TypeEnum type,const SimpleOptions& options)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			if (type == core::types::type_enum::COMPLEX)
				throw std::invalid_argument("complex type not allowed on simple");

			metadata()->put(name, size, type, options);

			return *this;
		}

		BinaryMetaDataBuilder& Simple(const char* name, size_t size, TypeEnum type)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			
			if (name == nullptr)
				throw std::invalid_argument("name");

			if (type == core::types::type_enum::COMPLEX)
				throw std::invalid_argument("complex type not allowed on simple");

			metadata()->put(name, size, type, SimpleOptions());

			return *this;
		}

		template <typename T>
		BinaryMetaDataBuilder& Simple(const char* name)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			TypeEnum type;
			type = utils::types::get_type<T>();

			return Simple(name, sizeof(T), type);

		}

		template <typename T>
		BinaryMetaDataBuilder& Simple(const char* name, const SimpleOptions& options)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			TypeEnum type;
			type = utils::types::get_type<T>();

			return Simple(name, sizeof(T), type,options);

		}

		BinaryMetaDataBuilder& Enum(const char* name, size_t size,EnumData& enumData,  const SimpleOptions& options)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			if (name == nullptr)
				throw std::invalid_argument("name");
			
			if (false == metadata()->put_enum(name, 
				size, 
				static_cast<core::parsers::enum_data_interface*>(enumData),
				options))
				throw std::runtime_error("put_enum");

			return *this;
		}

		BinaryMetaDataBuilder& Enum(const char* name, size_t size, EnumData& enumData)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			if (name == nullptr)
				throw std::invalid_argument("name");

			if (enumData.Empty())
				throw std::invalid_argument("enumData");
			SimpleOptions options;
			if (false == metadata()->put_enum(name,
				size,
				static_cast<core::parsers::enum_data_interface*>(enumData),
				options))
				throw std::runtime_error("put_enum");

			return *this;
		}

		BinaryMetaDataBuilder& Enum(const char* name, size_t size, const char* enumDataName, const SimpleOptions& options)
		{
			if (enumDataName == nullptr)
				throw std::invalid_argument("enumDataName");
			BinaryMetadataStore store;
			EnumData enumData = store.Enum(enumDataName);

			return Enum(name,size,enumData, options);
		}


		template<typename T>
		BinaryMetaDataBuilder& Enum(const char* name, const char* enumDataName,  const SimpleOptions& options)
		{
		    if(enumDataName == nullptr)
				throw std::invalid_argument("enumDataName");
			BinaryMetadataStore store;
			EnumData enumData  = store.Enum(enumDataName);
			
			return Enum<T>(name, enumData, options);
		}

		template<typename T>
		BinaryMetaDataBuilder& Enum(const char* name, EnumData& enumData)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			TypeEnum type = utils::types::get_type<T>();
			
			if (type != TypeEnum::ENUM)
				throw std::invalid_argument("type");

			Enum(name, sizeof(T), enumData);

			return *this;
		}

		template<typename T>
		BinaryMetaDataBuilder& Enum(const char* name, EnumData& enumData, const SimpleOptions& options)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			TypeEnum type = utils::types::get_type<T>();

			if (type != TypeEnum::ENUM)
				throw std::invalid_argument("type");

			Enum(name, sizeof(T), enumData,options);

			return *this;
		}

		BinaryMetaDataBuilder& Complex(const char* name, const char* metaDataName)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			utils::ref_count_ptr<core::parsers::binary_metadata_store_interface> store;

			if (false == metadata()->nest_metadata(name, metaDataName))
				throw std::invalid_argument("nest_metadata");

			return *this;
		}

		BinaryMetaDataBuilder& Complex(const char* name, const BinaryMetaData &metaData)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			utils::ref_count_ptr<core::parsers::binary_metadata_store_interface> store;
			
			metadata()->nest_metadata(name, static_cast<core::parsers::binary_metadata_interface*>(metaData));

			return *this;
		}

		BinaryMetaDataBuilder& Buffer(const char* name, size_t size)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			
			metadata()->put(name, size, TypeEnum::BUFFER, SimpleOptions());

			return *this;
		}

		BinaryMetaDataBuilder& String(const char* name, size_t maxSize)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			
			metadata()->put_string(name,maxSize,"");

			return *this;
		}

		BinaryMetaDataBuilder& String(const char* name,size_t maxSize, const std::string& defaultString)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			
			if (false == defaultString.empty())
			{
				if (defaultString.size() > maxSize)
					throw std::runtime_error("BinaryMetaDataBuilder String()  - defaultString > maxSize");
			}

			metadata()->put_string(name, maxSize,defaultString.c_str());
			return *this;
		}

		template <typename T>
		BinaryMetaDataBuilder& Array(const char* name, size_t length)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			core::types::type_enum type;
			type = utils::types::get_type<T>();

			if (false == utils::types::is_simple_type(type))
				throw std::invalid_argument("complex/buffer/string type not supported");

			SimpleOptions options;
			metadata()->array(name, sizeof(T), type, length, options,nullptr);

			return *this;

		}

		template <typename T>
		BinaryMetaDataBuilder& Array(const char* name, size_t length,  const SimpleOptions& options)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			core::types::type_enum type;
			type = utils::types::get_type<T>();
			if (type == core::types::type_enum::ENUM)
			{
				std::string enumName = utils::types::get_type_name<T>();

				return Array(name, length, sizeof(T), options, enumName.c_str());
			}

			if (false == utils::types::is_simple_type(type))
				throw std::invalid_argument("complex/buffer/string type not supported");


			metadata()->array(name, sizeof(T), type, length,options, nullptr);

			return *this;

		}
		BinaryMetaDataBuilder& Array(const char* name,TypeEnum type, size_t length, const SimpleOptions& options)
		{
			ThrowOnEmpty("BinaryMeytaDataBuilder");

			if (false == utils::types::is_simple_type(type))
				throw std::invalid_argument("complex/buffer/string type not supported");

			size_t size;
			size = utils::types::sizeof_type(type);

			metadata()->array(name, size, type, length, options, nullptr);

			return *this;

		}
		BinaryMetaDataBuilder& Array(const char* name, size_t length, size_t size, const SimpleOptions& options, EnumData& enumData)
		{
			ThrowOnEmpty("BinaryMeytaDataBuilder");
			BinaryMetadataStore store;
					
			metadata()->array(name,length,size, options,
				static_cast<core::parsers::enum_data_interface*>(enumData));

			return *this;

		}

		BinaryMetaDataBuilder& Array(const char* name, size_t length, size_t size, const SimpleOptions& options, const char* enumName)
		{
			ThrowOnEmpty("BinaryMeytaDataBuilder");
			BinaryMetadataStore store;
		
			EnumData enumData = store.Enum(enumName);
			if (enumData.Empty())
				throw std::invalid_argument("enumName");

			Array(name, length, size, options, enumData);

			return *this;

		}

		BinaryMetaDataBuilder& Array(const char* name, TypeEnum type, size_t length)
		{
			ThrowOnEmpty("BinaryMeytaDataBuilder");

			if (type == core::types::type_enum::COMPLEX ||
				type == core::types::type_enum::STRING ||
				type == core::types::type_enum::BUFFER)
				throw std::invalid_argument("complex/buffer/string type not supported");
			size_t size;
			size = utils::types::sizeof_type(type);
			
			metadata()->array(name, size, type, length, SimpleOptions(), nullptr);

			return *this;

		}
		BinaryMetaDataBuilder& Array(const char* name, TypeEnum type,size_t typeSize, size_t length)
		{
			ThrowOnEmpty("BinaryMeytaDataBuilder");

			if (type == core::types::type_enum::COMPLEX ||
				type == core::types::type_enum::STRING ||
				type == core::types::type_enum::BUFFER)
				throw std::invalid_argument("complex/buffer/string type not supported");
			

			metadata()->array(name, typeSize, type, length, SimpleOptions(), nullptr);

			return *this;

		}

		BinaryMetaDataBuilder& Array(const char* name, size_t length, BinaryMetaData MetaData)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			
			metadata()->array(name, MetaData.Size(), TypeEnum::COMPLEX, length, SimpleOptions(), static_cast<core::parsers::binary_metadata_interface*>(MetaData));
			return *this;

		}

		BinaryMetaDataBuilder& Array(const char* name, size_t length, const char* metadata_name)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			
			if (false == metadata()->array(name, length,SimpleOptions(), metadata_name))
				throw std::invalid_argument("metadata_name");

			return *this;

		}

		BinaryMetaDataBuilder& Bits(const char* name, size_t numOfBits,size_t size)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			if (false == metadata()->put_bits(name, numOfBits, size))
				throw std::runtime_error("put_bits");

			return *this;
		}

		template <typename T>
		BinaryMetaDataBuilder& Bits(const char* name, size_t numOfBits)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");
			
			static_assert(sizeof(T) < sizeof(uint64_t), "Bitmap exceeded uint64_t size");
			
			if (false == metadata()->put_bits(name, numOfBits,sizeof(T)))
				throw std::runtime_error("put_bits");

			return *this;
		}

		BinaryMetaDataBuilder& BigEndian(bool Big)
		{
			ThrowOnEmpty("BinaryMetaDataBuilder");

			metadata()->set_big_endian(Big);

			return *this;
		}
	};

	template<typename T>
	using GetAtFunc = std::function<bool(size_t, T&)>;
	
	static constexpr size_t UNDEFINED_ITERATOR_INDEX = (std::numeric_limits<size_t>::max)();
	// Common Iterator for XML Attributes or Elements
	template <typename T>
	class ArrayIterator
	{
	private:
		GetAtFunc<T> m_getAtFunc;
		size_t m_index;
		T m_value;

		void set_value()
		{
			T value;
			if (false == m_getAtFunc(m_index, value))
			{
				m_index = UNDEFINED_ITERATOR_INDEX;
			}
			else
			{
				m_value = value;
			}
		}

	public:
		ArrayIterator(const GetAtFunc<T>& getAtFunc, size_t index)
			: m_getAtFunc(getAtFunc),
			m_index(index)
		{
			set_value();
		}
		ArrayIterator():
			m_index(UNDEFINED_ITERATOR_INDEX)
		{
			
		}
		// prefix increment
		ArrayIterator operator++()
		{
			ArrayIterator retval = *this;
			m_index++;
			set_value();

			return retval;
		}

		// postfix increment
		ArrayIterator operator++(int)
		{
			m_index++;
			set_value();

			return *this;
		}

		T& operator*()
		{
			return m_value;
		}

		T* operator->()
		{
			return &m_value;
		}

		bool operator==(const ArrayIterator& rhs)
		{
			return (m_index == rhs.m_index);
		}

		bool operator!=(const ArrayIterator& rhs)
		{
			return !(m_index == rhs.m_index);
		}
	};

	// Object range class to support modern for-each loop with XML Attributes/Elements Iterators
	template <typename T>
	class ParserArray
	{
	private:
		
		GetAtFunc<T> m_getAtFunc;
		size_t m_length;
	public:
		ParserArray()
		{
			//Empty constructor
		}

		ParserArray(ParserArray &&other) :
			m_getAtFunc(std::move(other.m_getAtFunc)),
			m_length(std::move(other.m_length))
		{

		}

		ParserArray(const GetAtFunc<T>& getAtFunc, size_t length)
			: m_getAtFunc(getAtFunc),
			m_length(length)
		{
		}

		ParserArray& operator=(ParserArray&& other)
		{
			m_getAtFunc = std::move(other.m_getAtFunc);
			return *this;
		}

		ArrayIterator<T> begin()
		{
			if (m_getAtFunc == nullptr)
				throw std::runtime_error("Empty XmlObjectRange");

			return ArrayIterator<T>(m_getAtFunc, 0);
		}

		ArrayIterator<T> end()
		{
			if (m_getAtFunc == nullptr)
				throw std::runtime_error("Empty XmlObjectRange");

			return ArrayIterator<T>();
		}

		ArrayIterator<T> operator[](size_t index)
		{
			if (m_getAtFunc == nullptr)
				throw std::runtime_error("Empty Array");

			T data;

			if(false == m_getAtFunc(index,data))
				throw std::runtime_error("index out of range");

			return ArrayIterator<T>(m_getAtFunc, index);
		}

		bool Empty()
		{
			if (begin() == end())
				return true;
			return false;
		}

		size_t Length()
		{
			return m_length;
		}
	};
	
	/// A binary parser is a class that aim to help creating data structures at run time and manage to parse binary buffer 
	/// look At SimpleParser sample how to use it
	/// @date	10/10/2018
	class BinaryParser : public Common::CoreObjectWrapper<core::parsers::binary_parser_interface>
	{
	public:
		BinaryParser()
		{
		}

		BinaryParser(core::parsers::binary_parser_interface* parser) :
			CoreObjectWrapper<core::parsers::binary_parser_interface>(parser)
		{
		}

		/// Parses provided buffer according the metadata
		/// @date	03/10/2018
		/// @param	data	The data.
		/// @param	size	The data size.
		/// @return	True if it succeeds, false if it fails.
		bool Parse(const void* data, size_t size)
		{
			ThrowOnEmpty("BinaryParser");
			return m_core_object->parse(data, size);
		}
		/// Parse from string - parse an hex string into a buffer and call parse
		/// @date	03/10/2018
		/// @param	data	The data.
		/// @return	True if it succeeds, false if it fails.
		bool ParseFromString(const char* data)
		{
			ThrowOnEmpty("BinaryParser");
			return m_core_object->parse_from_string(data);
		}

		Buffers::SafeBuffer ParserBuffer()
		{
			utils::ref_count_ptr<core::safe_buffer_interface> buffer;
			ThrowOnEmpty("BinaryParser");
			if (m_core_object->query_buffer(&buffer))
				return Buffers::SafeBuffer(buffer);
			else
				return Buffers::SafeBuffer();

		}

		Buffers::Buffer Buffer()
		{
			utils::ref_count_ptr<core::safe_buffer_interface> safe_buffer;

			ThrowOnEmpty("BinaryParser");
			
			if (m_core_object->query_buffer(&safe_buffer))
			{
				Buffers::Buffer buffer (utils::make_ref_count_ptr<utils::ref_count_buffer>(safe_buffer->size()));
				if (safe_buffer->safe_read(buffer.Data(), buffer.Size(), 0))
					return buffer;
			}

			return Buffers::Buffer();

		}

		size_t BufferSize()
		{
			ThrowOnEmpty("Binary Parser");

			return m_core_object->buffer_size();
		}
		/// Converts this object to a JSON
		/// @date	03/10/2018
		/// @return	This object as a const char*.
		const char* ToJson(Parsers::JsonDetailsLevel detailsLevel = JsonDetailsLevel::JSON_ENUM_VALUES, bool compact = true) const
		{
			ThrowOnEmpty("BinaryParser");
			
			return m_core_object->to_json(detailsLevel, compact);
			
		}

		/// Check and get JSON
		/// @date	05/08/2020
		/// @param [in,out]	jsonStr	The JSON string.
		/// @param 		   	compact	(Optional) True to compact.
		/// @return	True if it succeeds, false if it fails.
		bool TryGetJson(std::string& jsonStr, JsonDetailsLevel detailsLevel = JsonDetailsLevel::JSON_ENUM_VALUES, bool compact = true) const
		{
			ThrowOnEmpty("BinaryParser");
			bool no_errors = true;
			
			jsonStr =  m_core_object->check_and_get_json(detailsLevel, compact, no_errors);
			return no_errors;
		}

		/// Initializes this object from the given from JSON
		/// the function will allow creating an object from json if fields are missing or there are extra fields
		/// @date	03/10/2018
		/// @param	json		The JSON.
		/// @param	validate	(Optional) True to validate the object fields.
		/// @return	True if it succeeds, false if it fails.
		bool FromJson(const char *json) 
		{			
			ThrowOnEmpty("BinaryParser");
			if (json == nullptr)
				throw std::invalid_argument("json");

			return m_core_object->from_json(json);
		}

		bool Validate(const char* fieldName) const
		{
			ThrowOnEmpty("BinaryParser");
			return m_core_object->validate(fieldName);
		}

		bool Validate(size_t index) const
		{
			ThrowOnEmpty("BinaryParser");
			return m_core_object->validate(index);
		}

		bool Validate() const
		{
			ThrowOnEmpty("BinaryParser");
			return m_core_object->validate();
		}
		size_t Count() const 
		{
			ThrowOnEmpty("BinaryParser");
			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("query_metadata");

			return metadata->node_count();
		}
		const char* FieldName(size_t index) const 
		{
			ThrowOnEmpty("BinaryParser");
			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("query_metadata");

			utils::ref_count_ptr <core::parsers::binary_node_interface> node;
			if(false == metadata->query_node_by_index(index,&node))
				throw std::invalid_argument("index");

			return node->name();
		}

		SimpleOptions FieldOptions(const char *fieldName)
		{
			ThrowOnEmpty("BinaryParser");
			
			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (m_core_object->query_metadata(&metadata))
			{
				utils::ref_count_ptr < core::parsers::binary_node_interface> node;
				metadata->query_node(fieldName, &node);
				return SimpleOptions(node->options());
			}

			return SimpleOptions();
		}

		/// Resets this object to its default values
		/// @date	05/09/2019
		void  Reset()
		{
			ThrowOnEmpty("BinaryParser");
			return m_core_object->set_default_values();
		}

		/// Resets the field described by fieldName to its default value
		/// @date	05/09/2019
		/// @param [in,out]	fieldName	If non-null, name of the field.
		void  ResetField(const char* fieldName)
		{
			ThrowOnEmpty("BinaryParser");
			return m_core_object->set_default_value(fieldName);
		}

		/// Reads a simple type from the parser
		/// @date	10/10/2018
		/// @exception	std::invalid_argument	Thrown when an invalid argument
		/// 	error condition occurs.
		/// @tparam	T	Generic type parameter.
		/// @param	name	The name of the field.
		/// @return	The the value.
		template <typename T>
		T Read(const char* name) const
		{
			ThrowOnEmpty("BinaryParser");
			T data;
			TypeEnum type;
			if(false == m_core_object->read_simple(name, (uint8_t*)&data, sizeof(T), type))
				throw std::runtime_error("read_simple by name");

			return data;			
		}

		/// eads a simple type from the parser
		/// @date	10/10/2018
		/// @exception	std::invalid_argument	Thrown when an invalid argument
		/// 	error condition occurs.
		/// @tparam	T	Generic type parameter.
		/// @param	index	Zero-based index of the.
		/// @return	The the value.
		template <typename T>
		T Read(size_t index) const
		{
			ThrowOnEmpty("BinaryParser");
			T data;
            TypeEnum type;
			if(false == m_core_object->read_simple(index, (uint8_t*)&data, sizeof(T), type))
				throw std::runtime_error("read_simple by index");

			return data;
		}
		
		/// Reads a complex parser from the parser (e.g. struct)
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name.
		/// @return	The a BinaryParser for the specific field.
		BinaryParser ReadComplex(const char* name) const
		{
			utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
			if (false == m_core_object->read_complex(name, &parser))
				throw std::runtime_error("read_complex failed");

			return BinaryParser(parser);
		}

		/// Reads a complex parser from the parser (e.g. struct)
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	index	Zero-based index of the.
		/// @return	The a BinaryParser for the specific field.
		BinaryParser ReadComplex(size_t index) const
		{
			utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
			if (false == m_core_object->read_complex(index, &parser))
				throw std::runtime_error("read_complex failed");

			return BinaryParser(parser);
		}

		/// Reads a buffer data from the parser 
		/// @date	10/10/2018
		/// @exception	std::runtime_error   	Raised when a runtime error
		/// 	condition occurs.
		/// @exception	std::invalid_argument	Thrown when an invalid argument
		/// 	error condition occurs.
		/// @param 		   	name	The name of the field.
		/// @param [out]	data	If non-null, the data red.
		/// @param 		   	size	The size of the allocated buffer.
		/// @return	A size_t the size of the data red..
		size_t Read(const char* name, void* data, size_t size) const
		{
			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("error query node's metadata");

			utils::ref_count_ptr<core::parsers::binary_node_interface> node;
			if (false == metadata->query_node(name, &node))
				throw std::runtime_error("error query node");

			if (data == nullptr)
			{//if data is null return the size of the buffer
				return node->size();
			}

			if (size < node->size())
				throw std::invalid_argument("size to small");

			TypeEnum type;
			if (false == m_core_object->read_by_node(data, node->size(), node, type))
				throw std::runtime_error("error on read");

			return node->size();
		}

		/// Reads a buffer data from the parser
		/// @date	10/10/2018
		/// @exception	std::runtime_error   	Raised when a runtime error
		/// 	condition occurs.
		/// @exception	std::invalid_argument	Thrown when an invalid argument
		/// 	error condition occurs.
		/// @param 			index	Zero-based index of the filed.
		/// @param [out]	data 	If non-null, the data.
		/// @param 			size 	The size of the allocated buffer.
		/// @return	A size_t the size of the data red.
		size_t Read(size_t index, void* data, size_t size) const
		{
			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("error query node's metadata");

			utils::ref_count_ptr<core::parsers::binary_node_interface> node;
			if (false == metadata->query_node_by_index(index, &node))
				throw std::runtime_error("error query node");

			if (data == nullptr)
			{//if data is null return the size of the buffer
				return node->size();
			}

			if (size < node->size())
				throw std::invalid_argument("size to small");

			TypeEnum type;
			if (false == m_core_object->read_by_node(data, node->size(), node, type))
				throw std::runtime_error("error on read");

			return node->size();
		}

		/// Reads a string from the parser
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	index	Zero-based index of the field.
		/// @return	The string.
		std::string ReadString(size_t index) const
		{
			ThrowOnEmpty("BinaryParser");
			char str[BUFF_MAX_SIZE];

			Read(index, (void*)str, sizeof(str));

			return str;
		}

		/// Reads a string from the parser
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name of the field.
		/// @return	The string.
		std::string ReadString(const char* name) const
		{
			ThrowOnEmpty("BinaryParser");
			char str[BUFF_MAX_SIZE];
			size_t size = sizeof(str);

			Read(name, (void*)str, size);

			return str;
		}
		/// Reads a buffer data from the parser by the filed name
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name of the field to read.
		/// @return	A std::vector of the buffer red
		std::vector<uint8_t> Read(const char* name) const
		{
			std::vector<uint8_t> buffer;
			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("error query node's metadata");

			utils::ref_count_ptr<core::parsers::binary_node_interface> node;
			if (false == metadata->query_node(name,&node))
				throw std::runtime_error("error query node");

			buffer.resize(node->size());
			TypeEnum type;
			if (false == m_core_object->read_by_node(buffer.data(), buffer.size(), node, type))
				throw std::runtime_error("error on read");

			return buffer;
		}

		/// Reads a buffer data from the parser by the filed index
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	index	The index to read.
		/// @return	A std::vector&lt;uint8_t&gt;
		std::vector<uint8_t> Read(size_t index) const
		{
			std::vector<uint8_t> buffer;
			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("error query node's metadata");

			utils::ref_count_ptr<core::parsers::binary_node_interface> node;
			if (false == metadata->query_node_by_index(index, &node))
				throw std::runtime_error("error query node");

			buffer.resize(node->size());
			TypeEnum type;
			if (false == m_core_object->read_by_node(buffer.data(), buffer.size(), node,type))
				throw std::runtime_error("error on read");

			return buffer;
		}

		/// Reads array at a specific index 
		/// this function signature only deals with simple data
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	  	The name of the array filed.
		/// @param	arrayIndex	Zero-based index of the array element.
        /// @return	The the data from array node at the specific index.
        template<typename T>
        typename std::enable_if<!std::is_same<T, BinaryParser>::value, T>::type
        ReadArrayAt(const char* name, size_t arrayIndex) const
        {
            T data;
            TypeEnum type;
            if (false == m_core_object->read_at(name, (uint8_t*)&data, sizeof(T), type, arrayIndex))
                throw std::runtime_error("read_at runtime error");

			return data;
		}

		/// Reads array at a specific index this function signature only deals
		/// with simple data
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @tparam	T				Generic type parameter.
		/// @param	index	  	index of the field of the array filed.
		/// @param	arrayIndex	Zero-based index of the array element.
		/// @return	The the data from array node at the specific index.
        template<typename T>
        typename std::enable_if<!std::is_same<T, BinaryParser>::value, T>::type
        ReadArrayAt(size_t index, size_t arrayIndex) const
		{
			T data;
			TypeEnum type;
			if (false == m_core_object->read_at(index, (uint8_t*)&data, sizeof(T), type, arrayIndex))
				throw std::runtime_error("read_at runtime error");

			return data;
		}
		
		/// Reads array at a specific index 
		/// this function signature only deals with complex data
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	  	The name of the array filed.
		/// @param	arrayIndex	Zero-based index of the array element.
		/// @return	The the data from array node at the specific index.
        template<typename T = BinaryParser>
        typename std::enable_if<std::is_same<T, BinaryParser>::value, BinaryParser>::type
        ReadArrayAt(const char* name, size_t arrayIndex) const
		{
			utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
			if (false == m_core_object->read_complex_at(name, &parser, arrayIndex))
				return BinaryParser();

			return BinaryParser(parser);
		}

		/// Reads complex array at this function wrap the ReadArryAt for complex types (to aoivd <>)
		/// @date	10/10/2018
		/// @param	name	  	The name.
		/// @param	arrayIndex	Zero-based index of the array.
		/// @return	The complex array at.
		BinaryParser ReadComplexArrayAt(const char* name, size_t arrayIndex)
		{
			return ReadArrayAt<>(name, arrayIndex);
		}

		/// Reads array at a specific index this function signature only deals
		/// with Complex data
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @tparam	T				Generic type parameter.
		/// @param	index	  	index of the field of the array filed.
		/// @param	arrayIndex	Zero-based index of the array element.
        /// @return	The the data from array node at the specific index.
        template<typename T = BinaryParser>
        typename std::enable_if<std::is_same<T, BinaryParser>::value, BinaryParser>::type
        ReadArrayAt(size_t index, size_t arrayIndex) const
		{
			utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
			if (false == m_core_object->read_complex_at(index, &parser, arrayIndex))
				throw std::runtime_error("read_complex_at runtime error");
            return BinaryParser(parser);
		}

		/// Reads complex array at this function wrap the ReadArryAt for complex types (to avoid <>)
		/// @date	10/10/2018
		/// @param	index	  	Zero-based index of the.
		/// @param	arrayIndex	Zero-based index of the array.
		/// @return	The complex element array at the arrayIndex.
		BinaryParser ReadComplexArrayAt(size_t index, size_t arrayIndex) const
		{
			return ReadArrayAt<>(index, arrayIndex);
		}

		size_t ArraySize(size_t index)
		{
			ThrowOnEmpty("Binary Parser");

			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("no metadata");

			utils::ref_count_ptr < core::parsers::binary_node_interface> node;
			if (false == metadata->query_node_by_index(index, &node))
				throw std::runtime_error("failed to get binary node");

			if (node->type() != TypeEnum::ARRAY)
				throw std::runtime_error("not an array");
			
			return node->count();
			

		}

		size_t ArraySize(const char* name)
		{
			ThrowOnEmpty("Binary Parser");

			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("no metadata");
			size_t index;
			if(false == metadata->get_index_by_name(name, index))
				throw std::runtime_error("index does not match");

			return ArraySize(index);

		}
		template<typename T = BinaryParser>
		typename std::enable_if<std::is_same<T, BinaryParser>::value, ParserArray<BinaryParser>>::type
		ArrayRange(size_t index)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			BinaryParser me = *this;
			
			return ParserArray<BinaryParser>([index, me](size_t arrayIndex, BinaryParser& data) mutable
			{
				utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
				if (me.m_core_object->read_complex_at(index, &parser, arrayIndex) == false)
				{
					//if not found return an empty element
					return false;
				}
				data = BinaryParser(parser);
				return true;
			},ArraySize(index));
		}

		template<typename T = BinaryParser>
		typename std::enable_if<std::is_same<T, BinaryParser>::value, ParserArray<BinaryParser>>::type
		ArrayRange(const char* childName)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			if (childName == nullptr)
				throw std::invalid_argument("childName");

			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("metadata");

			size_t index;
			if (false == metadata->get_index_by_name(childName, index))
				throw std::invalid_argument("childName");

			return ArrayRange<T>(index);
		}

		template<typename T>
		typename std::enable_if<!std::is_same<T, BinaryParser>::value, ParserArray<T>>::type
		ArrayRange(size_t index)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			BinaryParser me = *this;
			return ParserArray<T>([me, index](size_t arrayIndex, T& data) mutable
			{
				core::types::type_enum type = utils::types::get_type<T>();
				if (me.m_core_object->read_at(index, (uint8_t*)&data, sizeof(T), type, arrayIndex) == false)
				{
					//if not found return an empty element
					return false;
				}
				return true;
			},ArraySize(index));
		}

		template<typename T>
		typename std::enable_if<!std::is_same<T, BinaryParser>::value, ParserArray<T>>::type
		ArrayRange(const char* childName)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Element");

			if (childName == nullptr)
				throw std::invalid_argument("childName");
			
			utils::ref_count_ptr<const core::parsers::binary_metadata_interface> metadata;
			if (false == m_core_object->query_metadata(&metadata))
				throw std::runtime_error("metadata");

			size_t index;
			if (false == metadata->get_index_by_name(childName, index))
				throw std::invalid_argument("childName");

			return ArrayRange<T>(index);
		}

		
		/// Writes a simple type to the parser
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @tparam	T	Generic type parameter.
		/// @param	name	The name.
		/// @param	data	The data.
		///
		/// ### tparam	T	Generic type parameter.
		template <typename T>
		void Write(const char* name,const T &data)
		{
			ThrowOnEmpty("BinaryParser");

			TypeEnum type = utils::types::get_type<T>();
			if (false == m_core_object->write(name, (uint8_t*)&data, sizeof(T), type))
				throw std::runtime_error("write Simple error");
		}

		/// Writes a simple type to the parser
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @tparam	T	Generic type parameter.
		/// @param	index	Zero-based index of the.
		/// @param	data 	The data.
		///
		/// ### tparam	T	Generic type parameter.
		template <typename T>
		void Write(size_t index, const T &data)
		{
			ThrowOnEmpty("BinaryParser");

            TypeEnum type = utils::types::get_type<T>();
			if (false == m_core_object->write(index, (uint8_t*)&data, sizeof(T), type))
				throw std::runtime_error("write Simple error");
		}

		/// Writes data to the parsed buffer
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name.
		/// @param	data	The data to write.
		void Write(const char* name, const void* data,size_t size)
		{
			if (false == m_core_object->write(name, data, size, TypeEnum::BUFFER))
				throw std::runtime_error("write Simple error");
		}
		/// Writes data to the parsed buffer
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name.
		/// @param	data	The data to write.
		void Write(const char* name, const std::vector<uint8_t>&& data)
		{
			std::vector<uint8_t> vec = std::move(data);
			if (false == m_core_object->write(name, data.data(), data.size(), TypeEnum::BUFFER))
				throw std::runtime_error("write Simple error");
		}

		/// Writes data to the parsed buffer
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	index	The name.
		/// @param	data 	The data to write.
		void Write(size_t index, const std::vector<uint8_t>&& data)
		{
			std::vector<uint8_t> vec = std::move(data);
			if (false == m_core_object->write(index, data.data(), data.size(), TypeEnum::BUFFER))
				throw std::runtime_error("write Simple error");
		}

		/// Writes data to the parsed buffer
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name.
		/// @param	data	The data to write.
		void Write(const char* name, const std::vector<uint8_t>& data)
		{
			if(false == m_core_object->write(name, data.data(), data.size(), TypeEnum::BUFFER))
				throw std::runtime_error("write Simple error");
		}

		/// Writes data to the parsed buffer
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name.
		/// @param	data	The data to write.
		void Write(size_t index, const std::vector<uint8_t>& data)
		{
			if (false == m_core_object->write(index, data.data(), data.size(), TypeEnum::BUFFER))
				throw std::runtime_error("write Simple error");
		}

		/// Writes a string to the parse buffer
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name.
		/// @param	str 	The string.
		void Write(const char* name, const char* str)
		{
			if (false == m_core_object->write_string(name, str))
				throw std::runtime_error("write string error");
		}

		/// Writes a string to the parse buffer
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	name	The name.
		/// @param	str 	The string.
		void Write(size_t index, const char* str)
		{
			if (false == m_core_object->write_string(index, str))
				throw std::runtime_error("write string error");
		}

		/// Writes an array at a specific index
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @tparam	T	Generic type parameter.
		/// @param	name	  	The name.
		/// @param	arrayIndex	Zero-based index of the array.
		/// @param	data	  	The data.
		template <typename T>
		void WriteArrayAt(const char* name, size_t arrayIndex, const T &data)
		{
			if (false == m_core_object->write_at(name, (uint8_t*)&data, sizeof(T), arrayIndex))
				throw std::runtime_error("WriteArrayAt error");
		}

		/// Writes an array at a specific index
		/// @date	10/10/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @tparam	T	Generic type parameter.
		/// @param	index	  	Zero-based index of the.
		/// @param	arrayIndex	Zero-based index of the array.
		/// @param	data	  	The data.
		///
		/// ### tparam	T	Generic type parameter.
		/// ### param	name	The name.
        template <typename T>
        void WriteArrayAt(size_t index, size_t arrayIndex, const T &data)
        {
            if (false == m_core_object->write_at(index, (uint8_t*)&data, sizeof(T), arrayIndex))
                throw std::runtime_error("WriteArrayAt error");
        }
	};

	inline BinaryParser BinaryMetaData::CreateParser() const
	{
		ThrowOnEmpty("BinaryMetadata");

		utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
		m_core_object->create_parser(&parser);
		return 	BinaryParser(parser);

	}
	class BinaryParserFactory :
		public Common::NonConstructible
	{
	public:

		/// Creates a new BinaryParser
		/// @date	25/12/2018
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 	occurs.
		/// @param	metadata	The metadata.
		/// @return	A BinaryParser.
		static BinaryParser Create(BinaryMetaDataBuilder metadata, bool reset = false)
		{
			utils::ref_count_ptr<core::parsers::binary_parser_interface> instance;
			if (parsers::binary_parser::create(static_cast<core::parsers::binary_metadata_interface*>(metadata),reset, &instance) == false)
				throw std::runtime_error("Failed to create Binary Parser");

			return BinaryParser(instance);
		}

	};
}
template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, Parsers::BinaryParser& parser)
{
	os << parser.ToJson();
	return os;
}
