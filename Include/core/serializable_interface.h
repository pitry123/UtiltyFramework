/// @file	core/serializable_interface.h.
/// @brief	Declares the serialize-able interface class
#pragma once
#include <core/stream_interface.h>

namespace core
{
	/// @class	serializable_interface
	/// @brief	An interface defining a serialize-able object
	/// @date	14/05/2018
	class DLL_EXPORT serializable_interface
	{
	public:
		/// @fn	virtual serializable_interface::~serializable_interface() = default;
		/// @brief	Destructor
		/// @date	14/05/2018
		virtual ~serializable_interface() = default;

		/// @fn	virtual uint64_t serializable_interface::data_size() = 0;
		/// @brief	Gets Serialized data size
		/// @date	14/05/2018
		/// @return	An uint64_t.
		virtual uint64_t data_size() const = 0;

		/// @fn	virtual bool serializable_interface::serialize(core::stream_interface& stream) = 0;
		/// @brief	Serialize this object into the given stream
		/// @date	14/05/2018
		/// @param [in]		stream	The stream.
		/// @return	True if it succeeds, false if it fails.
		virtual bool serialize(core::stream_interface& stream) const = 0;

		/// @fn	virtual bool serializable_interface::deserialize(core::stream_interface& stream) = 0;
		/// @brief	Deserialize this object from the given stream
		/// @date	14/05/2018
		/// @param [in]		stream	The stream.
		/// @return	True if it succeeds, false if it fails.
		virtual bool deserialize(core::stream_interface& stream) = 0;
	};
}
