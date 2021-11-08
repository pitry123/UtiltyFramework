#pragma once
#include <core/database.h>
#include <core/application.h>
#include <utils/disposable_base.hpp>
#include <utils/disposable_ptr.hpp>
#include <utils/callback_handler.hpp>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/thread_safe_object.hpp>
#include <utils/signal.hpp>
#include <utils/dispatcher.hpp>
#include <utils/ref_count_object_pool.hpp>
#include <utils/buffer_allocator.hpp>
#include <utils/types.hpp>
#include <core/parser.h>

#include <unordered_map>
#include <vector>
#include <exception>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <list>

namespace utils
{
	namespace database
	{
		static constexpr size_t BUFFER_POOL_BASE_SIZE = 256;
		static constexpr size_t ACTIONS_POOL_BASE_SIZE = 64;

		using subscription_token = utils::signal_token;		
		static constexpr subscription_token subscription_token_undefined = utils::signal_token_undefined;

		struct buffered_key : public core::database::key
		{
			buffered_key(const void* buffer, size_t size)
			{
				if (buffer == nullptr)
					throw std::invalid_argument("buffer");

				if (size == 0)
					throw std::invalid_argument("size");

				if (size > sizeof(decltype(data)))
					throw std::runtime_error("key size exceeds MAX_KEY_SIZE");

				length = size;
				std::memcpy(data, buffer, length);
			}

			buffered_key(const core::database::key& key)
			{
				if (key.length == 0 || key.length > sizeof(decltype(data)))
					throw std::invalid_argument("key");

				length = key.length;
				std::memcpy(data, key.data, length);
			}
		};
		
		class row_data
		{
		private:
			 utils::ref_count_ptr<core::database::row_interface> m_row;
			buffered_key m_key;
			size_t m_size;
			const void* m_buffer;
			

			row_data(const row_data& other) = delete;       // non construction-copyable
			row_data& operator=(const row_data&) = delete;	// non copyable				
			row_data(const row_data&& other) = delete;      // non construction-movable
			row_data& operator=(const row_data&&) = delete;	// non movable				

		public:
			row_data(core::database::row_interface* row, size_t size, const void* buffer) :
				m_row(row),
				m_key(row->key()),
				m_size(size),
				m_buffer(buffer)
			{
			}

			bool query_row(core::database::row_interface** row) const
			{
				if (row == nullptr)
					return false;

				*row = m_row;
				(*row)->add_ref();

				return true;
			}
			const buffered_key& key() const
			{
				return m_key;
			}

			size_t data_size() const
			{
				return m_size;
			}

			const void* buffer() const
			{
				return m_buffer;
			}

			void read(void* buffer, size_t size) const
			{
				if (buffer == nullptr)
					throw std::invalid_argument("buffer");

				if ((size == 0 || size > m_size)&& m_row->info().type != core::types::EMPTY_TYPE)
					throw std::invalid_argument("size");

				std::memcpy(buffer, m_buffer, size);
			}

			template <typename T>
			void read(T& val) const
			{
				if (m_size < sizeof(T))
				{
					std::string name = m_row->info().name;
					core::types::type_enum type = utils::types::get_type<T>();
					std::stringstream str;
					str << "Reading size mismatch Read Size: "<<sizeof(T)<<" Row Size:"<<m_size << " with Type:" << type << " While Row Type:" << m_row->info().type;
					if (!name.empty())
						str << " Row Name:" << name;
					throw std::runtime_error(str.str().c_str());
				}

				val = *(static_cast<const T*>(m_buffer));
			}

			template <typename T>
			const T& read() const
			{
				if (m_size < sizeof(T))
				{
					std::string name = m_row->info().name;
					std::string type_name = utils::types::get_type_name<T>();
					std::stringstream str;
					str << "Reading size mismatch Read Size: " << sizeof(T) << " Row Size:" << m_size << " with Type:" << type_name << " While Row Type:" << m_row->info().type;
					if (!name.empty())
						str << " Row Name:" << name;
					throw std::runtime_error(str.str().c_str());
				}

				return *(static_cast<const T*>(m_buffer));
			}
		};

		class smart_row_callback :
			public utils::ref_count_base<core::database::row_callback_interface>
		{
		public:
			utils::signal<smart_row_callback, const row_data&> data_changed;
						
			virtual void on_data_changed(core::database::row_interface* row, size_t size, const void* buffer) override
			{
				row_data data(row, size, buffer);
				data_changed(data);
			}
		};

		class smart_table_callback :
			public utils::ref_count_base<core::database::table_callback_interface>
		{
		public:
			utils::signal<smart_table_callback, core::database::row_interface*> row_added;
			utils::signal<smart_table_callback, core::database::row_interface*> row_removed;

			virtual void on_row_added(core::database::row_interface* row) override
			{
				row_added(row);
			}

			virtual void on_row_removed(core::database::row_interface* row) override
			{
				row_removed(row);
			}
		};

		class smart_dataset_callback :
			public utils::ref_count_base<core::database::dataset_callback_interface>
		{
		public:
			utils::signal<smart_dataset_callback, core::database::table_interface*> table_added;
			utils::signal<smart_dataset_callback, core::database::table_interface*> table_removed;

			virtual void on_table_added(core::database::table_interface* table) override
			{
				table_added(table);
			}

			virtual void on_table_removed(core::database::table_interface* table) override
			{
				table_removed(table);
			}
		};

		template <typename T>
		class row_base :
			public utils::disposable_base<T>
		{
		private:
			core::database::key m_key;
			utils::disposable_ptr<core::database::table_interface> m_parent;
			core::database::row_info m_info;
			utils::ref_count_ptr<core::parsers::binary_metadata_interface> m_parser_metadata;
			utils::ref_count_ptr<utils::database::smart_table_callback> m_table_callback;

			utils::callback_handler<core::database::row_callback_interface> m_row_callback_handler;

		protected:
			row_base(const core::database::key& key, core::database::table_interface* parent, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser_metadata) :
				m_key(key),
				m_parent(parent),
				m_info(info),
				m_parser_metadata(parser_metadata)
			{
				
				if (parent != nullptr)
				{
					m_table_callback = utils::make_ref_count_ptr<utils::database::smart_table_callback>();
					m_table_callback->row_removed += [this](core::database::row_interface* row)
					{
						if (row == this)
						{
							utils::ref_count_ptr<core::database::table_interface> table;
							if (query_parent(&table) == true)
								table->unsubscribe_callback(m_table_callback);

							m_parent.reset();
						}
					};

					parent->subscribe_callback(m_table_callback);
				}
			}			

			void raise_callbacks(size_t size, const void* buffer) 
			{
				m_row_callback_handler.raise_callbacks([&](core::database::row_callback_interface* callback)
				{
					callback->on_data_changed(this, size, buffer);
				});
			}			

		public:
			virtual ~row_base()
			{
				m_row_callback_handler.clear();

				utils::ref_count_ptr<core::database::table_interface> table;
				if (query_parent(&table) == true)
					table->unsubscribe_callback(m_table_callback);

				m_parent.reset();				
			}

			virtual core::database::key key() const override
			{
				return m_key;
			}			

			virtual bool query_parent(core::database::table_interface** parent) const override
			{
				return m_parent.lock(parent);
			}

			virtual bool subscribe_callback(core::database::row_callback_interface* callback) override
			{
				return m_row_callback_handler.add_callback(callback);
			}

			virtual bool unsubscribe_callback(core::database::row_callback_interface* callback) override
			{
				return m_row_callback_handler.remove_callback(callback);
			}

			const core::database::row_info& info() const override
			{
				return m_info;
			}

			bool query_parser_metadata(core::parsers::binary_metadata_interface** parser_metadata) const override
			{
				if (m_parser_metadata == nullptr)
					return false;
				if (parser_metadata == nullptr)
					return false;

				*parser_metadata = m_parser_metadata;
				(*parser_metadata)->add_ref();
				return true;
				 
			}
			bool check_and_write_bytes(const void* buffer, size_t size, bool force_report, uint8_t priority) override
			{
				if (m_parser_metadata == nullptr)
					return false;
				utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;

				if(false == m_parser_metadata->create_parser(&parser))
					return false; //no parser, therefore check is not available

				if (size != parser->buffer_size())
					return false;

				if (false == parser->parse(buffer, size))
					return false;

				if (false == parser->validate())
					return false;
				core::database::row_interface* row = reinterpret_cast<core::database::row_interface*>(this);
				return row->write_bytes(buffer, size, force_report, priority);

			}

		};

		template <typename T>
		class table_base :
			public utils::disposable_base<T>
		{
		private:
			core::database::key m_key;
			utils::disposable_ptr<core::database::dataset_interface> m_parent;
			std::string m_name;
			std::string m_description;
			utils::ref_count_ptr<utils::database::smart_dataset_callback> m_dataset_callback;

			using rows_map = std::unordered_map<core::database::key, utils::ref_count_ptr<core::database::row_interface>, core::database::key_hash>;
			mutable utils::thread_safe_object<rows_map> m_rows;

			utils::callback_handler<core::database::table_callback_interface> m_table_callback_handler;
			
			// Guarded by m_rows mutex
			std::vector<utils::ref_count_ptr<core::database::row_callback_interface>> m_data_callbacks;

			void clear_rows()
			{
				m_rows.use([&](rows_map& rows)
				{
					for (auto& pair : rows)
					{
						for (auto& callback : this->m_data_callbacks)
							pair.second->unsubscribe_callback(callback);

						m_table_callback_handler.raise_callbacks([&](core::database::table_callback_interface* callback)
						{
							callback->on_row_removed(pair.second);
						});
					}

					rows.clear();
				});
			}

		protected:
			table_base(
				const core::database::key& key,
				core::database::dataset_interface* parent,
				const char* name,
				const char* description) :

				m_key(key),
				m_parent(parent),
				m_name(name == nullptr ? "":name),
                m_description(description == nullptr ? "":description)
			{
				if (parent != nullptr)
				{
					m_dataset_callback = utils::make_ref_count_ptr<utils::database::smart_dataset_callback>();					
					m_dataset_callback->table_removed += [this](core::database::table_interface* table)
					{
						if (table == this)
						{
							utils::ref_count_ptr<core::database::dataset_interface> dataset;
							if (query_parent(&dataset) == true)
								dataset->unsubscribe_callback(m_dataset_callback);

							m_parent.reset();
						}
					};

					parent->subscribe_callback(m_dataset_callback);
				}
			}

			virtual bool create_row(const core::database::key& key, size_t data_size, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser, core::database::row_interface** row) = 0;

		public:
			virtual ~table_base()
			{
				clear_rows();
				m_table_callback_handler.clear();

				utils::ref_count_ptr<core::database::dataset_interface> dataset;
				if (query_parent(&dataset) == true)
					dataset->unsubscribe_callback(m_dataset_callback);

				m_parent.reset();
			}

			virtual core::database::key key() const override
			{
				return m_key;
			}

			virtual const char* name() const override
			{
				if (m_name.empty())
					return nullptr;

				return m_name.c_str();
			}

			virtual const char* description() const override
			{
				if (m_name.empty())
					return nullptr;

				return m_description.c_str();
			}

			virtual bool query_parent(core::database::dataset_interface** parent) const override
			{
				return m_parent.lock(parent);
			}

			virtual size_t size() const override
			{
				return m_rows.use<size_t>([&](rows_map& rows)
				{
					return rows.size();
				});
			}

			virtual bool add_row(const core::database::key& key, size_t data_size) override
			{
				core::database::row_info info = { core::types::type_enum::UNKNOWN, "", "", "" };
				return add_row(key, data_size, info, nullptr);
			}

			virtual bool add_row(const core::database::key& key, size_t data_size, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser) override
			{
				if (data_size == 0 && info.type != core::types::EMPTY_TYPE)
					return false;

				return m_rows.use<bool>([&](rows_map& rows)
				{
					auto it = rows.find(key);
					if (it != rows.end())
						return false;

					utils::ref_count_ptr<core::database::row_interface> row;
					if (create_row(key, data_size,info,parser, &row) == false)
						return false;

					rows.emplace(row->key(), row);

					m_table_callback_handler.raise_callbacks([&](core::database::table_callback_interface* callback)
					{
						callback->on_row_added(row);
					});

					for (auto& callback : this->m_data_callbacks)
						row->subscribe_callback(callback);

					return true;
				});
			}

			virtual bool remove_row(const core::database::key& key, core::database::row_interface** removed_row) override
			{
				return m_rows.use<bool>([&](rows_map& rows)
				{
					auto it = rows.find(key);
					if (it == rows.end())
						return false;

					utils::scope_guard eraser([&]()
					{
						for (auto& callback : this->m_data_callbacks)
							it->second->unsubscribe_callback(callback);

						m_table_callback_handler.raise_callbacks([&](core::database::table_callback_interface* callback)
						{
							callback->on_row_removed(it->second);
						});

						rows.erase(it);
					});

					if (removed_row != nullptr)
					{
						*removed_row = it->second;
						(*removed_row)->add_ref();
					}

					return true;
				});
			}

			virtual bool query_row(const core::database::key& key, core::database::row_interface** row) const override
			{
				if (row == nullptr)
					return false;

				return m_rows.use<bool>([&](rows_map& rows)
				{
					auto it = rows.find(key);
					if (it == rows.end())
						return false;

					*row = it->second;
					(*row)->add_ref();

					return true;
				});
			}

			virtual bool query_row_by_index(size_t index, core::database::row_interface** row) const override
			{
				if (row == nullptr)
					return false;

				return m_rows.use<bool>([&](rows_map& rows)
				{
					if (index >= rows.size())
						return false;

					utils::ref_count_ptr<core::database::row_interface> instance;
					size_t counter = 0;
					for (auto& key_val : rows)
					{
						if (counter == index)
						{
							instance = key_val.second;
							break;
						}

						++counter;
					}

					if (instance == nullptr)
						return false;

					*row = instance;
					(*row)->add_ref();
					return true;
				});
			}

			virtual bool query_row_by_name(const char* name, core::database::row_interface** row) const override
			{
				if (row == nullptr)
					return false;

				return m_rows.use<bool>([&](rows_map& rows)
				{
					
					utils::ref_count_ptr<core::database::row_interface> instance;
					for (auto& key_val : rows)
					{
						//find first match
						if (strcmp(name, key_val.second->info().name) == 0)
						{
							instance = key_val.second;
							break;
						}

					}

					if (instance == nullptr)
						return false;

					*row = instance;
					(*row)->add_ref();
					return true;
				});
			}

			virtual bool subscribe_callback(core::database::table_callback_interface* callback) override
			{
				return m_table_callback_handler.add_callback(callback);
			}

			virtual bool unsubscribe_callback(core::database::table_callback_interface* callback) override
			{
				return m_table_callback_handler.remove_callback(callback);
			}

			virtual bool subscribe_data_callback(core::database::row_callback_interface* callback) override
			{
				return m_rows.use<bool>([&](rows_map& rows)
				{
					auto it = std::find_if(this->m_data_callbacks.begin(), this->m_data_callbacks.end(),
						[callback](const utils::ref_count_ptr<core::database::row_callback_interface>& current_callback) -> bool
					{
						return (current_callback == callback);
					});

					if (it != this->m_data_callbacks.end())
						return true; // Already subscribed, returning gracefully

					for (auto& pair : rows)
						pair.second->subscribe_callback(callback);

					this->m_data_callbacks.push_back(callback);
					return true;
				});
			}

			virtual bool unsubscribe_data_callback(core::database::row_callback_interface* callback) override
			{
				return m_rows.use<bool>([&](rows_map& rows)
				{
					auto it = std::find_if(this->m_data_callbacks.begin(), this->m_data_callbacks.end(),
						[callback](const utils::ref_count_ptr<core::database::row_callback_interface>& current_callback) -> bool
					{
						return (current_callback == callback);
					});

					if (it == this->m_data_callbacks.end())
						return false;

					for (auto& pair : rows)
						pair.second->unsubscribe_callback(callback);

					return true;
				});
			}
		};

		template <typename T>
		class dataset_base :
			public utils::disposable_base<T>
		{
		private:
			core::database::key m_key;

			using tables_map = std::unordered_map<core::database::key, utils::ref_count_ptr<core::database::table_interface>, core::database::key_hash>;
			mutable utils::thread_safe_object<tables_map> m_tables;		

			utils::callback_handler<core::database::dataset_callback_interface> m_set_callback_handler;

			void clear_tables()
			{
				m_tables.use([&](tables_map& tables)
				{
					for (auto& pair : tables)
					{
						m_set_callback_handler.raise_callbacks([&](core::database::dataset_callback_interface* callback)
						{
							callback->on_table_removed(pair.second);
						});
					}

					tables.clear();
				});
			}

		protected:
			dataset_base(const core::database::key& key) :
				m_key(key)
			{
			}

			virtual bool create_table(const core::database::key& table_key, const char* name, const char* description, core::database::table_interface** table) = 0;

		public:
			virtual ~dataset_base()
			{
				clear_tables();
				m_set_callback_handler.clear();
			}

			virtual core::database::key key() const override
			{
				return m_key;
			}

			virtual size_t size() const override
			{
				return m_tables.use<size_t>([&](tables_map& tables)
				{
					return tables.size();
				});
			}

			virtual bool add_table(const core::database::key& key) override
			{
				return add_table(key, nullptr, nullptr);
			}

			virtual bool add_table(const core::database::key& key, const char* name, const char* description) override
			{
				return m_tables.use<bool>([&](tables_map& tables)
				{
					auto it = tables.find(key);
					if (it != tables.end())
						return false;

					utils::ref_count_ptr<core::database::table_interface> table;
					if (create_table(key,name,description, &table) == false)
						return false;

					tables.emplace(table->key(), table);

					m_set_callback_handler.raise_callbacks([&](core::database::dataset_callback_interface* callback)
					{
						callback->on_table_added(table);
					});

					return true;
				});
			}

			virtual bool remove_table(const core::database::key& key, core::database::table_interface** removed_table) override
			{
				return m_tables.use<bool>([&](tables_map& tables)
				{
					auto it = tables.find(key);
					if (it == tables.end())
						return false;

					utils::scope_guard eraser([&]()
					{
						m_set_callback_handler.raise_callbacks([&](core::database::dataset_callback_interface* callback)
						{
							callback->on_table_removed(it->second);
						});

						tables.erase(it);
					});

					if (removed_table != nullptr)
					{
						*removed_table = it->second;
						(*removed_table)->add_ref();
					}

					return true;
				});
			}

			virtual bool query_table(const core::database::key& key, core::database::table_interface** table) const override
			{
				if (table == nullptr)
					return false;

				return m_tables.use<bool>([&](tables_map& tables)
				{
					auto it = tables.find(key);
					if (it == tables.end())
						return false;

					*table = it->second;
					(*table)->add_ref();

					return true;
				});
			}

			virtual bool query_table_by_index(size_t index, core::database::table_interface** table) const override
			{
				if (table == nullptr)
					return false;

				return m_tables.use<bool>([&](tables_map& tables)
				{
					if (index >= tables.size())
						return false;

					utils::ref_count_ptr<core::database::table_interface> instance;
					size_t counter = 0;
					for (auto& key_val : tables)
					{
						if (counter == index)
						{
							instance = key_val.second;
							break;
						}

						++counter;
					}

					if (instance == nullptr)
						return false;

					*table = instance;
					(*table)->add_ref();
					return true;
				});
			}

			virtual bool query_table_by_name(const char* name, core::database::table_interface** table) const override
			{
				if (table == nullptr)
					return false;

				return m_tables.use<bool>([&](tables_map& tables)
				{
					utils::ref_count_ptr<core::database::table_interface> instance;
					
					for (auto& key_val : tables)
					{
						//find first match
						if (std::strcmp(name, key_val.second->name()) == 0)
						{
							instance = key_val.second;
							break;
						}
					}

					if (instance == nullptr)
						return false;

					*table = instance;
					(*table)->add_ref();
					return true;
				});
			}

			virtual bool subscribe_callback(core::database::dataset_callback_interface* callback) override
			{
				return m_set_callback_handler.add_callback(callback);
			}

			virtual bool unsubscribe_callback(core::database::dataset_callback_interface* callback) override
			{
				return m_set_callback_handler.remove_callback(callback);
			}
		};
				
		class subscription_params;

		class subscriptions_interface : public core::disposable_interface
		{
		public:
			virtual ~subscriptions_interface() = default;

			virtual utils::database::subscription_params subscribe(core::database::row_interface* row, const std::function<void(const row_data&)>& func) = 0;
			virtual bool unsubscribe(core::database::row_interface* row, utils::database::subscription_token token) = 0;
			virtual bool unsubscribe(const utils::database::subscription_params& subscription_params) = 0;
		};

		class subscriptions;
		class auto_token;		

		class subscription_params
		{
			friend class subscriptions;
			friend class auto_token;

		private:
			utils::disposable_ptr<utils::database::subscriptions_interface> m_subscriptions;
			utils::disposable_ptr<core::database::row_interface> m_row;
			utils::database::subscription_token m_token;		

			void copy(const subscription_params& other)
			{
				m_subscriptions = other.m_subscriptions;
				m_row = other.m_row;
				m_token = other.m_token;
			}

			void move(subscription_params& other)
			{
				m_subscriptions = std::move(other.m_subscriptions);
				m_row = std::move(other.m_row);
				m_token = other.m_token;

				other.m_token = utils::database::subscription_token_undefined;
			}			

			void reset()
			{
				m_token = utils::database::subscription_token_undefined;
				m_row.reset();
				m_subscriptions.reset();				
			}

		public:
			subscription_params() :
				m_token(utils::database::subscription_token_undefined)
			{
			}

			subscription_params(std::nullptr_t) :
				subscription_params()
			{
			}

			subscription_params(utils::database::subscriptions_interface* subscriptions, core::database::row_interface* row, utils::database::subscription_token token) :
				m_subscriptions(subscriptions), m_row(row), m_token(token)
			{
			}		

			subscription_params(const subscription_params& other)
			{
				copy(other);
			}

			subscription_params(subscription_params&& other)
			{
				move(other);
			}			

			subscription_params& operator=(const subscription_params& other)
			{
				copy(other);
                return *this;
			}

			subscription_params& operator=(subscription_params&& other)
			{
				move(other);
                return *this;
			}

			bool operator==(std::nullptr_t) const
			{
				utils::ref_count_ptr<utils::database::subscriptions_interface> subscriptions;
				utils::ref_count_ptr<core::database::row_interface> row;
				return (
					m_subscriptions.lock(&subscriptions) == false ||
					m_row.lock(&row) == false ||
					m_token == utils::database::subscription_token_undefined);
			}

			bool operator==(const subscription_params& other) const
			{
				utils::ref_count_ptr<utils::database::subscriptions_interface> subscriptions1, subscriptions2;
				utils::ref_count_ptr<core::database::row_interface> row1,row2;
				m_subscriptions.lock(&subscriptions1);
				other.m_subscriptions.lock(&subscriptions2);
				m_row.lock(&row1);
				other.m_row.lock(&row2);
				
				if (row1 != row2)
					return false;
				
				if (subscriptions1 != subscriptions2)
					return false;

				if (m_token != other.m_token)
					return false;

				return true;
			}

			bool operator!=(std::nullptr_t) const
			{
				return !(*this == nullptr);
			}
			
			operator utils::database::subscription_token() const
			{
				return m_token;
			}

			bool query(utils::database::subscriptions_interface** subscriptions,
				core::database::row_interface** row,
				utils::database::subscription_token& token) const
			{
				if (subscriptions == nullptr)
					return false;

				if (row == nullptr)
					return false;

				utils::ref_count_ptr<utils::database::subscriptions_interface> strong_subscriptions;
				if (m_subscriptions.lock(&strong_subscriptions) == false)
					return false;

				utils::ref_count_ptr<core::database::row_interface> strong_row;
				if (m_row.lock(&strong_row) == false)
					return false;

				if (m_token == utils::database::subscription_token_undefined)
					return false;

				*subscriptions = strong_subscriptions;
				(*subscriptions)->add_ref();

				*row = strong_row;
				(*row)->add_ref();

				token = m_token;
				return true;
			}
		};		

		class auto_token : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			utils::database::subscription_params m_subscription_params;

			auto_token(const auto_token& other) = delete;				// non construction-copyable
			auto_token& operator=(const auto_token& other) = delete;	// non copyable						

		public:
			auto_token() :
				m_subscription_params()
			{
			}

			auto_token(const utils::database::subscription_params& subscription_params) :
				m_subscription_params(subscription_params)
			{
			}

			auto_token(utils::database::subscription_params&& subscription_params) :
				m_subscription_params(std::move(subscription_params))
			{
			}

			auto_token(auto_token&& other) :
				m_subscription_params(std::move(other.m_subscription_params))
			{
			}

			auto_token& operator=(auto_token&& other)
			{
				m_subscription_params = std::move(other.m_subscription_params);
                return *this;
			}
			
			void unregister()
			{
				utils::ref_count_ptr<utils::database::subscriptions_interface> subscriptions;
				utils::ref_count_ptr<core::database::row_interface> row;
				utils::database::subscription_token token = utils::database::subscription_token_undefined;
				if (m_subscription_params.query(&subscriptions, &row, token) == false)
					return;

				utils::scope_guard reset_params([this]()
				{
					m_subscription_params.reset();
				});

				subscriptions->unsubscribe(row, token);
			}

			virtual ~auto_token()
			{
				unregister();
			}
		};

		class subscriptions_collector : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			using tokens_container = std::vector<utils::ref_count_ptr<auto_token>>;
			utils::thread_safe_object<tokens_container> m_tokens;

			subscriptions_collector(const subscriptions_collector& other) = delete;				// non construction-copyable
			subscriptions_collector& operator=(const subscriptions_collector& other) = delete;	// non copyable
			subscriptions_collector(subscriptions_collector&& other) = delete;					// non construction-movable
			subscriptions_collector& operator=(subscriptions_collector&& other) = delete;		// non movable			

		public:
			subscriptions_collector()
			{
			}

			void clear()
			{
				m_tokens.use([&](tokens_container& tokens)
				{
					tokens.clear();
				});
			}

			void add(utils::database::auto_token* token)
			{
				if (token == nullptr)
					throw std::invalid_argument("token");

				m_tokens.use([&](tokens_container& tokens)
				{
					auto it = std::find_if(tokens.begin(), tokens.end(),
						[token](const utils::ref_count_ptr<utils::database::auto_token>& current_token) -> bool
					{
						return (current_token == token);
					});

					if (it != tokens.end())
						return;

					tokens.push_back(token);
				});
			}

			void add(const utils::database::subscription_params& subscription_params)
			{
				utils::ref_count_ptr<auto_token> token = utils::make_ref_count_ptr<auto_token>(subscription_params);
				add(token);
			}

			void add(utils::database::subscription_params&& subscription_params)
			{
				utils::ref_count_ptr<auto_token> token = utils::make_ref_count_ptr<auto_token>(std::move(subscription_params));
				add(token);
			}

			void operator+=(const utils::database::subscription_params& subscription_params)
			{
				add(subscription_params);
			}

			void operator+=(utils::database::subscription_params&& subscription_params)
			{
				add(std::forward<utils::database::subscription_params>(subscription_params));
			}

			void operator+=(utils::database::auto_token* token)
			{
				add(token);
			}

			bool remove(utils::database::auto_token* token)
			{
				if (token == nullptr)
					return false;

				return m_tokens.use<bool>([&](tokens_container& tokens)
					{
						auto it = std::find_if(tokens.begin(), tokens.end(),
							[token](const utils::ref_count_ptr<utils::database::auto_token>& current_token) -> bool
							{
								return (current_token == token);
							});

						if (it != tokens.end())
						{
							tokens.erase(it);
							return true;
						}
						return false;
					});
			}

			bool remove(size_t index)
			{
				return m_tokens.use<bool>([&](tokens_container& tokens)
					{
						if (index >= tokens.size())
							return false;

						auto it = tokens.begin() + index;
						tokens.erase(it);
						return true;
					});
			}

			bool get(size_t index, utils::database::auto_token** token) const
			{
				if (token == nullptr)
					return false;

				return m_tokens.use<bool>([&](const tokens_container& tokens)
					{
						if (tokens.size() <= index)
							return false;

						*token = tokens[index];
						(*token)->add_ref();
						return true;
						
					});
			}

			size_t size() const
			{
				return m_tokens.use<size_t>([&](const tokens_container& tokens)
					{
						return tokens.size();
					});
			}
			
		};

		class subscriptions :
			public utils::disposable_base<utils::database::subscriptions_interface>
		{
		private:
			struct row_callbacks
			{
				utils::ref_count_ptr<utils::smart_disposable_callback> disposable_callback;
				utils::ref_count_ptr<utils::database::smart_row_callback> row_callback;
			};

			// Note: subscription class does NOT take ownership of rows.
			// When subscribing, we connect to the on_disposed callback function.
			// This way, we're able to remove deleted rows entries before they become dangling pointers.
			using subscriptions_map =
				std::unordered_map<core::database::row_interface*, row_callbacks>;
			utils::thread_safe_object<subscriptions_map> m_callbacks;

		public:
			virtual ~subscriptions()
			{
				m_callbacks.use([&](subscriptions_map& callbacks)
				{
					utils::scope_guard clear_callbacks([&]()
					{
						callbacks.clear();
					});

					for (auto& pair : callbacks)
					{
						pair.first->unregister_disposable_callback(pair.second.disposable_callback);
						pair.first->unsubscribe_callback(pair.second.row_callback);
					}					
				});
			}

			virtual utils::database::subscription_params subscribe(core::database::row_interface* row, const std::function<void(const row_data&)>& func) override
			{
				if (row == nullptr)
					throw std::invalid_argument("row");

				if (func == nullptr)
					throw std::invalid_argument("func");

				return m_callbacks.use<utils::database::subscription_params>([&](subscriptions_map& callbacks)
				{
					utils::ref_count_ptr<utils::smart_disposable_callback> disposable_callback;
					utils::ref_count_ptr<smart_row_callback> row_callback;

					auto it = callbacks.find(row);
					if (it == callbacks.end())
					{						
						disposable_callback = utils::make_ref_count_ptr<utils::smart_disposable_callback>();
						row_callback = utils::make_ref_count_ptr<smart_row_callback>();						
					}
					else
					{
						disposable_callback = it->second.disposable_callback;
						row_callback = it->second.row_callback;
					}

					size_t client_count = 0;
					utils::scope_guard updater([&]()
					{
						if (client_count == 1)
						{
							disposable_callback->disposed += [this, row]()
							{
								m_callbacks.use([&](subscriptions_map& callbacks)
								{
									auto it = callbacks.find(row);
									if (it == callbacks.end())
										return;

									callbacks.erase(it);
								});
							};

							callbacks.emplace(row, row_callbacks{ disposable_callback, row_callback });
							
							if (row->register_disposable_callback(disposable_callback) == false)
								throw std::runtime_error("Unexpected: Failed to register disposable object callback");

							if (row->subscribe_callback(row_callback) == false)
								throw std::runtime_error("Unexpected: Failed to register row data callback");
						}
					});

					utils::database::subscription_token token = row_callback->data_changed.subscribe(func, client_count);
					if (token == utils::database::subscription_token_undefined)
						return utils::database::subscription_params();

					return utils::database::subscription_params(this, row, token);
				});
			}

			virtual bool unsubscribe(core::database::row_interface* row, utils::database::subscription_token token) override
			{
				if (row == nullptr)
					throw std::invalid_argument("row");

				return m_callbacks.use<bool>([&](subscriptions_map& callbacks)
				{
					auto it = callbacks.find(row);
					if (it == callbacks.end())
						return false;

					utils::ref_count_ptr<utils::smart_disposable_callback> disposable_callback = it->second.disposable_callback;
					utils::ref_count_ptr<smart_row_callback> row_callback = it->second.row_callback;

					size_t client_count = 0;
					utils::scope_guard updater([&]()
					{
						if (client_count == 0)
						{
							callbacks.erase(it);

							if (row->unregister_disposable_callback(disposable_callback) == false)
								throw std::runtime_error("Unexpected: Failed to unregister disposable object callback");

							if (row->unsubscribe_callback(row_callback) == false)
								throw std::runtime_error("Unexpected: Failed to unregister row data callback");
						}
					});

					return row_callback->data_changed.unsubscribe(token, client_count);
				});
			}			

			virtual bool unsubscribe(const utils::database::subscription_params& subscription_params) override
			{
				utils::ref_count_ptr<utils::database::subscriptions_interface> subscriptions;
				utils::ref_count_ptr<core::database::row_interface> row;
				utils::database::subscription_token token;
				if (subscription_params.query(&subscriptions, &row, token) == false)
					return false;

				if (subscriptions != this)
					return false;

				return unsubscribe(row, token);
			}
		};

		class table_subscription_params
		{
			friend class auto_table_token;

		private:
			utils::disposable_ptr<core::database::table_interface> m_table;
			utils::ref_count_ptr<core::database::row_callback_interface> m_data_callback;

			void copy(const table_subscription_params& other)
			{
				m_table = other.m_table;
				m_data_callback = other.m_data_callback;
			}

			void move(table_subscription_params& other)
			{
				m_table = std::move(other.m_table);
				m_data_callback = std::move(other.m_data_callback);
			}

			void reset()
			{
				m_data_callback.release();
				m_table.reset();
			}

			bool query(core::database::table_interface** table,
				core::database::row_callback_interface** data_callback) const
			{
				if (table == nullptr)
					return false;

				if (data_callback == nullptr)
					return false;

				utils::ref_count_ptr<core::database::table_interface> strong_table;
				if (m_table.lock(&strong_table) == false)
					return false;

				if (m_data_callback == nullptr)
					return false;

				*table = strong_table;
				(*table)->add_ref();

				*data_callback = m_data_callback;
				(*data_callback)->add_ref();

				return true;
			}

		public:
			table_subscription_params()
			{
			}

			table_subscription_params(std::nullptr_t) :
				table_subscription_params()
			{
			}

			table_subscription_params(core::database::table_interface* table, core::database::row_callback_interface* data_callback) :
				m_table(table), m_data_callback(data_callback)
			{
			}

			table_subscription_params(const table_subscription_params& other)
			{
				copy(other);
			}

			table_subscription_params(table_subscription_params&& other)
			{
				move(other);
			}

			table_subscription_params& operator=(const table_subscription_params& other)
			{
				copy(other);
				return *this;
			}

			table_subscription_params& operator=(table_subscription_params&& other)
			{
				move(other);
				return *this;
			}

			bool operator==(std::nullptr_t) const
			{
				utils::ref_count_ptr<core::database::table_interface> table;
				return (
					m_table.lock(&table) == false ||
					m_data_callback == nullptr);
			}

			bool operator==(const table_subscription_params& other) const
			{
				utils::ref_count_ptr<core::database::table_interface> table1, table2;
				
				m_table.lock(&table1);
				other.m_table.lock(&table2);

				if (table1 != table2)
					return false;

				if (m_data_callback != other.m_data_callback)
					return false;

				return true;
			}

			bool operator!=(std::nullptr_t) const
			{
				return !(*this == nullptr);
			}
		};

		class auto_table_token : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			utils::database::table_subscription_params m_table_subscription_params;

			auto_table_token(const auto_token& other) = delete;				// non construction-copyable
			auto_table_token& operator=(const auto_table_token& other) = delete;	// non copyable						

		public:
			auto_table_token() :
				m_table_subscription_params()
			{
			}

			auto_table_token(const utils::database::table_subscription_params& _table_subscription_params) :
				m_table_subscription_params(_table_subscription_params)
			{
			}

			auto_table_token(utils::database::table_subscription_params&& _table_subscription_params) :
				m_table_subscription_params(std::move(_table_subscription_params))
			{
			}

			auto_table_token(auto_table_token&& other) :
				m_table_subscription_params(std::move(other.m_table_subscription_params))
			{
			}

			auto_table_token& operator=(auto_table_token&& other)
			{
				m_table_subscription_params = std::move(other.m_table_subscription_params);
				return *this;
			}

			utils::database::table_subscription_params params() const
			{
				return m_table_subscription_params;
			}

			void unregister()
			{
				utils::ref_count_ptr<core::database::table_interface> table;
				utils::ref_count_ptr<core::database::row_callback_interface> data_callback;

				utils::scope_guard reset_handler([&]()
				{
					m_table_subscription_params.reset();
				});

				if (m_table_subscription_params.query(&table, &data_callback) == false)
					return;

				table->unsubscribe_data_callback(data_callback);
			}

			virtual ~auto_table_token()
			{
				unregister();
			}
		};

		class subscriber;

		class database_dispatcher_interface : public virtual core::ref_count_interface
		{
		public:
			virtual ~database_dispatcher_interface() = default;

			virtual bool query_context(utils::dispatcher** context) = 0;
			virtual subscription_params subscribe(core::database::row_interface* row, const std::function<void(const row_data&)>& func) = 0;
			virtual subscription_params subscribe(core::database::table_interface* table, const buffered_key& row_key, const std::function<void(const row_data&)>& func) = 0;
			virtual subscription_params subscribe(core::database::dataset_interface* dataset, const buffered_key& table_key, const buffered_key& row_key, const std::function<void(const row_data&)>& func) = 0;
			virtual bool unsubscribe(core::database::row_interface* row, subscription_token token) = 0;
			virtual bool unsubscribe(core::database::table_interface* table, const buffered_key& row_key, subscription_token token) = 0;
			virtual bool unsubscribe(core::database::dataset_interface* dataset, const buffered_key& table_key, const buffered_key& row_key, subscription_token token) = 0;
			virtual table_subscription_params subscribe_table(core::database::table_interface* row, const std::function<void(const row_data&)>& func) = 0;
			virtual utils::timer_registration_params register_timer(double interval, const std::function<void()>& func, unsigned int invocation_count = 0) = 0;
			virtual bool unregister_timer(const utils::timer_registration_params& registration_params) = 0;
		};

		/// A database dispatcher base - this is the basic dispatcher for all data_base dispatchers
		///
		/// @date	10/06/2018
		///
		/// @tparam	T	Generic type parameter.
		template <class T>
		class database_dispatcher_base : public utils::database::database_dispatcher_interface, public utils::ref_count_base<T>
		{
			friend class subscriber;

		private:
			class ref_count_row_data : public utils::ref_count_base<core::ref_count_interface>
			{
			private:
				utils::ref_count_ptr<core::database::row_interface> m_row;
				buffered_key m_key;
				size_t m_size;
				utils::ref_count_ptr<utils::ref_count_buffer> m_buffer;

			public:
				ref_count_row_data(core::database::row_interface* row, size_t max_data_size) :
					m_row(row),
					m_key(m_row->key())
				{
					m_buffer = utils::make_ref_count_ptr<utils::ref_count_buffer>(max_data_size);
				}

                ref_count_row_data(core::database::row_interface* row, utils::ref_count_buffer* buffer) :
					m_row(row),
					m_key(m_row->key())
				{
					if (buffer == nullptr || buffer->size() == 0)
						throw std::invalid_argument("buffer");

					m_buffer = buffer;
				}

				const buffered_key& key() const
				{
					return m_key;
				}

				bool query_row(core::database::row_interface** row) const
				{
					if (row == nullptr)
						return false;

					*row = m_row;
					(*row)->add_ref();

					return true;
				}

				size_t data_size() const
				{
					return m_size;
				}

				const void* data() const
				{
					return m_buffer->data();
				}

				void update(size_t size, const void* buffer)
				{
					if (size > m_buffer->size())
						m_buffer->resize(size);

					std::memcpy(m_buffer->data(), buffer, size);
					m_size = size;
				}
			};

			class data_action : public utils::base_async_action
			{
			private:
				utils::ref_count_ptr<ref_count_row_data> m_data;
				utils::ref_count_ptr<utils::func_wrapper<const row_data&>> m_func;

			protected:
				virtual void perform() override
				{
					utils::scope_guard releaser([this]()
					{
						m_func.release();
						m_data.release();
					});
					utils::ref_count_ptr<core::database::row_interface> row;
					if (m_data->query_row(&row))
					{
						row_data data(row, m_data->data_size(), m_data->data());
						m_func->invoke(data);
					}
				}

			public:
				data_action(utils::dispatcher* context) :
					base_async_action(*context)
				{
				}

				void set_data(utils::func_wrapper<const row_data&>* func, ref_count_row_data* data)
				{
					if (data == nullptr)
						throw std::invalid_argument("data");

					if (func == nullptr)
						throw std::invalid_argument("func");


					m_data = data;
					m_func = func;
				}
			};

			/// A registration wrapper to the dispatcher.
			/// This class allow a pre allocation of memory pool at initialization time and avoid as much as possible dynamic allocation throughout the application runtime
			///
			/// @date	10/06/2018
			class registration_wrapper :
				public utils::ref_count_base<core::ref_count_interface>
			{
			private:
				utils::ref_count_ptr<core::database::row_interface> m_row;
				buffered_key m_key;
				size_t m_data_size;

				utils::ref_count_ptr<utils::dispatcher> m_context;
				utils::ref_count_ptr<utils::func_wrapper<const row_data&>> m_func;
				utils::ref_count_ptr<utils::ref_count_object_pool<data_action>> m_actions_pool;
				utils::ref_count_ptr<utils::ref_count_object_pool<ref_count_row_data>> m_data_pool;

				bool create_data_pool(utils::ref_count_object_pool<ref_count_row_data>** pool)
				{
					if (pool == nullptr)
						return false;

					utils::ref_count_ptr<utils::ref_count_object_pool<ref_count_row_data>> instance;

					try
					{
						instance = utils::make_ref_count_ptr<utils::ref_count_object_pool<ref_count_row_data>>(
							BUFFER_POOL_BASE_SIZE, 
							utils::ref_count_object_pool<ref_count_row_data>::growing_mode::doubling, 
							false, 
							m_row, 
							m_data_size == core::database::UNBOUNDED_ROW_SIZE ? 0 : m_data_size);
					}
					catch (...)
					{
						return false;
					}

					*pool = instance;
					(*pool)->add_ref();
					return true;
				}

				bool create_actions_pool(utils::ref_count_object_pool<data_action>** pool)
				{
					if (pool == nullptr)
						return false;

					utils::ref_count_ptr<utils::ref_count_object_pool<data_action>> instance;
					try
					{
						instance = utils::make_ref_count_ptr<utils::ref_count_object_pool<data_action>>(
							ACTIONS_POOL_BASE_SIZE,
							utils::ref_count_object_pool<data_action>::growing_mode::none,
							false,
							m_context);
					}
					catch (...)
					{
						return false;
					}

					*pool = instance;
					(*pool)->add_ref();
					return true;
				}

			public:
				registration_wrapper(
					core::database::row_interface* row,
					size_t data_size,
					utils::dispatcher* context,
					const std::function<void(const row_data&)>& func) :
					m_row(row),
					m_key(m_row->key()),
					m_data_size(data_size),
					m_context(context),
					m_func(utils::make_ref_count_ptr<utils::func_wrapper<const row_data&>>(func))
				{
					if (create_data_pool(&m_data_pool) == false)
						throw std::runtime_error("Failed to create row data pool. Out of memory?");

					if (create_actions_pool(&m_actions_pool) == false)
						throw std::runtime_error("Failed to create context's actions pool. Out of memory?");
				}

				void invoke(const row_data& data)
				{
					utils::ref_count_ptr<ref_count_row_data> row_data_ref;
					if (m_data_pool->get_item(&row_data_ref) == false)
						throw std::runtime_error("Unexpected getting object from pool. Out of memory?");

					row_data_ref->update(data.data_size(), data.buffer());

					utils::ref_count_ptr<data_action> action;
					if (m_actions_pool->get_item(&action) == false)
						action = utils::make_ref_count_ptr<data_action>(m_context);

					action->set_data(m_func, row_data_ref);
					m_context->begin_invoke(action,true);
				}
			};
			
			class table_registration_wrapper : public utils::ref_count_base<core::ref_count_interface>
			{
			private:
				using registration_wrappers_map = std::map<
					utils::ref_count_ptr<core::database::row_interface>,
					utils::ref_count_ptr<registration_wrapper>>;

				utils::disposable_ptr<core::database::table_interface> m_table;
				utils::thread_safe_object<registration_wrappers_map> m_wrappers;

				utils::ref_count_ptr<core::disposable_callback_interface> m_disposable_callback;
				utils::ref_count_ptr<core::database::table_callback_interface> m_table_callback;

				void clear(core::database::row_interface* row)
				{
					m_wrappers.use([&](registration_wrappers_map& wrappers)
					{
						auto it = wrappers.find(row);
						if (it == wrappers.end())
							return;

						wrappers.erase(it);
					});
				}

				void clear()
				{
					m_wrappers.use([&](registration_wrappers_map& wrappers)
					{
						wrappers.clear();
					});
				}

			public:
				table_registration_wrapper(core::database::table_interface* table) :
					m_table(table)
				{
					if (table == nullptr)
						throw std::invalid_argument("table");

					utils::ref_count_ptr<utils::smart_disposable_callback> disposable_callback =
						utils::make_ref_count_ptr<utils::smart_disposable_callback>();

					disposable_callback->disposed += [&]()
					{
						this->clear();
					}; 

					if (table->register_disposable_callback(disposable_callback) == false)
						throw std::runtime_error("Unexpexted! this is a new callback");

					m_disposable_callback = disposable_callback;

					utils::ref_count_ptr<utils::database::smart_table_callback> table_callback =
						utils::make_ref_count_ptr<utils::database::smart_table_callback>();

					table_callback->row_removed += [this](core::database::row_interface* row)
					{
						this->clear(row);
					};

					if (table->subscribe_callback(table_callback) == false)
						throw std::runtime_error("Unexpexted! this is a new callback");

					m_table_callback = table_callback;
				}

				virtual ~table_registration_wrapper()
				{
					m_wrappers.use([&](registration_wrappers_map& wrappers)
					{
						utils::ref_count_ptr<core::database::table_interface> table;
						if (m_table.lock(&table) == false)
							return; // Disposed, no need to remove the table callback subscription

						table->unregister_disposable_callback(m_disposable_callback);
						m_disposable_callback = nullptr;

						table->unsubscribe_callback(m_table_callback);
						m_table_callback = nullptr;

						wrappers.clear();
					});
				}

				void query_registration_wrapper(
					const utils::database::row_data& data, 
					utils::dispatcher* context, 
					const std::function<void(const row_data&)>& func, 
					registration_wrapper** wrapper)
				{
					utils::ref_count_ptr<core::database::row_interface> row;
					if (data.query_row(&row) == false)
						throw std::runtime_error("Unexpected! row_data without a valid row");

					if (context == nullptr)
						throw std::invalid_argument("context");

					m_wrappers.use([&](registration_wrappers_map& wrappers)
					{
						utils::ref_count_ptr<registration_wrapper> instance;

						auto it = wrappers.find(row);
						if (it == wrappers.end())
							instance = utils::make_ref_count_ptr<registration_wrapper>(row, data.data_size(), context, func);
						else
							instance = it->second;

						(*wrapper) = instance;
						(*wrapper)->add_ref();
					});
				}
			};

			/// Pointer to the thread dispatcher
			utils::ref_count_ptr<utils::dispatcher> m_dispatcher;
			utils::ref_count_ptr<subscriptions> m_shared_subscriptions;
			
		protected:
			/// Constructor
			///
			/// @date	10/06/2018
			///
			/// @param	name		   	(Optional) The name.
			/// @param	start_suspended	(Optional) True if to start suspended.
			database_dispatcher_base(const char* name = nullptr, bool start_suspended = false) :
				m_dispatcher(start_suspended == true ?
					utils::ref_count_ptr<utils::dispatcher>(utils::make_ref_count_ptr<suspendable_dispatcher>(name, true)) :
					utils::make_ref_count_ptr<utils::dispatcher>(name)),
				m_shared_subscriptions(utils::make_ref_count_ptr<subscriptions>())
			{				
			}

			/// Constructor
			///
			/// @date	10/06/2018
			///
			/// @exception	std::runtime_error	Raised when a runtime error condition
			/// 	occurs.
			///
			/// @param [in]	context	(Optional) If non-null, the context to run under it.
			database_dispatcher_base(utils::dispatcher* context = nullptr) :
				m_dispatcher(context == nullptr ?
					utils::make_ref_count_ptr<utils::dispatcher>("Shared Context") :
					utils::ref_count_ptr<utils::dispatcher>(context)),
				m_shared_subscriptions(utils::make_ref_count_ptr<subscriptions>())
			{
				if (context != nullptr && context->suspendable() == true)
					throw std::runtime_error("Shared suspendable contexts are not allowed");
			}

			virtual ~database_dispatcher_base()
			{
				m_dispatcher->sync();
			}

            /// check if the thread is suspended
            ///
            /// @date	10/06/2018
            ///
            /// @return	True if it succeeds, false if it fails.
            virtual bool suspended()
			{
				return m_dispatcher->suspended();
			}

            /// Suspends the thread
            ///
            /// @date	10/06/2018
            virtual void suspend()
			{
				m_dispatcher->suspend();
			}

            /// Resumes this thread
            ///
            /// @date	10/06/2018
            virtual void resume()
			{
				m_dispatcher->resume();
			}

			/// Queries a context of the thread this dispatcher is running on and give ownership to the caller (add_ref)
			///
			/// @date	10/06/2018
			///
			/// @param [out]	context	If non-null, the context.
			///
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_context(utils::dispatcher** context) override
			{
				if (context == nullptr)
					return false;

				*context = m_dispatcher;
				(*context)->add_ref();
				return true;
			}

            /// Subscribes to update in a row
            ///
            /// @date	10/06/2018
            ///
            /// @param [in]		row 	the row to subscribe to
            /// @param 		   	func	The function callback
            ///
            /// @return	A subscription_token.
            virtual subscription_params subscribe(core::database::row_interface* row, const std::function<void(const row_data&)>& func) override
			{
				if (row == nullptr)
					return utils::database::subscription_params();

				utils::ref_count_ptr<registration_wrapper> wrapper =
					utils::make_ref_count_ptr<registration_wrapper>(
						row,
						row->data_size(),
						m_dispatcher,
                        [func](const row_data& data)
				{
					func(data);
				});

				return m_shared_subscriptions->subscribe(row, [wrapper](const row_data& data)
				{
					wrapper->invoke(data);
				});
			}

            /// Subscribes to update in a row
            ///
            /// @date	10/06/2018
            ///
            /// @param [in]	    table 	If non-null, the table that the row exist in.
            /// @param 		   	rowKey	The row key.
            /// @param 		   	func  	The function.
            ///
            /// @return	A subscription_token.
            virtual subscription_params subscribe(core::database::table_interface* table, const buffered_key& row_key, const std::function<void(const row_data&)>& func) override
			{
				if (table == nullptr)
					return utils::database::subscription_params();

				utils::ref_count_ptr<core::database::row_interface> row;
				if (table->query_row(row_key, &row) == false)
					return utils::database::subscription_params();

				return subscribe(row, func);
			}

            /// Subscribes to update in a row
            ///
            /// @date	10/06/2018
            ///
            /// @param [in]	    dataset 	the dataset.
            /// @param 		   	tableKey	The table key.
            /// @param 		   	rowKey  	The row key.
            /// @param 		   	func		The function.
            ///
            /// @return	A subscription_token.
            virtual subscription_params subscribe(core::database::dataset_interface* dataset, const buffered_key& table_key, const buffered_key& row_key, const std::function<void(const row_data&)>& func) override
			{
				if (dataset == nullptr)
					return utils::database::subscription_params();

				utils::ref_count_ptr<core::database::table_interface> table;
				if (dataset->query_table(table_key, &table) == false)
					return utils::database::subscription_params();

				return subscribe(table, row_key, func);
			}

            /// Unsubscribes to update in a row
            ///
            /// @date	10/06/2018
            ///
            /// @param [in]	    row  	the row.
            /// @param 		   	token	The token.
            ///
            /// @return	True if it succeeds, false if it fails.
            virtual bool unsubscribe(core::database::row_interface* row, subscription_token token) override
			{
				if (row == nullptr)
					return false;

				return m_dispatcher->invoke<bool>([&]()
				{
					return m_shared_subscriptions->unsubscribe(row, token);
				});
			}

            ///  Unsubscribes to update in a row
            ///
            /// @date	10/06/2018
            ///
            /// @param [in]	    table 	the table.
            /// @param 		   	rowKey	The row key.
            /// @param 		   	token 	The token.
            ///
            /// @return	True if it succeeds, false if it fails.
            virtual bool unsubscribe(core::database::table_interface* table, const buffered_key& row_key, subscription_token token) override
			{
				if (table == nullptr)
					return false;

				utils::ref_count_ptr<core::database::row_interface> row;
				if (table->query_row(row_key, &row) == false)
					return false;

				return unsubscribe(row, token);
			}

            ///  Unsubscribes to update in a row
            ///
            /// @date	10/06/2018
            ///
            /// @param [in]		dataset 	the dataset.
            /// @param 		   	tableKey	The table key.
            /// @param 		   	rowKey  	The row key.
            /// @param 		   	token   	The token.
            ///
            /// @return	True if it succeeds, false if it fails.
            virtual bool unsubscribe(core::database::dataset_interface* dataset, const buffered_key& table_key, const buffered_key& row_key, subscription_token token) override
			{
				if (dataset == nullptr)
					return false;

				utils::ref_count_ptr<core::database::table_interface> table;
				if (dataset->query_table(table_key, &table) == false)
					return false;

				return unsubscribe(table, row_key, token);
			}

			virtual table_subscription_params subscribe_table(core::database::table_interface* table, const std::function<void(const row_data&)>& func) override
			{
				if (table == nullptr)
					throw std::invalid_argument("table");

				if (func == nullptr)
					throw std::invalid_argument("func");

				utils::ref_count_ptr<utils::database::smart_row_callback> data_callback =
					utils::make_ref_count_ptr<utils::database::smart_row_callback>();

				utils::ref_count_ptr<table_registration_wrapper> table_registrations = 
					utils::make_ref_count_ptr<table_registration_wrapper>(table);

				auto func_wrapper = [&, table_registrations, func](const utils::database::row_data& data)
				{
					utils::ref_count_ptr<registration_wrapper> registration;
					table_registrations->query_registration_wrapper(data, this->m_dispatcher, func, &registration);
					registration->invoke(data);
				};

				data_callback->data_changed += func_wrapper;

				if (table->subscribe_data_callback(data_callback) == false)
					throw std::runtime_error("Unexpected, this is a newly created callback");

				return utils::database::table_subscription_params(table, data_callback);
			}
	
			virtual utils::timer_registration_params register_timer(double interval, const std::function<void()>& func, unsigned int invocation_count = 0) override
			{
				utils::ref_count_ptr<utils::dispatcher> core_context;
				if (this->query_context(&core_context) == false)
					return timer_registration_params();
				return core_context->register_timer(interval, func, invocation_count);
			}

			virtual bool unregister_timer(const utils::timer_registration_params& registration_params) override
			{
				utils::ref_count_ptr<utils::dispatcher> core_context;
				if (this->query_context(&core_context) == false)
					return false;
				return core_context->unregister_timer(registration_params);
			}

			virtual void init() override
			{
				// Do nothing...
			}

			virtual void start() override
			{
				// Do nothing...
			}			

			virtual void started() override
			{
				if (m_dispatcher->suspendable() == true)
					m_dispatcher->resume();
			}

			virtual void stop() override
			{
				// Do nothing...
			}			

			virtual void stopped() override
			{
				// Do nothing...
			}
		
		public:
			virtual int release() const override
			{
				return utils::ref_count_base<T>::release();
			}

			virtual int add_ref() const override
			{
				return utils::ref_count_base<T>::add_ref();
			}

			virtual int ref_count() const override
			{
				return utils::ref_count_base<T>::ref_count();
			}
		};	

		class subscriber : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			utils::ref_count_ptr<utils::database::database_dispatcher_interface> m_dispatcher;

		public:
			subscriber(utils::database::database_dispatcher_interface* dispatcher) :
				m_dispatcher(dispatcher)
			{
				if (dispatcher == nullptr)
					throw std::invalid_argument("dispatcher");
			}		

			virtual subscription_params subscribe(core::database::row_interface* row, const std::function<void(const row_data&)>& func)
			{
				return m_dispatcher->subscribe(row, func);
			}

			virtual subscription_params subscribe(core::database::table_interface* table, const buffered_key& row_key, const std::function<void(const row_data&)>& func)
			{
				return m_dispatcher->subscribe(table, row_key, func);
			}
			
			virtual subscription_params subscribe(core::database::dataset_interface* dataset, const buffered_key& table_key, const buffered_key& row_key, const std::function<void(const row_data&)>& func)
			{
				return m_dispatcher->subscribe(dataset, table_key, row_key, func);
			}
			
			virtual bool unsubscribe(core::database::row_interface* row, subscription_token token)
			{
				return m_dispatcher->unsubscribe(row, token);
			}
			
			virtual bool unsubscribe(core::database::table_interface* table, const buffered_key& row_key, subscription_token token)
			{
				return m_dispatcher->unsubscribe(table, row_key, token);
			}
			
			virtual bool unsubscribe(core::database::dataset_interface* dataset, const buffered_key& table_key, const buffered_key& row_key, subscription_token token)
			{
				return m_dispatcher->unsubscribe(dataset, table_key, row_key, token);
			}
			
			virtual table_subscription_params subscribe_table(core::database::table_interface* table, const std::function<void(const row_data&)>& func)
			{
				return m_dispatcher->subscribe_table(table, func);
			}
			virtual bool query_context(utils::dispatcher** context)
			{
				return m_dispatcher->query_context(context);
			}

			utils::timer_registration_params register_timer(double interval, const std::function<void()>& func, unsigned int invocation_count = 0)
			{
				return m_dispatcher->register_timer(interval, func, invocation_count);
			}

			bool unregister_timer(const utils::timer_registration_params& registration_params)
			{
				return m_dispatcher->unregister_timer(registration_params);
			}
		};

		static constexpr size_t READ_TO_STACK_MAX_SIZE = 1024;

		class data_binder : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			class data_dispatcher : public utils::database::database_dispatcher_base<utils::ref_count_base<core::application::runnable_interface>>
			{
			public:
				data_dispatcher(const char* name = nullptr) :
					utils::database::database_dispatcher_base<utils::ref_count_base<core::application::runnable_interface>>(name)
				{
				}

				data_dispatcher(utils::dispatcher* context) :
					utils::database::database_dispatcher_base<utils::ref_count_base<core::application::runnable_interface>>(context)
				{
				}
			};

			utils::ref_count_ptr<utils::database::subscriber> m_subscriber;
			utils::ref_count_ptr<utils::database::subscriptions_collector> m_collector;

		public:
			data_binder() :
				m_subscriber(utils::make_ref_count_ptr<utils::database::subscriber>(utils::make_ref_count_ptr<data_dispatcher>("Data Binder"))),
				m_collector(utils::make_ref_count_ptr<utils::database::subscriptions_collector>())
			{
			}

            data_binder(utils::dispatcher* context) :
                m_subscriber(utils::make_ref_count_ptr<utils::database::subscriber>(utils::make_ref_count_ptr<data_dispatcher>(context))),
                m_collector(utils::make_ref_count_ptr<utils::database::subscriptions_collector>())
            {
            }

			bool bind(core::database::row_interface* source, core::database::row_interface* target, bool copy_init_value = false, bool force_report = true)
			{
				if (source == nullptr || target == nullptr)
					return false;

				if (source->data_size() != target->data_size())
					return false;

				if (copy_init_value)
				{
					size_t data_size = source->data_size();
					if (data_size > 0)
					{
						// Trying to avoid dynamic allocation if possible
						if (data_size < READ_TO_STACK_MAX_SIZE)
						{
							uint8_t buffer[READ_TO_STACK_MAX_SIZE];
							source->read_bytes(buffer);
							target->write_bytes(buffer, data_size, force_report, 0);
						}
						else
						{
							std::vector<uint8_t> buffer(data_size);
							source->read_bytes(buffer.data());
							target->write_bytes(buffer.data(), data_size, force_report, 0);
						}
					}
				}

				utils::ref_count_ptr<core::database::row_interface> local_target = target; // Takes ownership
				m_collector->operator+=(m_subscriber->subscribe(source, [local_target, force_report](const utils::database::row_data& data)
				{
					local_target->write_bytes(data.buffer(), data.data_size(), force_report, 0);
				}));

				return true;
			}
		};
	}
}
