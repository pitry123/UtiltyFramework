#pragma once
#include "websocket_utility.hpp"

#include <websocket/websocket_client.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/disposable_ptr.hpp>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <random>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <ostream>
#include <list>
#include <thread>
#include <Core.hpp>

namespace websocket
{
	class websocket_connection : public utils::disposable_base<core::disposable_interface>
	{
	public:
		class q_out_data
		{
		public:
			q_out_data(std::shared_ptr<out_message_data> out_message_, std::function<void(const boost::system::error_code)> &&callback_) noexcept
				: out_message(std::move(out_message_)), callback(std::move(callback_))
			{
			}

			std::shared_ptr<out_message_data> out_message;
			std::function<void(const boost::system::error_code)> callback;
		};

		std::string m_http_version;
		std::string m_status_code;
		case_insensitive_multimap m_header;

		std::shared_ptr<in_message> m_in_message;
		std::shared_ptr<in_message> m_fragmented_in_message;
		std::unique_ptr<boost::asio::steady_timer> m_timer;
		std::mutex m_timer_mutex;
		std::atomic<bool> closed;
		std::mutex m_send_queue_mutex;
		std::list<q_out_data> m_send_queue;
		std::shared_ptr<scope_runner> m_handler_runner;

		// Socket must be unique_ptr since asio::ssl::stream<asio::ip::tcp::socket> is not movable
		std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
        long m_timeout_idle;


	public:
		websocket_connection(
			std::shared_ptr<scope_runner> handler_runner,
			long timeout_idle,
			std::shared_ptr<boost::asio::io_service> io_context) noexcept
			: m_handler_runner(std::move(handler_runner)),
			m_socket(new boost::asio::ip::tcp::socket(*io_context)),
			m_timeout_idle(timeout_idle)
		{
		}

		~websocket_connection() noexcept
		{
			cancel_timeout();

			if (m_socket &&
				m_socket != nullptr)
			{
				boost::system::error_code ec;
				m_socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			}
		}

		virtual void send(const char* msg, size_t size)
		{
			unsigned char fin_rsv_opcode = 129;
			auto out_message = std::make_shared<out_message_data>();
			std::string out_message_str(msg, size);
			out_message->write(out_message_str.data(), static_cast<std::streamsize>(out_message_str.size()));
			send(out_message, nullptr, fin_rsv_opcode);
		}

		virtual void close()
		{
			boost::system::error_code ec;
			m_socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			m_socket->lowest_layer().cancel(ec);
		}

		void set_timeout(long seconds = -1) noexcept
		{
			bool use_timeout_idle = false;
			if (seconds == -1) {
				use_timeout_idle = true;
				seconds = m_timeout_idle;
			}

			std::lock_guard<std::mutex> lock(this->m_timer_mutex);

			if (seconds == 0) {
				m_timer = nullptr;
				return;
			}

#if defined(BOOST_VERSION) && ((BOOST_VERSION / 100 % 1000) >= 70)
            m_timer = std::unique_ptr<boost::asio::steady_timer>(new boost::asio::steady_timer(m_socket->get_executor(), std::chrono::seconds(seconds)));
#else
            m_timer = std::unique_ptr<boost::asio::steady_timer>(new boost::asio::steady_timer(m_socket->get_io_service(), std::chrono::seconds(seconds)));
#endif
			utils::disposable_ptr<websocket_connection> connection_weak(this); // To avoid keeping Connection instance alive longer than needed
			m_timer->async_wait([connection_weak, use_timeout_idle](const boost::system::error_code &ec) {
				if (!ec) {
					utils::ref_count_ptr<websocket_connection> connection;
					if (connection_weak.lock(&connection)) {
						if (use_timeout_idle)
							connection->send_close(1000, "idle timeout"); // 1000=normal closure
						else
							connection->close();
					}
				}
			});
		}

		void send_close(int status, const std::string &reason = "", const std::function<void(const boost::system::error_code &)> &callback = nullptr) {
			// Send close only once (in case close is initiated by client)
			if (closed)
				return;
			closed = true;

			auto out_message = std::make_shared<websocket::out_message_data>();

            out_message->put((char)(status >> 8));
            out_message->put((char)(status % 256));

			*out_message << reason;

			// fin_rsv_opcode=136: message close
			send(out_message, callback, 136);
		}

		void cancel_timeout() noexcept {
			std::lock_guard<std::mutex> lock(this->m_timer_mutex);
			if (m_timer) {
				try {
					m_timer->cancel();
				}
				catch (...) {
				}
			}
		}

		void send_from_queue()
		{
			utils::ref_count_ptr<websocket_connection> self(this);
			boost::asio::async_write(*self->m_socket, m_send_queue.begin()->out_message->streambuf, [self](const boost::system::error_code &ec, std::size_t /*bytes_transferred*/)
			{
				auto lock = self->m_handler_runner->continue_lock();
				if (!lock)
					return;
				{
					std::unique_lock<std::mutex> queue_lock(self->m_send_queue_mutex);
					if (!ec) {
						auto it = self->m_send_queue.begin();
						auto callback = std::move(it->callback);
						self->m_send_queue.erase(it);
						if (self->m_send_queue.size() > 0)
							self->send_from_queue();

						queue_lock.unlock();
						if (callback)
							callback(ec);
					}
					else {
						// All handlers in the queue is called with ec:
						std::vector<std::function<void(const boost::system::error_code &)>> callbacks;
						for (auto &out_data : self->m_send_queue) {
							if (out_data.callback)
								callbacks.emplace_back(std::move(out_data.callback));
						}
						self->m_send_queue.clear();

						queue_lock.unlock();
						for (auto &callback : callbacks)
							callback(ec);
					}
				}
			});
		}
		/// fin_rsv_opcode: 129=one fragment, text, 130=one fragment, binary, 136=close connection.
		/// See http://tools.ietf.org/html/rfc6455#section-5.2 for more information.
		void send(const std::shared_ptr<out_message_data> &out_message, const std::function<void(const boost::system::error_code &)> &callback = nullptr, unsigned char fin_rsv_opcode = 129) {
			cancel_timeout();
			set_timeout();

			// Create mask
			std::array<unsigned char, 4> mask;
			std::uniform_int_distribution<unsigned short> dist(0, 255);
			std::random_device rd;
			for (std::size_t c = 0; c < 4; c++)
				mask[c] = static_cast<unsigned char>(dist(rd));

			std::size_t length = out_message->size();

			std::size_t max_additional_bytes = 14; // ws protocol adds at most 14 bytes
			auto out_header_and_message = std::make_shared<out_message_data>(length + max_additional_bytes);

			out_header_and_message->put(static_cast<char>(fin_rsv_opcode));
			// Masked (first length byte>=128)
			if (length >= 126) {
				std::size_t num_bytes;
				if (length > 0xffff) {
					num_bytes = 8;
					int c = 127 + 128;
					out_header_and_message->put(static_cast<char>(c));
				}
				else {
					num_bytes = 2;
					int c = 126 + 128;
					out_header_and_message->put(static_cast<char>(c));
				}

				for (std::size_t c = num_bytes - 1; c != static_cast<std::size_t>(-1); c--)
                    out_header_and_message->put(static_cast<char>((static_cast<unsigned long long>(length) >> (8 * c)) % 256));
			}
			else
				out_header_and_message->put(static_cast<char>(length + 128));

			for (std::size_t c = 0; c < 4; c++)
				out_header_and_message->put(static_cast<char>(mask[c]));

			for (std::size_t c = 0; c < length; c++)
				out_header_and_message->put((char)(out_message->get() ^ mask[c % 4]));

			std::lock_guard<std::mutex> lock(m_send_queue_mutex);
			m_send_queue.emplace_back(out_header_and_message, callback);
			if (m_send_queue.size() == 1)
				send_from_queue();
		}
	};

	class websocket_client_impl : public utils::ref_count_base<websocket::websocket_client>
	{
	private:
		/// Set before calling start().
		config_communication m_config;

		std::mutex m_start_stop_mutex;
		std::mutex m_connection_mutex;
		/// If you have your own io_context, store its pointer here before running start().
		std::shared_ptr<boost::asio::io_service> m_io_service;
		bool m_internal_io_service;
		utils::ref_count_ptr<websocket_connection> m_connection;
		std::shared_ptr<scope_runner> m_handler_runner;

		std::string m_host;
		unsigned short m_port;
		unsigned short m_default_port;
		std::string m_path;

		std::string m_last_msg;
		core::communication::communication_status m_communication_status;
		std::condition_variable m_cv_new_message;
		bool m_new_msg_ready;
		std::mutex m_new_msg_mutex;

		std::condition_variable m_cv_connection_status_response;
		std::mutex m_connection_status_response_mutex;
		bool m_connection_status_response_ready;

		std::thread m_io_thread;

		void on_new_message(const std::string& new_messege)
		{
			std::lock_guard<std::mutex> lk(m_new_msg_mutex);
			m_new_msg_ready = true;
			m_last_msg = new_messege;
			m_cv_new_message.notify_all();
		}

		void upgrade(const utils::ref_count_ptr<websocket_connection> &connection)
		{
			auto corrected_path = m_path;
			if (!m_config.proxy_server.empty() && std::is_same< boost::asio::ip::tcp::socket, boost::asio::ip::tcp::socket>::value)
				corrected_path = "http://" + m_host + ':' + std::to_string(m_port) + corrected_path;

			auto streambuf = std::make_shared<boost::asio::streambuf>();
			std::ostream ostream(streambuf.get());
			ostream << "GET " << corrected_path << " HTTP/1.1\r\n";
			ostream << "Host: " << m_host;
			if (m_port != m_default_port)
				ostream << ':' << std::to_string(m_port);
			ostream << "\r\n";
			ostream << "Upgrade: websocket\r\n";
			ostream << "Connection: Upgrade\r\n";

			// Make random 16-byte nonce
			std::string nonce;
			nonce.reserve(16);
			std::uniform_int_distribution<unsigned short> dist(0, 255);
			std::random_device rd;
			for (std::size_t c = 0; c < 16; c++)
				nonce += static_cast<char>(dist(rd));

			auto nonce_base64 = std::make_shared<std::string>(crypto::base64::base64_encode(reinterpret_cast<const unsigned char*>(nonce.c_str()), static_cast<unsigned int>(nonce.length())));
			ostream << "Sec-WebSocket-Key: " << *nonce_base64 << "\r\n";
			ostream << "Sec-WebSocket-Version: 13\r\n";
			for (auto &header_field : m_config.header)
				ostream << header_field.first << ": " << header_field.second << "\r\n";
			ostream << "\r\n";

			connection->m_in_message = std::shared_ptr<in_message>(new in_message());

			connection->set_timeout(m_config.timeout_request);
			boost::asio::async_write(*connection->m_socket, *streambuf, [this, connection, streambuf, nonce_base64](const boost::system::error_code  &ec, std::size_t /*bytes_transferred*/) {
				connection->cancel_timeout();
				auto lock = connection->m_handler_runner->continue_lock();
				if (!lock)
					return;
				if (!ec) {
					connection->set_timeout(this->m_config.timeout_request);
					boost::asio::async_read_until(*connection->m_socket, connection->m_in_message->streambuf, "\r\n\r\n", [this, connection, nonce_base64](const boost::system::error_code  &ec, std::size_t bytes_transferred) {
						connection->cancel_timeout();
						auto lock = connection->m_handler_runner->continue_lock();
						if (!lock)
							return;
						if (!ec) {
							// connection->in_message->streambuf.size() is not necessarily the same as bytes_transferred, from Boost-docs:
							// "After a successful async_read_until operation, the streambuf may contain additional data beyond the delimiter"
							// The chosen solution is to extract lines from the stream directly when parsing the header. What is left of the
							// streambuf (maybe some bytes of a message) is appended to in the next async_read-function
							std::size_t num_additional_bytes = connection->m_in_message->streambuf.size() - bytes_transferred;

							if (!response_message::parse(*connection->m_in_message, connection->m_http_version, connection->m_status_code, connection->m_header)) {
								this->connection_error(connection, boost::system::errc::make_error_code(boost::system::errc::protocol_error));
								return;
							}
							if (connection->m_status_code.compare(0, 4, "101 ") != 0) {
								this->connection_error(connection, boost::system::errc::make_error_code(boost::system::errc::permission_denied));
								return;
							}
							auto header_it = connection->m_header.find("Sec-WebSocket-Accept");
							static auto ws_magic_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
							std::string sha1_intput = std::string(*nonce_base64 + ws_magic_string);
							std::shared_ptr<char> bufferChar(new char[21], std::default_delete<char[]>());
							if (header_it != connection->m_header.end() &&
								//Crypto::Base64::decode(header_it->second) == Crypto::sha1(*nonce_base64 + ws_magic_string)) {
								crypto::base64::base64_decode(header_it->second) == crypto::sha1::calc(sha1_intput.c_str(), sha1_intput.length(), bufferChar.get())) {
								this->connection_open(connection);
								read_message(connection, num_additional_bytes);
							}
							else
								this->connection_error(connection, boost::system::errc::make_error_code(boost::system::errc::protocol_error));
						}
						else
							this->connection_error(connection, ec);
					});
				}
				else
					this->connection_error(connection, ec);
			});
		}

		void read_message(const utils::ref_count_ptr<websocket_connection> &connection, std::size_t num_additional_bytes)
		{
			boost::asio::async_read(*connection->m_socket, connection->m_in_message->streambuf, boost::asio::transfer_exactly(num_additional_bytes > 2 ? 0 : 2 - num_additional_bytes), [this, connection](const boost::system::error_code &ec, std::size_t bytes_transferred)
			{
				auto lock = connection->m_handler_runner->continue_lock();
				if (!lock)
					return;
				if (!ec) {
					if (bytes_transferred == 0 && connection->m_in_message->streambuf.size() == 0) { // TODO: This might happen on server at least, might also happen here
						this->read_message(connection, 0);
						return;
					}
					std::size_t num_additional_bytes = connection->m_in_message->streambuf.size() - bytes_transferred;

					std::array<unsigned char, 2> first_bytes;
					connection->m_in_message->read(reinterpret_cast<char *>(&first_bytes[0]), 2);

					connection->m_in_message->fin_rsv_opcode = first_bytes[0];

					// Close connection if masked message from server (protocol error)
					if (first_bytes[1] >= 128) {
						const std::string reason("message from server masked");
						connection->send_close(1002, reason);
						this->connection_close(connection, 1002, reason);
						return;
					}

					std::size_t length = (first_bytes[1] & 127);

					if (length == 126) {
						// 2 next bytes is the size of content
						boost::asio::async_read(*connection->m_socket, connection->m_in_message->streambuf, boost::asio::transfer_exactly(num_additional_bytes > 2 ? 0 : 2 - num_additional_bytes), [this, connection](const boost::system::error_code  &ec, std::size_t bytes_transferred)
						{
							auto lock = connection->m_handler_runner->continue_lock();
							if (!lock)
								return;
							if (!ec) {
								std::size_t num_additional_bytes = connection->m_in_message->streambuf.size() - bytes_transferred;

								std::array<unsigned char, 2> length_bytes;
								connection->m_in_message->read(reinterpret_cast<char *>(&length_bytes[0]), 2);

								std::size_t length = 0;
								std::size_t num_bytes = 2;
								for (std::size_t c = 0; c < num_bytes; c++)
									length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));

								connection->m_in_message->length = length;
								this->read_message_content(connection, num_additional_bytes);
							}
							else
								this->connection_error(connection, ec);
						});
					}
					else if (length == 127) {
						// 8 next bytes is the size of content
						boost::asio::async_read(*connection->m_socket, connection->m_in_message->streambuf, boost::asio::transfer_exactly(num_additional_bytes > 8 ? 0 : 8 - num_additional_bytes), [this, connection](const boost::system::error_code &ec, std::size_t bytes_transferred)
						{
							auto lock = connection->m_handler_runner->continue_lock();
							if (!lock)
								return;
							if (!ec) {
								std::size_t num_additional_bytes = connection->m_in_message->streambuf.size() - bytes_transferred;

								std::array<unsigned char, 8> length_bytes;
								connection->m_in_message->read(reinterpret_cast<char *>(&length_bytes[0]), 8);

								std::size_t length = 0;
								std::size_t num_bytes = 8;
								for (std::size_t c = 0; c < num_bytes; c++)
									length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));

								connection->m_in_message->length = length;
								this->read_message_content(connection, num_additional_bytes);
							}
							else
								this->connection_error(connection, ec);
						});
					}
					else {
						connection->m_in_message->length = length;
						this->read_message_content(connection, num_additional_bytes);
					}
				}
				else
					this->connection_error(connection, ec);
			});
		}

		void read_message_content(const utils::ref_count_ptr<websocket_connection> &connection, std::size_t num_additional_bytes)
		{
			if (connection->m_in_message->length + (connection->m_fragmented_in_message ? connection->m_fragmented_in_message->length : 0) > m_config.max_message_size) {
				connection_error(connection, boost::system::errc::make_error_code(boost::system::errc::message_size));
				const int status = 1009;
				const std::string reason = "message too big";
				connection->send_close(status, reason);
				connection_close(connection, status, reason);
				return;
			}
			boost::asio::async_read(*connection->m_socket, connection->m_in_message->streambuf,
				boost::asio::transfer_exactly(num_additional_bytes > connection->m_in_message->length ? 0 : connection->m_in_message->length - num_additional_bytes),
				[this, connection](const boost::system::error_code &ec, std::size_t bytes_transferred) {
				auto lock = connection->m_handler_runner->continue_lock();
				if (!lock)
					return;
				if (!ec) {
					std::size_t num_additional_bytes = connection->m_in_message->streambuf.size() - bytes_transferred;
					std::shared_ptr<in_message> next_in_message;
					if (num_additional_bytes > 0) { // Extract bytes that are not extra bytes in buffer (only happen when several messages are sent in upgrade response)
						next_in_message = connection->m_in_message;
						connection->m_in_message = std::shared_ptr<in_message>(new in_message(next_in_message->fin_rsv_opcode, next_in_message->length));
						std::ostream ostream(&connection->m_in_message->streambuf);
						for (std::size_t c = 0; c < next_in_message->length; ++c)
							ostream.put(static_cast<char>(next_in_message->get()));
					}
					else
						next_in_message = std::shared_ptr<in_message>(new in_message());

					// Liran -- If connection close
					if ((connection->m_in_message->fin_rsv_opcode & 0x0f) == 8) {
						connection->cancel_timeout();
						connection->set_timeout();

						int status = 0;
						if (connection->m_in_message->length >= 2) {
							unsigned char byte1 = static_cast<unsigned char>(connection->m_in_message->get());
							unsigned char byte2 = static_cast<unsigned char>(connection->m_in_message->get());
							status = (static_cast<int>(byte1) << 8) + byte2;
						}

						auto reason = connection->m_in_message->string();
						connection->send_close(status, reason);
						this->connection_close(connection, status, reason);
					}
					// If ping - On_ping event
					else if ((connection->m_in_message->fin_rsv_opcode & 0x0f) == 9) {
						connection->cancel_timeout();
						connection->set_timeout();

						// Send pong
						auto out_message = std::make_shared<out_message_data>();
						*out_message << connection->m_in_message->string();
                        connection->send(out_message, nullptr,static_cast<unsigned char>(connection->m_in_message->fin_rsv_opcode + 1));

						// Next message
						connection->m_in_message = next_in_message;
						this->read_message(connection, num_additional_bytes);
					}
					// If pong - On_pong event
					else if ((connection->m_in_message->fin_rsv_opcode & 0x0f) == 10) {
						connection->cancel_timeout();
						connection->set_timeout();

						// Next message
						connection->m_in_message = next_in_message;
						this->read_message(connection, num_additional_bytes);
					}
					// If fragmented message and not final fragment
					else if ((connection->m_in_message->fin_rsv_opcode & 0x80) == 0) {
						if (!connection->m_fragmented_in_message) {
							connection->m_fragmented_in_message = connection->m_in_message;
							connection->m_fragmented_in_message->fin_rsv_opcode |= 0x80;
						}
						else {
							connection->m_fragmented_in_message->length += connection->m_in_message->length;
							std::ostream ostream(&connection->m_fragmented_in_message->streambuf);
							ostream << connection->m_in_message->rdbuf();
						}

						// Next message
						connection->m_in_message = next_in_message;
						this->read_message(connection, num_additional_bytes);
					}
					else {
						connection->cancel_timeout();
						connection->set_timeout();


						// On_Messege event
						if (connection->m_fragmented_in_message) {
							connection->m_fragmented_in_message->length += connection->m_in_message->length;
							std::ostream ostream(&connection->m_fragmented_in_message->streambuf);
							ostream << connection->m_in_message->rdbuf();

							this->on_new_message(connection->m_fragmented_in_message->string());
						}
						else
							this->on_new_message(connection->m_in_message->string());

						// Next message
						connection->m_in_message = next_in_message;
						// Only reset fragmented_message for non-control frames (control frames can be in between a fragmented message)
						connection->m_fragmented_in_message = nullptr;
						this->read_message(connection, num_additional_bytes);
					}
				}
				else
					this->connection_error(connection, ec);
			});
		}

		std::pair<std::string, unsigned short> parse_host_port(const std::string &host_port, unsigned short default_port) const noexcept
		{
			std::pair<std::string, unsigned short> parsed_host_port;
			std::size_t host_end = host_port.find(':');
			if (host_end == std::string::npos) {
				parsed_host_port.first = host_port;
				parsed_host_port.second = default_port;
			}
			else {
				parsed_host_port.first = host_port.substr(0, host_end);
				parsed_host_port.second = static_cast<unsigned short>(stoul(host_port.substr(host_end + 1)));
			}
			return parsed_host_port;
		}

		void connection_error(const utils::ref_count_ptr<websocket_connection> &connection, const boost::system::error_code &ec)
		{
			// on_error event.
			connection->cancel_timeout();
			connection->set_timeout();

			m_communication_status = core::communication::DISCONNECTED;
			m_connection_status_response_ready = true;
			m_cv_connection_status_response.notify_all();
		}

		void connection_close(const utils::ref_count_ptr<websocket_connection> &connection, int status, const std::string &reason)
		{
			// on_close event. 
			connection->cancel_timeout();
			connection->set_timeout();

			m_communication_status = core::communication::DISCONNECTED;
			m_connection_status_response_ready = true;
			m_cv_connection_status_response.notify_all();
		}

		void connection_open(const utils::ref_count_ptr<websocket_connection> &connection)
		{
			// on_open event.
			connection->cancel_timeout();
			connection->set_timeout();

			m_communication_status = core::communication::CONNECTED;
			m_connection_status_response_ready = true;
			m_cv_connection_status_response.notify_all();
		}

		void connect_ws()
		{
			std::unique_lock<std::mutex> lock(m_connection_mutex);
			//Core::Console::ColorPrint(Core::Console::Colors::BLUE, "Create new connection... \n");
			this->m_connection = utils::make_ref_count_ptr<websocket_connection>(m_handler_runner, m_config.timeout_idle, m_io_service);
			auto connection = this->m_connection;
			lock.unlock();

			std::pair<std::string, std::string> host_port;
			if (m_config.proxy_server.empty())
				host_port = { m_host, std::to_string(m_port) };
			else {
				auto proxy_host_port = parse_host_port(m_config.proxy_server, 8080);
				host_port = { proxy_host_port.first, std::to_string(proxy_host_port.second) };
			}


			auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(*m_io_service);
			connection->set_timeout(m_config.timeout_request);
			(*resolver).async_resolve(boost::asio::ip::tcp::resolver::query(host_port.first, host_port.second), [this, connection, resolver](const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator results)
			{
				connection->cancel_timeout();
				auto lock = connection->m_handler_runner->continue_lock();
				if (!lock)
					return;
				if (!ec)
				{
					connection->set_timeout(this->m_config.timeout_request);
					boost::asio::async_connect(*connection->m_socket, results, [this, connection, resolver](const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator /*endpoint*/)
					{
						connection->cancel_timeout();
						auto lock = connection->m_handler_runner->continue_lock();
						if (!lock)
							return;
						if (!ec)
						{
							boost::asio::ip::tcp::no_delay option(true);
							connection->m_socket->set_option(option);

							this->upgrade(connection);
						}
						else
							this->connection_error(connection, ec);
					});
				}
				else
					this->connection_error(connection, ec);
			});
		}

		bool stop() noexcept
		{
			std::lock_guard<std::mutex> lock(m_start_stop_mutex);
			{
				std::lock_guard<std::mutex> conn_lock(m_connection_mutex);
				if (m_connection)
					m_connection->close();
			}

			if (m_internal_io_service)
			{
				m_io_service->stop();
				if (m_io_thread.joinable())
					m_io_thread.join();
			}

			return true;
		}

	public:
		websocket_client_impl(const char* host_port_path, uint16_t default_port) noexcept :
            m_handler_runner(new scope_runner()),
            m_default_port(static_cast<unsigned short>(default_port))
		{
			std::string host_port_path_string(host_port_path);
			m_communication_status = core::communication::communication_status::DISCONNECTED;
			m_connection_status_response_ready = false;
			m_new_msg_ready = false;
			m_internal_io_service = false;
			auto host_port_end = host_port_path_string.find('/');
			auto host_port = parse_host_port(host_port_path_string.substr(0, host_port_end), static_cast<unsigned short>(default_port));
			m_host = std::move(host_port.first);
			m_port = host_port.second;

			if (host_port_end != std::string::npos)
				m_path = host_port_path_string.substr(host_port_end);
			else
				m_path = "/";
		}	

		~websocket_client_impl()
		{
			m_handler_runner->stop();
			stop();
		}

		virtual core::communication::communication_status status() const override
		{
			return m_communication_status;
		}

		virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override
		{
			std::unique_lock<std::mutex> new_msg_lock(m_new_msg_mutex);
			if (m_cv_new_message.wait_for(new_msg_lock, std::chrono::milliseconds(1000), [&]() { return m_new_msg_ready; }) == false)
				return 0;

			uint8_t* bytes_arr = static_cast<uint8_t*>(buffer);
			memcpy(bytes_arr, m_last_msg.c_str(), m_last_msg.size());
			m_new_msg_ready = false;

			return m_last_msg.size();
		}

		virtual size_t send(const void* buffer, size_t size) const override
		{
			if (m_connection == nullptr ||
				m_communication_status == core::communication::DISCONNECTED || size == 0)
				return 0;

			m_connection->send((char*)buffer, size);
			return size;
		}

		virtual bool connect() override
		{
			{
				std::lock_guard<std::mutex> lock(m_start_stop_mutex);
				this->m_connection = nullptr;
				m_io_service = nullptr;
				
				if (m_io_thread.joinable())
					m_io_thread.join();

				if (!m_io_service) 
				{
					m_io_service = std::make_shared<boost::asio::io_service>();
					m_internal_io_service = true;
				}

				if (m_io_service->stopped())
					(*m_io_service).reset();
			}

			connect_ws();

			if (m_internal_io_service)
			{
				if (m_io_thread.joinable() == false)
				{
					//Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Create io thread... \n");
					m_io_thread = std::thread([&]()
					{
						try
						{
							//		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Before Io service\n");
							m_io_service->run();
							std::unique_lock<std::mutex> lock(m_connection_mutex);
							lock.unlock();
							//	Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Connection closed \n");
							m_communication_status = core::communication::DISCONNECTED;

						}
						catch (...)
						{
							//Error connection ...
						}
					});
				}
				std::unique_lock<std::mutex> connection_status_res_lock(m_connection_status_response_mutex);
				if (m_cv_connection_status_response.wait_for(connection_status_res_lock,
					std::chrono::milliseconds(3000),
					[&]() {return m_connection_status_response_ready; }) == false)
				{
					m_communication_status = core::communication::DISCONNECTED;
					return false;
				}

				m_connection_status_response_ready = false;
				if (m_communication_status == core::communication::DISCONNECTED)
					return false;
			}
			return true;
		}

		virtual bool disconnect() override
		{
			return stop();
		}		
	};
}


