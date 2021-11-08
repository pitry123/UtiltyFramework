#pragma once

static constexpr unsigned int BUFF_MAX_SIZE = 5000;

namespace core
{
	namespace types
	{
		/// @enum	endian
		/// @brief	Values that represent data endianess
		enum endian
		{
			LITTLE,
			BIG
		};

		//this enum aligns with ezFramework 1.0 in the enum code
		enum type_enum
		{
			DEBUG_TYPE		= -1,
			UNKNOWN			=  0,
			BYTE			=  1,
			SHORT			=  2,
			INT32			=  3,
			FLOAT			=  4,
			COMPLEX			=  5,
			EMPTY_TYPE		=  6,
			BOOL			=  7,
			DOUBLE			=  8,
			STRING			=  9,
			BUFFER			= 10,
			POINTER			= 11,
			BITMAP,
			UINT8,
			UINT16,
			UINT32,
			UINT64,
			INT8,
			INT16,
			CHAR,
			USHORT,
			INT64,
			ENUM,
			ARRAY,
			
		};
	}
}