/// @file	core/guid.h.
/// @brief	Declares the unique identifier struct
#pragma once
#include <core/ref_count_interface.h>
#include <cstring>
#include <cstdint>
namespace core
{
	/// @struct	guid
	/// @brief	A unique identifier.
	/// @date	14/05/2018
	struct guid
	{
		uint32_t data1;
		uint16_t data2;
		uint16_t data3;
		uint8_t  data4[8];

		/// @fn	size_t operator<(const core::guid& other) const
		/// @brief	operator <
		/// @date	14/05/2018
		/// @param	other	The guid to compare with.
		/// @return	True if (this < other), otherwise False.
		bool operator<(const guid& other) const
		{
			if (data1 != other.data1)
				return (data1 < other.data1);

			if (data2 != other.data2)
				return (data2 < other.data2);

			if (data3 != other.data3)
				return (data3 < other.data3);

			for (int i = 0; i < 8; i++)
			{
				if (data4[i] != other.data4[i])
					return (data4[i] < other.data4[i]);
			}

			return false;
		}

        /// @fn	static const guid undefined()
        /// @brief	A static definition for the undefined guid
        /// @date	14/05/2018
        /// @return	A const undefined GUID.
        static const guid undefined() { return core::guid{ 0x0, 0x0, 0x0,{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } }; }
	};

	/// @fn	inline bool operator==(const guid& lhs, const guid& rhs)
	/// @brief	Equality operator
	/// @date	14/05/2018
	/// @param	lhs	The first instance to compare.
	/// @param	rhs	The second instance to compare.
	/// @return	True if the parameters are considered equivalent.
	inline bool operator==(const guid& lhs, const guid& rhs)
	{
		return ((lhs.data1 == rhs.data1) &&
			(lhs.data2 == rhs.data2) &&
			(lhs.data3 == rhs.data3) &&
			memcmp(lhs.data4, rhs.data4, sizeof(decltype(lhs.data4))) == 0);
	}

	/// @fn	inline bool operator!=(const guid& lhs, const guid& rhs)
	/// @brief	Inequality operator
	/// @date	14/05/2018
	/// @param	lhs	The first instance to compare.
	/// @param	rhs	The second instance to compare.
	/// @return	True if the parameters are not considered equivalent.
	inline bool operator!=(const guid& lhs, const guid& rhs) 
	{
		return !(lhs == rhs); 
	}

	class DLL_EXPORT guid_generator : public core::ref_count_interface
	{
	public:
		virtual core::guid generate() = 0;
		static bool instance(guid_generator** instance);
	};
}
