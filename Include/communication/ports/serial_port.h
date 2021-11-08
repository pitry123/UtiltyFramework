/// @file	ports/serial_port.h.
/// @brief	Declares the serial port class
#pragma once
#include <core/communication.h>

namespace communication
{
	namespace ports
	{
		/// @class	serial_port
		/// @brief	A serial port.
		/// @date	15/05/2018
		class DLL_EXPORT serial_port : public core::communication::client_channel_interface
		{
		public:
			/// @enum	parity
			/// @brief	Specifies the parity bit for a SerialPort
			enum parity
			{				
				/// @brief	No parity check occurs.				
				None = 0,				
				/// @brief	Sets the parity bit so that the count of bits set is an odd number.
				Odd = 1,
				/// @brief	Sets the parity bit so that the count of bits set is an even number.
				Even = 2,								
				/// @brief	Leaves the parity bit set to 1.
				Mark = 3,
				/// @brief	Leaves the parity bit set to 0.
				Space = 4,
			};

			/// @enum	stop_bits
			/// @brief	Specifies the number of stop bits used on the
			enum stop_bits
			{
				/// @brief	No stop bits are used.
				StopBitsNone = 0,
				/// @brief	One stop bit is used.
				One = 1,
				/// @brief	Two stop bits are used.
				Two = 2,
				/// @brief	1.5 stop bits are used.
				OnePointFive = 3,
			};

			/// @fn	virtual serial_port::~serial_port() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~serial_port() = default;

			/// @fn	static bool serial_port::create(const char* portName, uint32_t baudRate, communication::ports::serial_port::parity parity, size_t dataBits, communication::ports::serial_port::stop_bits stop_bits, core::communication::client_channel_interface** channel);
			/// @brief	Static factory: Creates a new serial port instance
			/// @date	15/05/2018
			/// @param 		   	portName 	Name of the port.
			/// @param 		   	baudRate 	The baud rate.
			/// @param 		   	parity   	The parity.
			/// @param 		   	dataBits 	The data bits.
			/// @param 		   	stop_bits	The stop bits.
			/// @param [out]	channel  	An address of a pointer to core::communication::client_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* portName, uint32_t baudRate, 
				communication::ports::serial_port::parity parity, 
				size_t dataBits, 
				communication::ports::serial_port::stop_bits stop_bits, 
				core::communication::client_channel_interface** channel);
		};
	}
}