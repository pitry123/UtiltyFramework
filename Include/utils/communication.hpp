#pragma once
#include <core/communication.h>
#include <core/ref_count_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/buffer_allocator.hpp>
#include <utils/dispatcher.hpp>
#include <utils/signal.hpp>

#include <atomic>
#include <thread>
#include <stdexcept>
#include <cstring>

namespace utils
{
	namespace communication
	{
		class data_reader
		{
		private:
			void* m_buffer;
			size_t m_size;

			data_reader(const data_reader& other) = delete;       // non construction-copyable
			data_reader& operator=(const data_reader&) = delete;	// non copyable				
			data_reader(const data_reader&& other) = delete;      // non construction-movable
			data_reader& operator=(const data_reader&&) = delete;	// non movable				

		public:
			data_reader(void* buffer, size_t size) :
				m_buffer(buffer),
				m_size(size)
			{
			}

			const void* buffer() const
			{
				return m_buffer;
			}

			size_t size() const
			{
				return m_size;
			}

			template <typename T>
			void read(T& val) const
			{
				if (m_size != sizeof(T))
					throw std::runtime_error("Reading size mismatch. Wrong data type?");

				val = *(static_cast<const T*>(m_buffer));
			}

			template <typename T>
			T& read() const
			{
				if (m_size < sizeof(T))
					throw std::runtime_error("Reading size mismatch. Wrong data type?");

				return *(static_cast<T*>(m_buffer));
			}
		};

		class async_server_channel : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			std::atomic<bool> m_running;
			std::thread m_accept_thread;

			utils::ref_count_ptr<core::communication::server_channel_interface> m_server;

			void accept()
			{
				while (m_running == true)
				{
					utils::ref_count_ptr<core::communication::client_channel_interface> client;
					if (m_server->accept(&client) == false)
					{
						if (m_running == true)
						{
							// wait for a while...
							std::this_thread::sleep_for(std::chrono::milliseconds(500));
							m_server->connect();
						}
					}
					else
					{
						on_accept(client);
					}
				}
			}

		public:
			utils::signal<async_server_channel, core::communication::client_channel_interface*> on_accept;

			async_server_channel(core::communication::server_channel_interface* server) :
				m_server(server)
			{
				if (server == nullptr)
					throw std::invalid_argument("server");
			}

			~async_server_channel()
			{
				disconnect();
				m_server = nullptr;
			}

			bool query_server(core::communication::server_channel_interface** server) const
			{
				if (server == nullptr)
					return false;

				*server = m_server;
				(*server)->add_ref();
				return true;
			}

			bool connect()
			{
				m_running = true;
				m_server->connect();
				
				m_accept_thread = std::thread([this]() { accept(); });
				return true;
			}

			bool disconnect()
			{
				m_running = false;
				m_server->disconnect();				
				if (m_accept_thread.joinable() == true)
					m_accept_thread.join();

				return true;
			}
		};

		class comm_client_channel : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			size_t m_max_message_size;
			std::thread m_recieving_thread;
			utils::ref_count_ptr<core::communication::client_channel_interface> m_channel;
			bool m_automaticReconnect;
			std::atomic<bool> m_thread_is_alive;

		public:
			utils::signal<comm_client_channel, const data_reader&> data_read;
			utils::signal<comm_client_channel, const core::communication::communication_status&> comm_stat;
			utils::signal<comm_client_channel, const core::communication::communication_error&> comm_err;

			/// Recieves data from the CommChannel 
			///This ii the worker thread of data recieve 
			/// @date	05/06/2018
			///
			/// @exception	std::runtime_error	Raised when a runtime error condition
			/// 	occurs.
			void recieve()
			{
				utils::ref_count_buffer buffer(m_max_message_size);

				size_t bytes_read;
				core::communication::communication_error err = core::communication::communication_error::NO_ERRORS;
				core::communication::communication_status status = core::communication::communication_status::DISCONNECTED;

				comm_stat(m_channel->status());
				if (m_automaticReconnect)
				{
					while (m_thread_is_alive)
					{
						switch (m_channel->status())
						{
						case core::communication::communication_status::CONNECTED:
							status = core::communication::communication_status::CONNECTED;
							bytes_read = m_channel->recieve(buffer.data(), m_max_message_size, &err);
							if (bytes_read != 0)
							{
								data_reader reader(buffer.data(), bytes_read);
								data_read(reader);
							}

							comm_err(err);
							comm_stat(status);
							break;

						case core::communication::communication_status::DISCONNECTED:
							//signal that the communication channel is disconnected, and signal the error.
							status = core::communication::communication_status::DISCONNECTED;
							comm_stat(status);
							comm_err(err);

							while (m_thread_is_alive == true && m_channel->connect() == false)
							{
								//TODO: need to change time to protocol timeout
								std::this_thread::sleep_for(std::chrono::milliseconds(1000));
							}

							//signal that the communication channel is connected
							status = core::communication::communication_status::CONNECTED;
							comm_stat(status);
							break;

						default:
							throw std::runtime_error("Invalid Commuincation channel status");
						}
					}
				}
				else
				{
					while ((bytes_read = m_channel->recieve(buffer.data(), m_max_message_size, &err)) != 0)
					{
						comm_err(err);
						comm_stat(core::communication::communication_status::CONNECTED);

						data_reader reader(buffer.data(), bytes_read);
						data_read(reader);
					}

					comm_err(err);
					comm_stat(core::communication::communication_status::DISCONNECTED);
				}

				// End of thread
			}

		public:
			comm_client_channel(size_t max_message_size, core::communication::client_channel_interface* channel, bool automaticReconnect) :
				m_max_message_size(max_message_size),
				m_channel(channel),
				m_automaticReconnect(automaticReconnect),
				m_thread_is_alive(false)
			{
				if (max_message_size == 0)
				{
					throw std::invalid_argument("max_message_size");
				}

				if (channel == nullptr)
				{
					throw std::invalid_argument("protocol");
				}

				if (channel->status() == core::communication::communication_status::CONNECTED)
					connect();
			}

			~comm_client_channel()
			{
				disconnect();
			}

			size_t max_message_size() const
			{
				return m_max_message_size;
			}

			bool send(const void* buffer, size_t size) const
			{
				if (m_channel == nullptr)
					return false;

				if (buffer == nullptr)
					return false;

				if (size == 0 || size > m_max_message_size)
					return false;

				//if size > 0 return true
				return  (m_channel->send(buffer, size) > 0);
			}

			core::communication::communication_status status() const
			{
				if (m_channel == nullptr)
					return core::communication::communication_status::DISCONNECTED;

				return m_channel->status();
			}

			bool connect()
			{
				if (m_channel == nullptr)
					return false;				

				// When auto-reconnect is enabled, the connection is managed in the recieve thread
				// Hence, there is no need to try to connect here
				if (m_automaticReconnect == false)
				{
					if (m_channel->status() != core::communication::communication_status::CONNECTED)
					{
						disconnect();

						if (m_channel->connect() == false)
							return false;
					}
				}

				bool expected = false;
				bool desired = true;
				bool create_thread = (m_thread_is_alive.compare_exchange_strong(expected, desired) == true);

				if (create_thread == true)
				{
					m_recieving_thread = std::thread([this]() { recieve(); });
				}

				return true;
			}

			bool disconnect()
			{
				if (m_channel == nullptr)
					return false;

				bool expected = true;
				bool desired = false;
				bool sync = (m_thread_is_alive.compare_exchange_strong(expected, desired) == true);

				m_channel->disconnect();

				if (sync == true)
				{
					if (m_recieving_thread.joinable() == true)
						m_recieving_thread.join();
				}

				return true;
			}
		};

		class comm_server_channel :
			public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			utils::ref_count_ptr<utils::communication::async_server_channel> m_server;
			utils::signal_token m_accept_token;
			std::vector<utils::ref_count_ptr<utils::dispatcher>> m_send_thread_pool;

		public:
			utils::signal<comm_server_channel, core::communication::client_channel_interface*, size_t> on_accept;

			comm_server_channel(
				utils::communication::async_server_channel* server,
				size_t max_message_size,
				bool use_send_pool = false) :
                m_server(server)
			{
				if (server == nullptr)
					throw std::invalid_argument("server");

				if (use_send_pool == true)
					m_send_thread_pool.resize(std::thread::hardware_concurrency());

                m_accept_token = (m_server->on_accept += [this, max_message_size](core::communication::client_channel_interface* client)
				{
					if (client == nullptr)
					{
						// TODO: Log...
						return;
					}

					on_accept(client, max_message_size);
				});
			}

			~comm_server_channel()
			{
				m_server->on_accept -= m_accept_token;
			}

			bool connect()
			{
				return m_server->connect();
			}

			bool disconnect()
			{
				return m_server->disconnect();
			}
		};
	}
}
