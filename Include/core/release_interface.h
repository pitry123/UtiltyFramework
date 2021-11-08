/// @file	.\Include\core\release_interface.h.
/// @brief	Declares the release interface class
#pragma once
#include <core/os.h>

namespace core
{	
	/// @class	release_interface
	/// @brief	An interface for releasing ownership of an object.
	/// @date	14/05/2018
	class DLL_EXPORT release_interface
	{
	public:		
		/// @brief	Releases ownership.
		/// @date	14/05/2018
		/// @return	remaining owners count.
		virtual int release() const = 0;
	
	protected:		
		virtual ~release_interface() = default;
	};
}