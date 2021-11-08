/// @file	core/messaging.h.
/// @brief	Declares the messaging interface classes
#pragma once
#include <core/ref_count_interface.h>
#include <core/serializable_interface.h>
#include <core/guid.h>

namespace core
{
	namespace messaging
	{
		/// @class	message_interface
		/// @brief	An interface to a message.
		/// 		Messages are serialize-able and clone-able data containers which also have an id.
		/// @date	14/05/2018
		class DLL_EXPORT message_interface :
			public core::ref_count_interface,
			public core::serializable_interface
		{
		public:		
			/// @fn	virtual message_interface::~message_interface() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~message_interface() = default;
			
			/// @fn	virtual core::guid id() const = 0;
			/// @brief	Gets the message ID.
			/// @date	14/05/2018
			/// @return	core::guid::id
			virtual core::guid id() const = 0;

			/// @fn	virtual bool clone(core::messaging::message_interface** msg) = 0;
			/// @brief	Creates a clone of this message
			/// @date	14/05/2018
			/// @param [out]	msg 	An address to a message pointer.
			/// @return	True if it succeeds, false if it fails.
			virtual bool clone(core::messaging::message_interface** msg) = 0;
		};

		/// @class	message_factory
		/// @brief	An interface defining a message factory.
		/// @date	14/05/2018
		class DLL_EXPORT message_factory :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual message_factory::~message_factory() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~message_factory() = default;

			/// @fn	virtual bool message_factory::create(const core::guid& guid, core::messaging::message_interface** msg) = 0;
			/// @brief	Creates a message according to a guid
			/// @date	14/05/2018
			/// @param 		   	guid	Unique identifier.
			/// @param [out]	msg 	An address to a message pointer.
			/// @return	True if it succeeds, false if it fails.
			virtual bool create(const core::guid& guid, core::messaging::message_interface** msg) = 0;
		};

		/// @class	message_callback
		/// @brief	An interface defining a message callback.
		/// @date	14/05/2018
		class DLL_EXPORT message_callback :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual message_callback::~message_callback() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~message_callback() = default;

			/// @fn	virtual bool message_callback::supported_message(const core::guid& guid) = 0;
			/// @brief	A filter mechanism which allows a message dispatcher to skip calling on_message if message is not supported
			/// @date	14/05/2018
			/// @param	guid	The message ID.
			/// @return	True if supported, false if it's not.
			virtual bool supported_message(const core::guid& guid) = 0;

			/// @fn	virtual void message_callback::on_message(core::messaging::message_interface* message) = 0;
			/// @brief	Handles message signals
			/// @date	14/05/2018
			/// @param [in]		The message.
			virtual void on_message(core::messaging::message_interface* message) = 0;
		};

		/// @class	message_dispatcher
		/// @brief	An interface defining a message dispatcher.
		/// 		Message dispatchers are used to post messages from 
		/// 		any context (generally, a thread) and synchronously 
		/// 		signal them to subscribers from a dedicated context.
		/// @date	14/05/2018
		class DLL_EXPORT message_dispatcher :
			public core::ref_count_interface
		{
		public:
			/// @fn	virtual message_dispatcher::~message_dispatcher() = default;
			/// @brief	Destructor
			/// @date	14/05/2018
			virtual ~message_dispatcher() = default;

			/// @fn	virtual bool message_dispatcher::post(core::messaging::message_interface* message) = 0;
			/// @brief	Posts a message
			/// @date	14/05/2018
			/// @param [in]		The message to post.
			/// @return	True if it succeeds, false if it fails.
			virtual bool post(core::messaging::message_interface* message) = 0;

			/// @fn	virtual bool message_dispatcher::add_callback(core::messaging::message_callback* callback) = 0;
			/// @brief	Subscribes a message callback
			/// @date	14/05/2018
			/// @param [in]		callback	The callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_callback(core::messaging::message_callback* callback) = 0;

			/// @fn	virtual bool message_dispatcher::remove_callback(core::messaging::message_callback* callback) = 0;
			/// @brief	Unsubscribe a message callback
			/// @date	14/05/2018
			/// @param [in]		callback	The callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool remove_callback(core::messaging::message_callback* callback) = 0;
		};
	}
}