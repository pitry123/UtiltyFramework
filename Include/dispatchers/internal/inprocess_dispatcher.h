/// @file	internal/inprocess_dispatcher.h.
/// @brief	Declares the in-process dispatcher class
#pragma once
#include <core/messaging.h>

namespace dispatchers
{
	namespace internal
	{
		/// @class	inprocess_dispatcher
		/// @brief	The in-process dispatcher is a message dispatcher 
		/// 		implementation to be used within a process
		/// @date	15/05/2018
		class DLL_EXPORT inprocess_dispatcher : public core::messaging::message_dispatcher
		{
		public:
			/// @fn	virtual inprocess_dispatcher::~inprocess_dispatcher() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~inprocess_dispatcher() = default;

			/// @fn	static bool inprocess_dispatcher::create(core::messaging::message_dispatcher** dispatcher);
			/// @brief	Static factory: Creates a new in-process dispatcher instance
			/// @date	15/05/2018
			/// @param [out]	dispatcher	An address of a pointer to core::messaging::message_dispatcher
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::messaging::message_dispatcher** dispatcher);
		};
	}
}