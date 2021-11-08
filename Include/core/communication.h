/// @file	core/communication.h.
/// @brief	Declares the communication interface classes
#pragma once
#include <core/ref_count_interface.h>
#include <cstdint>
#include <cstddef>

#define IP_ADDRESS_MAX_LENGTH 16

namespace core
{
	namespace communication
	{
		/// @enum	communication_status
		/// @brief	Values that represent communication status
		enum communication_status
		{
			DISCONNECTED,
			CONNECTED
		};

		/// @enum	communication_error
		/// @brief	Values that represent communication errors
		enum communication_error
		{
			NO_ERRORS,
			NO_PERMISSION,
			ACCESS_DENIED,
			HOST_UNREACHABLE,
			TIMED_OUT,
			CONNECTION_REFUSED,
			UNKNOWN_ERROR
		};

		/// @class	connection_interafce
		/// @brief	An interface defining base connection.
		/// @date	14/05/2018
		class DLL_EXPORT connection_interafce : public core::ref_count_interface
		{
		public:
			/// @fn	virtual connection_interafce::~connection_interafce() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~connection_interafce() = default;

			/// @fn	virtual core::communication::communication_status connection_interafce::status() const = 0;
			/// @brief	Gets the connection status
			/// @date	14/05/2018
			/// @return	The core::communication::communication_status.
			virtual core::communication::communication_status status() const = 0;

			/// @fn	virtual bool connection_interafce::connect() = 0;
			/// @brief	Connect the connection
			/// @date	14/05/2018
			/// @return	True if it succeeds, false if it fails.
			virtual bool connect() = 0;

			/// @fn	virtual bool connection_interafce::disconnect() = 0;
			/// @brief	Disconnects the connection
			/// @date	14/05/2018
			/// @return	True if it succeeds, false if it fails.
			virtual bool disconnect() = 0;
		};

		/// @class	client_channel_interface
		/// @brief	An interface defining a communication client.
		/// @date	14/05/2018
		class DLL_EXPORT client_channel_interface: public connection_interafce
		{
		public:
			/// @fn	virtual client_channel_interface::~client_channel_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~client_channel_interface() = default;

			/// @fn	virtual size_t client_channel_interface::send(const void* buffer,size_t size) const = 0;
			/// @brief	Sends data
			/// @date	14/05/2018
			/// @param	buffer	The data to be sent.
			/// @param	size  	The data size.
			/// @return	A size_t. The actual data size that was sent.
			virtual size_t send(const void* buffer,size_t size) const = 0;

			/// @fn	virtual size_t client_channel_interface::recieve(void* buffer,size_t size, core::communication::communication_error* commError) = 0;
			/// @brief	Recieve data (blocking)
			/// @date	14/05/2018
			/// @param [in]		buffer   	A buffer for writing the received data.
			/// @param 		   	size	 	The max data size to receive.
			/// @param [out]	commError	If non-null, the communications error/status.
			/// @return	A size_t. The actual data size that was received.
			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) = 0;
		};

#pragma pack(1)
		enum ip_address_type
		{
			IP_ADDRESS_TYPE_UDEFINED = 0,
			IP_VER4 = 4,
			IP_VER6 = 6
		};

		struct ip_address
		{
			ip_address_type type;
		    uint8_t val[IP_ADDRESS_MAX_LENGTH];
			uint32_t scope_id;
		};

		struct ip_endpoint
		{
			ip_address address;
			uint16_t port;
		};
#pragma pack()

		class DLL_EXPORT ip_client_channel_interface : public core::communication::client_channel_interface
		{
		public:
			virtual ~ip_client_channel_interface() = default;
			virtual bool query_local_endpoint(core::communication::ip_endpoint& end_point) const = 0 ;
			virtual bool query_remote_endpoint(core::communication::ip_endpoint& end_point) const = 0 ;
		};

		/// @class	server_channel_interface
		/// @brief	An interface defining a communication server.
		/// @date	14/05/2018
		class DLL_EXPORT server_channel_interface : public connection_interafce
		{
		public:
			/// @fn	virtual server_channel_interface::~server_channel_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~server_channel_interface() = default;

			/// @fn	virtual bool server_channel_interface::accept(core::communication::client_channel_interface** client) = 0;
			/// @brief	Accepts a connecting client (blocking)
			/// @date	14/05/2018
			/// @param [out]	client	If succeeds, the connected client.
			/// @return	True if it succeeds, false if connection was closed or dropped.
			virtual bool accept(core::communication::client_channel_interface** client) = 0;
		};
	}
}
