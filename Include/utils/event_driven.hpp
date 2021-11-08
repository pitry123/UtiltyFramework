#pragma once
#include <utils/dispatcher.hpp>

namespace utils
{
	class empty_event
	{
	private:
		int m_id;

	public:
		empty_event(int id) : m_id(id)
		{
		}

		virtual ~empty_event() = default;

		int ID() const
		{
			return m_id;
		}
	};

	class event_dispatcher;

	class event_handler
	{
		friend class event_dispatcher;

	public:
		enum transaction_type
		{
			POST,
			SEND
		};

		event_handler() {}
		virtual ~event_handler() {}

	protected:
		virtual void on_event(empty_event& event, transaction_type transaction) = 0;
	};

	class event_dispatcher
	{
	private:
		utils::dispatcher m_dispathcer;

	public:
#define downcast_event(polyType, empty_event) dynamic_cast<polyType&>(empty_event)	
		event_dispatcher()
		{
		}

		template<typename EVENT>
		void post(EVENT event, event_handler& handler)
		{
			m_dispathcer.begin_invoke([this, event, &handler]() mutable -> void
			{
				handler.on_event(event, event_handler::transaction_type::POST);
			});
		}

		template<typename EVENT>
		void send(EVENT& event, event_handler& handler)
		{
			m_dispathcer.invoke([this, &event, &handler]() -> void
			{
				handler.on_event(event, event_handler::transaction_type::SEND);
			});
		}
	};
}