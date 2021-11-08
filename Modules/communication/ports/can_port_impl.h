#pragma once

#ifndef _WIN32
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#include <communication/ports/can_port.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/thread_safe_object.hpp>
#include <mutex>
#include <queue>

namespace communication
{
	namespace ports
    {
		class socketcan_interface_impl : public utils::ref_count_base <communication::ports::can_port_interface>
		{
			bool is_extended(uint32_t id)
			{
#ifndef _WIN32
				return ((id & CAN_EFF_FLAG) != 0);
#else
				return ((id & 0x10000000) != 0);
#endif

			}

			uint32_t extend_id(uint32_t id, bool extention_mode)
			{
				if (extention_mode == true && is_extended(id) == false)
				{
#ifndef _WIN32
					return (id | CAN_EFF_FLAG);
#else
					return (id | 0x10000000);
#endif
				}
				return id;
			}

			std::string m_ch_name;
			uint32_t m_baudrate;
			bool m_extention_mode;
			communication::ports::can_mode m_can_mode;

			// Socket 
			int m_socket;
			core::communication::communication_status m_communication_status;




		public:
			socketcan_interface_impl(const char* channel_name,
				uint32_t buadrate,
				bool extension_mode,
				communication::ports::can_mode mode):
				m_ch_name(channel_name),
				m_baudrate(buadrate),
				m_extention_mode(extension_mode),
				m_can_mode(mode),
				m_communication_status(core::communication::communication_status::DISCONNECTED)
			{
			}

			virtual size_t can_send(const void* buffer, size_t size)  const override
			{
#ifndef _WIN32
				communication::ports::can_msg frame_buffer;
				memcpy(&frame_buffer, buffer, size);

				struct canfd_frame frame;
				memset(&frame, 0, sizeof(frame)); /* init CAN FD frame, e.g. LEN = 0 */
				//convert CanFrame to canfd_frame
				frame.can_id = extend_id(frame_buffer.id);
				frame.len = 8;
				// frame.flags = msg.flags;

				if (m_can_mode == communication::ports::can_mode::MODE_CANFD_MTU)
				{
					frame.len = 64;
				}
				memcpy(frame.data, frame_buffer.data, frame.len);

				/* send frame */
				if (::write(m_socket, &frame, int(m_can_mode)) != int(m_can_mode)).
				{
					return 0;//STATUS_WRITE_ERROR;
				}
#else
				return 0;
#endif
				return size;
		}


			virtual size_t can_read(void* buffer, size_t size) override
			{
#ifndef _WIN32
				struct can_frame frame;
				auto num_bytes = ::read(m_socket, &frame, CAN_MTU);
				while (num_bytes != CAN_MTU)// && num_bytes != CANFD_MTU)
				{
					num_bytes = ::read(m_socket, &frame, CAN_MTU);
				}
				communication::ports::can_msg* frame_buffer = static_cast<communication::ports::can_msg*>(buffer);

				frame.can_id = (is_extended(frame.can_id)) ? (frame.can_id & CAN_EFF_MASK) : (frame.can_id & CAN_SFF_MASK);

				frame_buffer->id = frame.can_id;
				memcpy(frame_buffer->data, frame.data, frame.len);

#ifdef VERBOS
				printf("id: %x, data: %02x %02x %02x %02x %02x %02x %02x %02x  \n", frame_buffer->nID,
					frame_buffer->data[0], frame_buffer->data[1], frame_buffer->data[2], frame_buffer->data[3],
					frame_buffer->data[4], frame_buffer->data[5], frame_buffer->data[6], frame_buffer->data[7]);
#endif

#else 
				return 0;
#endif
				return sizeof(communication::ports::can_msg);
			}
			 
			virtual bool can_open() override
			{
#ifndef _WIN32
				/* open socket */
				if ((m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
				{
					return false;
					// return STATUS_SOCKET_CREATE_ERROR;
				}

				int mtu, enable_canfd = 1;
				struct sockaddr_can addr;
				struct ifreq ifr;

				strncpy(ifr.ifr_name, m_ch_name.c_str(), IFNAMSIZ - 1);
				ifr.ifr_name[IFNAMSIZ - 1] = '\0';
				ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
				if (!ifr.ifr_ifindex) {
					return false;
					//        return STATUS_INTERFACE_NAME_TO_IDX_ERROR;
				}

				addr.can_family = AF_CAN;
				addr.can_ifindex = ifr.ifr_ifindex;

				if (m_can_mode == communication::ports::can_mode::MODE_CANFD_MTU)
				{
					/* check if the frame fits into the CAN netdevice */
					if (ioctl(m_socket, SIOCGIFMTU, &ifr) < 0)
					{
						perror("SIOCGIFMTU");
						return false; //STATUS_MTU_ERROR
					}
					mtu = ifr.ifr_mtu;

					if (mtu != CANFD_MTU)
					{
						return false; //STATUS_CANFD_NOT_SUPPORTED
					}

					/* interface is ok - try to switch the socket into CAN FD mode */
					if (setsockopt(m_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES,
						&enable_canfd, sizeof(enable_canfd)))
					{
						return false; //STATUS_ENABLE_FD_SUPPORT_ERROR
					}

				}

				struct timeval tv;
				tv.tv_sec = 30; 
				tv.tv_usec = 0; 
				setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));

				if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
					perror("errorn bind");
					return false; //STATUS_BIND_ERROR
				}

#else
				return false;
#endif
				m_communication_status = core::communication::communication_status::CONNECTED;
				return true;
			}

			virtual bool can_close() override
			{
#ifndef _WIN32
				::close(m_socket);
#endif
				m_communication_status = core::communication::communication_status::DISCONNECTED;
				return true;
			}

			virtual core::communication::communication_status communication_status() const override
			{
				return m_communication_status;
			}
		};
// 
// 		class slcan_interface_impl : public utils::ref_count_base <communication::ports::can_port_interface>
// 		{
// 		public:
// 
// 			bool can_open() override;
// 
// 
// 			bool can_close() override;
// 
// 
// 			size_t can_read(void* buffer, size_t size, core::communication::communication_error* commError) override;
// 
// 
// 			size_t can_send(const void* buffer, size_t size) const override;
// 
// 
// 			core::communication::communication_status communication_status() const override;
// 
// 		};


		class can_port_adapter_impl : public utils::ref_count_base<communication::ports::can_port_adapter>
        {
			utils::ref_count_ptr<communication::ports::can_port_interface> m_can_interface;
			utils::thread_safe_object<std::queue<communication::ports::can_msg>> m_msg_queue;
			std::atomic<bool> m_is_running;

			std::thread m_recv_thread;

			size_t m_max_queue_size;
			std::condition_variable m_cv_msg;
			std::mutex m_cv_msg_mutex;

		public:
			can_port_adapter_impl(
				communication::ports::can_interface ican,
				communication::ports::can_mode mode,
				const char* channel_name,
				uint32_t buadrate,
				bool extension_mode):
				m_ch_name(channel_name),
				m_baudrate(buadrate),
				m_extention_mode(extension_mode),
				m_communication_status(core::communication::communication_status::DISCONNECTED),
				m_max_queue_size(2000)
			{
				if (ican == socket_can)
				{
					m_can_interface = utils::make_ref_count_ptr<socketcan_interface_impl>(m_ch_name, m_baudrate, m_extention_mode, mode);
				}


				
			}


			can_port_adapter_impl(communication::ports::can_port_interface* can_interface) :
				m_can_interface(can_interface)
			{

			}

			// Inherited via ref_count_base
			virtual core::communication::communication_status status() const override
			{
				return m_can_interface->communication_status();
			}

			virtual bool connect() override
			{
				m_is_running = m_can_interface->can_open();

				if (m_recv_thread.joinable())
				{
					m_recv_thread.join();
				}

				m_recv_thread = std::thread([&]()
				{
					while (m_is_running)
					{
						communication::ports::can_msg msg = {};
						if (m_can_interface->can_read((void*)&msg, sizeof(msg)) > 0)
						{
							m_msg_queue.use([&, msg](std::queue<communication::ports::can_msg>& q)
							{
								if (q.size() >= m_max_queue_size)
								{
									q.pop();
								}
								q.push(msg);
								m_cv_msg.notify_all();
							});
						}
					}
				});

				return m_is_running;
			}

			virtual bool disconnect() override
			{
				bool ret = m_can_interface->can_close();
				m_is_running = false;
				return ret;
			}

			virtual size_t send(const void* buffer, size_t size) const override
			{
				return m_can_interface->can_send(buffer, size);
			}

			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override
			{
				std::unique_lock<std::mutex> lock(m_cv_msg_mutex);
				m_cv_msg.wait(lock, [&]()
				{
					return m_msg_queue.use<bool>([&](std::queue<communication::ports::can_msg>& q)
					{
						if (q.size() > 0)
						{
							return true;
						}
						return false;
					});
				});


				return m_msg_queue.use<size_t>([&](std::queue<communication::ports::can_msg>& q)
				{
					communication::ports::can_msg msg = q.front();
					q.pop();
					memcpy(buffer, &msg, sizeof(msg));
					return sizeof(msg);
				});
			}


        private:					
			std::string m_ch_name;
			uint32_t m_baudrate;
			bool m_extention_mode;
			core::communication::communication_status m_communication_status;
		};
	}	
}

