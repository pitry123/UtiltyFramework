#pragma once
#include <utils/database.hpp>

#include <Common.hpp>
#include <Utils.hpp>
#include <Application.hpp>
#include <BinaryParser.hpp>

#include <type_traits>
#include <algorithm>
#include <cctype>
#include <stack>
#include <unordered_map>

namespace Database
{
	using SubscriptionToken = utils::database::subscription_token;
	static constexpr SubscriptionToken SubscriptionTokenUndefined = utils::database::subscription_token_undefined;

	using SubscriptionParams = utils::database::subscription_params;
	using TableSubscriptionParams = utils::database::table_subscription_params;
	using RowInfo = core::database::row_info;

	static constexpr size_t UnboundedRowSize = core::database::UNBOUNDED_ROW_SIZE;

	struct AnyKey : public utils::database::buffered_key
	{
		AnyKey(const AnyKey& key) :
			utils::database::buffered_key(key)
		{
		}

		AnyKey(const utils::database::buffered_key& key) :
			utils::database::buffered_key(key)
		{
		}

		AnyKey(const core::database::key& key) :
			utils::database::buffered_key(key)
		{
		}		

		template <typename T>
		AnyKey(const T& val) :
			utils::database::buffered_key(&val, sizeof(T))
		{
			static_assert(sizeof(T) <= sizeof(decltype(data)), "Key size exceeds MAX_KEY_SIZE");
			static_assert(std::is_pointer<T>::value == false, "AnyKey CAN'T be initiated with a pointer type (with exception of <const char*>)");
		}

		AnyKey(const char* str) :
			utils::database::buffered_key(str, std::strlen(str))
		{
		}

		AnyKey(const std::string& str) :
			AnyKey(str.c_str())
		{
		}		

		template <typename T>
		bool Equals(const T& val) const
		{
			AnyKey other = val;
			return core::database::operator==(*this, other);
		}

		template <typename T>
		operator T() const
		{
			static_assert(std::is_pointer<T>::value == false, "AnyKey CAN'T be casted to a pointer type");
			static_assert(sizeof(T) <= sizeof(decltype(data)), "Template type size exceeds MAX_KEY_SIZE");

			const void* data_ptr = static_cast<const void*>(data);
			return *(static_cast<const T*>(data_ptr));
		}
	};
	
	// Forward declarations
	class Row;
	class Table;
	class DataSet;

	class RowData
	{
	private:
		const utils::database::row_data& m_rowData;

		RowData(const RowData& other) = delete;			// non construction-copyable
		RowData& operator=(const RowData&) = delete;	// non copyable				
		RowData(const RowData&& other) = delete;		// non construction-movable
		RowData& operator=(const RowData&&) = delete;	// non movable				

	public:
		RowData(const utils::database::row_data& rowData);

		const AnyKey Key() const;
		size_t DataSize() const;
		const void* Buffer() const;

		void Read(void* buffer, size_t size) const;
		template <typename T> void Read(T& val) const;
		template <typename T> const T& Read() const;
		Parsers::BinaryMetaData ParserMetadata() const;
		Parsers::BinaryParser Parser() const;
		Row DBRow() const;

		std::string ToJson(Parsers::JsonDetailsLevel detailsLevel, bool comapact = false) const;
		bool TryGetJson(std::string& json, Parsers::JsonDetailsLevel detailsLevel, bool compact=true) const;
	};	
	
	template<typename CONTAINER, typename VALUE>
	class Iterator
	{
	public:
		typedef Iterator self_type;
		typedef VALUE value_type;
		typedef VALUE& reference;
		typedef VALUE* pointer;
		typedef std::forward_iterator_tag iterator_category;
		typedef size_t difference_type;

	private:
		CONTAINER m_container;		
		size_t m_size;
		size_t m_index;
		value_type m_value;

		void set_value() 
		{
			m_value = m_container.GetAt(m_index); 
		}

		inline void check_container()
		{
			if (m_container.Size() != m_size)
				throw std::runtime_error("Container was modified while iterating");
		}

	public:
		Iterator(const CONTAINER& container, size_t size, size_t index) :
			m_container(container),
			m_size(size),
			m_index(index)
		{
			set_value();
		}

		// prefix increment
		self_type operator++() 
		{
			check_container();

			self_type retval = *this;
			m_index++;
			set_value();

			return retval;
		}

		// postfix increment
		self_type operator++(int) 
		{
			check_container();

			m_index++;
			set_value();

			return *this; 
		}

		reference operator*() 
		{
			return m_value;
		}

		pointer operator->() 
		{
			return &m_value;
		}

		bool operator==(const self_type& rhs)
		{
			return m_value == rhs.m_value;
		}

		bool operator!=(const self_type& rhs)
		{
			return m_value != rhs.m_value;
		}
	};

	template<typename CONTAINER, typename VALUE>
	class ConstIterator
	{
	public:
		typedef ConstIterator self_type;
		typedef VALUE value_type;
		typedef VALUE& reference;
		typedef VALUE* pointer;
		typedef std::forward_iterator_tag iterator_category;
		typedef size_t difference_type;

	private:
		CONTAINER m_container;
		size_t m_size;
		size_t m_index;
		value_type m_value;

		void set_value()
		{
			m_value = m_container.GetAt(m_index);
		}

		inline void check_container()
		{
			if (m_container.Size() != m_size)
				throw std::runtime_error("Container was modified while iterating");
		}

	public:
		ConstIterator(const CONTAINER& table, size_t size, size_t index) :
			m_container(table),
			m_size(size),
			m_index(index)
		{
			set_value();
		}

		// prefix increment
		self_type operator++()
		{
			check_container();

			self_type retval = *this;
			m_index++;
			set_value();

			return retval;
		}

		// postfix increment
		self_type operator++(int junk)
		{
			check_container();

			m_index++;
			set_value();

			return *this;
		}

        reference operator*()
		{
			return m_value;
		}

		const pointer operator->()
		{
			return &m_value;
		}

		bool operator==(const self_type& rhs)
		{
			return m_value == rhs.m_value;
		}

		bool operator!=(const self_type& rhs)
		{
			return m_value != rhs.m_value;
		}
	};
	
	class Subscriptions :
		public Common::CoreObjectWrapper<utils::database::subscriptions>
	{
	public:
		Subscriptions(std::nullptr_t)
		{
			// Empty Subscriptions
		}

		Subscriptions() :
			CoreObjectWrapper<utils::database::subscriptions>(utils::make_ref_count_ptr<utils::database::subscriptions>())
		{
		}

		SubscriptionParams Subscribe(core::database::row_interface* row, const std::function<void(const RowData&)>& func) const
		{
			return m_core_object->subscribe(row, func);			
		}

		bool Unsubscribe(core::database::row_interface* row, SubscriptionToken token) const
		{
			return m_core_object->unsubscribe(row, token);		
		}		

		bool Unsubscribe(const Database::SubscriptionParams& subscriptionParams)
		{
			return m_core_object->unsubscribe(subscriptionParams);
		}
	};

	template <typename T>
	class Subscribable :
		public Common::CoreObjectWrapper<T>
	{
	protected:
		using SubscribableBase = Subscribable<T>;
		Subscriptions m_subscriptions;

	public:
		Subscribable() :m_subscriptions(nullptr)
		{
			// Empty
		}

		Subscribable(T* core_object) :
            Common::CoreObjectWrapper<T>(core_object)
		{
		}

		Subscribable(T* core_object, const Database::Subscriptions& subscriptions) :
            Common::CoreObjectWrapper<T>(core_object),
			m_subscriptions(subscriptions)
		{
		}

		Subscribable(const Subscribable& other) :
            Common::CoreObjectWrapper<T>(other),
			m_subscriptions(other.m_subscriptions)
		{
		}

		Subscribable(Subscribable&& other) :
            Common::CoreObjectWrapper<T>(other),
			m_subscriptions(std::move(other.m_subscriptions))
		{
		}

		virtual ~Subscribable() = default;

		virtual Subscribable& operator=(const Subscribable& other)
		{
            Common::CoreObjectWrapper<T>::operator=(other);
			m_subscriptions = other.m_subscriptions;
			return *this;
		}

		virtual Subscribable& operator=(Subscribable&& other)
		{
            Common::CoreObjectWrapper<T>::operator=(other);
			m_subscriptions = std::move(other.m_subscriptions);
			return *this;
		}

		virtual bool operator==(const Subscribable& other)
		{
            if (Common::CoreObjectWrapper<T>::operator==(other) == false)
				return false;

			return (m_subscriptions == other.m_subscriptions);
		}		
	};

	class AutoToken :
		public Common::CoreObjectWrapper<utils::database::auto_token>
	{
	public:
		AutoToken()
		{
			// Empty Token
		}		

		AutoToken(utils::database::auto_token* token) :
			Common::CoreObjectWrapper<utils::database::auto_token>(token)
		{
		}

		AutoToken(const Database::SubscriptionParams& subscriptionParams) :
			AutoToken(utils::make_ref_count_ptr<utils::database::auto_token>(subscriptionParams))
		{
		}

		AutoToken(Database::SubscriptionParams&& subscriptionParams) :
			AutoToken(utils::make_ref_count_ptr<utils::database::auto_token>(std::forward<Database::SubscriptionParams>(subscriptionParams)))
		{
		}		

		void Unregister()
		{
			if (Empty() == true)
				return;

			m_core_object->unregister();
		}
	};

	class SubscriptionsCollector :
		public Common::CoreObjectWrapper<utils::database::subscriptions_collector>
	{
	public:
		SubscriptionsCollector(utils::database::subscriptions_collector* collector) :
			Common::CoreObjectWrapper<utils::database::subscriptions_collector>(collector)
		{
		}

		SubscriptionsCollector() :
			SubscriptionsCollector(utils::make_ref_count_ptr<utils::database::subscriptions_collector>())
		{
		}

		void Clear()
		{
			m_core_object->clear();
		}

		void Add(const Database::AutoToken& token)
		{
			if (token.Empty() == true)
				throw std::invalid_argument("token");

			m_core_object->add(static_cast<utils::database::auto_token*>(token));
		}

		bool Remove(Database::AutoToken& token)
		{
			return m_core_object->remove(static_cast<utils::database::auto_token*>(token));
		}

		bool Remove(size_t index)
		{
			return m_core_object->remove(index);
		}

		void Add(Database::SubscriptionParams&& subscriptionParams)
		{
			m_core_object->add(std::forward<Database::SubscriptionParams>(subscriptionParams));
		}

		void operator+=(const Database::AutoToken& token)
		{
			Add(token);
		}

		void operator+=(Database::SubscriptionParams&& subscriptionParams)
		{
			Add(std::forward<Database::SubscriptionParams>(subscriptionParams));
		}
	};

	class AutoTableToken :
		public Common::CoreObjectWrapper<utils::database::auto_table_token>
	{
	public:
		AutoTableToken()
		{
			// Empty Token
		}

		AutoTableToken(utils::database::auto_table_token* token) :
			Common::CoreObjectWrapper<utils::database::auto_table_token>(token)
		{
		}

		AutoTableToken(const Database::TableSubscriptionParams& subscriptionParams) :
			AutoTableToken(utils::make_ref_count_ptr<utils::database::auto_table_token>(subscriptionParams))
		{
		}

		AutoTableToken(Database::TableSubscriptionParams&& subscriptionParams) :
			AutoTableToken(utils::make_ref_count_ptr<utils::database::auto_table_token>(std::forward<Database::TableSubscriptionParams>(subscriptionParams)))
		{
		}

		TableSubscriptionParams Params()
		{
			return m_core_object->params();
		}

		void Unregister()
		{
			if (Empty() == true)
				return;

			m_core_object->unregister();
		}
	};

	struct EmptyType
	{
	};

	template <typename T>
	constexpr bool IsEmptyType()
	{
		return std::is_same<T, Database::EmptyType>::value;
	}

	class Row : 
		public Subscribable<core::database::row_interface>
	{
	public:
		Row();
		Row(core::database::row_interface* row);
		Row(core::database::row_interface* row, const Database::Subscriptions& subscriptions);

		AnyKey Key() const;
		Table Parent() const;
		size_t DataSize() const;
		uint8_t WritePriority() const;
		
		bool Read(void* buffer, size_t size) const;
		template <typename T> bool Read(T& val) const;
		template <typename T> T Read() const;
		void  Info(RowInfo &info);
		const RowInfo& Info() const;
		Parsers::BinaryMetaData ParserMetadata() const;
		bool Reset();

		void Write(const void* buffer, size_t size, bool forceReport, uint8_t priority);
		void Write(const void* buffer, size_t size, bool forceReport);
		void Write(const void* buffer, size_t size, uint8_t priority);
		void Write(const void* buffer, size_t size);

		/// @fn	void Row::Write();
		/// @brief	Use only for Empty rows
		/// @date	30/07/2019
		void Write();

		template <typename T> void Write(const T& val, bool forceReport, uint8_t priority);
		template <typename T> void Write(const T& val, bool forceReport);
		template <typename T> void Write(const T& val, uint8_t priority);
		template <typename T> void Write(const T& val);

		bool CheckAndWrite(const void* buffer, size_t size, bool forceReport, uint8_t priority);
		bool CheckAndWrite(const void* buffer, size_t size, bool forceReport);
		bool CheckAndWrite(const void* buffer, size_t size, uint8_t priority);
		bool CheckAndWrite(const void* buffer, size_t size);
		template <typename T> bool CheckAndWrite(const T& val, bool forceReport, uint8_t priority);
		template <typename T> bool CheckAndWrite(const T& val, bool forceReport);
		template <typename T> bool CheckAndWrite(const T& val, uint8_t priority);
		template <typename T> bool CheckAndWrite(const T& val);
		std::string ToJson(Parsers::JsonDetailsLevel detailsLevel, bool compact = true) const;
		bool TryGetJson(std::string& json, Parsers::JsonDetailsLevel detailsLevel, bool compact = true) const;
		bool FromJson(const char* json);
		bool SetWritePriority(uint8_t priority) const;

		SubscriptionParams Subscribe(const std::function<void(const RowData&)>& func) const;
		SubscriptionParams operator+=(const std::function<void(const RowData&)>& func) const;
		bool Unsubscribe(SubscriptionToken token) const;
		bool operator-=(SubscriptionToken token) const;
	};

	class Table :
		public Subscribable<core::database::table_interface>
	{
	public:
		Table();
		Table(core::database::table_interface* table);
		Table(core::database::table_interface* table, const Database::Subscriptions& subscriptions);
		Table(const Table& table, const Database::Subscriptions& subscriptions);

		AnyKey Key() const;
		DataSet Parent() const;
		size_t Size() const;
		const char* Name() const;
		const char* Description() const;
		void Reset();
		Row operator [](const AnyKey& rowKey) const;
		Row GetRowByName(const char* name) const;
		bool TryGet(const AnyKey& rowKey, Row& row);				
		Row GetAt(size_t index);

		void AddRow(const AnyKey& rowKey, size_t size, const RowInfo& info, Parsers::BinaryMetaData parser);
		void AddRow(const AnyKey& rowKey, size_t size);
		template <typename T> void AddRow(const AnyKey& rowKey, const RowInfo& info, Parsers::BinaryMetaData parserMetadata);
		template <typename T> void AddRow(const AnyKey& rowKey);		
		template <typename T> void AddRow(const AnyKey& rowKey, const T& val, const RowInfo& info, Parsers::BinaryMetaData parser);
		template <typename T> void AddRow(const AnyKey& rowKey, const T& val);
		void RemoveRow(const AnyKey& rowKey);
		SubscriptionsCollector Subscribe(const std::function<void(const RowData&)>& func) const;
		bool Unsubscribe(SubscriptionsCollector token) const;
		Iterator<Table, Row> begin();
		Iterator<Table, Row> end();
		ConstIterator<Table, Row> begin() const;
		ConstIterator<Table, Row> end() const;
	};

	class DataSet :
		public Subscribable<core::database::dataset_interface>
	{
	public:
		DataSet();
		DataSet(core::database::dataset_interface* dataset);
		DataSet(core::database::dataset_interface* dataset, const Database::Subscriptions& subscriptions);

		AnyKey Key() const;
		size_t Size() const;
		void Reset();
		Table operator [](const AnyKey& tableKey) const;
		Table GetTableByName(const char* name) const;
		bool TryGet(const AnyKey& tableKey, Table& table);				
		Table GetAt(size_t index);

		void AddTable(const AnyKey& tableKey);
		void AddTable(const AnyKey& tableKey, const char* name, const char* description);
		void RemoveTable(const AnyKey& tableKey);

		Iterator<DataSet, Table> begin();
		Iterator<DataSet, Table> end();
		ConstIterator<DataSet, Table> begin() const;
		ConstIterator<DataSet, Table> end() const;
	};

	inline RowData::RowData(const utils::database::row_data& rowData) :
		m_rowData(rowData)
	{
	}

	inline const AnyKey RowData::Key() const
	{
		return m_rowData.key();
	}

	inline size_t RowData::DataSize() const
	{
		return m_rowData.data_size();
	}

	inline const void* RowData::Buffer() const
	{
		return m_rowData.buffer();
	}

	inline void RowData::Read(void* buffer, size_t size) const
	{
		m_rowData.read(buffer, size);
	}

	template <typename T>
	inline void RowData::Read(T& val) const
	{
		m_rowData.read<T>(val);
	}

	template <typename T>
	inline const T& RowData::Read() const
	{
		return m_rowData.read<T>();
	}

	inline Row RowData::DBRow() const
	{
		utils::ref_count_ptr<core::database::row_interface> row;
		if (m_rowData.query_row(&row))
		{
			return Database::Row(row);
		}

		return Database::Row();
	}

	inline Parsers::BinaryMetaData  RowData::ParserMetadata() const
	{
		return DBRow().ParserMetadata();
	}

	inline Parsers::BinaryParser RowData::Parser() const
	{
		Parsers::BinaryParser parser = ParserMetadata().CreateParser();

		if(parser.Empty())
			throw std::runtime_error("failed to create parser");

		if(false == parser.Parse(Buffer(), DataSize()))
			throw std::runtime_error("Parser does not match RowData buffer size");

		return parser;

	}
	inline std::string RowData::ToJson(Parsers::JsonDetailsLevel detailsLevel, bool compact) const
	{
		Parsers::BinaryMetaData metadata = ParserMetadata();
		if (metadata.Empty())
			throw std::runtime_error("Row::ToJson - Empty Metadata");

		Parsers::BinaryParser parser = ParserMetadata().CreateParser();
		if (parser.Empty())
			throw std::runtime_error("Row::ToJson - Failed to create Parser");
				
		if (false == parser.Parse(Buffer(), DataSize()))
			throw std::runtime_error("Row::ToJson - Failed to parse row");

		std::string json = parser.ToJson(detailsLevel, compact);
		return json;

	}
	inline bool RowData::TryGetJson(std::string& json, Parsers::JsonDetailsLevel detailsLevel, bool compact) const
	{
		if (DBRow().Info().type == TypeEnum::EMPTY_TYPE)
		{
			json = "{}";
			return true;
		}

		Parsers::BinaryMetaData metadata = ParserMetadata();
		if (metadata.Empty())
			throw std::runtime_error("Row::ToJson - Empty Metadata");

		Parsers::BinaryParser parser = ParserMetadata().CreateParser();
		if (parser.Empty())
			throw std::runtime_error("Row::ToJson - Failed to create Parser");

		if (false == parser.Parse(Buffer(), DataSize()))
			throw std::runtime_error("Row::ToJson - Failed to parse row");

		return parser.TryGetJson(json, detailsLevel, compact);
	}
	inline Row::Row()
	{
		// Empty Row
	}

	inline Row::Row(core::database::row_interface* row) :
		SubscribableBase(row)
	{
	}

	inline Row::Row(core::database::row_interface* row, const Database::Subscriptions& subscriptions) :
		SubscribableBase(row, subscriptions)
	{
	}
	
	inline AnyKey Row::Key() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		return m_core_object->key();
	}

	inline Table Row::Parent() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		utils::ref_count_ptr<core::database::table_interface> core_parent;
		if (m_core_object->query_parent(&core_parent) == false)
			return Table();

		return Table(core_parent, m_subscriptions);
	}

	inline size_t Row::DataSize() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		return m_core_object->data_size();
	}

	inline uint8_t Row::WritePriority() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		return m_core_object->write_priority();
	}

	inline bool Row::Read(void* buffer, size_t size) const
	{
		if (buffer == nullptr)
			throw std::invalid_argument("buffer");

		if (size == 0)
			throw std::invalid_argument("size");

		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		if (m_core_object->read_bytes(buffer, size) == false)
			return false;

		return true;
	}

	template <typename T>
	inline bool Row::Read(T& val) const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		if (m_core_object->data_size() > sizeof(T))
			throw std::runtime_error("Row read size mismatch. Wrong data type?");

		if (m_core_object->read_bytes(&val) == false)
			return false;

		return true;
	}

	template <typename T>
	inline T Row::Read() const
	{
		T retval;
		if (Read(retval) == false)
			throw std::runtime_error("Failed to read row data");

		return retval;
	}

	inline void Row::Info(RowInfo &info)
	{
#ifdef _WIN32
		memcpy_s(&info, sizeof(info), &Info(), sizeof(RowInfo));
#else
        std::memcpy(&info, &Info(), sizeof(RowInfo));
#endif
	}

	inline Parsers::BinaryMetaData Row::ParserMetadata() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		utils::ref_count_ptr<core::parsers::binary_metadata_interface> binary_metadata;

		if(m_core_object->query_parser_metadata(&binary_metadata))
		{
			return Parsers::BinaryMetaData(binary_metadata);
		}

		return Parsers::BinaryMetaData();
	}

	inline bool Row::Reset()
	{
		Parsers::BinaryMetaData metadata = ParserMetadata();
		if (metadata.Empty())
			return false;
		Parsers::BinaryParser parser = metadata.CreateParser();
		parser.Reset(); //Set the default
		Buffers::Buffer buffer = parser.Buffer();
		if (false == buffer.Empty())
		{
			Write((const void*)buffer.Data(), buffer.Size());
			return true;
		}

		return false;
	}

	inline const RowInfo& Row::Info() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		return m_core_object->info();
	}
		
	inline void Row::Write(const void* buffer, size_t size, bool forceReport, uint8_t priority)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		if (m_core_object->write_bytes(buffer, size, forceReport, priority) == false)
			throw std::runtime_error("Failed to write row data");
	}

	inline void Row::Write(const void* buffer, size_t size, bool forceReport)
	{
		Write(buffer, size, forceReport, 0);
	}

	inline void Row::Write(const void* buffer, size_t size, uint8_t priority)
	{
		Write(buffer, size, true, priority);
	}

	inline void Row::Write(const void* buffer, size_t size)
	{
		Write(buffer, size, true, 0);
	}

	inline void Row::Write()
	{
		Write(static_cast<const void*>(nullptr), 0);
	}

	template <typename T>
	inline void Row::Write(const T& val, bool forceReport, uint8_t priority)
	{
		Write(&val, sizeof(T), forceReport, priority);
	}

	template <typename T>
	inline void Row::Write(const T& val, bool forceReport)
	{
		Write(&val, sizeof(T), forceReport, 0);
	}

	template <typename T>
	inline void Row::Write(const T& val, uint8_t priority)
	{
		Write(&val, sizeof(T), true, priority);
	}

	template <typename T>
	inline void Row::Write(const T& val)
	{
		Write(&val, sizeof(T), true, 0);
	}

	inline bool Row::CheckAndWrite(const void* buffer, size_t size, bool forceReport, uint8_t priority)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");
		
		return m_core_object->check_and_write_bytes(buffer, size, forceReport, priority);
	}

	inline bool Row::CheckAndWrite(const void* buffer, size_t size, bool forceReport)
	{
		return CheckAndWrite(buffer, size, forceReport, 0);
	}

	inline bool Row::CheckAndWrite(const void* buffer, size_t size, uint8_t priority)
	{
		return CheckAndWrite(buffer, size, true, priority);
	}

	inline bool Row::CheckAndWrite(const void* buffer, size_t size)
	{
		return CheckAndWrite(buffer, size, true, 0);
	}

	template <typename T>
	inline bool Row::CheckAndWrite(const T& val, bool forceReport, uint8_t priority)
	{
		return CheckAndWrite(&val, sizeof(T), forceReport, priority);
	}

	template <typename T>
	inline bool Row::CheckAndWrite(const T& val, bool forceReport)
	{
		return CheckAndWrite(&val, sizeof(T), forceReport, 0);
	}

	template <typename T>
	inline bool Row::CheckAndWrite(const T& val, uint8_t priority)
	{
		return CheckAndWrite(&val, sizeof(T), true, priority);
	}

	template <typename T>
	inline bool Row::CheckAndWrite(const T& val)
	{
		return CheckAndWrite(&val, sizeof(T), true, 0);
	}
	inline bool Row::SetWritePriority(uint8_t priority) const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		return m_core_object->set_write_priority(priority);
	}

	inline std::string Row::ToJson(Parsers::JsonDetailsLevel detailsLevel,bool compact) const
	{
		std::string json = "{}";
		if (Info().type != TypeEnum::EMPTY_TYPE)
		{
			Parsers::BinaryMetaData metadata = ParserMetadata();
			if (metadata.Empty())
				throw std::runtime_error("Row::ToJson - Empty Metadata");

			Parsers::BinaryParser parser = ParserMetadata().CreateParser();
			if (parser.Empty())
				throw std::runtime_error("Row::ToJson - Failed to create Parser");

			Buffers::Buffer buffer(utils::make_ref_count_ptr<utils::ref_count_buffer>(parser.BufferSize()));
			if (false == Read(buffer.Data(), buffer.Size()))
				throw std::runtime_error("Row::ToJson - Failed to read row");

			if (false == parser.Parse(buffer.Data(), buffer.Size()))
				throw std::runtime_error("Row::ToJson - Failed to parse row");
			
			const char* jsonStr = parser.ToJson(detailsLevel, compact);

			if (jsonStr == nullptr)
				throw std::runtime_error("Row::ToJson - Failed to create Row data JSON");

			json = jsonStr;
		}
		return json;

	}
	inline bool Row::TryGetJson(std::string& json, Parsers::JsonDetailsLevel detailsLevel, bool compact) const
	{
		
		if (Info().type == TypeEnum::EMPTY_TYPE)
		{
			json = "{}";
			return true;
		}
		Parsers::BinaryMetaData metadata = ParserMetadata();
		if (metadata.Empty())
			throw std::runtime_error("Row::ToJson - Empty Metadata");

		Parsers::BinaryParser parser = ParserMetadata().CreateParser();
		if (parser.Empty())
			throw std::runtime_error("Row::ToJson - Failed to create Parser");

		Buffers::Buffer buffer(utils::make_ref_count_ptr<utils::ref_count_buffer>(parser.BufferSize()));
		if (false == Read(buffer.Data(), buffer.Size()))
			throw std::runtime_error("Row::ToJson - Failed to read row");

		if (false == parser.Parse(buffer.Data(), buffer.Size()))
			throw std::runtime_error("Row::ToJson - Failed to parse row");

		return parser.TryGetJson(json, detailsLevel,compact);

	}
	 
	inline bool Row::FromJson(const char* json)
	{
		if (Info().type != TypeEnum::EMPTY_TYPE)
		{
			Parsers::BinaryMetaData metadata = ParserMetadata();
			if (metadata.Empty())
				throw std::runtime_error("Row::FromJson - Empty Metadata");

			Parsers::BinaryParser parser = ParserMetadata().CreateParser();
			if (parser.Empty())
				throw std::runtime_error("Row::FromJson - Failed to create Parser");

			//Read the data and parse the JSON on it so unset data (of struct) will remain
			Buffers::Buffer buffer = parser.Buffer();
			Read(buffer.Data(), buffer.Size());
			parser.Parse(buffer.Data(), buffer.Size());
			
			if (parser.FromJson(json))
			{
				buffer = parser.Buffer();
				if (false == buffer.Empty())
				{
					Write((const void*)buffer.Data(), buffer.Size());
					return true;
				}

			}
		}

		return false;
;	}
	inline SubscriptionParams Row::Subscribe(const std::function<void(const RowData&)>& func) const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		if (m_subscriptions.Empty())
			throw std::runtime_error("Empty Subscriptions - Direct registration is disabled");

		return m_subscriptions.Subscribe(m_core_object, func);
	}

	inline SubscriptionParams Row::operator+=(const std::function<void(const RowData&)>& func) const
	{
		return Subscribe(std::forward<const std::function<void(const RowData&)>&>(func));
	}

	inline bool Row::Unsubscribe(SubscriptionToken token) const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Row");

		if (m_subscriptions.Empty())
			throw std::runtime_error("Empty Subscriptions - Direct registration is disabled");

		return m_subscriptions.Unsubscribe(m_core_object, token);
	}

	inline bool Row::operator-=(SubscriptionToken token) const
	{
		return Unsubscribe(std::forward<SubscriptionToken>(token));
	}
	
	inline Table::Table()
	{
		// Empty Table
	}

	inline Table::Table(core::database::table_interface* table) :
		SubscribableBase(table)
	{
	}

	inline Table::Table(core::database::table_interface* table, const Database::Subscriptions& subscriptions) :
		SubscribableBase(table, subscriptions)
	{
	}

	inline Table::Table(const Table& table, const Database::Subscriptions& subscriptions) :
		Table(static_cast<core::database::table_interface*>(table), subscriptions)
	{
	}

	inline AnyKey Table::Key() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		return m_core_object->key();
	}

	inline DataSet Table::Parent() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		utils::ref_count_ptr<core::database::dataset_interface> core_parent;
		if (m_core_object->query_parent(&core_parent) == false)
			return DataSet();

		return DataSet(core_parent, m_subscriptions);
	}

	inline size_t Table::Size() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		return m_core_object->size();
	}
	
	inline const char* Table::Name() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		return m_core_object->name();
	}

	inline const char* Table::Description() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		return m_core_object->description();
	}

	inline void Table::Reset()
	{
		for (auto& row : *this)
			row.Reset();
	}

	inline Row Table::operator [](const AnyKey& rowKey) const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		utils::ref_count_ptr<core::database::row_interface> row;
		if (m_core_object->query_row(rowKey, &row) == false)
			throw std::runtime_error("Row does not exist");

		return Row(row, m_subscriptions);
	}

	inline Row Table::GetRowByName(const char* name) const
	{
		if (Empty() == true)
			return Row();

		utils::ref_count_ptr<core::database::row_interface> row;
		if (m_core_object->query_row_by_name(name, &row) == false)
			return Row();

		return Row(row, m_subscriptions);
	}

	inline bool Table::TryGet(const AnyKey& rowKey, Row& row)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		utils::ref_count_ptr<core::database::row_interface> core_row;
		if (m_core_object->query_row(rowKey, &core_row) == false)
			return false;

		row = Row(core_row, m_subscriptions);
		return true;
	}

	// Note: If index is out of bounds,
	// function returns Empty Row.
	// Please verify before using return value
	// by checking: 'retval.Empty() == false'
	inline Row Table::GetAt(size_t index)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		utils::ref_count_ptr<core::database::row_interface> row;
		if (m_core_object->query_row_by_index(index, &row) == false)
			return Row(); // Empty Row

		return Row(row, m_subscriptions);
	}

	inline void Table::AddRow(const AnyKey& rowKey, size_t size,const RowInfo& info, Parsers::BinaryMetaData parserMetadata)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");
		if (size == 0 && info.type != core::types::EMPTY_TYPE)
			throw std::invalid_argument("size");

		core::parsers::binary_metadata_interface* internal_parser;
		if (parserMetadata.Empty())
			internal_parser = nullptr;
		else
			internal_parser = static_cast<core::parsers::binary_metadata_interface*>(parserMetadata);

		if (m_core_object->add_row(rowKey, size, info, internal_parser) == false)
			throw std::runtime_error("Failed to add row. Already exists?");
	}

	inline void Table::AddRow(const AnyKey& rowKey, size_t size)
	{
		RowInfo info = { core::types::type_enum::UNKNOWN , "\0", "\0"};
		Parsers::BinaryMetaData parser;
		AddRow(rowKey, size, info,parser);
	}

	template <typename T>
	inline void Table::AddRow(const AnyKey& rowKey, const RowInfo &info, Parsers::BinaryMetaData parser)
	{
		AddRow(rowKey, sizeof(T), info,parser);
	}	

	template <typename T>
	inline void Table::AddRow(const AnyKey& rowKey)
	{	
		if (IsEmptyType<T>() == true)
		{
			RowInfo info = { TypeEnum::EMPTY_TYPE, "\0", "\0", "\0" };
			AddRow(rowKey, 0, info, nullptr);
		}
		else
		{
			RowInfo info = { utils::types::get_type<T>() , "\0", "\0", "\0" };
			Parsers::BinaryMetaData parser;
			AddRow<T>(rowKey, info, parser);
		}		
	}

	template <typename T>
	inline void Table::AddRow(const AnyKey& rowKey, const T& val, const RowInfo& info, Parsers::BinaryMetaData parser)
	{
		AddRow<T>(rowKey,info,parser);
		this->operator [](rowKey).Write<T>(val);
	}
	
	template <typename T>
	inline void Table::AddRow(const AnyKey& rowKey, const T& val)
	{
		AddRow<T>(rowKey);
		this->operator [](rowKey).Write<T>(val);
	}

	inline void Table::RemoveRow(const AnyKey& rowKey)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty Table");

		if (m_core_object->remove_row(rowKey, nullptr) == false)
			throw std::runtime_error("Failed to remove row. Not exists?");
	}

	inline Iterator<Table, Row> Table::begin()
	{
		size_t size = Size();
		return Iterator<Table, Row>(*this, size, 0);
	}

	inline Iterator<Table, Row> Table::end()
	{
		size_t size = Size();
		return Iterator<Table, Row>(*this, size, size);
	}

	inline ConstIterator<Table, Row> Table::begin() const
	{
		size_t size = Size();
		return ConstIterator<Table, Row>(*this, size, 0);
	}

	inline ConstIterator<Table, Row> Table::end() const
	{
		size_t size = Size();
		return ConstIterator<Table, Row>(*this, size, size);
	}
	
	inline DataSet::DataSet()
	{
		// Empty DataSet
	}

	inline DataSet::DataSet(core::database::dataset_interface* dataset) :
		SubscribableBase(dataset)
	{
	}

	inline DataSet::DataSet(core::database::dataset_interface* dataset, const Database::Subscriptions& subscriptions) :
		SubscribableBase(dataset, subscriptions)
	{
	}

	inline AnyKey DataSet::Key() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty DataSet");

		return m_core_object->key();
	}

	inline size_t DataSet::Size() const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty DataSet");

		return m_core_object->size();
	}

	inline void DataSet::Reset()
	{
		for (auto& table : *this)
			table.Reset();
	}

	inline Table DataSet::operator [](const AnyKey& tableKey) const
	{
		if (Empty() == true)
			throw std::runtime_error("Empty DataSet");

		utils::ref_count_ptr<core::database::table_interface> table;
		if (m_core_object->query_table(tableKey, &table) == false)
			throw std::runtime_error("Table does not exist");

		return Table(table, m_subscriptions);
	}

	inline Table DataSet::GetTableByName(const char* name) const
	{
		if (Empty() == true)
			return Table();

		utils::ref_count_ptr<core::database::table_interface> table;
		if (m_core_object->query_table_by_name(name, &table) == false)
			return Table();

		return Table(table, m_subscriptions);
	}

	inline bool DataSet::TryGet(const AnyKey& tableKey, Table& table)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty DataSet");

		utils::ref_count_ptr<core::database::table_interface> core_table;
		if (m_core_object->query_table(tableKey, &core_table) == false)
			return false;

		table = Table(core_table, m_subscriptions);
		return true;
	}

	// Note: If index is out of bounds,
	// function returns Empty Table.
	// Please verify before using return value
	// by checking: 'retval.Empty() == false'
	inline Table DataSet::GetAt(size_t index)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty DataSet");

		utils::ref_count_ptr<core::database::table_interface> table;
		if (m_core_object->query_table_by_index(index, &table) == false)
			return Table(); // Empty Table

		return Table(table, m_subscriptions);
	}

	inline void DataSet::AddTable(const AnyKey& tableKey)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty DataSet");

		if (m_core_object->add_table(tableKey) == false)
			throw std::runtime_error("Failed to add table. Already exists?");
	}

	inline void DataSet::AddTable(const AnyKey& tableKey, const char* name, const char* description)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty DataSet");

		if (m_core_object->add_table(tableKey,name,description) == false)
			throw std::runtime_error("Failed to add table. Already exists?");
	}

	inline void DataSet::RemoveTable(const AnyKey& tableKey)
	{
		if (Empty() == true)
			throw std::runtime_error("Empty DataSet");

		utils::ref_count_ptr<core::database::table_interface> table;
		if (m_core_object->query_table(tableKey, &table) == false)
			throw std::runtime_error("Failed to find table. Not exists?");

		if (m_core_object->remove_table(tableKey, nullptr) == false)
			throw std::runtime_error("Failed to remove table. Not exists?");
	}

	inline Iterator<DataSet, Table> DataSet::begin()
	{
		size_t size = Size();
		return Iterator<DataSet, Table>(*this, size, 0);
	}

	inline Iterator<DataSet, Table> DataSet::end()
	{
		size_t size = Size();
		return Iterator<DataSet, Table>(*this, size, size);
	}

	inline ConstIterator<DataSet, Table> DataSet::begin() const
	{
		size_t size = Size();
		return ConstIterator<DataSet, Table>(*this, size, 0);
	}

	inline ConstIterator<DataSet, Table> DataSet::end() const
	{
		size_t size = Size();
		return ConstIterator<DataSet, Table>(*this, size, size);
	}
		
	template <class T>
    class DispatcherBase : public utils::database::database_dispatcher_base<T>
	{
	public:
		DispatcherBase(const char* name = nullptr, bool startSuspended = false) :
			utils::database::database_dispatcher_base<T>(name, startSuspended)
		{
		}

		DispatcherBase(const Utils::Context& context) :
            utils::database::database_dispatcher_base<T>(static_cast<utils::dispatcher*>(context))
		{
		}

		virtual ~DispatcherBase() = default;
				
	protected:
		bool Suspended()
		{
			return this->suspended();
		}

		void Suspend()
		{
			this->suspend();
		}

		void Resume()
		{
			this->resume();
		}

		virtual void Init()
		{
			// Do nothing...
		}

		virtual void Start()
		{
			// Do nothing...
		}

		virtual void Started()
		{
			// Do nothing...
		}

		virtual void Stop()
		{
			// Do nothing...
		}

		virtual void Stopped()
		{
			// Do nothing...
		}

		virtual Utils::Context Context()
		{
			utils::ref_count_ptr<utils::dispatcher> core_context;
			if (this->query_context(&core_context) == false)
				return Utils::Context(nullptr); // Empty context

			return Utils::Context(core_context);
		}

		virtual SubscriptionParams Subscribe(const Row& row, const std::function<void(const Database::RowData&)>& func)
		{
			utils::ref_count_ptr<core::database::row_interface> core_row;
			row.UnderlyingObject(&core_row);

			return this->subscribe(core_row, func);
		}

		template <typename V>
		SubscriptionParams SubGet(const Row& row, const std::function<void(const Database::RowData&)>& func, V& val)
		{		
			// Subscription should be handled before reading the current value
			utils::scope_guard reader([&]()
			{
				row.Read<V>(val);
			});

			return Subscribe(row, func);
		}

		virtual SubscriptionParams Subscribe(const Table& table, const AnyKey& rowKey, const std::function<void(const Database::RowData&)>& func)
		{
			return Subscribe(table[rowKey], func);
		}

		virtual SubscriptionParams Subscribe(const DataSet& dataset, const AnyKey& tableKey, const AnyKey& rowKey, const std::function<void(const Database::RowData&)>& func)
		{
			return Subscribe(dataset[tableKey], rowKey, func);
		}

		virtual bool Unsubscribe(const Row& row, SubscriptionToken token)
		{
			utils::ref_count_ptr<core::database::row_interface> core_row;
			row.UnderlyingObject(&core_row);

			return this->unsubscribe(core_row, token);
		}

		virtual bool Unsubscribe(const Table& table, const AnyKey& rowKey, SubscriptionToken token)
		{
			return Unsubscribe(table[rowKey], token);
        }

		virtual bool Unsubscribe(const DataSet& dataset, const AnyKey& tableKey, const AnyKey& rowKey, SubscriptionToken token)
		{
			return Unsubscribe(dataset[tableKey], rowKey, token);
		}

		virtual TableSubscriptionParams SubscribeTable(const Table& table, const std::function<void(const Database::RowData&)>& func)
		{
			utils::ref_count_ptr<core::database::table_interface> core_table;
			table.UnderlyingObject(&core_table);
			return this->subscribe_table(core_table, func);
		}

		Utils::TimerRegistrationParams RegisterTimer(double interval, const std::function<void()>& func, unsigned int invocationCount = 0)
		{
			return Context().RegisterTimer(interval, func, invocationCount);
		}

		bool UnregisterTimer(Utils::TimerToken token)
		{
			return Context().UnregisterTimer(token);
		}

		bool UnregisterTimer(const Utils::TimerRegistrationParams registrationParams)
		{
			return Context().UnregisterTimer(registrationParams);
		}	
	
	private:
        // Hide core API

        virtual bool suspended() override
        {
            return utils::database::database_dispatcher_base<T>::suspended();
        }

        virtual void suspend() override
        {
            utils::database::database_dispatcher_base<T>::suspend();
        }

        virtual void resume() override
        {
            utils::database::database_dispatcher_base<T>::resume();
        }

        virtual utils::database::subscription_params subscribe(core::database::row_interface* row, const std::function<void(const utils::database::row_data&)>& func) override
        {
            return utils::database::database_dispatcher_base<T>::subscribe(row, func);
        }

        virtual utils::database::subscription_params subscribe(core::database::table_interface* table, const utils::database::buffered_key& rowKey, const std::function<void(const utils::database::row_data&)>& func) override
        {
            return utils::database::database_dispatcher_base<T>::subscribe(table, rowKey, func);
        }

        virtual utils::database::subscription_params subscribe(core::database::dataset_interface* dataset, const utils::database::buffered_key& tableKey, const utils::database::buffered_key& rowKey, const std::function<void(const utils::database::row_data&)>& func) override
        {
            return utils::database::database_dispatcher_base<T>::subscribe(dataset, tableKey, rowKey, func);
        }

        virtual bool unsubscribe(core::database::row_interface* row, utils::database::subscription_token token) override
        {
            return utils::database::database_dispatcher_base<T>::unsubscribe(row, token);
        }

        virtual bool unsubscribe(core::database::table_interface* table, const utils::database::buffered_key& rowKey, utils::database::subscription_token token) override
        {
            return utils::database::database_dispatcher_base<T>::unsubscribe(table, rowKey, token);
        }

        virtual bool unsubscribe(core::database::dataset_interface* dataset, const utils::database::buffered_key& tableKey, const utils::database::buffered_key& rowKey, utils::database::subscription_token token) override
        {
            return utils::database::database_dispatcher_base<T>::unsubscribe(dataset, tableKey, rowKey, token);
        }

        virtual bool query_context(utils::dispatcher** context) override
        {
            return utils::database::database_dispatcher_base<T>::query_context(context);
        }

        // Delegate virtual calls to ModernAPI

		virtual void init() override
		{
			Init();
			utils::database::database_dispatcher_base<T>::init();
		}

		virtual void start() override
		{
			Start();
			utils::database::database_dispatcher_base<T>::start();
		}

		virtual void started() override
		{
			Started();
			utils::database::database_dispatcher_base<T>::started();
		}

		virtual void stop() override
		{
			Stop();
			utils::database::database_dispatcher_base<T>::stop();
		}

		virtual void stopped() override
		{
			Stopped();
			utils::database::database_dispatcher_base<T>::stopped();
		}
	};

	class Dispatcher : public DispatcherBase<core::application::runnable_interface>
	{
	public:
		Dispatcher(const char* name = nullptr, bool startSuspended = false) :
			DispatcherBase(name, startSuspended)
		{
		}

		Dispatcher(const Utils::Context& context) :
			DispatcherBase(context)
		{
		}		
	};
	
	class Subscriber : public Common::CoreObjectWrapper<utils::database::subscriber>
	{
	public:
		Subscriber()
		{
			// Empty Subscriber
		}

		Subscriber(std::nullptr_t)
		{
			// Empty Subscriber
		}

		Subscriber(utils::database::subscriber* subscriber) :
			Common::CoreObjectWrapper<utils::database::subscriber>(subscriber)
		{
		}

		Subscriber(utils::database::database_dispatcher_interface* dispatcher) :
			Common::CoreObjectWrapper<utils::database::subscriber>(
				utils::make_ref_count_ptr<utils::database::subscriber>(dispatcher))
		{
			if (dispatcher == nullptr)
				throw std::invalid_argument("dispatcher");
		}	

		virtual Utils::Context Context()
		{
			ThrowOnEmpty("Database::Dispacther");

			utils::ref_count_ptr<utils::dispatcher> core_context;
			if (m_core_object->query_context(&core_context) == false)
				return Utils::Context(nullptr); // Empty context

			return Utils::Context(core_context);
		}

		virtual SubscriptionParams Subscribe(const Row& row, const std::function<void(const Database::RowData&)>& func)
		{
			ThrowOnEmpty("Database::Dispacther");

			utils::ref_count_ptr<core::database::row_interface> core_row;
			row.UnderlyingObject(&core_row);

			return m_core_object->subscribe(core_row, func);
		}

		virtual TableSubscriptionParams SubscribeTable(const Table& table, const std::function<void(const Database::RowData&)>& func)
		{
			ThrowOnEmpty("Database::Dispacther");

			utils::ref_count_ptr<core::database::table_interface> core_table;
			table.UnderlyingObject(&core_table);

			return m_core_object->subscribe_table(core_table, func);

		}
		virtual SubscriptionParams Subscribe(const Table& table, const AnyKey& rowKey, const std::function<void(const Database::RowData&)>& func)
		{
			return Subscribe(table[rowKey], func);
		}

		virtual SubscriptionParams Subscribe(const DataSet& dataset, const AnyKey& tableKey, const AnyKey& rowKey, const std::function<void(const Database::RowData&)>& func)
		{
			return Subscribe(dataset[tableKey], rowKey, func);
		}

		virtual bool Unsubscribe(const Row& row, SubscriptionToken token)
		{
			ThrowOnEmpty("Database::Dispacther");

			utils::ref_count_ptr<core::database::row_interface> core_row;
			row.UnderlyingObject(&core_row);

			return m_core_object->unsubscribe(core_row, token);
		}

		virtual bool Unsubscribe(const Table& table, const AnyKey& rowKey, SubscriptionToken token)
		{
			return Unsubscribe(table[rowKey], token);
		}

		virtual bool Unsubscribe(const DataSet& dataset, const AnyKey& tableKey, const AnyKey& rowKey, SubscriptionToken token)
		{
			return Unsubscribe(dataset[tableKey], rowKey, token);
		}

		Utils::TimerRegistrationParams RegisterTimer(double interval, const std::function<void()>& func, unsigned int invocationCount = 0)
		{
			return Context().RegisterTimer(interval, func, invocationCount);
		}

		virtual bool UnregisterTimer(Utils::TimerToken token)
		{
			return Context().UnregisterTimer(token);
		}

		virtual bool UnregisterTimer(const Utils::TimerRegistrationParams registrationParams)
		{
			return Context().UnregisterTimer(registrationParams);
		}
	};

	

	static constexpr int ARRAY_UINT8_MAX_SIZE = 2048; //maximum size of byte array if larger than this nummber it will be treated as a buffer

	/// Schema is a class that allow Loading and parsing the Data set XML created by the DebugEnvironement code parser.
	/// This allows automatic creation of Rows and Table in the database, and parser for the data structure used by the data base
	/// @date	29/10/2018
	class Schema
	{
	private:
		struct LimitsData
		{
			std::string min;
			std::string max;
			std::string def;
		};
		/// A database row structure.
		/// @date	01/12/2018
		struct DBRowStruct
		{
			int dbIndex;
			int dbEntry;
			std::string DBEntryName;
			std::string rowDescription;
			TypeEnum type;
			std::string typeMame;
			LimitsData limits;
			size_t size;
			Parsers::BinaryMetaData parser;
		};

		/// A database table structure.
		/// @date	10/11/2018
		struct DBTableStruct
		{
			int dbIndex;
			std::string dbName;
			std::string dbPath;
			std::vector<DBRowStruct> dbRows;
		};

		struct StructData
		{
			std::string origfield_Name;
			std::string field_Name;
			std::string typeName;
			TypeEnum type;
			bool isArray;
			int count;
			size_t size;
			LimitsData limits;
		};
		struct StructInfo
		{
			std::string structName;
			std::vector<StructData> data;
		};

		Parsers::BinaryMetadataStore m_store;
		using DBTableMap = std::unordered_map<std::string, DBTableStruct>;
		std::unordered_map<std::string,DBTableStruct> m_DBSchema;
		std::unordered_map<std::string,Parsers::EnumData> m_enums;
		std::unordered_map<std::string,StructInfo> m_structs;
		bool m_verbose;

		Logging::Logger SCHEMA_LOG;
		
		Schema(Parsers::BinaryMetadataStore& store,bool verbose):m_store(store),
			m_verbose(verbose)
		{
			Logging::Severity sevirity  = Logging::Severity::WARNING;
			if (verbose)
			{
				sevirity = Logging::Severity::DEBUG;
			}
						
			SCHEMA_LOG = Core::Framework::CreateLogger("Schema Logger", sevirity);
		}
		
		TypeEnum GetType(const char* typeName, Files::XmlElement& element,std::string& enumName)
		{
			TypeEnum type = utils::types::get_type(typeName);
			enumName = "";
			if (type == TypeEnum::UNKNOWN ||
				type == TypeEnum::COMPLEX)
			{//Verify whether it might be enum
				std::string typeStr = typeName;
				if (typeStr.find_last_of("Struct") == std::string::npos ||
					typeStr.find("::") == std::string::npos)
				{
					Files::XmlElement enumType = element.QueryChild("Enumeration");
					if (false == enumType.Empty() && enumType.Value() != nullptr && enumType.Value()[0] != '\0')
					{
						enumName = enumType.Value();
						return TypeEnum::ENUM;
					}
					else
					{
						//if the first letter is digit, consider this as bitwise
						if (isdigit(typeStr[0]))
						{
							return TypeEnum::BITMAP;
						}

						std::vector<std::string> typeArr = split_string(typeStr, ":");
						if (typeArr.size() == 2)
						{
							type = utils::types::get_type(typeArr[0].c_str());
							if (utils::types::is_simple_type(type) ||
								type == TypeEnum::ENUM)
							{
								return TypeEnum::BITMAP;
							}
						}
					}
				}
				
			}
			else if (utils::types::is_simple_type(type))
			{
				Files::XmlElement enumType = element.QueryChild("Enumeration");
				if (false == enumType.Empty() && enumType.Value() != nullptr && enumType.Value()[0] != '\0')
				{
					enumName = enumType.Value();
					return TypeEnum::ENUM;
				}

				if (type == TypeEnum::CHAR)
				{
					Files::XmlElement countType = element.QueryChild("Size");
					if (false == countType.Empty() && countType.Value() != nullptr && countType.Value()[0] != '\0')
					{
						int size = countType.ValueAsInt(-1);
						if (size > static_cast<int>(sizeof(char)))
							type = TypeEnum::STRING;
					}
				}
			}

			return type;
		}

		bool GetBitmapSizes(const char* str, size_t& numOfBits, size_t& sizeInBytes)
		{
			return utils::types::get_bitmap_sizes(str, numOfBits, sizeInBytes);
		}

		std::string BuildStringForXML(const std::string &tagType, bool ignreCase = false)
		{
			std::vector<std::string> split_str = split_string(tagType, "::");
			if (split_str.size() == 1)
			{
				if (ignreCase)
					return ToLowerCase(tagType);
				else
					return tagType;
			}
			//check if first string is an h file reference
			
			//search for space
			size_t i;
			for (i = 0; i < split_str.size() - 1; i++)
			{
				size_t index = split_str[i].find_last_of(" ");
				if (index != std::string::npos)
				{
					split_str[i].erase(index);
					split_str[i] += "_x0020_";
				}
			}
			std::string transformedString;
			for (i = 0; i < split_str.size() - 1; i++)
			{
				transformedString += split_str[i] + "_x003A__x003A_"; //_x003A__x003A_ = :: in XML
			}
			transformedString += split_str[split_str.size() - 1];
			if (ignreCase)
			{
				return ToLowerCase(transformedString);
			}
			else
				return transformedString;
		}

		std::string BuildRegularStringName(const std::string &tagType, bool ignreCase = false)
		{
			static std::string dbSpace = "_x0020_";
			std::vector<std::string> split_str = split_string(tagType, "_x003A__x003A_");
			if (split_str.size() == 1)
				return tagType;
			//check if first string is an h file reference
			if (ignreCase)
			{
				//if it is an h file - this should reduce to lowercase
				std::transform(split_str[0].begin(), split_str[0].end(), split_str[0].begin(), [](unsigned char c) {
					std::locale loc;
					return std::tolower(static_cast<char>(c), loc);
				});
			}
			//search for space
			size_t i;
			for (i = 0; i < split_str.size() - 1; i++)
			{
				
				size_t index = split_str[i].rfind(dbSpace.c_str());
				if (index != std::string::npos)
				{
					split_str[i].erase(index,dbSpace.size());
					split_str[i] += " ";
				}
			}
			std::string transformedString;
			for (i = 0; i < split_str.size() - 1; i++)
			{
				transformedString += split_str[i] + "::"; //_x003A__x003A_ = :: in XML
			}
			transformedString += split_str[split_str.size() - 1];

			return transformedString;
		}

		bool IsTable(const std::string& name)
		{
			auto it = m_DBSchema.find(name);
			if (it == m_DBSchema.end())
				return false;
			else
				return true;
		}

		bool IsEnum(const std::string& name)
		{
			auto it = m_enums.find(ToLowerCase(name));
			if (it == m_enums.end())
				return false;
			else
				return true;
		}

		bool IsStruct(const std::string &name)
		{
			auto it = m_structs.find(ToLowerCase(name));
			if (it == m_structs.end())
				return false;
			else
				return true;
		}
		
		int IndexOfArray(const std::string& name, bool& inexZeroExist, std::string &arrayName)
		{
			int index = -1;
			std::vector<std::string> splits = split_string(name, "_");
			if (splits.size() >= 2)
			{
				bool has_only_digits = (splits[splits.size()-1].find_first_not_of("0123456789") == std::string::npos);
				if (has_only_digits)
				{
					index = std::stoi(splits[splits.size() - 1]);
					
					if (index == 0)
					{
						inexZeroExist = true;
					}

					if (inexZeroExist)
					{
						arrayName = name.substr(0, name.find_last_of("_"));
						return index;
					}
					else
					{
						index = -1;
					}
				}
			}
			inexZeroExist = false;
			arrayName = name;
			return index;
		}

		size_t GetSizefromCharBufString(const char* charbuffstring)
		{
			std::vector<std::string> split_str = split_string(charbuffstring, "[");
			size_t size = 0;
			if (split_str.size() == 2)
			{
				split_str[1].erase(split_str[1].length() - 1, 1);
				size = static_cast<size_t>(std::stoi(split_str[1]));
			}
			return size;
		}

		bool CheckIfArray(size_t size, size_t typeSize)
		{
			if (size > typeSize && size % typeSize == 0)
				return true;

			return false;
		}

		bool CheckAndGetRawVal(const std::string& val, TypeEnum type, const std::string& enumName, uint8_t data[core::parsers::VAL_SIZE])
		{
			int64_t numericVal = 0;
			std::string normalizedVal;
			Parsers::BinaryMetadataStore store;
			Parsers::EnumData enumInfo = store.Enum(enumName.c_str());

			if (enumInfo.Empty())
				return false;

			if (false == enumInfo.GetItemValueByName(val.c_str(), numericVal))
				return false;

#ifdef WIN32
			memcpy_s(data, core::parsers::VAL_SIZE, &numericVal, sizeof(numericVal));
#else
			std::memcpy(data, &numericVal, sizeof(numericVal));
#endif
			return true;
		}
		void ReadLimits(Files::XmlElement& element,LimitsData& limits)
		{
			
			Files::XmlElement child;
			child = element.QueryChild("min_value");
			if (false == child.Empty())
			{
				limits.min = child.Value();
			}

			child = element.QueryChild("max_value");
			if (false == child.Empty())
			{
				limits.max = child.Value();
			}

			child = element.QueryChild("defaultvalue");
			if (false == child.Empty())
			{
				limits.def = child.Value();
			}
		}
		Parsers::SimpleOptions GetOptions(LimitsData& limits, TypeEnum type, const std::string& typeName)
		{

			uint8_t raw_val[core::parsers::VAL_SIZE] = { 0 };
			utils::parsers::simple_options options;
			if (type == core::types::type_enum::STRING)
				return options;
			
			if (false == limits.min.empty())
			{
				if (utils::types::is_numeric(limits.min))
				{
					options.minval(limits.min.c_str(), type);
				}
				else if (type == core::types::type_enum::ENUM)
				{
					if (false == CheckAndGetRawVal(limits.min.c_str(), type, typeName, raw_val))
						throw std::runtime_error("value or enum was not found");

					options.raw_minval(raw_val, utils::types::sizeof_type(type));
				}
			}

			if (false == limits.max.empty())
			{
				if (utils::types::is_numeric(limits.max))
				{
					options.maxval(limits.max.c_str(), type);
				}
				else if (type == core::types::type_enum::ENUM)
				{
					if (false == CheckAndGetRawVal(limits.max.c_str(), type, typeName, raw_val))
						throw std::runtime_error("value or enum was not found");

					options.raw_maxval(raw_val, utils::types::sizeof_type(type));
				}
			}

			if (false == limits.def.empty())
			{
				if (utils::types::is_numeric(limits.def))
				{
					options.defval(limits.def.c_str(), type);
				}
				else if (type == core::types::type_enum::ENUM)
				{
					if (false == CheckAndGetRawVal(limits.def.c_str(), type, typeName, raw_val))
						throw std::runtime_error("value or enum was not found");

					options.raw_defval(raw_val, utils::types::sizeof_type(type));
				}
			}
			return options;
		}
		template<typename T> void AddSimple(const std::string& fieldName, size_t size,const Parsers::SimpleOptions& options,Parsers::BinaryMetaDataBuilder& parser)
		{
			if (size == sizeof(T))
			{
				parser.Simple<T>(fieldName.c_str(),options);
			}
			else
			{
				//Check if it is an array
				if (CheckIfArray(size, sizeof(T)))
				{
					parser.Array<T>(fieldName.c_str(), size,options);
				}
			}
		}
		void AddArray(Parsers::BinaryMetaDataBuilder& parser, std::string& name,size_t numOfElement,size_t typeSize,TypeEnum type,Parsers::SimpleOptions options ,std::string& typeName)
		{
			if (TypeEnum::COMPLEX == type)
			{
				if (name == "")
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Missing Name");
				parser.Array(name.c_str(), numOfElement, typeName.c_str());
			}
			else
			{
				if (name == "")
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Missing Name");
				if (TypeEnum::ENUM == type)
				{
					parser.Array(name.c_str(), numOfElement,typeSize, options,typeName.c_str());
				}
				else
					parser.Array(name.c_str(), type, numOfElement,options);
			}
		}
		Parsers::BinaryMetaData BuildParserMetadata(StructInfo& structInfo)
		{
			LOG_SCOPE(SCHEMA_LOG, structInfo.structName.c_str());
			Parsers::BinaryMetaDataBuilder parser;
			if (false == m_store.Empty())
			{
				parser = m_store.Metadata(structInfo.structName.c_str());
				if (false == parser.Empty())
				{
					return std::move(parser);
				}
				
			}
			
			parser = Parsers::BinaryMetaDataBuilder::Create();
			
			parser.BigEndian(false);
		
			std::string tempArrayName, arrayName, arrayTypeName, tempType;
			std::string fieldName, prevFieldName, enumName;
			Parsers::SimpleOptions options, prevOptions;
			
			size_t tempSize;

			for (auto& data : structInfo.data)
			{
				fieldName = data.field_Name;
				TypeEnum type = data.type;
				tempSize = data.size;
				fieldName = data.field_Name;
				tempType = data.typeName;
				options = GetOptions(data.limits, type, data.typeName);
				if (data.isArray)
				{
                    AddArray(parser, data.field_Name, static_cast<size_t>(data.count), data.size, type, options, data.typeName);
					continue;
				}
				
				switch (type)
				{
				case TypeEnum::UINT8:
				case TypeEnum::BYTE:
				case TypeEnum::BOOL:
				{
					AddSimple<uint8_t>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::INT8:
				case TypeEnum::CHAR:
				{
					if (tempSize == sizeof(char))
					{
						parser.Simple<int8_t>(fieldName.c_str(), options);
					}
					else
					{
						//Check if it is an array
						if (CheckIfArray(tempSize, sizeof(char)))
						{
							parser.String(fieldName.c_str(), tempSize,data.limits.def);
						}
					}
				}
				break;
				case TypeEnum::USHORT:
				case TypeEnum::UINT16:
				{
					AddSimple<uint16_t>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::SHORT:
				case TypeEnum::INT16:
				{
					AddSimple<int16_t>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::UINT32:
				{
					AddSimple<uint32_t>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::INT32:
				{
					AddSimple<int32_t>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::FLOAT:
				{
					AddSimple<float>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::DOUBLE:
				{
					AddSimple<double>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::INT64:
				{
					AddSimple<int64_t>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::UINT64:
				{
					AddSimple<uint64_t>(fieldName, tempSize, options, parser);
				}
				break;
				case TypeEnum::BITMAP:
				{
					size_t numOfBits = 0, size = 0;
					if (false == GetBitmapSizes(tempType.c_str(), numOfBits, size))
						throw std::runtime_error("Bitmap does not match pattern");

					parser.Bits(fieldName.c_str(), numOfBits, size);
					break;
				}
				case TypeEnum::ENUM:
				{
					parser.Enum(fieldName.c_str(), tempSize, data.typeName.c_str(), options);
				}
				break;
				case TypeEnum::STRING:
				{
					if(data.limits.def.empty())
						parser.String(fieldName.c_str(), tempSize);
					else
						parser.String(fieldName.c_str(), tempSize,data.limits.def);

				}
				break;
				case TypeEnum::COMPLEX:
				{
					// Check if inner struct
					Parsers::BinaryMetaData innserParser = m_store.Metadata(tempType.c_str());
					if (false == innserParser.Empty())
						parser.Complex(fieldName.c_str(), innserParser);
					else
						LOG_WARNING(SCHEMA_LOG) << "Missing complex type:" << tempType.c_str();
				}
				break;
				case TypeEnum::BUFFER:
					parser.Buffer(fieldName.c_str(), tempSize);
					break;
				default:
				{
					LOG_WARNING(SCHEMA_LOG) << "unexpected unknown type size:" << tempSize << " added to the parser";
					parser.Buffer(fieldName.c_str(), tempSize);
				}
				break;
				}
			}
			
			parser.Namely(structInfo.structName.c_str());
			return std::move(parser);

		}
		std::string ToLowerCase(const std::string& str)
		{
			std::string lowerCase = str;
			std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), [](unsigned char c) {
				std::locale loc;
				return std::tolower(static_cast<char>(c), loc);
			});
			return lowerCase;
		}
		bool BuildEnums()
		{
			Parsers::BinaryMetadataStore store;
			for (auto& enumData : m_enums)
			{
				if (enumData.second.Size() == 0)
				{
					LOG_ERROR(SCHEMA_LOG) << "enum data" << enumData.second.Name() << " is empty";
					return false;
				}
				store.SetEnum(enumData.second.Name(), enumData.second);
			}
			return true;
		}
		bool UpdateStack(std::stack<std::string>& stack, const StructInfo& structInfo)
		{
			bool isComplex = false;
			for (auto& data : structInfo.data)
			{
				if (data.type == TypeEnum::COMPLEX)
				{
					if (m_store.Metadata(data.typeName.c_str()).Empty())
					{
						stack.push(data.typeName);
						isComplex = true;
					}
				}

			}
			return isComplex;
		}
		bool BuildStructs()
		{
			std::stack<std::string> structStack;
			for (auto& it : m_structs)
			{
				bool isComplex=false;
				StructInfo& structInfo = it.second;
				structStack.push(structInfo.structName);
				isComplex = UpdateStack(structStack, structInfo);

				if (false == isComplex)
					BuildParserMetadata(it.second);
			}

			bool isComplex = false;
			std::string str, newStr;
			while (false == structStack.empty())
			{
				str = structStack.top();
				
				if (m_store.Metadata(str.c_str()).Empty())
				{
					newStr = BuildStringForXML(str, true);
					auto it = m_structs.find(newStr);
					if (it == m_structs.end())
					{
						if (m_verbose)
							Core::Console::ColorPrint(Core::Console::Colors::RED, "Failed to find: %d", newStr.c_str());
						return false;
					}

					isComplex = UpdateStack(structStack, it->second);
					if (false == isComplex)
					{
						BuildParserMetadata(it->second);
						structStack.pop();
					}

				}
				else
					structStack.pop();
			}
			return true;
		}
		bool PrepareMetaData(Files::XmlElement element, size_t numOfTables)
		{
			LOG_FUNC(SCHEMA_LOG);
			if (numOfTables == 0)
			{
				LOG_ERROR(SCHEMA_LOG) << "Error - number of Tables = 0";
				return false;
			}

			int i = 0;
			for (auto& child : element.Children())
			{
				if (i < static_cast<int>(numOfTables))
				{
					Files::XmlAttribute attr(child.QueryAttribute("msdata:Prefix"));
					DBTableStruct dbTable;
					dbTable.dbIndex = i;
					if (false == attr.Empty())
					{
						dbTable.dbPath = attr.Value();
					}
					else
						dbTable.dbPath = "";

					dbTable.dbName = child.QueryAttribute("name").Value();
					m_DBSchema.emplace(dbTable.dbName,dbTable);
					i++;
				}
				else
				{
					Files::XmlAttribute additionalAttr(child.QueryAttribute("msdata:TableId"));
					if (true == additionalAttr.Empty()) //might be enum Enum Metadata
					{
						Files::XmlElement internalElement =  child.QueryChild("xs:complexType").QueryChild("xs:sequence");
						auto fieldChild = internalElement.QueryChild((size_t)0);
						std::string field = fieldChild.QueryAttribute("name").Value();
						if(field != "Field")
							continue;

						std::string enumNameXml(child.QueryAttribute("name").Value());
						std::string enumName = BuildRegularStringName(enumNameXml);
						Parsers::EnumData  enumData = Parsers::EnumDataFactory::Create(enumName.c_str());
						m_enums.emplace(ToLowerCase(enumNameXml), enumData);
					}
					else //struct Metadata
					{
						std::string structNameXml(child.QueryAttribute("name").Value());
						std::string structName = BuildRegularStringName(structNameXml);
						auto it = m_structs.find(structName);
						if (it != m_structs.end())
						{
							LOG_ERROR(SCHEMA_LOG) << "Duplicate Struct Exist " << structName.c_str();
							if (m_verbose)
								Core::Console::ColorPrint(Core::Console::Colors::RED, "Duplicate Struct Exist - %s", structName.c_str());
							return false;
						}
						else
						{	
							StructInfo structInfo = {
								structName,
							{}
							};
							m_structs.emplace(ToLowerCase(structNameXml),
								structInfo);
						}
					}
				}
			}
			return true;
		}
		
		bool ReadMetadata(Files::XmlElement element)
		{
			std::string typeStr;
			TypeEnum type;
			int count = -1;
			
			std::string fieldName, enumName, origFieldName;
			std::unordered_map<std::string, Parsers::EnumData> enumDataMap;
			std::unordered_map<std::string, Parsers::EnumData> structDataMap;
			bool arryActive = false;
			bool wasActive = false;
			std::stack<StructData> structStack;
			std::unordered_map<std::string, StructInfo>::iterator structIt,prevIt;

			for (Files::XmlElement& child : element.Children())
			{
				if (IsTable(child.Name()))
				{ 
					auto it = m_DBSchema.find(child.Name());
					fieldName = child.QueryChild("Field_Name").Value();
					typeStr = child.QueryChild("Type").Value();
					Files::XmlElement comment = child.QueryChild("Comment");
					std::string idStr = child.QueryChild("Field_x0020_Id").Value();
					type = GetType(typeStr.c_str(), child, enumName);
                    size_t tempSize = static_cast<size_t>(std::stoi(child.QueryChild("Size").Value()));
					if (type == TypeEnum::ENUM)
					{
						auto typeEnumKey = m_enums.find(BuildStringForXML(enumName, true));
						if (typeEnumKey != m_enums.end())
						{
							typeStr = typeEnumKey->second.Name();
						}
						else
						{
							LOG_ERROR(SCHEMA_LOG) << "Enum" << enumName.c_str() << "not Found";
							return false;
						}
					}
					else if (type == TypeEnum::BUFFER)
					{
						tempSize = GetSizefromCharBufString(typeStr.c_str());
					}
					else if (false == utils::types::is_simple_type(type) && 
						type != TypeEnum::EMPTY_TYPE)
					{
						auto typeStructKey = m_structs.find(BuildStringForXML(typeStr, true));
						if (typeStructKey != m_structs.end())
						{
							typeStr = typeStructKey->second.structName;
						}
						else
						{
							LOG_ERROR(SCHEMA_LOG) << "Struct" << typeStr.c_str() << "not Found";
							return false;
						}

					}

					LimitsData limits;
					ReadLimits(child, limits);
					DBRowStruct entry =
					{
						it->second.dbIndex,
						std::stoi(child.QueryChild("Field_x0020_Id").Value()),
						fieldName,
						comment.Empty() ? "" : comment.Value(),
						type,
						typeStr,
						limits,
						tempSize,
						Parsers::BinaryMetaData()
					};

					it->second.dbRows.emplace_back(entry);
				}
				else if (IsStruct(child.Name()))
				{
					prevIt = structIt;
					structIt = m_structs.find(ToLowerCase(child.Name()));
					fieldName = child.QueryChild("Field_Name").Value();
					origFieldName = fieldName;
					typeStr = child.QueryChild("Type").Value();
					type = GetType(typeStr.c_str(), child, enumName);
                    size_t tempSize = static_cast<size_t>(std::stoi(child.QueryChild("Size").Value()));
					if (tempSize == 0 && 
						(utils::types::is_simple_type(type) || type == TypeEnum::ENUM) )
					{
						LOG_ERROR(SCHEMA_LOG) << "Struct:" << child.Name() << "With size = 0";
						return false;
					}
					if (type == TypeEnum::ENUM)
					{
						typeStr = enumName;
					}
					else if (type == TypeEnum::COMPLEX)
					{
						auto typeStructKey = m_structs.find(BuildStringForXML(typeStr, true));
						if (typeStructKey != m_structs.end())
						{
							typeStr = typeStructKey->second.structName;
						}
						else
						{
							LOG_ERROR(SCHEMA_LOG) << "Struct:" << typeStr.c_str() << " No Found";
							return false;
						}
					}
					else if (type == TypeEnum::BUFFER)
					{
						tempSize = GetSizefromCharBufString(typeStr.c_str());
					}

					LimitsData limits;
					ReadLimits(child, limits);
					Files::XmlElement countElement = child.QueryChild("count");
					if (false == countElement.Empty() && type != TypeEnum::BUFFER)
					{
						if (countElement.Value() != nullptr)
						{
							count = countElement.ValueAsInt(-1);
							if (count > ARRAY_UINT8_MAX_SIZE &&
								(type == TypeEnum::UINT8 || type == TypeEnum::BYTE))
							{
								type = TypeEnum::BUFFER;
                                tempSize = tempSize * static_cast<size_t>(count);
								count = 0;
							}
						}
					}
					else
					{
						wasActive = arryActive;
						count = IndexOfArray(fieldName, arryActive, fieldName);
						if (arryActive)
						{
							StructData arrayData;
							if (count == 0 && structStack.size() > 0)
							{
								wasActive = false;
								if (structStack.top().count == 1) //just one field with _0
								{
									structStack.top().field_Name = structStack.top().origfield_Name;
									structStack.top().isArray = false;
								}
								prevIt->second.data.emplace_back(structStack.top());
								structStack.pop();
								count = 0;
							}

							if (count == 0)
							{
								arrayData.origfield_Name = origFieldName;
								arrayData.field_Name = fieldName;
								arrayData.typeName = typeStr;
								arrayData.type = type;
								arrayData.isArray = true;
								arrayData.size = tempSize;
								arrayData.limits = limits;
								structStack.push(arrayData);
							}
							
							StructData& refArrayData = structStack.top();
							refArrayData.count = count + 1;
							
							continue;
						}
						if (wasActive && false == arryActive)
						{
							wasActive = false;
							if (structStack.top().count == 1) //just one field with _0
							{
								structStack.top().field_Name = structStack.top().origfield_Name;
								structStack.top().isArray = false;
							}
							prevIt->second.data.emplace_back(structStack.top());
							structStack.pop();
							count = 0;
							if(structStack.size() >0)
								continue;
						}
					}
					
					StructData data = {
						  origFieldName,
						  fieldName,
						  typeStr,
						  type,
						  count>0 ? true:false,
						  count>0 ? count:1,
						  tempSize,
						  limits
					};
					
					structIt->second.data.emplace_back(data);
				}
				else if (IsEnum(child.Name()))
				{
					auto it = m_enums.find(ToLowerCase(child.Name()));
					Files::XmlElement value = child.QueryChild("Value");
					if (value.Empty())
					{
						LOG_ERROR(SCHEMA_LOG) << "Error on getting value for enum:" << fieldName.c_str();
						return false;
					}

					int val = value.ValueAsInt((std::numeric_limits<int32_t>::max)() - 1);
					if (val == (std::numeric_limits<int32_t>::max)() - 1)
					{
						LOG_WARNING(SCHEMA_LOG) << "Might be value transformation error value from string for enum:" << fieldName.c_str() << " Value:" << value.Value();
					}

					it->second.AddNewItem(val, child.QueryChild("Field").Value());

				}
			}

			if (structStack.size() > 0)
			{
				wasActive = false;
				prevIt->second.data.emplace_back(structStack.top());
				structStack.pop();
				count = 0;
			}
			return true;
		}
		bool FinalizeTable(DBTableStruct& tableData)
		{
			
			Parsers::SimpleOptions options;
			for (auto& row : tableData.dbRows)
			{
				if (row.type == TypeEnum::COMPLEX)
				{
					
					row.parser  = m_store.Metadata(row.typeMame.c_str());
					if (row.parser.Empty())
					{
						LOG_ERROR(SCHEMA_LOG) << "Missing Struct Parser:" << row.typeMame.c_str();
						if (m_verbose)
							Core::Console::ColorPrint(Core::Console::Colors::RED, "Missing Struct Parser:%s", row.typeMame.c_str());
						return false;
					}
					row.size = row.parser.Size();
					
				}
				else
				{
					Parsers::BinaryMetaDataBuilder parser = Parsers::BinaryMetaDataBuilder::Create();
					options = GetOptions(row.limits, row.type, row.typeMame);
					if (utils::types::is_simple_type(row.type))
						parser.Simple(row.DBEntryName.c_str(), row.size, row.type, options);
					else if (row.type == TypeEnum::ENUM)
						parser.Enum(row.DBEntryName.c_str(), row.size, row.typeMame.c_str(), options);
					else if (row.type == TypeEnum::BUFFER)
						parser.Buffer(row.DBEntryName.c_str(), row.size);
					if(row.type != TypeEnum::EMPTY_TYPE)
						row.parser = parser;
				}
				
				
			}
			return true;
		}
		bool BuildDBSchema(Files::XmlFile file)
		{
			LOG_FUNC(SCHEMA_LOG);
			Files::XmlElement element = file.QueryElement("/NewDataSet/xs:schema/xs:element/xs:complexType/xs:choice");
			Files::XmlElement schemaElement = file.QueryElement("/NewDataSet/xs:schema/xs:element");
			if (schemaElement.Empty())
				return false;

			std::string numOfTablesStr = schemaElement.QueryAttribute("msdata:DATA_BASE_COUNTER").Value();
            size_t numOfTables = static_cast<size_t>(std::stoi(numOfTablesStr));

			Files::XmlElement rootElement = file.QueryElement("/NewDataSet");
			if (element.Empty())
				return false;
			
			LOG_INFO(SCHEMA_LOG) << "Calling PrepareMetaData";
			if (false == PrepareMetaData(element, numOfTables))
				return false;

			LOG_INFO(SCHEMA_LOG) << "Calling ReadMetadata";
			if (false == ReadMetadata(rootElement))
				return false;
			
			LOG_INFO(SCHEMA_LOG) << "Calling BuildEnums";
			if (false == BuildEnums())
				return false;
		   
			LOG_INFO(SCHEMA_LOG) << "Calling BuildStructs";
			if (false == BuildStructs())
				return false;

			for (auto& dbTable : m_DBSchema)
			{
				LOG_INFO(SCHEMA_LOG) << "Finalizing "<< dbTable.second.dbName.c_str();
				if (false == FinalizeTable(dbTable.second))
					return false;
			}
			
			return true;
		}

		/// Database schema 
		/// @date	10/11/2018
		/// @return	A reference to a std::vector<DBTableStruct>;
		DBTableMap& DBSchema()
		{
			return m_DBSchema;
		}
	public:
		/// Loads data base (table and Rows)schema from a file
		/// 		/// @date	09/12/2018
		/// @date	30/12/2018
		/// @param [in]		dataset	   	The dataset.
		/// @param 			dataSetPath	Full pathname of the data set file.
		/// @param [in]		store	   	The store of the metadata to allow reuse of already created metadata in different parsers.
		/// @return	True if it succeeds, false if it fails.
		static bool Load(DataSet &dataset, const std::string& dataSetPath, bool verbose = false, bool setDefaultsOnLoad = true)
		{
			Files::XmlFile xmlFile;
			Parsers::BinaryMetadataStore store;
			Schema schameLoader(store,verbose);
			LOG_FUNC(schameLoader.SCHEMA_LOG);
			if (dataset.Empty())
				return false;

			try
			{
				xmlFile = Files::XmlFile::Create(dataSetPath.c_str());
			}
			catch (std::exception &e)
			{
				if (verbose == true)
					Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Error Reading dataset file : %s, error %s\n", dataSetPath.c_str(), e.what());

				return false;
			}

			try
			{
				if (schameLoader.BuildDBSchema(xmlFile))
				{
					LOG_SCOPE(schameLoader.SCHEMA_LOG, "AddTable/Row");
					for (auto& entry : schameLoader.DBSchema())
					{
						Database::Table table;
						if (dataset.TryGet(entry.second.dbIndex, table) == false)
						{
							if (verbose)
								Core::Console::ColorPrint(Core::Console::Colors::MAGENTA, "Add Table ID=%d - Name: %s\n", entry.second.dbIndex, entry.second.dbName.c_str());

							dataset.AddTable(entry.second.dbIndex, entry.second.dbName.c_str(), entry.second.dbPath.c_str());
							table = dataset[entry.second.dbIndex];
						}
						else
						{
							if (verbose)
								Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Load schema failed: Table ID=%d - Name: %s Already exists\n", entry.second.dbIndex, entry.second.dbName.c_str());

							return false;
						}

						for (DBRowStruct& rowStruct : entry.second.dbRows)
						{
							Database::RowInfo info;
							info.type = rowStruct.type;
#ifdef _WIN32
							strcpy_s(info.name, sizeof(info.name), rowStruct.DBEntryName.c_str());
							strcpy_s(info.description, sizeof(info.description), rowStruct.rowDescription.c_str());
							strcpy_s(info.type_name, sizeof(info.type_name), rowStruct.typeMame.c_str());
#else
							std::strcpy(info.name, rowStruct.DBEntryName.c_str());
							std::strcpy(info.description, rowStruct.rowDescription.c_str());
							std::strcpy(info.type_name, rowStruct.typeMame.c_str());
#endif						

							Database::Row row;
							if (table.TryGet(rowStruct.dbEntry, row) == false)
							{
								if (verbose)
									Core::Console::ColorPrint(Core::Console::Colors::MAGENTA, "     Add Row ID=%d - Name: %s\n", rowStruct.dbEntry, rowStruct.DBEntryName.c_str());
								
								table.AddRow(rowStruct.dbEntry, rowStruct.size, info, rowStruct.parser);
								row = table[rowStruct.dbEntry];

								//if the parser is not empty - set it to write the default values into the database
								if (setDefaultsOnLoad)
								{
									row.Reset();
								}
							}
							else
							{
								if (verbose)
									Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Load schema failed: Row ID=%d - Name: %s Already exists\n", rowStruct.dbEntry, rowStruct.DBEntryName.c_str());

								return false;
							}
						}
					}

					return true;
				}

				return false;
			}
			catch (std::exception& e)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Failed to Load Schema - %s", e.what());
				return false;
			}
		}		
	};	

	class DataBinder : public Common::CoreObjectWrapper<utils::database::data_binder>
	{
	public:
		DataBinder(std::nullptr_t) :
			Common::CoreObjectWrapper<utils::database::data_binder>()
		{
			// Empty
		}

		DataBinder(utils::database::data_binder* core_object) :
			Common::CoreObjectWrapper<utils::database::data_binder>(core_object)
		{
		}

		DataBinder() :
			DataBinder(utils::make_ref_count_ptr<utils::database::data_binder>())
		{
		}

        DataBinder(const Utils::Context& context) :
            DataBinder(utils::make_ref_count_ptr<utils::database::data_binder>(static_cast<utils::dispatcher*>(context)))
        {
        }

		void Bind(const Database::Row& source, const Database::Row& target, bool copyInitValue = false, bool forceReport = true)
		{
			if (m_core_object->bind(static_cast<core::database::row_interface*>(source),
									static_cast<core::database::row_interface*>(target),
									copyInitValue,
									forceReport) == false)
				{
					throw std::runtime_error("bind operation failed");
				}
		}

		void Bind(const Database::Table& source, const Database::Table& target, bool copyInitValue = false, bool forceReport = true)
		{
			Database::Table targetTable(target);

			// Check empty objects.
			if (source.Empty() || target.Empty())
			{
				throw std::runtime_error("bind operation failed, empty objects");
			}

			// Check databases are the same size.
			if (source.Size() != target.Size())
			{
				throw std::runtime_error("bind operation failed, database sizes are different");
			}

			// Database Bind operation
			size_t index = 0;
			for (auto& row : source)
			{
				m_core_object->bind(static_cast<core::database::row_interface*>(row),
									static_cast<core::database::row_interface*>(targetTable.GetAt(index)),
									copyInitValue,
									forceReport);

				index++;
			}
		}
	};
}
