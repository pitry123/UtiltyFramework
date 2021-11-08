#pragma once
#include <database/memory_database.h>

#include <communication/ports/serial_port.h>
#include <communication/ports/tcp_client_port.h>
#include <communication/ports/udp_client_port.h>
#include <communication/ports/tcp_server_port.h>
#include <communication/protocols/delimiter_protocol.h>
#include <communication/protocols/fixed_length_protocol.h>
#include <communication/protocols/variable_length_protocol.h>
#include <communication/protocols/udp_datagram_protocol.h>

#include <streams/file_stream_interface.h>
#include <streams/memory_stream_interface.h>

#include <utils/buffer_allocator.hpp>

#include <accessories/controllers/logitech_f310.h>
#include <accessories/controllers/xinput_device.h>
#include <accessories/controllers/direct_input_device.h>

#include <parsers/binary_parser.h>

#include <imaging/image_utils.h>

#include <video/sources/gstreamer_auto_source.h>
#include <video/sources/gstreamer_file_source.h>
#include <video/sources/gstreamer_raw_data_source.h>
#include <video/sources/gstreamer_rtsp_source.h>
#include <video/sources/gstreamer_test_source.h>
#include <video/sources/gstreamer_custom_source.h>
#include <video/sources/shared_memory_video_source.h>

#include <video/publishers/shared_memory_video_publisher.h>

#include <Types.hpp>
#include <Buffers.hpp>
#include <Database.hpp>
#include <Communication.hpp>
#include <Streams.hpp>
#include <Accessories.hpp>
#include <BinaryParser.hpp>
#include <Imaging.hpp>
#include <Video.hpp>

#include <websocket/websocket_client.h>

namespace Buffers
{
	class MemoryBuffer :
		public Common::NonConstructible
	{
	public:
		static Buffer Create(size_t size)
		{
			if (size == 0)
				throw std::invalid_argument("size");
			
			utils::ref_count_ptr<core::buffer_interface> instance = 
				utils::make_ref_count_ptr<utils::ref_count_buffer>(size);

			return Buffer(instance);
		}
	};

	class RelativeBuffer :
		public Common::NonConstructible
	{
	public:
		static Buffer Create(const Buffer& buffer, size_t offset, size_t size)
		{
			if (buffer.Empty() == true)
				throw std::invalid_argument("buffer");

			utils::ref_count_ptr<core::buffer_interface> core_buffer;
			buffer.UnderlyingObject(&core_buffer);

			utils::ref_count_ptr<core::buffer_interface> instance =
				utils::make_ref_count_ptr<utils::ref_count_relative_buffer>(core_buffer, offset, size);

			return Buffer(instance);
		}
	};

	class SafeMemoryBuffer :
		public Common::NonConstructible
	{
	public:
		static SafeBuffer Create(size_t size)
		{
			if (size == 0)
				throw std::invalid_argument("size");

			utils::ref_count_ptr<core::safe_buffer_interface> instance =
				utils::make_ref_count_ptr<utils::safe_buffer>(size);

			return SafeBuffer(instance);
		}

		static SafeBuffer Create(size_t size, const uint8_t* data)
		{
			if (size == 0)
				throw std::invalid_argument("size");

			if (data == nullptr)
				throw std::invalid_argument("data");

			utils::ref_count_ptr<core::safe_buffer_interface> instance =
				utils::make_ref_count_ptr<utils::safe_buffer>(data, size);

			return SafeBuffer(instance);
		}
	};

}

namespace Database
{
	/// A memory database factory 
	///Static function to simplify creation of core memory database 
	/// Non Constructible
	/// @date	05/06/2018
	class MemoryDatabase : 
		public Common::NonConstructible
	{
	public:

		/// Creates a new Memory DataSet
		///
		/// @date	05/06/2018
		///
		/// @exception	std::runtime_error	Raised creation failed
		/// 	occurs.
		///
		/// @param	key	The key of the dataset.
		///
		/// @return	A DataSet.
		static DataSet Create(const AnyKey& key)
		{
			utils::ref_count_ptr<core::database::dataset_interface> instance;
			if (database::memory_dataset::create(key, &instance) == false)
				throw std::runtime_error("Failed to create memory dataset");

			return DataSet(instance);
		}
	};	
}

namespace Communication
{
	namespace Ports
	{
		/// A serial port factory.
		///Non Constructible
		/// @date	05/06/2018
		class SerialPort :
			public Common::NonConstructible
		{
		public:
			using Parity = communication::ports::serial_port::parity;
			using StopBits = communication::ports::serial_port::stop_bits;

			/// Static constructor to create serial port
			///
			/// @date	05/06/2018
			///
			/// @exception	std::invalid_argument	Thrown when an port name is null
			/// 	
			/// @exception	std::runtime_error   	Raised when a creation  failed
			///
			/// @param	portName	COM port name.
			/// @param	baudRate	The baud rate.
			/// @param	parity  	The parity.
			/// @param	dataBits	The data bits.
			/// @param	stopBits	The stop bits.
			/// @return	A ClientChannel.
			static ::Communication::ClientChannel Create(
				const char* portName,
				uint32_t baudRate,
				Parity parity,
				size_t dataBits,
				StopBits stopBits)
			{
				if (portName == nullptr)
					throw std::invalid_argument("portName");

				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (communication::ports::serial_port::create(
					portName,
					baudRate,
					parity,
					dataBits,
					stopBits,
					&instance) == false)
					throw std::runtime_error("Failed to create serial port");

				return ::Communication::ClientChannel(instance);
			}
		};

		/// A TCP port Factory.
		///Non Constructible
		/// @date	05/06/2018
		class TcpPort :
			public Common::NonConstructible
		{
		public:

			/// Static constructor of TCP Channel
			///
			/// @date	05/06/2018
			///
			/// @exception	std::invalid_argument	Thrown when an invalid argument
			/// 	error condition occurs.
			/// @exception	std::runtime_error   	Raised when a runtime error
			/// 	condition occurs.
			///
			/// @param	remoteHostName	Name of the remote host.
			/// @param	remotePort	  	The remote port.
			/// @param	localHostName 	Name of the local host.
			/// @param	localPort	  	The local port.
			/// @param	bNoDelaySend	True to cancel Nagel Algorithm that is batching messages to one packet
			/// @return	A ClientChannel.
			static ::Communication::ClientChannel Create(
				const char* remoteHostName,
				uint16_t remotePort,
				const char* localHostName,
				uint16_t localPort,
				bool bNoDelaySend = false)
			{
				if (remoteHostName == nullptr)
					throw std::invalid_argument("remoteHostName");

				if (localHostName == nullptr)
					throw std::invalid_argument("localHostName");

				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (communication::ports::tcp_client_port::create(
					remoteHostName,
					remotePort,
					localHostName,
					localPort,
					bNoDelaySend,
					&instance) == false)
					throw std::runtime_error("Failed to create TCP port");

				return ::Communication::ClientChannel(instance);
			}
		};


		class WebsocketClient :
			public Common::NonConstructible
		{
		public:

			static ::Communication::ClientChannel Create(
				const char* Uri)
			{

				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (websocket::websocket_client::create(
					Uri,
					&instance) == false)
					throw std::runtime_error("Failed to create WebSocket client port");

				return ::Communication::ClientChannel(instance);
			}

			static ::Communication::ClientChannel Create(
				const char* Uri,
				uint16_t Port)
			{
				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (websocket::websocket_client::create(
					Uri,
					Port,
					&instance) == false)
					throw std::runtime_error("Failed to create WebSocket client port");

				return ::Communication::ClientChannel(instance);
			}
		};

		/// An UDP port Factory
		///Non Constructible
		/// @date	05/06/2018
		class UdpPort :
			public Common::NonConstructible
		{
		public:

			/// Static constructor of UDP Client Channel
			///
			/// @date	05/06/2018
			///
			/// @exception	std::invalid_argument	Thrown when remoteHostName or localHostName argument is null
			/// @exception	std::runtime_error   	Raised when a creation fails
			/// 	condition occurs.
			///
			/// @param	remoteHostName	Name of the remote host.
			/// @param	remotePort	  	The remote port.
			/// @param	localHostName 	Name of the local host.
			/// @param	localPort	  	The local port.
			/// @return	A ClientChannel.
			static ::Communication::ClientChannel Create(
				const char* remoteHostName,
				uint16_t remotePort,
				const char* localHostName,
				uint16_t localPort,
				bool multicast = false,
				int receiveBufferSize = 0,
				int sendBufferSize = 0)
			{
				if (remoteHostName == nullptr)
					throw std::invalid_argument("remoteHostName");

				if (localHostName == nullptr)
					throw std::invalid_argument("localHostName");

				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (communication::ports::udp_client_port::create(
					remoteHostName,
					remotePort,
					localHostName,
					localPort,
					multicast,
					receiveBufferSize,
					sendBufferSize,
					&instance) == false)
					throw std::runtime_error("Failed to create UDP port");

				return ::Communication::ClientChannel(instance);
			}
		};

		/// A TCP server port Factory.
		///Non Constructible
		/// @date	05/06/2018
		class TcpServerPort :
			public Common::NonConstructible
		{
		public:

			/// Static constructor
			///
			/// @date	05/06/2018
			///
			/// @exception	std::invalid_argument	Thrown when an invalid argument
			/// 	error condition occurs.
			/// @exception	std::runtime_error   	Raised when a runtime error
			/// 	condition occurs.
			///
			/// @param	localHostName	Name of the local host.
			/// @param	localPort	 	The local port.
			/// @param	bNoDelaySend	True to cancel Nagel Algorithm that is batching messages to one packet
			/// @return ServerChannel
			static ::Communication::ServerChannel Create(
				const char* localHostName,
				uint16_t localPort,
				bool bNoDelaySend = false)
			{
				if (localHostName == nullptr)
					throw std::invalid_argument("localHostName");

				utils::ref_count_ptr<core::communication::server_channel_interface> instance;
				if (communication::ports::tcp_server_port::create(
					localHostName,
					localPort,
					bNoDelaySend,
					&instance) == false)
					throw std::runtime_error("Failed to create TCP port");

				return ::Communication::ServerChannel(instance);
			}
		};
	}

	namespace Protocols
	{
		using DelimiterCouple = communication::protocols::delimiter_couple;

		/// A delimiter protocol factory.
		///Non Constructible
		/// @date	05/06/2018
		class DelimiterProtocol :
			public Common::NonConstructible
		{
		public:

			/// Static constructor for Protocol
			///
			/// @date	05/06/2018
			///
			/// @exception	std::invalid_argument	Thrown when Port is empty or couples = null or maxMessageSize = 0
			/// @exception	std::runtime_error   	Raised when creation fails
			/// 	condition occurs.
			///
			/// @param	port		  	The port.
			/// @param	couples		  	The couples.
			/// @param	couples_count 	Number of couples.
			/// @param	maxMessageSize	Size of the maximum message.
			/// @retrun ClientChannel (which is a Protocol)
			static ::Communication::ClientChannel Create(
				const ::Communication::ClientChannel& port,
				const DelimiterCouple* couples,
				size_t couples_count,
				size_t maxMessageSize)
			{
				if (port.Empty() == true)
					throw std::invalid_argument("port");

				if (couples == nullptr)
					throw std::invalid_argument("couples");

				if (maxMessageSize == 0)
					throw std::invalid_argument("maxMessageSize");

				utils::ref_count_ptr<core::communication::client_channel_interface> corePort;
				port.UnderlyingObject(&corePort);

				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (communication::protocols::delimiter_protocol::create(
					corePort, 
					couples,
					couples_count,
					maxMessageSize, 
					&instance) == false)
					throw std::runtime_error("Failed to create Delimiter protocol");

				return ::Communication::ClientChannel(instance);
			}
		};

		/// A fixed length protocol Factory.
		///Non Constructible
		/// @date	05/06/2018
		class FixedLengthProtocol :
			public Common::NonConstructible
		{
		public:

			/// Static constructor for FixedLengthProtocol
			///
			/// @date	05/06/2018
			///
			/// @exception	std::invalid_argument	Thrown when Port is empty or recieveLength == 0.
			/// 	error condition occurs.
			/// @exception	std::runtime_error   	Raised when creation failed
			///
			/// @param	port		  	The port.
			/// @param	recieveTimeout	The recieve timeout.
			/// @param	recieveLength 	Length of the recieve.
			/// @retrun ClientChannel (which is a Protocol)
			static ::Communication::ClientChannel Create(
				const ::Communication::ClientChannel& port,
				size_t recieveTimeout,
				size_t recieveLength)
			{
				if (port.Empty() == true)
					throw std::invalid_argument("port");

				if (recieveLength == 0)
					throw std::invalid_argument("recieveLength");

				utils::ref_count_ptr<core::communication::client_channel_interface> corePort;
				port.UnderlyingObject(&corePort);

				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (communication::protocols::fixed_length_protocol::create(
					corePort,
					recieveTimeout,
					recieveLength,
					&instance) == false)
					throw std::runtime_error("Failed to create Fixed Length protocol");

				return ::Communication::ClientChannel(instance);
			}
		};

		/// A variable length protocol Factory. 
		///Non Constructible
		/// @date	05/06/2018
		class VariableLengthProtocol :
			public Common::NonConstructible
		{
		public:

			/// Static constructor for VariableLengthProtocol
			///
			/// @date	05/06/2018
			///
			/// @exception	std::invalid_argument	Thrown when port is empty, sizeOfDataSize is 0, maxMessageSize is 0.
			/// 	error condition occurs.
			/// @exception	std::runtime_error   	Raised when a creation fails.
			///
			/// @param	port			   	The port.
			/// @param	sizeOfDataSize	   	Size of the data size.
			/// @param	offsetOfDataSize   	Size of the offset of data.
			/// @param	absoluteMessageSize	True to absolute message size.
			/// @param	maxMessageSize	   	Size of the maximum message.
			/// @param	endian			   	Data endianess.
			/// @param	constatSizeAddition	Tail additional reading after message's reported size.
			/// @retrun ClientChannel (which is a Protocol)
			static ::Communication::ClientChannel Create(
				const ::Communication::ClientChannel& port,
				size_t sizeOfDataSize,
				size_t offsetOfDataSize,				
				bool absoluteMessageSize,
				size_t maxMessageSize,			
				Types::Endian endian,
				size_t constatSizeAddition = 0)
			{
				if (port.Empty() == true)
					throw std::invalid_argument("port");

				if (sizeOfDataSize == 0)
					throw std::invalid_argument("sizeOfDataSize");

				if (maxMessageSize == 0)
					throw std::invalid_argument("maxMessageSize");

				utils::ref_count_ptr<core::communication::client_channel_interface> corePort;
				port.UnderlyingObject(&corePort);

				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (communication::protocols::variable_length_protocol::create(
					corePort,
					sizeOfDataSize,
					offsetOfDataSize,
					constatSizeAddition,
					absoluteMessageSize,
					maxMessageSize,
					endian,
					&instance) == false)
					throw std::runtime_error("Failed to create Variable Length protocol");

				return ::Communication::ClientChannel(instance);
			}

			/// Static constructor for VariableLengthProtocol
			///
			/// @date	05/06/2018
			///
			/// @exception	std::invalid_argument	Thrown when port is empty, sizeOfDataSize is 0, maxMessageSize is 0.
			/// 	error condition occurs.
			/// @exception	std::runtime_error   	Raised when a creation fails.
			///
			/// @param	port			   	The port.
			/// @param	sizeOfDataSize	   	Size of the data size.
			/// @param	offsetOfDataSize   	Size of the offset of data.
			/// @param	absoluteMessageSize	True to absolute message size.
			/// @param	maxMessageSize	   	Size of the maximum message.
			/// @param	constatSizeAddition	Tail additional reading after message's reported size.
			/// @retrun ClientChannel (which is a Protocol)
			static ::Communication::ClientChannel Create(
				const ::Communication::ClientChannel& port,
				size_t sizeOfDataSize,
				size_t offsetOfDataSize,
				bool absoluteMessageSize,
				size_t maxMessageSize,
				size_t constatSizeAddition = 0)
			{
				return Create(port, sizeOfDataSize, offsetOfDataSize, absoluteMessageSize, maxMessageSize, Types::PlatformEndian(), constatSizeAddition);
			}
		};

		class UdpDatagramProtocol :
			public Common::NonConstructible
		{
		public:
			static ::Communication::ClientChannel Create(
				const char* remoteHostName,
				uint16_t remotePort,
				const char* localHostName,
				uint16_t localPort,
				bool multicast = false,
				int receiveBufferSize = 0,
				int sendBufferSize = 0)
			{
				if (remoteHostName == nullptr)
					throw std::invalid_argument("remoteHostName");

				if (localHostName == nullptr)
					throw std::invalid_argument("localHostName");

				utils::ref_count_ptr<core::communication::client_channel_interface> instance;
				if (communication::protocols::udp_datagram_protocol::create(
					remoteHostName,
					remotePort,
					localHostName,
					localPort,
					multicast,
					receiveBufferSize,
					sendBufferSize,
					&instance) == false)
					throw std::runtime_error("Failed to create UDP datagram protocol");

				return ::Communication::ClientChannel(instance);
			}
		};
	}
}

namespace Streams
{
	/// A file stream Factory.
	///Non Constructible
	/// @date	05/06/2018
	class FileStream :
		public Common::NonConstructible
	{
	public:
		using AccessMode = streams::file_stream_interface::access_mode;

		/// Creates a new FileStream
		///
		/// @date	05/06/2018
		///
		/// @exception	std::invalid_argument	Thrown if file path is null
		/// 	error condition occurs.
		/// @exception	std::runtime_error   	Raised when stream creation fails
		/// 	condition occurs.
		///
		/// @param	filePath  	Full pathname of the file.
		/// @param	accessMode	The access mode.
		///
		/// @return	A Stream.
		static Stream Create(const char* filePath, AccessMode accessMode)
		{
			if (filePath == nullptr)
				throw std::invalid_argument("filePath");

			utils::ref_count_ptr<core::stream_interface> instance;
			if (streams::file_stream_interface::create(filePath, accessMode, &instance) == false)
				throw std::runtime_error("Failed to create file stream");

			return Stream(instance);
		}
	};

	/// A memory stream Factory.
	///Non Constructible
	/// @date	05/06/2018
	class MemoryStream :
		public Common::NonConstructible
	{
	public:

		/// Creates a new Stream by size 
		///
		/// @date	05/06/2018
		///
		/// @exception	std::runtime_error	Raised when a creation fails.
		///
		/// @param	size	The size of the memory sream.
		///
		/// @return	A Stream.
		static Stream Create(uint32_t size)
		{
			utils::ref_count_ptr<core::stream_interface> instance;
			if (streams::memory_stream_interface::create(size, &instance) == false)
				throw std::runtime_error("Failed to create memory stream");

			return Stream(instance);
		}

		/// Creates a new Stream by Buffer
		///
		/// @date	05/06/2018
		///
		/// @exception	std::invalid_argument	Thrown when  buffer is empry
		/// 									
		/// @exception	std::runtime_error   	Raised when creation fails
		/// @param	buffer	The buffer.
		///
		/// @return	A Stream.
		static Stream Create(const Buffers::Buffer& buffer)
		{
			if (buffer.Empty() == true)
				throw std::invalid_argument("buffer");

			utils::ref_count_ptr<core::buffer_interface> core_buffer;
			buffer.UnderlyingObject(&core_buffer);

			utils::ref_count_ptr<core::stream_interface> instance;
			if (streams::memory_stream_interface::create(core_buffer, &instance) == false)
				throw std::runtime_error("Failed to create memory stream");

			return Stream(instance);
		}
	};
}

namespace Accessories
{
	namespace Controllers
	{
		static constexpr double DEFAULT_CONTROLLER_SAMPLING_INTERVAL = 20.0;

		class LogitechF310 :
			public Common::NonConstructible
		{
		public:
			static Accessories::Controllers::Joystick Create(
				const Database::Table& statusTable, 
				const Database::Table& commandsTable, 
				uint16_t deadzone = 0, 
				uint32_t deviceIndex = 0,
				double pollingIntervalMilliseconds = DEFAULT_CONTROLLER_SAMPLING_INTERVAL)
			{
				if (statusTable.Empty() == true)
					throw std::invalid_argument("statusTable");

				if (statusTable.Empty() == true)
					throw std::invalid_argument("commandsTable");

				utils::ref_count_ptr<core::accessories::controllers::joystick_runnable> instance;
				if (accessories::controllers::logitech_f310::create(
					static_cast<core::database::table_interface*>(statusTable),
					static_cast<core::database::table_interface*>(commandsTable),
					deadzone,
					deviceIndex,
					pollingIntervalMilliseconds,
					&instance) == false)
					throw std::runtime_error("Failed to create Logitech F310 runnable");

				return Accessories::Controllers::Joystick(instance);
			}
		};

		class XInputDevice :
			public Common::NonConstructible
		{
		public:
			static Accessories::Controllers::Joystick Create(
				const Database::Table& statusTable,
				const Database::Table& commandsTable,
				uint16_t deadzone = 0,
				uint32_t deviceIndex = 0,
				double pollingIntervalMilliseconds = DEFAULT_CONTROLLER_SAMPLING_INTERVAL)
			{
				if (statusTable.Empty() == true)
					throw std::invalid_argument("statusTable");

				if (statusTable.Empty() == true)
					throw std::invalid_argument("commandsTable");

				utils::ref_count_ptr<core::accessories::controllers::joystick_runnable> instance;
				if (accessories::controllers::xinput_device::create(
					static_cast<core::database::table_interface*>(statusTable),
					static_cast<core::database::table_interface*>(commandsTable),
					deadzone,
					deviceIndex,
					pollingIntervalMilliseconds,
					&instance) == false)
					throw std::runtime_error("Failed to create XInput device runnable");

				return Accessories::Controllers::Joystick(instance);
			}
		};

		class DirectInputDevice :
			public Common::NonConstructible
		{
		public:
			static Accessories::Controllers::Joystick Create(
				const Database::Table& statusTable,
				const Database::Table& commandsTable,
				uint16_t deadzone = 0,
				uint32_t deviceIndex = 0,
				const char* deviceName = nullptr,
				double pollingIntervalMilliseconds = DEFAULT_CONTROLLER_SAMPLING_INTERVAL)
			{
				if (statusTable.Empty() == true)
					throw std::invalid_argument("statusTable");

				if (statusTable.Empty() == true)
					throw std::invalid_argument("commandsTable");

				utils::ref_count_ptr<core::accessories::controllers::joystick_runnable> instance;
				if (accessories::controllers::direct_input_device::create(
					static_cast<core::database::table_interface*>(statusTable),
					static_cast<core::database::table_interface*>(commandsTable),
					deadzone,
					deviceName,
					deviceIndex,
					pollingIntervalMilliseconds,
					&instance) == false)
					throw std::runtime_error("Failed to create XInput device runnable");

				return Accessories::Controllers::Joystick(instance);
			}
		};
	}
}

namespace Imaging
{
	class ImageUndistortAlgorithm :
		public Common::NonConstructible
	{
	public:
		static Imaging::ImageAlgorithm Create(
			uint32_t width,
			uint32_t height,
			float(&camera_matrix_chess)[3][3],
			float(&distCoeffs_chess)[5])
		{
			utils::ref_count_ptr<core::imaging::image_algorithm_interface> instance;
			if (imaging::image_undistort::create(width, height, camera_matrix_chess, distCoeffs_chess, &instance) == false)
				throw std::runtime_error("Failed to create Image Algorithm");

			return Imaging::ImageAlgorithm(instance);
		}
	};
}

namespace Video
{
	namespace Sources
	{
		class AutoVideoSource :
			public Common::NonConstructible
		{
		public:			
			static Video::VideoSource Create(bool sync = false, uint32_t width = 0, uint32_t height = 0, Imaging::PixelFormat format = Imaging::PixelFormat::UNDEFINED_PIXEL_FORMAT, const Imaging::ImageAlgorithm algo = nullptr)
			{
				utils::ref_count_ptr<core::video::video_source_interface> instance;
				if (video::sources::gstreamer_auto_source::create(sync, width, height, format, static_cast<core::imaging::image_algorithm_interface*>(algo), &instance) == false)
					throw std::runtime_error("Failed to create Video Source");

				return Video::VideoSource(instance);
			}
		};				

		class AutoVideoSourceFactory : public Video::VideoSourceFactoryBase
		{
		private:
			bool m_sync;
			uint32_t m_width;
			uint32_t m_height;
			Imaging::PixelFormat m_format;
			Imaging::ImageAlgorithm m_algo;
			
		public:
			AutoVideoSourceFactory(bool sync, uint32_t width, uint32_t height, Imaging::PixelFormat format, const Imaging::ImageAlgorithm& algo) :
				m_sync(sync), m_width(width), m_height(height), m_format(format), m_algo(algo)
			{
			}

			virtual Video::VideoSource Create() const override
			{
				return AutoVideoSource::Create(m_sync, m_width, m_height, m_format, m_algo);
			}
		};

		class FileVideoSource :
			public Common::NonConstructible
		{
		public:
			static Video::VideoSource Create(const char* path, const Imaging::ImageAlgorithm algo = nullptr)
			{
				utils::ref_count_ptr<core::video::video_source_interface> instance;
				if (video::sources::gstreamer_file_source::create(path, static_cast<core::imaging::image_algorithm_interface*>(algo), &instance) == false)
					throw std::runtime_error("Failed to create Video Source");

				return Video::VideoSource(instance);
			}
		};

		class FileVideoSourceFactory : public Video::VideoSourceFactoryBase
		{
		private:
			std::string m_uri;
			Imaging::ImageAlgorithm m_algo;
		public:
			FileVideoSourceFactory(const char* uri, const Imaging::ImageAlgorithm& algo) :
				m_uri(uri), m_algo(algo)
			{
			}

			virtual Video::VideoSource Create() const override
			{
				return FileVideoSource::Create(m_uri.c_str(), m_algo);
			}
		};		

		class RawDataVideoSource :
			public Common::NonConstructible
		{
		public:
			static Video::VideoSource Create(
				const char* path, 
				uint32_t width, 
				uint32_t height, 
				Imaging::PixelFormat pixelFormat, 
				const Video::Framerate& framerate, 
				const Imaging::ImageAlgorithm algo = nullptr)
			{
				utils::ref_count_ptr<core::video::video_source_interface> instance;
				if (video::sources::gstreamer_raw_data_source::create(path, width, height, pixelFormat, framerate, static_cast<core::imaging::image_algorithm_interface*>(algo), &instance) == false)
					throw std::runtime_error("Failed to create Video Source");

				return Video::VideoSource(instance);
			}
		};

		class RawDataVideoSourceFactory : public Video::VideoSourceFactoryBase
		{
		private:
			std::string m_file_path;
			uint32_t m_width;
			uint32_t m_height;
			core::imaging::pixel_format m_format;
			core::video::framerate m_framerate;
			Imaging::ImageAlgorithm m_algo;

		public:
			RawDataVideoSourceFactory(
				const char* file_path,
				uint32_t width, uint32_t height,
				core::imaging::pixel_format format,
				core::video::framerate framerate,
				const Imaging::ImageAlgorithm& algo) :
				m_file_path(file_path), m_width(width), m_height(height), m_format(format), m_framerate(framerate), m_algo(algo)
			{
			}

			virtual Video::VideoSource Create() const override
			{
				return RawDataVideoSource::Create(m_file_path.c_str(), m_width, m_height, m_format, m_framerate, m_algo);
			}
		};		

		class RtspVideoSource :
			public Common::NonConstructible
		{
		public:
			static Video::VideoSource Create(
				const char* url, 
				bool live = true, 
				bool multicast = false, 
				bool ntpTimestamps = false, 
				const char* multicastNicName = "",
				Imaging::PixelFormat outputFormat = Imaging::PixelFormat::UNDEFINED_PIXEL_FORMAT, 
				const Imaging::ImageAlgorithm algo = nullptr,
				uint32_t streamIndex = (std::numeric_limits<uint32_t>::max)(),
				VideoDataType dataType = VideoDataType::RAW)
			{
				utils::ref_count_ptr<core::video::video_source_interface> instance;				
				if (video::sources::gstreamer_rtsp_source::create(url, live, multicast, ntpTimestamps, multicastNicName, outputFormat, static_cast<core::imaging::image_algorithm_interface*>(algo), streamIndex, dataType, &instance) == false)
					throw std::runtime_error("Failed to create Video Source");

				return Video::VideoSource(instance);
			}
		};

		class RtspVideoSourceFactory : public Video::VideoSourceFactoryBase
		{
		private:
			std::string m_uri;
			bool m_live;
			bool m_multicast;
			bool m_rtp_timestamps;
			std::string m_nicname;
			core::imaging::pixel_format m_format;
			Imaging::ImageAlgorithm m_algo;
			uint32_t m_streamIndex;

		public:
			RtspVideoSourceFactory(
				const char* uri, 
				bool live, 
				bool multicast, 
				bool rtp_timestamps, 
				const char* nicname, 
				core::imaging::pixel_format format, 
				const Imaging::ImageAlgorithm& algo,
				uint32_t streamIndex) :
				m_uri(uri), 
				m_live(live),
				m_multicast(multicast),
				m_rtp_timestamps(rtp_timestamps), 
				m_nicname(nicname == nullptr ? "" : nicname), 
				m_format(format), 
				m_algo(algo),
				m_streamIndex(streamIndex)
			{
			}

			virtual Video::VideoSource Create() const override
			{
				return RtspVideoSource::Create(m_uri.c_str(),
					m_live,
					m_multicast,
					m_rtp_timestamps,
					m_nicname.empty() == false ? m_nicname.c_str() : nullptr,
					m_format,
					m_algo,
					m_streamIndex);
			}
		};

		class TestVideoSource :
			public Common::NonConstructible
		{
		public:
			static Video::VideoSource Create(uint32_t width, uint32_t height, Imaging::PixelFormat pixelFormat, const Video::Framerate& framerate, const Imaging::ImageAlgorithm algo = nullptr)
			{
				utils::ref_count_ptr<core::video::video_source_interface> instance;
				if (video::sources::gstreamer_test_source::create(width, height, pixelFormat, framerate, static_cast<core::imaging::image_algorithm_interface*>(algo), &instance) == false)
					throw std::runtime_error("Failed to create Video Source");

				return Video::VideoSource(instance);
			}
		};

		class TestVideoSourceFactory : public Video::VideoSourceFactoryBase
		{
		private:
			uint32_t m_width;
			uint32_t m_height;
			core::imaging::pixel_format m_format;
			core::video::framerate m_framerate;
			Imaging::ImageAlgorithm m_algo;

		public:
			TestVideoSourceFactory(
				uint32_t width, uint32_t height,
				core::imaging::pixel_format format,
				core::video::framerate framerate,
				const Imaging::ImageAlgorithm& algo) :
				m_width(width), m_height(height), m_format(format), m_framerate(framerate), m_algo(algo)
			{
			}

			virtual Video::VideoSource Create() const override
			{
				return TestVideoSource::Create(m_width, m_height, m_format, m_framerate, m_algo);
			}
		};

		class GStreamerCustomVideoSource :
			public Common::NonConstructible
		{
		public:
			static Video::VideoSource Create(const char* pipeline, const char* appsinkName, const Imaging::ImageAlgorithm algo = nullptr)
			{
				utils::ref_count_ptr<core::video::video_source_interface> instance;
				if (video::sources::gstreamer_custom_source::create(pipeline, appsinkName, static_cast<core::imaging::image_algorithm_interface*>(algo), &instance) == false)
					throw std::runtime_error("Failed to create Video Source");

				return Video::VideoSource(instance);
			}
		};

		class GStreamerCustomVideoSourceFactory : public Video::VideoSourceFactoryBase
		{
		private:
			std::string m_pipeline;
			std::string m_appsinkName;
			Imaging::ImageAlgorithm m_algo;

		public:
			GStreamerCustomVideoSourceFactory(
				const char* pipeline,
				const char* appsinkName,
				const Imaging::ImageAlgorithm& algo) :
				m_pipeline(pipeline == nullptr ? "" : pipeline), 
				m_appsinkName(appsinkName == nullptr ? "" : appsinkName), 
				m_algo(algo)
			{
			}

			virtual Video::VideoSource Create() const override
			{
				return GStreamerCustomVideoSource::Create(m_pipeline.c_str(), m_appsinkName.c_str(), m_algo);
			}
		};

		class SharedMemoryVideoSource :
			public Common::NonConstructible
		{
		public:
			static Video::VideoSource Create(const char* name)
			{
				utils::ref_count_ptr<core::video::video_source_interface> instance;
				if (video::sources::shared_memory_video_source::create(name, &instance) == false)
					throw std::runtime_error("Failed to create Video Source");

				return Video::VideoSource(instance);
			}
		};

		class SharedMemoryVideoSourceFactory : public Video::VideoSourceFactoryBase			
		{
		private:
			std::string m_name;
		public:
			SharedMemoryVideoSourceFactory(const char* name) :
				m_name(name)
			{
			}

			virtual Video::VideoSource Create() const override
			{
				return SharedMemoryVideoSource::Create(m_name.c_str());
			}
		};
	}

	namespace Publishers
	{
		class SharedMemoryVideoPublisher :
			public Common::NonConstructible
		{
		public:
			static constexpr uint32_t DEFAULT_VIDEO_BUFFER_SIZE = 10485760; // 10 MBs
			static constexpr uint32_t DEFAULT_VIDEO_BUFFER_POOL_SIZE = 20;

			static Video::VideoPublisher Create(
				const char* name,
				const Video::VideoSource source,
				uint32_t buffer_size = DEFAULT_VIDEO_BUFFER_SIZE,
				uint32_t buffer_pool_size = DEFAULT_VIDEO_BUFFER_POOL_SIZE)
			{
				utils::ref_count_ptr<core::video::video_publisher_interface> instance;
				if (video::publishers::shared_memory_video_publisher::create(name, static_cast<core::video::video_source_interface*>(source), buffer_size, buffer_pool_size, &instance) == false)
					throw std::runtime_error("Failed to create Video Publisher");

				return Video::VideoPublisher(instance);
			}

			static Video::VideoPublisher Create(
				const char* name,
				const Video::VideoSourceFactory factory,
				uint32_t buffer_size = DEFAULT_VIDEO_BUFFER_SIZE,
				uint32_t buffer_pool_size = DEFAULT_VIDEO_BUFFER_POOL_SIZE)
			{
				utils::ref_count_ptr<core::video::video_publisher_interface> instance;
				if (video::publishers::shared_memory_video_publisher::create(name, static_cast<core::video::video_source_factory_interface*>(factory), buffer_size, buffer_pool_size, &instance) == false)
					throw std::runtime_error("Failed to create Video Publisher");

				return Video::VideoPublisher(instance);
			}
		};
	}
}