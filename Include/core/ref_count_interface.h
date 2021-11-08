/// @file	.\Include\core\ref_count_interface.h.
/// @brief	Declares the reference count interface class
#pragma once
#include <core/release_interface.h>

namespace core
{	
	/// @class	ref_count_interface
	/// @brief	An interface for managing ownership of an object.
	/// @date	14/05/2018
	class DLL_EXPORT ref_count_interface : public release_interface
	{
	public:
		/// @brief	Adds reference (takes ownership)
		/// @date	14/05/2018
		/// @return	new owners count.
		virtual int add_ref() const = 0;

		/// @brief	Reference count
		/// @date	14/05/2018
		/// @return	current owners count.
		virtual int ref_count() const = 0;
	
	protected:
		virtual ~ref_count_interface() = default;
	};
}
