#pragma once
#include <dispatchers/internal/inprocess_dispatcher.h>
#include <utils/ref_count_base.hpp>
#include <utils/messaging.hpp>

namespace dispatchers
{
	namespace internal
	{
		class inprocess_dispatcher_impl : 
			public utils::ref_count_base<utils::messaging::message_dispatcher_base<dispatchers::internal::inprocess_dispatcher>>
		{
		public:
			virtual bool post(core::messaging::message_interface* message) override;
		};
	}
}

